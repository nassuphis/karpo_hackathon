"""
Shared Step Functions payload contracts.

These definitions are the first pass of a single source of truth for the
highest-risk render workflow payloads. The ASL still lives in checked-in JSON,
but tests should compare against these maps instead of re-spelling contract
fields by hand.
"""

import re

RENDER_COLOR_CLIP_TASK_PAYLOAD = {
    "phase": "clip",
    "job_id.$": "$.job_id",
    "degree.$": "$.plan.calc.degree",
    "solve_score_chain.$": "$.plan.solve_score.chain",
    "solve_score_normalize.$": "$.plan.solve_score.normalize",
    "solve_score_threads.$": "$.plan.solve_score.threads",
    "lores_bin_key.$": "$.plan.calc.lores_bin_key",
    "lores_coeffs_key.$": "$.plan.calc.lores_coeffs_key",
    "lores_params_key.$": "$.plan.calc.lores_params_key",
    "n_coeffs.$": "$.plan.calc.n_coeffs",
    "root_transforms.$": "$.plan.params.root_transforms",
    "out_key.$": "$.plan.solve_score.clip_key",
    "task_id.$": "States.Format('render_{}_solve_score_clip', $.run_id)",
}

RENDER_COLOR_RASTER_ITEM_SELECTOR = {
    "job_id.$": "$.job_id",
    "section_idx.$": "$$.Map.Item.Value.section_idx",
    "section_count.$": "$$.Map.Item.Value.section_count",
    "task_id.$": "States.Format('render_{}_raster_{}', $.run_id, $$.Map.Item.Value.section_idx)",
    "step_start.$": "$$.Map.Item.Value.step_start",
    "step_count.$": "$$.Map.Item.Value.step_count",
    "logical_section.$": "$.plan.raster.logical_section",
    "solve_source_manifest_key.$": "$.plan.solve_source_manifest_key",
    "n_coeffs.$": "$.plan.calc.n_coeffs",
    "pix.$": "$.plan.grid.pix",
    "min_re.$": "$.plan.viewport.min_re",
    "max_re.$": "$.plan.viewport.max_re",
    "min_im.$": "$.plan.viewport.min_im",
    "max_im.$": "$.plan.viewport.max_im",
    "rotation.$": "$.plan.params.rotation",
    "degree.$": "$.plan.calc.degree",
    "root_transforms.$": "$.plan.params.root_transforms",
    "solve_score_clip_key.$": "$.plan.solve_score.clip_key",
    "solve_score_chain.$": "$.plan.solve_score.chain",
    "prelude_rows.$": "$.plan.raster.prelude_rows",
    "score_coeff_prelude_rows.$": "$.plan.raster.score_coeff_prelude_rows",
    "score_param_prelude_rows.$": "$.plan.raster.score_param_prelude_rows",
    "raster_function_name.$": "$.plan.raster.function_name",
    "raster_mt_threads.$": "$.plan.raster.threads",
    "raster_input_mode.$": "$.plan.raster.input_mode",
    "raster_sectioned_retries.$": "$.plan.raster.sectioned_retries",
    "associated_palette_mode.$": "$.plan.associated_palette.mode",
    "associated_palette_fragment_prefix.$": "$.plan.associated_palette.fragment_prefix",
    "associated_palette_grid_n.$": "$.plan.calc.N",
    "fragment_prefix.$": "$.plan.outputs.fragment_prefix",
}

RENDER_FINALIZE_MT_FRAGMENT_MANIFEST = {
    "version": 1,
    "pair_encoding.$": "$.solve_score_clip.parsed.fragment_pair_encoding",
    "fragment_encoding.$": "$.solve_score_clip.parsed.fragment_encoding",
    "channels.$": "$.solve_score_clip.parsed.score_output_channel_count",
    "record_size_bytes.$": "$.solve_score_clip.parsed.fragment_record_size_bytes",
    "item_count.$": "$.plan.raster.item_count",
    "fragment_prefix.$": "$.plan.outputs.fragment_prefix",
    "chain_fingerprint.$": "$.plan.outputs.metadata.solve_score_chain_fingerprint",
    "solve_score_spec_version.$": "$.plan.outputs.metadata.solve_score_spec_version",
}

