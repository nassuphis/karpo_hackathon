import ast
import pathlib
import re
import shlex
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
DEPLOY_TEXT = (ROOT / "deploy.sh").read_text()
LOCAL_MODULES = {p.stem for p in LAMBDA_DIR.glob("*.py")}
HANDLER_STORAGE_TEXT = (LAMBDA_DIR / "handler_storage.py").read_text()
API_MANIFEST_PATH = ROOT / "api_manifest.json"
PREDEPLOY_SCRIPT_PATH = ROOT / "scripts" / "predeploy_check.sh"
WORKFLOW_RENDERER_PATH = ROOT / "workflow_template_render.py"


def _joined_shell_lines(text):
    return re.sub(r"\\\n\s*", " ", text)


def _packaged_handlers():
    joined = _joined_shell_lines(DEPLOY_TEXT)
    files_by_dir = {}
    for line in joined.splitlines():
        stripped = line.strip()
        if not stripped.startswith("cp ") or ' "$' not in stripped:
            continue
        try:
            parts = shlex.split(stripped)
        except ValueError:
            continue
        if len(parts) < 3:
            continue
        dest = parts[-1]
        if not dest.startswith("$") or not dest.endswith("/"):
            continue
        dir_var = dest[1:-1]
        files_by_dir.setdefault(dir_var, set()).update(
            pathlib.Path(tok).name for tok in parts[1:-1] if tok.startswith("lambda/")
        )

    packaged = {}
    for _, files in files_by_dir.items():
        handler_files = [name for name in files if name.startswith("handler_") and name.endswith(".py")]
        for handler in handler_files:
            packaged[handler] = set(files)
    return packaged


def _local_dependencies(py_name, seen=None):
    if seen is None:
        seen = set()
    if py_name in seen:
        return set()
    seen.add(py_name)

    path = LAMBDA_DIR / py_name
    source = path.read_text()
    tree = ast.parse(source, filename=str(path))
    deps = set()

    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            for alias in node.names:
                name = alias.name.split(".")[0]
                if name in LOCAL_MODULES:
                    dep = f"{name}.py"
                    deps.add(dep)
                    deps.update(_local_dependencies(dep, seen))
        elif isinstance(node, ast.ImportFrom):
            if node.level:
                continue
            if not node.module:
                continue
            name = node.module.split(".")[0]
            if name in LOCAL_MODULES:
                dep = f"{name}.py"
                deps.add(dep)
                deps.update(_local_dependencies(dep, seen))

    for match in re.finditer(r'os\.path\.join\(\s*os\.path\.dirname\(__file__\)\s*,\s*[\'"]([^\'"]+)[\'"]\s*\)', source):
        deps.add(match.group(1))

    return deps


