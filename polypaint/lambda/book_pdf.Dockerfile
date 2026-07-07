# Book Maker compose/prepare image (book-maker-design.md §5).
# The app's first container-image lambda: TeX cannot ship as a zip layer
# (250MB unzipped cap); this image carries a TinyTeX-style minimal TeX Live
# with LuaLaTeX, libvips for image prep, and the tracked fonts.
#
# Build (arm64, from polypaint/ context):
#   docker buildx build --platform linux/arm64 -f lambda/book_pdf.Dockerfile .
# --- stage 1: libvips, built with the SAME recipe as the lambda layer
# (lambda/build-libvips-layer.sh, vips 8.16.1) — no vips package exists in
# AL2023 repos, and container images cannot attach the layer. ---
FROM public.ecr.aws/amazonlinux/amazonlinux:2023 AS vipsbuild
ARG VIPS_VERSION=8.16.1
RUN dnf install -y gcc gcc-c++ make meson ninja-build pkg-config \
      glib2-devel expat-devel libjpeg-turbo-devel libpng-devel \
      libtiff-devel libwebp-devel jbigkit-devel libxml2-devel \
      libarchive-devel zlib-devel tar xz wget bzip2-devel \
    && dnf clean all
RUN cd /tmp \
    && wget -q "https://github.com/libvips/libvips/releases/download/v${VIPS_VERSION}/vips-${VIPS_VERSION}.tar.xz" \
    && tar xJf "vips-${VIPS_VERSION}.tar.xz" \
    && cd "vips-${VIPS_VERSION}" \
    && meson setup builddir --prefix=/opt --buildtype=release \
       -Dmodules=disabled -Dintrospection=disabled -Dheif=disabled \
       -Dpoppler=disabled -Drsvg=disabled -Dtiff=enabled -Dopenexr=disabled \
       -Dfits=disabled -Dimagequant=disabled -Dcgif=disabled -Dspng=disabled \
       -Dwebp=disabled -Dpdfium=disabled -Dnifti=disabled -Dopenslide=disabled \
       -Dexif=disabled -Dlcms=disabled \
    && cd builddir && ninja -j"$(nproc)" && ninja install

FROM public.ecr.aws/lambda/python:3.12

# --- system deps: perl for install-tl; vips runtime libs for stage-1 copy;
# poppler-utils (pdftoppm) rasterizes flipbook pages — this image's vips is
# built -Dpoppler=disabled and cannot load PDFs (flipbook.md §1) ---
RUN dnf install -y perl-core wget tar gzip fontconfig \
      glib2 expat libjpeg-turbo libpng libtiff libwebp jbigkit-libs \
      libxml2 libarchive poppler-utils \
    && dnf clean all
COPY --from=vipsbuild /opt/lib*/libvips*.so* /opt/lib/
COPY --from=vipsbuild /opt/bin/vipsthumbnail /opt/bin/vipsthumbnail
ENV LD_LIBRARY_PATH=/opt/lib

# --- TinyTeX-style TeX Live: scheme-basic + pinned package list ---
# (mirrors the user's local TinyTeX recipe: minimal scheme, tlmgr adds
# exactly what the fixed template needs)
ENV TEXDIR=/opt/texlive
RUN cd /tmp \
    && wget -q https://mirror.ctan.org/systems/texlive/tlnet/install-tl-unx.tar.gz \
    && tar xzf install-tl-unx.tar.gz \
    && cd install-tl-2* \
    && printf 'selected_scheme scheme-basic\nTEXDIR %s\nTEXMFLOCAL %s/texmf-local\ntlpdbopt_install_docfiles 0\ntlpdbopt_install_srcfiles 0\n' "$TEXDIR" "$TEXDIR" > tl.profile \
    && ./install-tl --profile=tl.profile \
    && rm -rf /tmp/install-tl-2* /tmp/install-tl-unx.tar.gz
ENV PATH="$TEXDIR/bin/aarch64-linux:$TEXDIR/bin/x86_64-linux:$PATH"
RUN tlmgr install fontspec microtype geometry xcolor eso-pic pgf luaotfload
RUN tlmgr path add || true

# Lambda mounts the image filesystem READ-ONLY except /tmp. luaotfload builds
# its font cache at runtime and needs a writable path, so point HOME/TEXMFVAR
# at /tmp (Lambda persists /tmp across warm invocations, so the cache is built
# once per cold container).
ENV HOME=/tmp
ENV TEXMFVAR=/tmp/texmf-var

# --- fonts: tracked TTFs installed into the texmf tree so fontspec resolves
# them by name from a STABLE path (a stable luaotfload cache key across
# compiles, unlike per-build-dir copies). Build FAILS if missing. ---
COPY fonts/ /opt/book-fonts/
RUN test -s /opt/book-fonts/TiemposText-Regular-Trial.ttf \
    && mkdir -p "$TEXDIR/texmf-local/fonts/truetype/polypaint" \
    && cp /opt/book-fonts/*.ttf "$TEXDIR/texmf-local/fonts/truetype/polypaint/" \
    && mktexlsr

# --- python deps + handler code ---
RUN pip install --no-cache-dir "boto3>=1.34" "Pillow>=10,<12" "reportlab>=4,<5"
COPY lambda/book_tex.py lambda/book_pdf.py lambda/spread_pdf.py lambda/shared.py ${LAMBDA_TASK_ROOT}/

CMD ["book_pdf.handler"]