RENDER_FINALIZE_MT_TASK_PAYLOAD = {
    "phase": "finalize_mt",
    "job_id.$": "$.job_id",
    "run_id.$": "$.run_id",
    "task_id.$": "States.Format('render_{}_finalize_mt', $.run_id)",
    "mode.$": "$.mode",
    "source_item_count.$": "$.plan.raster.item_count",
    "pix.$": "$.plan.grid.pix",
    "format.$": "$.plan.params.fmt",
    "quality.$": "$.plan.params.quality",
    "palette.$": "$.plan.params.palette",
    "background_color.$": "$.plan.outputs.metadata.background_color",
    "finalize_workers.$": "$.plan.finalize.workers",
    "render_execution.$": "$.plan.render_execution",
    "metadata.$": "$.plan.outputs.metadata",
    "image_key.$": "$.plan.outputs.image_key",
    "preview_key.$": "$.plan.outputs.preview_key",
    "meta_key.$": "$.plan.outputs.meta_key",
    "raw_key.$": "$.plan.outputs.raw_key",
    "raw_meta_key.$": "$.plan.outputs.raw_meta_key",
    "plan_params_digest.$": "$.plan.outputs.plan_params_digest",
    "clip_slots.$": "$.solve_score_clip.parsed.clip_slots",
    "score_program.$": "$.solve_score_clip.parsed.score_program",
    "score_output_normalize.$": "$.solve_score_clip.parsed.score_output_normalize",
    "score_output_clip_lo.$": "$.solve_score_clip.parsed.score_output_clip_lo",
    "score_output_clip_hi.$": "$.solve_score_clip.parsed.score_output_clip_hi",
    "score_output_channel_count.$": "$.solve_score_clip.parsed.score_output_channel_count",
    "score_output_has_explicit_outputs.$": "$.solve_score_clip.parsed.score_output_has_explicit_outputs",
    "score_output_interpretation.$": "$.plan.outputs.metadata.score_output_interpretation",
    "score_output_channels.$": "$.solve_score_clip.parsed.score_output_channels",
    "chain_fingerprint.$": "$.solve_score_clip.parsed.chain_fingerprint",
    "solve_score_spec_version.$": "$.plan.outputs.metadata.solve_score_spec_version",
    "fragment_prefix.$": "$.plan.outputs.fragment_prefix",
    "fragment_manifest": RENDER_FINALIZE_MT_FRAGMENT_MANIFEST,
    "associated_palette.$": "$.plan.associated_palette",
    "associated_palette_grid_n.$": "$.plan.calc.N",
    "associated_palette_times.$": "$.plan.calc.times",
    "associated_palette_degree.$": "$.plan.calc.degree",
}

RENDER_BILEVEL_RASTER_ITEM_SELECTOR = {
    "phase": "section_raster",
    "job_id.$": "$.job_id",
    "section_idx.$": "$$.Map.Item.Value.section_idx",
    "section_count.$": "$$.Map.Item.Value.section_count",
    "step_start.$": "$$.Map.Item.Value.step_start",
    "step_count.$": "$$.Map.Item.Value.step_count",
    "task_id.$": "States.Format('render_{}_bilevel_section_{}', $.run_id, $$.Map.Item.Value.section_idx)",
    "solve_source_manifest_key.$": "$.plan.solve_source_manifest_key",
    "fragment_prefix.$": "$.plan.bilevel.fragment_prefix",
    "pix.$": "$.plan.grid.pix",
    "min_re.$": "$.plan.viewport.min_re",
    "max_re.$": "$.plan.viewport.max_re",
    "min_im.$": "$.plan.viewport.min_im",
    "max_im.$": "$.plan.viewport.max_im",
    "rotation.$": "$.plan.params.rotation",
    "degree.$": "$.plan.calc.degree",
    "root_transforms.$": "$.plan.params.root_transforms",
}