class TestDeployPackaging(unittest.TestCase):
    def test_raster_mt_memory_is_maxed_for_large_fused_renders(self):
        self.assertRegex(
            DEPLOY_TEXT,
            r"(?m)^RASTER_MT_MEMORY=10240\s+# max memory/CPU tier for native pthread raster on large fused renders$",
            "fused raster Lambda memory should stay at 10240 MB for large solve-score renders",
        )

    def test_color_to_bilevel_memory_is_maxed_for_large_raw_threshold_derivations(self):
        self.assertRegex(
            DEPLOY_TEXT,
            r"(?m)^COLOR_TO_BILEVEL_MEMORY=10240\s+# raw-sidecar thresholding is single-shot and CPU-bound on large images$",
            "Color2Bilevel Lambda memory should stay at 10240 MB for large raw-threshold derivations",
        )

    def test_render_workflow_definition_uses_shared_renderer_in_deploy_and_tests(self):
        self.assertTrue(WORKFLOW_RENDERER_PATH.exists(), "workflow_template_render.py should exist")
        renderer_text = WORKFLOW_RENDERER_PATH.read_text()
        self.assertIn("def render_render_workflow_definition(", renderer_text)
        self.assertIn("def render_render_workflow_definition_for_tests(", renderer_text)
        self.assertIn("RENDER_COLOR_RASTER_ITEM_SELECTOR", renderer_text)
        self.assertIn("RENDER_FINALIZE_MT_TASK_PAYLOAD", renderer_text)

        helper_match = re.search(
            r'render_render_workflow_definition\(\)\s*\{(?P<body>.*?)\n\}',
            DEPLOY_TEXT,
            re.DOTALL,
        )
        self.assertIsNotNone(helper_match, "deploy.sh should define render_render_workflow_definition()")
        helper_body = helper_match.group("body")
        self.assertIn('"$SCRIPT_DIR/workflow_template_render.py" render-workflow', helper_body)
        self.assertIn('--plan-function-arn "$RENDER_PLAN_ARN"', helper_body)
        self.assertNotIn("coeff-bilevel-stitch", helper_body)
        self.assertNotIn("preview-function-arn", helper_body)

        joined = _joined_shell_lines(DEPLOY_TEXT)
        self.assertEqual(
            joined.count('render_render_workflow_definition /tmp/render_workflow.asl.json "$ACCT"'),
            2,
            "expected create and update paths to both use render_render_workflow_definition",
        )
        self.assertNotIn('stepfunctions/render_workflow.asl.json.template > /tmp/render_workflow.asl.json', joined)

    def test_palette_workflow_template_substitutes_attach_palette_arn_in_both_deploy_paths(self):
        joined = _joined_shell_lines(DEPLOY_TEXT)
        matches = list(re.finditer(
            r'sed -e "s\|\\\$\{PlanFunctionArn\}\|\$\{PALETTE_PLAN_ARN\}\|g".*?'
            r'stepfunctions/palette_workflow\.asl\.json\.template > /tmp/palette_workflow\.asl\.json',
            joined,
        ))
        self.assertEqual(len(matches), 2, "expected create and update palette-workflow sed blocks")
        for idx, match in enumerate(matches, start=1):
            block = match.group(0)
            self.assertIn(r'-e "s|\${AttachPaletteFunctionArn}|${ATTACH_PALETTE_ARN}|g"', block,
                          f"palette workflow sed block {idx} missing AttachPaletteFunctionArn substitution")

    def test_all_api_gateway_integrations_have_invoke_permission(self):
        joined = _joined_shell_lines(DEPLOY_TEXT)
        integrated = set(re.findall(r'create_integration "\$(\w+)"', joined))
        self.assertGreater(len(integrated), 10, "failed to discover API Gateway integrations")

        perm_match = re.search(r'for FNAME in (.*?); do\s+aws lambda add-permission', joined)
        self.assertIsNotNone(perm_match, "failed to find API Gateway add-permission loop")
        permitted = set(re.findall(r'"\$(\w+)"', perm_match.group(1)))

        missing = sorted(integrated - permitted)
        self.assertEqual(
            missing,
            [],
            f"API Gateway integrations missing invoke permission: {', '.join(missing)}",
        )

    def test_packaged_handlers_include_all_local_dependencies(self):
        packaged = _packaged_handlers()
        self.assertGreater(len(packaged), 10, "failed to parse deploy packaging blocks")

        missing = []
        for handler, bundled_files in sorted(packaged.items()):
            required = _local_dependencies(handler)
            absent = sorted(req for req in required if req not in bundled_files)
            if absent:
                missing.append((handler, absent, sorted(bundled_files)))

        if missing:
            lines = []
            for handler, absent, bundled in missing:
                lines.append(
                    f"{handler} missing from deploy bundle: {', '.join(absent)} "
                    f"(bundled: {', '.join(bundled)})"
                )
            self.fail("\n".join(lines))

    def test_handlers_with_local_binaries_have_chmod_entries(self):
        packaged = _packaged_handlers()
        missing = []
        for handler, bundled_files in sorted(packaged.items()):
            dir_match = re.search(rf'cp .*?\b{re.escape(handler)}\b.*?"\$(\w+)/"', _joined_shell_lines(DEPLOY_TEXT))
            self.assertIsNotNone(dir_match, f"could not find package dir for {handler}")
            dir_var = dir_match.group(1)
            for dep in sorted(_local_dependencies(handler)):
                if dep.endswith((".py", ".json", ".html")):
                    continue
                chmod_pat = rf'chmod \+x .*"\${dir_var}"/{re.escape(dep)}'
                if not re.search(chmod_pat, _joined_shell_lines(DEPLOY_TEXT)):
                    missing.append(f"{handler} packages {dep} but deploy.sh has no chmod +x for ${dir_var}/{dep}")
        if missing:
            self.fail("\n".join(missing))

    def test_known_regressions_are_covered(self):
        packaged = _packaged_handlers()
        self.assertIn("handler_render_plan.py", packaged)
        self.assertIn("palette_names.py", packaged["handler_render_plan.py"])
        self.assertIn("tri_palette_names_generated.py", packaged["handler_render_plan.py"])
        self.assertIn("long_palette_names_generated.py", packaged["handler_render_plan.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_render_plan.py"])
        self.assertIn("color_render_contract.py", packaged["handler_render_plan.py"])
        self.assertIn("param_source.py", packaged["handler_render_plan.py"])
        self.assertIn("calc_chunks.py", packaged["handler_render_plan.py"])
        self.assertIn("handler_palette_render_plan.py", packaged)
        self.assertIn("color_artifact_meta.py", packaged["handler_palette_render_plan.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_palette_render_plan.py"])
        self.assertIn("param_source.py", packaged["handler_palette_render_plan.py"])
        self.assertIn("calc_chunks.py", packaged["handler_palette_render_plan.py"])
        self.assertIn("handler_color_repalette.py", packaged)
        self.assertIn("color_artifact_meta.py", packaged["handler_color_repalette.py"])
        self.assertIn("color_recolor_raw.py", packaged["handler_color_repalette.py"])
        self.assertIn("color_render_contract.py", packaged["handler_color_repalette.py"])
        self.assertIn("raw_score_render.py", packaged["handler_color_repalette.py"])
        self.assertIn("score_raw_render", packaged["handler_color_repalette.py"])
        self.assertIn("handler_recolor_from_raw.py", packaged)
        self.assertIn("color_recolor_raw.py", packaged["handler_recolor_from_raw.py"])
        self.assertIn("color_render_contract.py", packaged["handler_recolor_from_raw.py"])
        self.assertIn("raw_score_render.py", packaged["handler_recolor_from_raw.py"])
        self.assertIn("score_raw_render", packaged["handler_recolor_from_raw.py"])
        self.assertIn("handler_extract_palette_from_step_scores.py", packaged)
        self.assertIn("raw_score_render.py", packaged["handler_extract_palette_from_step_scores.py"])
        self.assertIn("raw_sidecar.py", packaged["handler_extract_palette_from_step_scores.py"])
        self.assertIn("color_render_contract.py", packaged["handler_extract_palette_from_step_scores.py"])
        self.assertIn("color_artifact_meta.py", packaged["handler_extract_palette_from_step_scores.py"])
        self.assertIn("score_raw_render", packaged["handler_extract_palette_from_step_scores.py"])
        self.assertIn("step_scores_to_palette_raw", packaged["handler_extract_palette_from_step_scores.py"])
        self.assertIn("handler_deepzoom_export.py", packaged)
        self.assertIn("dz_export", packaged["handler_deepzoom_export.py"])
        self.assertNotIn("raw2jpeg", packaged["handler_deepzoom_export.py"])
        self.assertIn("handler_deepzoom_from_raw.py", packaged)
        self.assertIn("handler_deepzoom_export.py", packaged["handler_deepzoom_from_raw.py"])
        self.assertIn("handler_bilevel.py", packaged)
        self.assertIn("logical_sections.py", packaged["handler_bilevel.py"])
        self.assertIn("raw_sidecar.py", packaged["handler_bilevel.py"])
        self.assertIn("color_render_contract.py", packaged["handler_bilevel.py"])
        self.assertIn("color_artifact_meta.py", packaged["handler_bilevel.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_bilevel.py"])
        self.assertIn("bilevel_section_raster", packaged["handler_bilevel.py"])
        self.assertIn("coeffs_bilevel_raster", packaged["handler_bilevel.py"])
        self.assertIn("raw_to_bilevel", packaged["handler_bilevel.py"])
        self.assertIn("assemble_greyscale", packaged["handler_bilevel.py"])
        self.assertIn('create_lambda "$COLOR_TO_BILEVEL_NAME" "handler_bilevel.handler" "/tmp/polypaint-bilevel.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$COLOR_TO_BILEVEL_NAME" "handler_bilevel.handler" "/tmp/polypaint-bilevel.zip"', DEPLOY_TEXT)
        self.assertIn("COLOR_TO_BILEVEL_FUNCTION", DEPLOY_TEXT)
        self.assertIn("assemble_greyscale", packaged["handler_finalize_mt.py"])
        self.assertIn("score_raw_render", packaged["handler_finalize_mt.py"])
        self.assertIn("raw_score_render.py", packaged["handler_finalize_mt.py"])
        self.assertIn("color_render_contract.py", packaged["handler_finalize_mt.py"])

        self.assertIn("handler_solve_proximity.py", packaged)
        self.assertIn("solve_proximity_stats", packaged["handler_solve_proximity.py"])
        self.assertIn("solve_proximity_hist_sectioned", packaged["handler_solve_proximity.py"])
        self.assertIn("handler_solve_proximity_bench.py", packaged)
        self.assertIn("handler_solve_proximity.py", packaged["handler_solve_proximity_bench.py"])
        self.assertIn("solve_proximity_stats", packaged["handler_solve_proximity_bench.py"])
        self.assertIn("solve_proximity_hist_sectioned", packaged["handler_solve_proximity_bench.py"])
        self.assertIn('create_lambda "$SOLVE_PROXIMITY_BENCH_NAME" "handler_solve_proximity_bench.handler" "/tmp/polypaint-solve-proximity-bench.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$SOLVE_PROXIMITY_BENCH_NAME" "handler_solve_proximity_bench.handler" "/tmp/polypaint-solve-proximity-bench.zip"', DEPLOY_TEXT)

        self.assertIn("handler_raster_mt.py", packaged)
        self.assertIn("roots2pix_mt", packaged["handler_raster_mt.py"])
        self.assertIn('create_lambda "$RASTER_MT_NAME" "handler_raster_mt.handler" "/tmp/polypaint-raster-mt.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$RASTER_MT_NAME" "handler_raster_mt.handler" "/tmp/polypaint-raster-mt.zip"', DEPLOY_TEXT)
        self.assertIn("RASTER_MT_THREADS", DEPLOY_TEXT)
        self.assertIn('gcc -O3 -pthread -o /src/roots2pix_mt /src/roots2pix_mt.c /src/multispan_reader.c', DEPLOY_TEXT)
        self.assertIn('cp lambda/roots2pix_mt "$RASTER_MT_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/handler_raster_mt.py lambda/shared.py lambda/solve_score_chain.py lambda/logical_sections.py "$RASTER_MT_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/roots2pix_mt_lib/* "$RASTER_MT_DIR/lib/"', DEPLOY_TEXT)
        self.assertIn('LD_LIBRARY_PATH=/var/task/lib', DEPLOY_TEXT)
        docker_runtime_text = (ROOT / "tests" / "docker_runtime_regression.py").read_text()
        self.assertIn("--score_output_normalize=1", docker_runtime_text)
        self.assertIn("test_roots2pix_mt_score_output_normalization_runtime()", docker_runtime_text)
        self.assertIn("test_roots2pix_mt_explicit_rgb_outputs_runtime()", docker_runtime_text)
        self.assertIn("test_roots2pix_mt_local_file_manifest_runtime()", docker_runtime_text)
        self.assertIn("aarch64-linux-musl-gcc -O3 -static -pthread -o lambda/solve_proximity_stats lambda/solve_proximity_stats.c -lm", DEPLOY_TEXT)
        self.assertNotIn("handler_raster.py", packaged)
        self.assertNotIn('create_lambda "$RASTER_NAME" "handler_raster.handler" "/tmp/polypaint-raster.zip"', DEPLOY_TEXT)
        self.assertNotIn('update_lambda "$RASTER_NAME" "handler_raster.handler" "/tmp/polypaint-raster.zip"', DEPLOY_TEXT)

        self.assertIn("handler_sweep_mt.py", packaged)
        self.assertIn("sweep_mt", packaged["handler_sweep_mt.py"])
        self.assertIn('create_lambda "$SWEEP_MT_NAME" "handler_sweep_mt.handler" "/tmp/polypaint-sweep-mt.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$SWEEP_MT_NAME" "handler_sweep_mt.handler" "/tmp/polypaint-sweep-mt.zip"', DEPLOY_TEXT)
        self.assertNotIn("handler_sweep.py", packaged)
        self.assertNotIn('create_lambda "$SWEEP_NAME" "handler_sweep.handler" "/tmp/polypaint-sweep.zip"', DEPLOY_TEXT)
        self.assertNotIn('update_lambda "$SWEEP_NAME" "handler_sweep.handler" "/tmp/polypaint-sweep.zip"', DEPLOY_TEXT)
        self.assertNotIn('ensure_route "POST /sweep" "$SWEEP_INT"', DEPLOY_TEXT)
        self.assertIn('REMOVED_SWEEP_NAME="polypaint-sweep"', DEPLOY_TEXT)
        self.assertEqual(DEPLOY_TEXT.count('delete_lambda_if_exists "$REMOVED_SWEEP_NAME"'), 2)
        self.assertIn('delete_route_if_exists "POST /sweep"', DEPLOY_TEXT)
        self.assertIn('delete_integration_for_lambda_if_exists "$REMOVED_SWEEP_NAME"', DEPLOY_TEXT)
        self.assertIn("first_api_gateway_id", DEPLOY_TEXT)
        self.assertNotIn(".RouteId | [0]", DEPLOY_TEXT)
        self.assertNotIn(".IntegrationId | [0]", DEPLOY_TEXT)
        self.assertIn("aws lambda delete-function", DEPLOY_TEXT)
        self.assertIn("aws apigatewayv2 delete-integration", DEPLOY_TEXT)
        self.assertIn("Warning: failed to delete removed Lambda", DEPLOY_TEXT)
        self.assertIn("Warning: failed to delete removed route", DEPLOY_TEXT)
        self.assertIn("Warning: failed to delete removed integration", DEPLOY_TEXT)
        self.assertNotIn('"sweep": "%s/sweep"', DEPLOY_TEXT)
        joined = _joined_shell_lines(DEPLOY_TEXT)
        self.assertRegex(joined, r'create_lambda "\$SWEEP_MT_NAME" "handler_sweep_mt\.handler" "/tmp/polypaint-sweep-mt\.zip"\s+"\$SWEEP_MT_MEMORY" "\$ROLE_ARN" "" "BUCKET=\$BUCKET,JOBS_TABLE=\$JOBS_TABLE" "\$BINARY_TMP"')
        self.assertRegex(joined, r'update_lambda "\$SWEEP_MT_NAME" "handler_sweep_mt\.handler" "/tmp/polypaint-sweep-mt\.zip"\s+"\$SWEEP_MT_MEMORY" "" "BUCKET=\$BUCKET,JOBS_TABLE=\$JOBS_TABLE" "\$BINARY_TMP"')
        self.assertIn("SWEEP_MT_FUNCTION", DEPLOY_TEXT)
        self.assertIn('ensure_route "POST /sweep-mt" "$SWEEP_MT_INT"', DEPLOY_TEXT)
        self.assertIn('"sweep-mt": "%s/sweep-mt"', DEPLOY_TEXT)
        self.assertIn(
            'gcc -O3 -pthread -o /src/solve_proximity_hist_sectioned /src/solve_proximity_hist_sectioned.c /src/multispan_reader.c',
            DEPLOY_TEXT,
        )
        self.assertIn('cp lambda/solve_proximity_stats lambda/solve_proximity_hist_sectioned "$SP_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/solve_proximity_stats lambda/solve_proximity_hist_sectioned "$SP_BENCH_DIR/"', DEPLOY_TEXT)
        self.assertIn('ensure_route "POST /list-favorites" "$STORAGE_INT"', DEPLOY_TEXT)
        self.assertIn('ensure_route "POST /add-favorite" "$STORAGE_INT"', DEPLOY_TEXT)
        self.assertIn('ensure_route "POST /delete-favorite" "$STORAGE_INT"', DEPLOY_TEXT)
        self.assertIn("handler_storage.py", packaged)
        self.assertIn("color_artifact_meta.py", packaged["handler_storage.py"])
        self.assertIn("color_render_contract.py", packaged["handler_storage.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_storage.py"])
        self.assertIn("handler_palette_chunk.py", packaged)
        self.assertIn("solve_palette_chunk", packaged["handler_palette_chunk.py"])
        self.assertIn("solve_palette_chunk_mt", packaged["handler_palette_chunk.py"])
        self.assertIn(
            'gcc -O3 -pthread -o /src/solve_palette_chunk_mt /src/solve_palette_chunk_mt.c /src/multispan_reader.c',
            DEPLOY_TEXT,
        )
        self.assertIn('cp lambda/solve_palette_chunk lambda/solve_palette_chunk_mt "$PAL_CHUNK_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/solve_palette_chunk_mt_lib/* "$PAL_CHUNK_DIR/lib/"', DEPLOY_TEXT)
        self.assertRegex(joined, r'create_lambda "\$PALETTE_CHUNK_NAME" "handler_palette_chunk\.handler" "/tmp/polypaint-palette-chunk\.zip"\s+"\$PALETTE_CHUNK_MEMORY" "\$ROLE_ARN" "" "BUCKET=\$BUCKET,JOBS_TABLE=\$JOBS_TABLE,LD_LIBRARY_PATH=/var/task/lib" "\$BINARY_TMP"')
        self.assertRegex(joined, r'update_lambda "\$PALETTE_CHUNK_NAME" "handler_palette_chunk\.handler" "/tmp/polypaint-palette-chunk\.zip"\s+"\$PALETTE_CHUNK_MEMORY" "" "BUCKET=\$BUCKET,JOBS_TABLE=\$JOBS_TABLE,LD_LIBRARY_PATH=/var/task/lib" "\$BINARY_TMP"')

        self.assertIn("handler_autolevels.py", packaged)
        self.assertIn("autolevels_render", packaged["handler_autolevels.py"])
        self.assertIn("color_artifact_meta.py", packaged["handler_autolevels.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_autolevels.py"])

        self.assertIn("handler_resize_artifact.py", packaged)
        self.assertIn("shared.py", packaged["handler_resize_artifact.py"])
        self.assertIn("color_artifact_meta.py", packaged["handler_resize_artifact.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_resize_artifact.py"])
        self.assertIn('create_lambda "$RESIZE_ARTIFACT_NAME" "handler_resize_artifact.handler" "/tmp/polypaint-resize-artifact.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$RESIZE_ARTIFACT_NAME" "handler_resize_artifact.handler" "/tmp/polypaint-resize-artifact.zip"', DEPLOY_TEXT)
        self.assertIn('RESIZE_ARTIFACT_FUNCTION=$RESIZE_ARTIFACT_NAME', DEPLOY_TEXT)
        self.assertIn('"$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"', DEPLOY_TEXT)

        self.assertIn("handler_repalette.py", packaged)
        self.assertIn("palette_names.py", packaged["handler_repalette.py"])
        self.assertIn("tri_palette_names_generated.py", packaged["handler_repalette.py"])
        self.assertIn("long_palette_names_generated.py", packaged["handler_repalette.py"])
        self.assertIn("palette_bins_render", packaged["handler_repalette.py"])
        self.assertIn("raw2jpeg", packaged["handler_repalette.py"])

        self.assertIn("handler_finalize_mt.py", packaged)
        self.assertIn("color_artifact_meta.py", packaged["handler_finalize_mt.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_finalize_mt.py"])
        self.assertIn("assemble_greyscale", packaged["handler_finalize_mt.py"])
        self.assertIn("score_raw_render", packaged["handler_finalize_mt.py"])
        self.assertNotIn("handler_finalize.py", packaged)
        self.assertNotIn('create_lambda "$FINALIZE_NAME" "handler_finalize.handler" "/tmp/polypaint-finalize.zip"', DEPLOY_TEXT)
        self.assertNotIn('update_lambda "$FINALIZE_NAME" "handler_finalize.handler" "/tmp/polypaint-finalize.zip"', DEPLOY_TEXT)
        self.assertIn('create_lambda "$FINALIZE_MT_NAME" "handler_finalize_mt.handler" "/tmp/polypaint-finalize-mt.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$FINALIZE_MT_NAME" "handler_finalize_mt.handler" "/tmp/polypaint-finalize-mt.zip"', DEPLOY_TEXT)
        self.assertIn('create_lambda "$RECOLOR_FROM_RAW_NAME" "handler_recolor_from_raw.handler" "/tmp/polypaint-recolor-from-raw.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$RECOLOR_FROM_RAW_NAME" "handler_recolor_from_raw.handler" "/tmp/polypaint-recolor-from-raw.zip"', DEPLOY_TEXT)
        self.assertIn('create_lambda "$EXTRACT_PALETTE_FUSED_NAME" "handler_extract_palette_from_step_scores.handler" "/tmp/polypaint-extract-palette-fused.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$EXTRACT_PALETTE_FUSED_NAME" "handler_extract_palette_from_step_scores.handler" "/tmp/polypaint-extract-palette-fused.zip"', DEPLOY_TEXT)
        self.assertIn('create_lambda "$DZ_FROM_RAW_NAME" "handler_deepzoom_from_raw.handler" "/tmp/polypaint-deepzoom-from-raw.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$DZ_FROM_RAW_NAME" "handler_deepzoom_from_raw.handler" "/tmp/polypaint-deepzoom-from-raw.zip"', DEPLOY_TEXT)
        self.assertIn('gcc -O3 -pthread -o /src/assemble_greyscale /src/assemble_greyscale.c', DEPLOY_TEXT)
        self.assertIn('gcc -O3 -o /src/score_raw_render /src/score_raw_render.c', DEPLOY_TEXT)
        self.assertIn('aarch64-linux-musl-gcc -O3 -static -o lambda/bilevel_section_raster lambda/bilevel_section_raster.c -lm', DEPLOY_TEXT)
        self.assertIn('aarch64-linux-musl-gcc -O3 -static -o lambda/coeffs_bilevel_raster lambda/coeffs_bilevel_raster.c -lm', DEPLOY_TEXT)
        self.assertIn('gcc -O3 -o /src/raw_to_bilevel /src/raw_to_bilevel.c', DEPLOY_TEXT)
        self.assertIn('aarch64-linux-musl-gcc -O3 -static -o lambda/step_scores_to_palette_raw lambda/step_scores_to_palette_raw.c', DEPLOY_TEXT)
        self.assertIn('cp lambda/assemble_greyscale lambda/score_raw_render "$FINALIZE_MT_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/assemble_greyscale_lib/* "$FINALIZE_MT_DIR/lib/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/handler_bilevel.py lambda/shared.py lambda/logical_sections.py lambda/raw_sidecar.py lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$BILEVEL_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/bilevel_section_raster lambda/coeffs_bilevel_raster lambda/raw_to_bilevel lambda/assemble_greyscale "$BILEVEL_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/assemble_greyscale_lib/* "$BILEVEL_DIR/lib/"', DEPLOY_TEXT)
        self.assertNotIn('cp lambda/assemble_greyscale_lib/* "$FINALIZE_MT_DIR/lib/" 2>/dev/null || true', DEPLOY_TEXT)
        self.assertNotIn('cp lambda/assemble_greyscale_lib/* "$BILEVEL_DIR/lib/" 2>/dev/null || true', DEPLOY_TEXT)
        self.assertIn('cp lambda/handler_color_repalette.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py \\', DEPLOY_TEXT)
        self.assertIn('lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py lambda/color_recolor_raw.py \\', DEPLOY_TEXT)
        self.assertIn('cp lambda/score_raw_render "$COLOR_REPALETTE_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/handler_recolor_from_raw.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py lambda/color_recolor_raw.py \\', DEPLOY_TEXT)
        self.assertIn('cp lambda/score_raw_render "$RECOLOR_FROM_RAW_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/handler_extract_palette_from_step_scores.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py \\', DEPLOY_TEXT)
        self.assertIn('lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$EXTRACT_PALETTE_FUSED_DIR/"', DEPLOY_TEXT)
        self.assertIn('cp lambda/score_raw_render lambda/step_scores_to_palette_raw "$EXTRACT_PALETTE_FUSED_DIR/"', DEPLOY_TEXT)
        self.assertIn('"$COLOR_REPALETTE_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"', DEPLOY_TEXT)
        self.assertIn('"$RECOLOR_FROM_RAW_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"', DEPLOY_TEXT)
        self.assertIn('"$EXTRACT_PALETTE_FUSED_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"', DEPLOY_TEXT)
        self.assertIn("EXTRACT_PALETTE_FUSED_FUNCTION", DEPLOY_TEXT)
        self.assertIn('--finalize-mt-function-arn "$FINALIZE_MT_ARN"', DEPLOY_TEXT)
        self.assertIn("handler_palette_finalize.py", packaged)
        self.assertIn("solve_score_chain.py", packaged["handler_palette_finalize.py"])

        self.assertIn("handler_coeffgen.py", packaged)
        self.assertIn("sweep_coeffgen", packaged["handler_coeffgen.py"])
        self.assertIn('cp lambda/sweep_coeffgen "$COEFFGEN_DIR/"', DEPLOY_TEXT)
        self.assertIn('create_lambda "$COEFFGEN_NAME" "handler_coeffgen.handler" "/tmp/polypaint-coeffgen.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$COEFFGEN_NAME" "handler_coeffgen.handler" "/tmp/polypaint-coeffgen.zip"', DEPLOY_TEXT)
        self.assertIn('"$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib"', DEPLOY_TEXT)

        self.assertIn("handler_compute_preview.py", packaged)
        self.assertIn("sweep_coeffgen", packaged["handler_compute_preview.py"])
        self.assertIn("param_program_chain.py", packaged["handler_compute_preview.py"])
        self.assertIn("param_legacy_registry.json", packaged["handler_compute_preview.py"])
        self.assertNotIn("sweep", packaged["handler_compute_preview.py"])
        self.assertIn("sweep_mt", packaged["handler_compute_preview.py"])
        self.assertIn("sweep_cm", packaged["handler_compute_preview.py"])
        self.assertIn('create_lambda "$COMPUTE_PREVIEW_NAME" "handler_compute_preview.handler" "/tmp/polypaint-compute-preview.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$COMPUTE_PREVIEW_NAME" "handler_compute_preview.handler" "/tmp/polypaint-compute-preview.zip"', DEPLOY_TEXT)
        self.assertIn('ensure_route "POST /compute-preview" "$COMPUTE_PREVIEW_INT"', DEPLOY_TEXT)
        self.assertIn('"compute-preview": "%s/compute-preview"', DEPLOY_TEXT)
        self.assertIn('"$LAPACK_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"', DEPLOY_TEXT)

        self.assertIn("handler_render_lores_preview.py", packaged)
        self.assertNotIn("handler_solve_proximity.py", packaged["handler_render_lores_preview.py"])
        self.assertIn("logical_sections.py", packaged["handler_render_lores_preview.py"])
        self.assertIn("logical_lores.py", packaged["handler_render_lores_preview.py"])
        self.assertIn("calc_chunks.py", packaged["handler_render_lores_preview.py"])
        self.assertIn("param_source.py", packaged["handler_render_lores_preview.py"])
        self.assertIn("raw_score_render.py", packaged["handler_render_lores_preview.py"])
        self.assertIn("color_render_contract.py", packaged["handler_render_lores_preview.py"])
        self.assertIn("roots2pix_mt", packaged["handler_render_lores_preview.py"])
        self.assertIn("solve_proximity_stats", packaged["handler_render_lores_preview.py"])
        self.assertNotIn("solve_proximity_hist_sectioned", packaged["handler_render_lores_preview.py"])
        self.assertIn("score_raw_render", packaged["handler_render_lores_preview.py"])
        self.assertIn("sweep_coeffgen", packaged["handler_render_lores_preview.py"])
        self.assertIn("sweep_mt", packaged["handler_render_lores_preview.py"])
        self.assertIn("sweep_cm", packaged["handler_render_lores_preview.py"])
        self.assertIn('create_lambda "$RENDER_LORES_PREVIEW_NAME" "handler_render_lores_preview.handler" "/tmp/polypaint-render-lores-preview.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$RENDER_LORES_PREVIEW_NAME" "handler_render_lores_preview.handler" "/tmp/polypaint-render-lores-preview.zip"', DEPLOY_TEXT)
        self.assertIn('"$LIBVIPS_LAYER $LAPACK_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/var/task/lib:/opt/lib" "$BINARY_TMP"', DEPLOY_TEXT)
        self.assertIn('ensure_route "POST /render-lores-preview" "$RENDER_LORES_PREVIEW_INT"', DEPLOY_TEXT)
        self.assertIn('"render-lores-preview": "%s/render-lores-preview"', DEPLOY_TEXT)
        self.assertIn('cp lambda/roots2pix_mt_lib/* "$RENDER_LORES_PREVIEW_DIR/lib/"', DEPLOY_TEXT)
        self.assertNotIn('cp lambda/roots2pix_mt_lib/* "$RENDER_LORES_PREVIEW_DIR/lib/" 2>/dev/null || true', DEPLOY_TEXT)
        docker_script = (ROOT / "scripts" / "test-docker-runtime.sh").read_text()
        self.assertIn('"$ROOT/lambda/solve_proximity_stats"', docker_script)

        self.assertIn("handler_color_repalette.py", packaged)
        self.assertIn("palette_names.py", packaged["handler_color_repalette.py"])
        self.assertIn("tri_palette_names_generated.py", packaged["handler_color_repalette.py"])
        self.assertIn("long_palette_names_generated.py", packaged["handler_color_repalette.py"])
        self.assertIn("score_raw_render", packaged["handler_color_repalette.py"])

        self.assertIn("handler_attach_palette_to_color.py", packaged)
        self.assertIn("color_artifact_meta.py", packaged["handler_attach_palette_to_color.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_attach_palette_to_color.py"])
        self.assertIn('create_lambda "$ATTACH_PALETTE_NAME" "handler_attach_palette_to_color.handler" "/tmp/polypaint-attach-palette-to-color.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$ATTACH_PALETTE_NAME" "handler_attach_palette_to_color.handler" "/tmp/polypaint-attach-palette-to-color.zip"', DEPLOY_TEXT)

        self.assertIn("handler_compute_orchestrator.py", packaged)
        self.assertIn("handler_compute_plan.py", packaged)
        self.assertIn("handler_compute_status.py", packaged)
        self.assertIn('create_lambda "$COMPUTE_ORCHESTRATOR_NAME" "handler_compute_orchestrator.handler" "/tmp/polypaint-compute-orchestrator.zip"', DEPLOY_TEXT)
        self.assertIn('create_lambda "$COMPUTE_PLAN_NAME" "handler_compute_plan.handler" "/tmp/polypaint-compute-plan.zip"', DEPLOY_TEXT)
        self.assertIn('create_lambda "$COMPUTE_STATUS_NAME" "handler_compute_status.handler" "/tmp/polypaint-compute-status.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$COMPUTE_ORCHESTRATOR_NAME" "handler_compute_orchestrator.handler" "/tmp/polypaint-compute-orchestrator.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$COMPUTE_PLAN_NAME" "handler_compute_plan.handler" "/tmp/polypaint-compute-plan.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$COMPUTE_STATUS_NAME" "handler_compute_status.handler" "/tmp/polypaint-compute-status.zip"', DEPLOY_TEXT)
        self.assertIn("COMPUTE_STATE_MACHINE_NAME", DEPLOY_TEXT)
        self.assertIn("COMPUTE_ORCHESTRATOR_FUNCTION", DEPLOY_TEXT)

        self.assertIn("handler_pdf_artifact.py", packaged)
        self.assertIn("spread_pdf.py", packaged["handler_pdf_artifact.py"])
        self.assertIn("color_artifact_meta.py", packaged["handler_pdf_artifact.py"])
        self.assertIn("solve_score_chain.py", packaged["handler_pdf_artifact.py"])
        self.assertIn("PDF_PY_LAYER_NAME", DEPLOY_TEXT)
        self.assertIn("build-pdf-python-layer.sh", DEPLOY_TEXT)
        self.assertIn('create_lambda "$PDF_ARTIFACT_NAME" "handler_pdf_artifact.handler" "/tmp/polypaint-pdf-artifact.zip"', DEPLOY_TEXT)
        self.assertIn('update_lambda "$PDF_ARTIFACT_NAME" "handler_pdf_artifact.handler" "/tmp/polypaint-pdf-artifact.zip"', DEPLOY_TEXT)
        self.assertIn("lambda/gen_parity_results.py", DEPLOY_TEXT)

    def test_deploy_regenerates_parity_overlay_before_js_catalog(self):
        build_idx = DEPLOY_TEXT.index('cc -O2 -pthread -o lambda/sweep_test lambda/sweep_cli.c -lm')
        parity_idx = DEPLOY_TEXT.index('lambda/gen_parity_results.py')
        js_idx = DEPLOY_TEXT.index('from gen_catalog import load_catalog, load_metrics, generate_js, JS_OUT')
        self.assertLess(build_idx, parity_idx)
        self.assertLess(parity_idx, js_idx)

    def test_pdf_layer_build_script_overrides_lambda_entrypoint(self):
        build_script = (LAMBDA_DIR / "build-pdf-python-layer.sh").read_text()
        self.assertIn("--entrypoint /bin/bash", build_script)
        self.assertIn("public.ecr.aws/lambda/python:3.12", build_script)
        self.assertIn("-lc '", build_script)
        self.assertIn("zipfile.ZipFile", build_script)
        self.assertNotIn("find /out/python", build_script)
        self.assertNotIn("zip -r9", build_script)

    def test_deploy_verifies_uploaded_frontend_content_matches_local(self):
        self.assertIn('verify_frontend_assets() {', DEPLOY_TEXT)
        self.assertIn('curl -s -o /dev/null -w "%{http_code}" "${SITE_URL}/${asset}"', DEPLOY_TEXT)
        self.assertIn('mkdir -p "$(dirname "${TMP_DIR}/${asset}")"', DEPLOY_TEXT)
        self.assertIn('curl -fsS "${SITE_URL}/${asset}" -o "${TMP_DIR}/${asset}"', DEPLOY_TEXT)
        self.assertIn('LOCAL_HASH=$(shasum "$SCRIPT_DIR/${asset}" | cut -d\' \' -f1)', DEPLOY_TEXT)
        self.assertIn('REMOTE_HASH=$(shasum "${TMP_DIR}/${asset}" | cut -d\' \' -f1)', DEPLOY_TEXT)
        self.assertIn('FATAL: deployed ${asset} does not match local file', DEPLOY_TEXT)

    def test_deploy_includes_solve_score_program_assets(self):
        self.assertIn('frontend_asset_keys() {', DEPLOY_TEXT)
        self.assertIn('find "solve-score-programs" -type f | sort', DEPLOY_TEXT)
        self.assertIn('frontend_asset_content_type() {', DEPLOY_TEXT)
        self.assertIn('*.json) echo "application/json" ;;', DEPLOY_TEXT)
        self.assertIn('done < <(frontend_asset_keys)', DEPLOY_TEXT)
        self.assertTrue((ROOT / "solve-score-programs" / "index.json").exists())

    def test_updated_summary_prints_http_and_https_site_urls_together(self):
        self.assertIn('echo "  Site:"', DEPLOY_TEXT)
        self.assertIn('echo "  Build:    $BUILD_ID"', DEPLOY_TEXT)
        self.assertIn('echo "    HTTP:   http://$BUCKET.s3-website-$REGION.amazonaws.com"', DEPLOY_TEXT)
        self.assertIn('echo "    HTTPS:  https://$BUCKET.s3.$REGION.amazonaws.com/index.html"', DEPLOY_TEXT)
        self.assertIn('echo "  SolvPrxB: $SOLVE_PROXIMITY_BENCH_NAME ($SOLVE_PROXIMITY_BENCH_MEMORY MB)"', DEPLOY_TEXT)

    def test_deploy_usage_includes_show_build_mode(self):
        self.assertIn('# Usage: ./deploy.sh [create|update|show-build]', DEPLOY_TEXT)
        self.assertIn('print_usage() {', DEPLOY_TEXT)
        self.assertIn('echo "Usage: $0 [create|update|show-build]"', DEPLOY_TEXT)

    def test_show_build_mode_compares_frontend_render_plan_and_workflow(self):
        self.assertIn('show_build() {', DEPLOY_TEXT)
        self.assertIn('if [ "$ACTION" = "show-build" ]; then', DEPLOY_TEXT)
        self.assertIn('package_render_plan_zip "$LOCAL_RENDER_PLAN_ZIP" "$LOCAL_RENDER_PLAN_DIR"', DEPLOY_TEXT)
        self.assertIn('aws lambda get-function \\', DEPLOY_TEXT)
        self.assertIn('--function-name "$RENDER_PLAN_NAME"', DEPLOY_TEXT)
        self.assertIn('--region "$REGION"', DEPLOY_TEXT)
        self.assertIn("aws sts get-caller-identity --region \"$REGION\"", DEPLOY_TEXT)
        self.assertIn('aws stepfunctions describe-state-machine \\', DEPLOY_TEXT)
        self.assertIn('"arn:aws:states:${REGION}:${ACCT}:stateMachine:${RENDER_STATE_MACHINE_NAME}"', DEPLOY_TEXT)
        self.assertIn("Deployed frontend", DEPLOY_TEXT)
        self.assertIn("Render plan bundle", DEPLOY_TEXT)
        self.assertIn("Render workflow", DEPLOY_TEXT)
        self.assertIn("Overall: {'MATCH' if overall_match else 'MISMATCH'}", DEPLOY_TEXT)

    def test_deploy_writes_build_metadata_into_config_json(self):
        self.assertIn('build_deploy_metadata()', DEPLOY_TEXT)
        self.assertIn('"build": {', DEPLOY_TEXT)
        self.assertIn('"build_id": "%s"', DEPLOY_TEXT)
        self.assertIn('"deployed_at_utc": "%s"', DEPLOY_TEXT)
        self.assertIn('"git_rev": "%s"', DEPLOY_TEXT)
        self.assertIn('"git_dirty": %s', DEPLOY_TEXT)
        self.assertIn('"frontend_sha256": "%s"', DEPLOY_TEXT)
        self.assertIn('echo "  Build ID: $BUILD_ID"', DEPLOY_TEXT)

    def test_storage_handler_routes_are_published_by_deploy(self):
        storage_routes = sorted(set(re.findall(r'path\.endswith\("/([^"]+)"\)', HANDLER_STORAGE_TEXT)))
        self.assertGreater(len(storage_routes), 10, "failed to discover storage handler routes")
        missing = []
        for route in storage_routes:
            needle = f'ensure_route "POST /{route}" "$STORAGE_INT"'
            if needle not in DEPLOY_TEXT:
                missing.append(route)
        if missing:
            self.fail(
                "deploy.sh is missing API Gateway storage routes for: "
                + ", ".join(missing)
            )

    def test_phase1_phase2_files_exist_and_are_wired(self):
        self.assertTrue(API_MANIFEST_PATH.exists(), "api_manifest.json should be tracked")
        self.assertTrue(PREDEPLOY_SCRIPT_PATH.exists(), "scripts/predeploy_check.sh should exist")
        self.assertIn('bash "$SCRIPT_DIR/scripts/predeploy_check.sh"', DEPLOY_TEXT)
        predeploy_text = PREDEPLOY_SCRIPT_PATH.read_text()
        self.assertIn("api_manifest.py --check", predeploy_text)
        self.assertIn("tests/test_api_route_contracts.py", predeploy_text)
        self.assertIn("tests/test_deploy_packaging.py", predeploy_text)
        self.assertIn("tests/test_render_workflow_definition.py", predeploy_text)
        self.assertIn("tests/test_render_plan.py", predeploy_text)
        self.assertIn("tests/test_finalize_mt_handler.py", predeploy_text)
        self.assertIn("tests/test_raster_mt.py", predeploy_text)
        self.assertIn("tests/test_bilevel_handler.py", predeploy_text)
        self.assertIn("tests/test_solve_proximity_handler.py", predeploy_text)
        self.assertNotIn("test_coeff_bilevel_stitch_handler", predeploy_text)
        self.assertIn("tests/test_frontend_js.sh", predeploy_text)

    def test_python_runner_prefers_uv_with_local_fallbacks(self):
        predeploy_text = PREDEPLOY_SCRIPT_PATH.read_text()
        self.assertIn("command -v uv", DEPLOY_TEXT)
        self.assertIn("command -v uv", predeploy_text)
        self.assertIn('TEST_PYTHON=(uv run python)', DEPLOY_TEXT)
        self.assertIn('TEST_PYTHON=(uv run python)', predeploy_text)
        self.assertIn('$SCRIPT_DIR/.venv/bin/python', DEPLOY_TEXT)
        self.assertIn('$SCRIPT_DIR/../.venv/bin/python', DEPLOY_TEXT)
        self.assertIn('$ROOT/.venv/bin/python', predeploy_text)
        self.assertIn('$ROOT/../.venv/bin/python', predeploy_text)
        self.assertLess(
            DEPLOY_TEXT.index('TEST_PYTHON=(uv run python)'),
            DEPLOY_TEXT.index('$SCRIPT_DIR/.venv/bin/python'),
            "deploy.sh should prefer uv before local fallbacks",
        )
        self.assertLess(
            predeploy_text.index('TEST_PYTHON=(uv run python)'),
            predeploy_text.index('$ROOT/.venv/bin/python'),
            "predeploy_check.sh should prefer uv before local fallbacks",
        )

    def test_show_build_render_plan_packager_matches_real_render_plan_bundle(self):
        match = re.search(
            r'package_render_plan_zip\(\)\s*\{(?P<body>.*?)\n\}',
            DEPLOY_TEXT,
            re.DOTALL,
        )
        self.assertIsNotNone(match, "could not find package_render_plan_zip helper")
        helper_body = match.group("body")
        self.assertIn("lambda/calc_chunks.py", helper_body)
        self.assertIn("lambda/logical_sections.py", helper_body)
        self.assertIn("lambda/param_source.py", helper_body)
        self.assertIn("lambda/solve_score_chain.py", helper_body)


if __name__ == "__main__":
    unittest.main()