RENDER_BILEVEL_FINALIZE_TASK_PAYLOAD = {
    "phase": "finalize",
    "job_id.$": "$.job_id",
    "task_id.$": "States.Format('render_{}_bilevel_finalize', $.run_id)",
    "pix.$": "$.plan.grid.pix",
    "source_item_count.$": "$.plan.bilevel.item_count",
    "fragment_prefix.$": "$.plan.bilevel.fragment_prefix",
    "out_key.$": "$.plan.outputs.bilevel_key",
    "preview_key.$": "$.plan.outputs.preview_key",
    "metadata.$": "$.plan.outputs.metadata",
}

RENDER_COEFF_RASTER_ITEM_SELECTOR = {
    "phase": "coeff_raster",
    "job_id.$": "$.job_id",
    "section_idx.$": "$$.Map.Item.Value.section_idx",
    "section_count.$": "$$.Map.Item.Value.section_count",
    "step_start.$": "$$.Map.Item.Value.step_start",
    "step_count.$": "$$.Map.Item.Value.step_count",
    "task_id.$": "States.Format('render_{}_coeff_bilevel_section_{}', $.run_id, $$.Map.Item.Value.section_idx)",
    "solve_source_manifest_key.$": "$.plan.solve_source_manifest_key",
    "fragment_prefix.$": "$.plan.coeff_bilevel.fragment_prefix",
    "pix.$": "$.plan.grid.pix",
    "min_re.$": "$.plan.viewport.min_re",
    "max_re.$": "$.plan.viewport.max_re",
    "min_im.$": "$.plan.viewport.min_im",
    "max_im.$": "$.plan.viewport.max_im",
    "rotation.$": "$.plan.params.rotation",
    "degree.$": "$.plan.calc.degree",
    "n_coeffs.$": "$.plan.calc.n_coeffs",
}

RENDER_COEFF_FINALIZE_TASK_PAYLOAD = {
    "phase": "finalize",
    "job_id.$": "$.job_id",
    "task_id.$": "States.Format('render_{}_coeff_bilevel_finalize', $.run_id)",
    "pix.$": "$.plan.grid.pix",
    "source_item_count.$": "$.plan.coeff_bilevel.item_count",
    "fragment_prefix.$": "$.plan.coeff_bilevel.fragment_prefix",
    "out_key.$": "$.plan.outputs.coeff_bilevel_key",
    "preview_key.$": "$.plan.outputs.preview_key",
    "metadata.$": "$.plan.outputs.metadata",
}

LEGACY_VIEWPORT_SELECTOR_FIELDS = (
    "center_re.$",
    "center_im.$",
    "scale.$",
)

FINALIZE_MT_METADATA_REQUIRED_FIELDS = (
    "background_color",
    "color_mode",
    "solve_score_chain_fingerprint",
    "score_program",
)

FINALIZE_MT_ASSOCIATED_PALETTE_REQUIRED_FIELDS = (
    "enabled",
    "mode",
    "palette_id",
    "display_name",
    "image_key",
    "preview_key",
    "meta_key",
    "raw_key",
    "raw_meta_key",
    "fragment_prefix",
    "source_color_artifact_id",
    "metric",
    "palette",
    "color_interpretation",
    "quantile",
    "omega",
    "omega_enabled",
    "score_chain",
)


def iter_jsonpath_values(obj):
    if isinstance(obj, dict):
        for value in obj.values():
            yield from iter_jsonpath_values(value)
        return
    if isinstance(obj, (list, tuple)):
        for value in obj:
            yield from iter_jsonpath_values(value)
        return
    if isinstance(obj, str):
        for match in re.finditer(r"(?<!\$)(\$\.[A-Za-z0-9_]+(?:\.[A-Za-z0-9_]+)*)", obj):
            yield match.group(1)
