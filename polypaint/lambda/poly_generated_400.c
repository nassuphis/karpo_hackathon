/* AUTO-GENERATED from poly400.py — do not edit manually */
/* 100 coefficient functions */

static void poly_301_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_re4r, _re4i, j, 0, &_mul5r, &_mul5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(_mul5r, _mul5i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 4.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double mag_part1 = _mul14r; /* +_mul14ii */
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 0.5; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_abs19r, _abs19i, 0.5, &_pow21r, &_pow21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_cos18r, _cos18i, _pow21r, _pow21i, &_mul22r, &_mul22i);
        double mag_part2 = _mul22r; /* +_mul22ii */
        double _add23r = 0, _add23i = 0;
        _add23r = mag_part1 + mag_part2; _add23i = 0 + 0;
        double _div24r = 0, _div24i = 0;
        c_div(0, 0, 0, 0, &_div24r, &_div24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _add23r + _div24r; _add25i = _add23i + _div24i;
        double magnitude = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double angle_part1 = _mul30r; /* +_mul30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 3.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double angle_part2 = _mul35r; /* +_mul35ii */
        double _add36r = 0, _add36i = 0;
        _add36r = angle_part1 + angle_part2; _add36i = 0 + 0;
        double _sin37r = 0, _sin37i = 0;
        c_sin(j, 0, &_sin37r, &_sin37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 2.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(j, 0, _c38r, _c38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_sin37r, _sin37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _add36r + _mul41r; _add42i = _add36i + _mul41i;
        double phase = _add42r; /* +_add42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, phase, 0, &_mul44r, &_mul44i);
        double _exp45r = 0, _exp45i = 0;
        c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _exp45r, _exp45i, &_mul46r, &_mul46i);
        double _conj47r = 0, _conj47i = 0;
        _conj47r = x1r; _conj47i = -(x1i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(j, 0, M_PI, 0, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 6.0; _c49i = 0;
        double _div50r = 0, _div50i = 0;
        c_div(_mul48r, _mul48i, _c49r, _c49i, &_div50r, &_div50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(_div50r, _div50i, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_conj47r, _conj47i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul46r + _mul52r; _add53i = _mul46i + _mul52i;
        double _conj54r = 0, _conj54i = 0;
        _conj54r = x2r; _conj54i = -(x2i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(j, 0, M_PI, 0, &_mul55r, &_mul55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 5.0; _c56i = 0;
        double _div57r = 0, _div57i = 0;
        c_div(_mul55r, _mul55i, _c56r, _c56i, &_div57r, &_div57i);
        double _cos58r = 0, _cos58i = 0;
        c_cos(_div57r, _div57i, &_cos58r, &_cos58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(_conj54r, _conj54i, _cos58r, _cos58i, &_mul59r, &_mul59i);
        double _sub60r = 0, _sub60i = 0;
        _sub60r = _add53r - _mul59r; _sub60i = _add53i - _mul59i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub60r; cIm[_idx] = _sub60i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_302_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs8r + j; _add9i = _abs8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _arr12r = 0, _arr12i = 0;
        { int _idx = (j - 1); _arr12r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr12i = 0; }
        double _arr13r = 0, _arr13i = 0;
        { int _idx = (j - 1); _arr13r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr13i = 0; }
        double _mul14r = 0, _mul14i = 0;
        c_mul(_arr12r, _arr12i, _arr13r, _arr13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c11r + _sin15r; _add16i = _c11i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log10r, _log10i, _add16r, _add16i, &_mul17r, &_mul17i);
        double _re18r = 0, _re18i = 0;
        _re18r = x1r; _re18i = 0;
        double _im19r = 0, _im19i = 0;
        _im19r = x2i; _im19i = 0;
        double _prod20r = 0, _prod20i = 0;
        c_mul(_re18r, _re18i, _im19r, _im19i, &_prod20r, &_prod20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_prod20r, _prod20i, _add22r, _add22i, &_div23r, &_div23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul17r + _div23r; _add24i = _mul17i + _div23i;
        double mag = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(j, 0, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _cos29r = 0, _cos29i = 0;
        c_cos(j, 0, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul27r + _mul30r; _add31i = _mul27i + _mul30i;
        double _arr32r = 0, _arr32i = 0;
        { int _idx = (j - 1); _arr32r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr32i = 0; }
        double _arr33r = 0, _arr33i = 0;
        { int _idx = (j - 1); _arr33r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr33i = 0; }
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _arr33r + _c34r; _add35i = _arr33i + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_arr32r, _arr32i, _add35r, _add35i, &_div36r, &_div36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add31r + _sin37r; _add38i = _add31i + _sin37i;
        double angle = _add38r; /* +_add38ii */
        double _cos39r = 0, _cos39i = 0;
        c_cos(angle, 0, &_cos39r, &_cos39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _sin41r = 0, _sin41i = 0;
        c_sin(angle, 0, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c40r, _c40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cos39r + _mul42r; _add43i = _cos39i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x1r; _conj45i = -(x1i);
        double _c46r = 0, _c46i = 0;
        _c46r = 3.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(j, 0, _c46r, _c46i, &_div47r, &_div47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_div47r, _div47i, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_conj45r, _conj45i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul44r + _mul49r; _add50i = _mul44i + _mul49i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_303_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double r1 = _re1r; /* +_re1ii */
        double _im2r = 0, _im2i = 0;
        _im2r = x1i; _im2i = 0;
        double i1 = _im2r; /* +_im2ii */
        double _re3r = 0, _re3i = 0;
        _re3r = x2r; _re3i = 0;
        double r2 = _re3r; /* +_re3ii */
        double _im4r = 0, _im4i = 0;
        _im4r = x2i; _im4i = 0;
        double i2 = _im4r; /* +_im4ii */
        double _mul5r = 0, _mul5i = 0;
        c_mul(r1, 0, j, 0, &_mul5r, &_mul5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(_mul5r, _mul5i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 3.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double term1 = _mul14r; /* +_mul14ii */
        double _add15r = 0, _add15i = 0;
        _add15r = i2 + j; _add15i = 0 + 0;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_add15r, _add15i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log19r, _log19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double term2 = _mul24r; /* +_mul24ii */
        double _conj25r = 0, _conj25i = 0;
        _conj25r = x1r; _conj25i = -(x1i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_conj25r, _conj25i, x2r, x2i, &_mul26r, &_mul26i);
        double _re27r = 0, _re27i = 0;
        _re27r = _mul26r; _re27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_re27r, _re27i, _add29r, _add29i, &_div30r, &_div30i);
        double term3 = _div30r; /* +_div30ii */
        double _add31r = 0, _add31i = 0;
        _add31r = term1 + term2; _add31i = 0 + 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _add31r + term3; _add32i = _add31i + 0;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_add32r, _add32i); _abs33i = 0;
        double mag = _abs33r; /* +_abs33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 6.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang34r, _ang34i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(x2r, x2i); _ang40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, M_PI, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 8.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(_mul41r, _mul41i, _c42r, _c42i, &_div43r, &_div43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_div43r, _div43i, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_ang40r, _ang40i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul39r + _mul45r; _add46i = _mul39i + _mul45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(i1, 0, i2, 0, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = r1 + _mul47r; _add48i = 0 + _mul47i;
        double _abs49r = 0, _abs49i = 0;
        _abs49r = c_abs(_add48r, _add48i); _abs49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 1.0; _c50i = 0;
        double _add51r = 0, _add51i = 0;
        _add51r = _abs49r + _c50r; _add51i = _abs49i + _c50i;
        double _log52r = 0, _log52i = 0;
        c_log(_add51r, _add51i, &_log52r, &_log52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 2.0; _c53i = 0;
        double _add54r = 0, _add54i = 0;
        _add54r = j + _c53r; _add54i = 0 + _c53i;
        double _div55r = 0, _div55i = 0;
        c_div(_log52r, _log52i, _add54r, _add54i, &_div55r, &_div55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _add46r + _div55r; _add56i = _add46i + _div55i;
        double angle = _add56r; /* +_add56ii */
        double _cos57r = 0, _cos57i = 0;
        c_cos(angle, 0, &_cos57r, &_cos57i);
        double _c58r = 0, _c58i = 0;
        _c58r = 0.0; _c58i = 1.0;
        double _sin59r = 0, _sin59i = 0;
        c_sin(angle, 0, &_sin59r, &_sin59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_c58r, _c58i, _sin59r, _sin59i, &_mul60r, &_mul60i);
        double _add61r = 0, _add61i = 0;
        _add61r = _cos57r + _mul60r; _add61i = _cos57i + _mul60i;
        double _mul62r = 0, _mul62i = 0;
        c_mul(mag, 0, _add61r, _add61i, &_mul62r, &_mul62i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul62r; cIm[_idx] = _mul62i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_304: too complex for auto-transpile, stubbed */
static void poly_304_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_305_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double real_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        real_seq[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imag_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imag_seq[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int k = 1; k < (int)(_add7r); k++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (k - 1); _arr8r = (_idx >= 0 && _idx < 35) ? real_seq[_idx] : 0.0; _arr8i = 0; }
        double r = _arr8r; /* +_arr8ii */
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (k - 1); _arr9r = (_idx >= 0 && _idx < 35) ? imag_seq[_idx] : 0.0; _arr9i = 0; }
        double im = _arr9r; /* +_arr9ii */
        double _mul10r = 0, _mul10i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul10r, &_mul10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_mul10r, _mul10i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(k, 0, k, 0, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _abs11r + _pow13r; _add14i = _abs11i + _pow13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(k, 0, &_sin17r, &_sin17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(k, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_sin17r, _sin17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _c16r + _mul21r; _add22i = _c16i + _mul21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log15r, _log15i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag_pattern = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(k, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 4.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(k, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul28r + _mul33r; _add34i = _mul28i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(k, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 5.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add34r + _sin38r; _add39i = _add34i + _sin38i;
        double angle_pattern = _add39r; /* +_add39ii */
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, angle_pattern, 0, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(mag_pattern, 0, _exp42r, _exp42i, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, angle_pattern, 0, &_mul45r, &_mul45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 2.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(_mul45r, _mul45i, _c46r, _c46i, &_div47r, &_div47i);
        double _exp48r = 0, _exp48i = 0;
        c_exp2(_div47r, _div47i, &_exp48r, &_exp48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(mag_pattern, 0, _exp48r, _exp48i, &_mul49r, &_mul49i);
        double _conj50r = 0, _conj50i = 0;
        _conj50r = _mul49r; _conj50i = -(_mul49i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul43r + _conj50r; _add51i = _mul43i + _conj50i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_306_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double rec1 = _re2r; /* +_re2ii */
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec2 = _re3r; /* +_re3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double imc1 = _im4r; /* +_im4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc2 = _im5r; /* +_im5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(j, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _mod9r + _c10r; _add11i = _mod9i + _c10i;
        double k = _add11r; /* +_add11ii */
        double _mul12r = 0, _mul12i = 0;
        c_mul(rec1, 0, rec2, 0, &_mul12r, &_mul12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_mul12r, _mul12i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_mul17r, _mul17i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _log16r + _log21r; _add22i = _log16i + _log21i;
        double r = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _sin24r = 0, _sin24i = 0;
        c_sin(j, 0, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _mul28r; _add29i = _mul25i + _mul28i;
        double angle = _add29r; /* +_add29ii */
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(_abs30r, _abs30i, k, &_pow31r, &_pow31i);
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x2r, x2i); _abs32i = 0;
        double _sub33r = 0, _sub33i = 0;
        _sub33r = n - k; _sub33i = 0 - 0;
        double _pow34r = 0, _pow34i = 0;
        c_powr(_abs32r, _abs32i, _sub33r, &_pow34r, &_pow34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _pow31r + _pow34r; _add35i = _pow31i + _pow34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 7.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(r, 0, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _add35r + _mul40r; _add41i = _add35i + _mul40i;
        double magnitude = _add41r; /* +_add41ii */
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, M_PI, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 5.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _add46r = 0, _add46i = 0;
        _add46r = angle + _cos45r; _add46i = 0 + _cos45i;
        double phase = _add46r; /* +_add46ii */
        double _cos47r = 0, _cos47i = 0;
        c_cos(phase, 0, &_cos47r, &_cos47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _sin49r = 0, _sin49i = 0;
        c_sin(phase, 0, &_sin49r, &_sin49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c48r, _c48i, _sin49r, _sin49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _cos47r + _mul50r; _add51i = _cos47i + _mul50i;
        double _mul52r = 0, _mul52i = 0;
        c_mul(magnitude, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
        double _c53r = 0, _c53i = 0;
        _c53r = 7.0; _c53i = 0;
        double _mod54r = 0, _mod54i = 0;
        _mod54r = fmod(j, _c53r); _mod54i = 0;
        double _c55r = 0, _c55i = 0;
        _c55r = 0.0; _c55i = 0;
        if (_mod54r == _c55r) {
            double _cf56r = 0, _cf56i = 0;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf56r = cRe[_idx]; _cf56i = cIm[_idx]; } }
            double _conj57r = 0, _conj57i = 0;
            _conj57r = x1r; _conj57i = -(x1i);
            double _mul58r = 0, _mul58i = 0;
            c_mul(_cf56r, _cf56i, _conj57r, _conj57i, &_mul58r, &_mul58i);
            double _conj59r = 0, _conj59i = 0;
            _conj59r = x2r; _conj59i = -(x2i);
            double _add60r = 0, _add60i = 0;
            _add60r = _mul58r + _conj59r; _add60i = _mul58i + _conj59i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add60r; cIm[_idx] = _add60i; } }
        }
        double _c61r = 0, _c61i = 0;
        _c61r = 3.0; _c61i = 0;
        double _mod62r = 0, _mod62i = 0;
        _mod62r = fmod(j, _c61r); _mod62i = 0;
        double _c63r = 0, _c63i = 0;
        _c63r = 0.0; _c63i = 0;
        if (_mod62r == _c63r) {
            double _mul64r = 0, _mul64i = 0;
            c_mul(x1r, x1i, j, 0, &_mul64r, &_mul64i);
            double _sin65r = 0, _sin65i = 0;
            c_sin(_mul64r, _mul64i, &_sin65r, &_sin65i);
            double _div66r = 0, _div66i = 0;
            c_div(x2r, x2i, j, 0, &_div66r, &_div66i);
            double _cos67r = 0, _cos67i = 0;
            c_cos(_div66r, _div66i, &_cos67r, &_cos67i);
            double _mul68r = 0, _mul68i = 0;
            c_mul(_sin65r, _sin65i, _cos67r, _cos67i, &_mul68r, &_mul68i);
            cRe[(j - 1)] += _mul68r; cIm[(j - 1)] += _mul68i;
        }
        double _c69r = 0, _c69i = 0;
        _c69r = 4.0; _c69i = 0;
        double _mod70r = 0, _mod70i = 0;
        _mod70r = fmod(j, _c69r); _mod70i = 0;
        double _c71r = 0, _c71i = 0;
        _c71r = 0.0; _c71i = 0;
        if (_mod70r == _c71r) {
            double _c72r = 0, _c72i = 0;
            _c72r = 0.0; _c72i = 1.0;
            double _mul73r = 0, _mul73i = 0;
            c_mul(rec1, 0, j, 0, &_mul73r, &_mul73i);
            double _sub74r = 0, _sub74i = 0;
            _sub74r = _mul73r - imc2; _sub74i = _mul73i - 0;
            double _mul75r = 0, _mul75i = 0;
            c_mul(_c72r, _c72i, _sub74r, _sub74i, &_mul75r, &_mul75i);
            double _exp76r = 0, _exp76i = 0;
            c_exp2(_mul75r, _mul75i, &_exp76r, &_exp76i);
            cRe[(j - 1)] += _exp76r; cIm[(j - 1)] += _exp76i;
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_307_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 4.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 3.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _sin11r + _cos15r; _add16i = _sin11i + _cos15i;
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang17r, _ang17i, j, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 10.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add16r + _div20r; _add21i = _add16i + _div20i;
        double phase = _add21r; /* +_add21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs22r + j; _add23i = _abs22i + 0;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 6.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _c25r + _sin29r; _add30i = _c25i + _sin29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log24r, _log24i, _add30r, _add30i, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.5; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_powr(0, 0, 0.5, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 8.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_pow33r, _pow33i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul31r + _mul38r; _add39i = _mul31i + _mul38i;
        double magnitude = _add39r; /* +_add39ii */
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, phase, 0, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude, 0, _exp42r, _exp42i, &_mul43r, &_mul43i);
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x2r; _conj44i = -(x2i);
        double _c45r = 0, _c45i = 0;
        _c45r = 5.0; _c45i = 0;
        double _mod46r = 0, _mod46i = 0;
        _mod46r = fmod(j, _c45r); _mod46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_conj44r, _conj44i, _mod46r, _mod46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul43r + _mul47r; _add48i = _mul43i + _mul47i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_308_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(j, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 0.0; _c10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _arr12r = 0, _arr12i = 0;
        { int _idx = (j - 1); _arr12r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr12i = 0; }
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_arr12r, _arr12i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_abs11r, _abs11i, _log16r, _log16i, &_mul17r, &_mul17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul17r, _mul17i, j, 0, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _arr20r = 0, _arr20i = 0;
        { int _idx = (j - 1); _arr20r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr20i = 0; }
        double _sin21r = 0, _sin21i = 0;
        c_sin(_arr20r, _arr20i, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_abs19r, _abs19i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = x1r + x2r; _add23i = x1i + x2i;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_add23r, _add23i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _div27r = 0, _div27i = 0;
        c_div(_abs24r, _abs24i, _add26r, _add26i, &_div27r, &_div27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul22r + _div27r; _add28i = _mul22i + _div27i;
        double _tern29r = 0, _tern29i = 0;
        if (_mod9r == _c10r) { _tern29r = _mul18r; _tern29i = _mul18i; }
        else { _tern29r = _add28r; _tern29i = _add28i; }
        double magnitude = _tern29r; /* +_tern29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(n, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _arr33r = 0, _arr33i = 0;
        { int _idx = (j - 1); _arr33r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr33i = 0; }
        double _mul34r = 0, _mul34i = 0;
        c_mul(_arr33r, _arr33i, M_PI, 0, &_mul34r, &_mul34i);
        double _div35r = 0, _div35i = 0;
        c_div(_mul34r, _mul34i, j, 0, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _ang32r + _sin36r; _add37i = _ang32i + _sin36i;
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _arr39r = 0, _arr39i = 0;
        { int _idx = (j - 1); _arr39r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr39i = 0; }
        double _mul40r = 0, _mul40i = 0;
        c_mul(_arr39r, _arr39i, M_PI, 0, &_mul40r, &_mul40i);
        double _div41r = 0, _div41i = 0;
        c_div(_mul40r, _mul40i, j, 0, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _ang38r + _cos42r; _add43i = _ang38i + _cos42i;
        double _tern44r = 0, _tern44i = 0;
        if (j <= _div31r) { _tern44r = _add37r; _tern44i = _add37i; }
        else { _tern44r = _add43r; _tern44i = _add43i; }
        double angle = _tern44r; /* +_tern44ii */
        double _cos45r = 0, _cos45i = 0;
        c_cos(angle, 0, &_cos45r, &_cos45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _sin47r = 0, _sin47i = 0;
        c_sin(angle, 0, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c46r, _c46i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _cos45r + _mul48r; _add49i = _cos45i + _mul48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(magnitude, 0, _add49r, _add49i, &_mul50r, &_mul50i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    double _c51r = 0, _c51i = 0;
    _c51r = 1.0; _c51i = 0;
    double _add52r = 0, _add52i = 0;
    _add52r = n + _c51r; _add52i = 0 + _c51i;
    for (int k = 1; k < (int)(_add52r); k++) {
        double _cf53r = 0, _cf53i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf53r = cRe[_idx]; _cf53i = cIm[_idx]; } }
        double _conj54r = 0, _conj54i = 0;
        _conj54r = _cf53r; _conj54i = -(_cf53i);
        double _c55r = 0, _c55i = 0;
        _c55r = 0.0; _c55i = 1.0;
        double _sin56r = 0, _sin56i = 0;
        c_sin(k, 0, &_sin56r, &_sin56i);
        double _cos57r = 0, _cos57i = 0;
        c_cos(k, 0, &_cos57r, &_cos57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _sin56r + _cos57r; _add58i = _sin56i + _cos57i;
        double _mul59r = 0, _mul59i = 0;
        c_mul(_c55r, _c55i, _add58r, _add58i, &_mul59r, &_mul59i);
        double _exp60r = 0, _exp60i = 0;
        c_exp2(_mul59r, _mul59i, &_exp60r, &_exp60i);
        double _mul61r = 0, _mul61i = 0;
        c_mul(_conj54r, _conj54i, _exp60r, _exp60i, &_mul61r, &_mul61i);
        cRe[(k - 1)] += _mul61r; cIm[(k - 1)] += _mul61i;
    }
    double _c62r = 0, _c62i = 0;
    _c62r = 1.0; _c62i = 0;
    double _add63r = 0, _add63i = 0;
    _add63r = n + _c62r; _add63i = 0 + _c62i;
    for (int r = 1; r < (int)(_add63r); r++) {
        double _c64r = 0, _c64i = 0;
        _c64r = 1.0; _c64i = 0;
        double _cf65r = 0, _cf65i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf65r = cRe[_idx]; _cf65i = cIm[_idx]; } }
        double _abs66r = 0, _abs66i = 0;
        _abs66r = c_abs(_cf65r, _cf65i); _abs66i = 0;
        double _c67r = 0, _c67i = 0;
        _c67r = 1.0; _c67i = 0;
        double _add68r = 0, _add68i = 0;
        _add68r = _abs66r + _c67r; _add68i = _abs66i + _c67i;
        double _log69r = 0, _log69i = 0;
        c_log(_add68r, _add68i, &_log69r, &_log69i);
        double _add70r = 0, _add70i = 0;
        _add70r = _c64r + _log69r; _add70i = _c64i + _log69i;
        double _c71r = 0, _c71i = 0;
        _c71r = 1.0; _c71i = 0;
        double _div72r = 0, _div72i = 0;
        c_div(r, 0, n, 0, &_div72r, &_div72i);
        double _add73r = 0, _add73i = 0;
        _add73r = _c71r + _div72r; _add73i = _c71i + _div72i;
        double _div74r = 0, _div74i = 0;
        c_div(_add70r, _add70i, _add73r, _add73i, &_div74r, &_div74i);
        { double _tr = cRe[(r - 1)]*_div74r - cIm[(r - 1)]*_div74i; cIm[(r - 1)] = cRe[(r - 1)]*_div74i + cIm[(r - 1)]*_div74r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_309_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr8i = 0; }
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (j - 1); _arr9r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr9i = 0; }
        double _c10r = 0, _c10i = 0;
        _c10r = 0.0; _c10i = 1.0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_arr9r, _arr9i, _c10r, _c10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _arr8r + _mul11r; _add12i = _arr8i + _mul11i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_add12r, _add12i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 4.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _c17r + _sin21r; _add22i = _c17i + _sin21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log16r, _log16i, _add22r, _add22i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 5.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_div27r, _div27i, &_cos28r, &_cos28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _c24r + _cos28r; _add29i = _c24i + _cos28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul23r, _mul23i, _add29r, _add29i, &_mul30r, &_mul30i);
        double mag = _mul30r; /* +_mul30ii */
        double _arr31r = 0, _arr31i = 0;
        { int _idx = (j - 1); _arr31r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr31i = 0; }
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, _arr31r, _arr31i, &_mul32r, &_mul32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_mul32r, _mul32i, &_sin33r, &_sin33i);
        double _arr34r = 0, _arr34i = 0;
        { int _idx = (j - 1); _arr34r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr34i = 0; }
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, _arr34r, _arr34i, &_mul35r, &_mul35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_mul35r, _mul35i, &_cos36r, &_cos36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sin33r + _cos36r; _add37i = _sin33i + _cos36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul38r, &_mul38i);
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(_mul38r, _mul38i); _ang39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = j + _c40r; _add41i = 0 + _c40i;
        double _div42r = 0, _div42i = 0;
        c_div(_ang39r, _ang39i, _add41r, _add41i, &_div42r, &_div42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add37r + _div42r; _add43i = _add37i + _div42i;
        double angle = _add43r; /* +_add43ii */
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, angle, 0, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(mag, 0, _exp46r, _exp46i, &_mul47r, &_mul47i);
        double _conj48r = 0, _conj48i = 0;
        _conj48r = x2r; _conj48i = -(x2i);
        double _div49r = 0, _div49i = 0;
        c_div(j, 0, n, 0, &_div49r, &_div49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_div49r, _div49i, M_PI, 0, &_mul50r, &_mul50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(_mul50r, _mul50i, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_conj48r, _conj48i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul47r + _mul52r; _add53i = _mul47i + _mul52i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add53r; cIm[_idx] = _add53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_310_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs4r + _c5r; _add6i = _abs4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 5.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log7r, _log7i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 7.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log16r, _log16i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul12r + _mul21r; _add22i = _mul12i + _mul21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.5; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(j, 0, 1.5, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _add22r + _pow24r; _add25i = _add22i + _pow24i;
        double mag_part = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 4.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang26r, _ang26i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang32r, _ang32i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul31r + _mul37r; _add38i = _mul31i + _mul37i;
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(j, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _add38r + _sin41r; _add42i = _add38i + _sin41i;
        double ang_part = _add42r; /* +_add42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, ang_part, 0, &_mul44r, &_mul44i);
        double _exp45r = 0, _exp45i = 0;
        c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(mag_part, 0, _exp45r, _exp45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_311_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_arr8r, _arr8i, x1r, x1i, &_mul9r, &_mul9i);
        double _arr10r = 0, _arr10i = 0;
        { int _idx = (j - 1); _arr10r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr10i = 0; }
        double _mul11r = 0, _mul11i = 0;
        c_mul(_arr10r, _arr10i, x2r, x2i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul9r + _mul11r; _add12i = _mul9i + _mul11i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_add12r, _add12i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _sin18r = 0, _sin18i = 0;
        c_sin(j, 0, &_sin18r, &_sin18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(j, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sin18r, _sin18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _c17r + _mul20r; _add21i = _c17i + _mul20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log16r, _log16i, _add21r, _add21i, &_mul22r, &_mul22i);
        double mag_part = _mul22r; /* +_mul22ii */
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _arr24r = 0, _arr24i = 0;
        { int _idx = (j - 1); _arr24r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr24i = 0; }
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul23r, _mul23i, _arr24r, _arr24i, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _arr28r = 0, _arr28i = 0;
        { int _idx = (j - 1); _arr28r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr28i = 0; }
        double _mul29r = 0, _mul29i = 0;
        c_mul(_mul27r, _mul27i, _arr28r, _arr28i, &_mul29r, &_mul29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_mul29r, _mul29i, &_cos30r, &_cos30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _sin26r + _cos30r; _add31i = _sin26i + _cos30i;
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _add31r + _ang32r; _add33i = _add31i + _ang32i;
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _add33r - _ang34r; _sub35i = _add33i - _ang34i;
        double angle_part = _sub35r; /* +_sub35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, angle_part, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag_part, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = n + _c40r; _add41i = 0 + _c40i;
    for (int k = 1; k < (int)(_add41r); k++) {
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = k + _c42r; _add43i = 0 + _c42i;
        double _log44r = 0, _log44i = 0;
        c_log(_add43r, _add43i, &_log44r, &_log44i);
        double r = _log44r; /* +_log44ii */
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _sin46r = 0, _sin46i = 0;
        c_sin(r, 0, &_sin46r, &_sin46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _c45r + _sin46r; _add47i = _c45i + _sin46i;
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _cos49r = 0, _cos49i = 0;
        c_cos(r, 0, &_cos49r, &_cos49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c48r, _c48i, _cos49r, _cos49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _add47r + _mul50r; _add51i = _add47i + _mul50i;
        { double _tr = cRe[(k - 1)]*_add51r - cIm[(k - 1)]*_add51i; cIm[(k - 1)] = cRe[(k - 1)]*_add51i + cIm[(k - 1)]*_add51r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_312_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 1; j < 36; j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = (j - 1); _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(_arr6r, _arr6i, _arr7r, _arr7i, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_mul8r, _mul8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 4.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log12r, _log12i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul17r + _cos21r; _add22i = _mul17i + _cos21i;
        double magnitude = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _sin24r = 0, _sin24i = 0;
        c_sin(j, 0, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _mul28r; _add29i = _mul25i + _mul28i;
        double _arr30r = 0, _arr30i = 0;
        { int _idx = (j - 1); _arr30r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr30i = 0; }
        double _arr31r = 0, _arr31i = 0;
        { int _idx = (j - 1); _arr31r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr31i = 0; }
        double _add32r = 0, _add32i = 0;
        _add32r = _arr30r + _arr31r; _add32i = _arr30i + _arr31i;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_add32r, _add32i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _abs33r + _c34r; _add35i = _abs33i + _c34i;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add29r + _log36r; _add37i = _add29i + _log36i;
        double angle = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, angle, 0, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_313_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double ang = 0;
        double mag = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 4.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(j, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        if (_mod9r == _c10r) {
            double _arr11r = 0, _arr11i = 0;
            { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr11i = 0; }
            double _arr12r = 0, _arr12i = 0;
            { int _idx = (j - 1); _arr12r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr12i = 0; }
            double _mul13r = 0, _mul13i = 0;
            c_mul(_arr11r, _arr11i, _arr12r, _arr12i, &_mul13r, &_mul13i);
            double _abs14r = 0, _abs14i = 0;
            _abs14r = c_abs(_mul13r, _mul13i); _abs14i = 0;
            double _c15r = 0, _c15i = 0;
            _c15r = 1.0; _c15i = 0;
            double _add16r = 0, _add16i = 0;
            _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
            double _log17r = 0, _log17i = 0;
            c_log(_add16r, _add16i, &_log17r, &_log17i);
            double _c18r = 0, _c18i = 0;
            _c18r = 1.3; _c18i = 0;
            double _pow19r = 0, _pow19i = 0;
            c_powr(j, 0, 1.3, &_pow19r, &_pow19i);
            double _sqrt20r = 0, _sqrt20i = 0;
            c_powr(j, 0, 0.5, &_sqrt20r, &_sqrt20i);
            double _add21r = 0, _add21i = 0;
            _add21r = _pow19r + _sqrt20r; _add21i = _pow19i + _sqrt20i;
            double _mul22r = 0, _mul22i = 0;
            c_mul(_log17r, _log17i, _add21r, _add21i, &_mul22r, &_mul22i);
            mag = _mul22r;
            double _mul23r = 0, _mul23i = 0;
            c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
            double _arr24r = 0, _arr24i = 0;
            { int _idx = (j - 1); _arr24r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr24i = 0; }
            double _mul25r = 0, _mul25i = 0;
            c_mul(_mul23r, _mul23i, _arr24r, _arr24i, &_mul25r, &_mul25i);
            double _sin26r = 0, _sin26i = 0;
            c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
            double _c27r = 0, _c27i = 0;
            _c27r = 2.0; _c27i = 0;
            double _pow28r = 0, _pow28i = 0;
            c_mul(j, 0, j, 0, &_pow28r, &_pow28i);
            double _mul29r = 0, _mul29i = 0;
            c_mul(_pow28r, _pow28i, M_PI, 0, &_mul29r, &_mul29i);
            double _arr30r = 0, _arr30i = 0;
            { int _idx = (j - 1); _arr30r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr30i = 0; }
            double _mul31r = 0, _mul31i = 0;
            c_mul(_mul29r, _mul29i, _arr30r, _arr30i, &_mul31r, &_mul31i);
            double _cos32r = 0, _cos32i = 0;
            c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
            double _add33r = 0, _add33i = 0;
            _add33r = _sin26r + _cos32r; _add33i = _sin26i + _cos32i;
            double _ang34r = 0, _ang34i = 0;
            _ang34r = c_arg(x1r, x1i); _ang34i = 0;
            double _re35r = 0, _re35i = 0;
            _re35r = x2r; _re35i = 0;
            double _mul36r = 0, _mul36i = 0;
            c_mul(_ang34r, _ang34i, _re35r, _re35i, &_mul36r, &_mul36i);
            double _add37r = 0, _add37i = 0;
            _add37r = _add33r + _mul36r; _add37i = _add33i + _mul36i;
            ang = _add37r;
        } else {
            double _c38r = 0, _c38i = 0;
            _c38r = 4.0; _c38i = 0;
            double _mod39r = 0, _mod39i = 0;
            _mod39r = fmod(j, _c38r); _mod39i = 0;
            double _c40r = 0, _c40i = 0;
            _c40r = 2.0; _c40i = 0;
            if (_mod39r == _c40r) {
                double _arr41r = 0, _arr41i = 0;
                { int _idx = (j - 1); _arr41r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr41i = 0; }
                double _arr42r = 0, _arr42i = 0;
                { int _idx = (j - 1); _arr42r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr42i = 0; }
                double _add43r = 0, _add43i = 0;
                _add43r = _arr41r + _arr42r; _add43i = _arr41i + _arr42i;
                double _abs44r = 0, _abs44i = 0;
                _abs44r = c_abs(_add43r, _add43i); _abs44i = 0;
                double _c45r = 0, _c45i = 0;
                _c45r = 1.0; _c45i = 0;
                double _add46r = 0, _add46i = 0;
                _add46r = _abs44r + _c45r; _add46i = _abs44i + _c45i;
                double _log47r = 0, _log47i = 0;
                c_log(_add46r, _add46i, &_log47r, &_log47i);
                double _c48r = 0, _c48i = 0;
                _c48r = 0.05; _c48i = 0;
                double _mul49r = 0, _mul49i = 0;
                c_mul(_c48r, _c48i, j, 0, &_mul49r, &_mul49i);
                double _exp50r = 0, _exp50i = 0;
                c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
                double _add51r = 0, _add51i = 0;
                _add51r = _exp50r + j; _add51i = _exp50i + 0;
                double _mul52r = 0, _mul52i = 0;
                c_mul(_log47r, _log47i, _add51r, _add51i, &_mul52r, &_mul52i);
                mag = _mul52r;
                double _mul53r = 0, _mul53i = 0;
                c_mul(j, 0, M_PI, 0, &_mul53r, &_mul53i);
                double _arr54r = 0, _arr54i = 0;
                { int _idx = (j - 1); _arr54r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr54i = 0; }
                double _mul55r = 0, _mul55i = 0;
                c_mul(_mul53r, _mul53i, _arr54r, _arr54i, &_mul55r, &_mul55i);
                double _cos56r = 0, _cos56i = 0;
                c_cos(_mul55r, _mul55i, &_cos56r, &_cos56i);
                double _c57r = 0, _c57i = 0;
                _c57r = 1.5; _c57i = 0;
                double _pow58r = 0, _pow58i = 0;
                c_powr(j, 0, 1.5, &_pow58r, &_pow58i);
                double _mul59r = 0, _mul59i = 0;
                c_mul(_pow58r, _pow58i, M_PI, 0, &_mul59r, &_mul59i);
                double _arr60r = 0, _arr60i = 0;
                { int _idx = (j - 1); _arr60r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr60i = 0; }
                double _mul61r = 0, _mul61i = 0;
                c_mul(_mul59r, _mul59i, _arr60r, _arr60i, &_mul61r, &_mul61i);
                double _sin62r = 0, _sin62i = 0;
                c_sin(_mul61r, _mul61i, &_sin62r, &_sin62i);
                double _sub63r = 0, _sub63i = 0;
                _sub63r = _cos56r - _sin62r; _sub63i = _cos56i - _sin62i;
                double _ang64r = 0, _ang64i = 0;
                _ang64r = c_arg(x2r, x2i); _ang64i = 0;
                double _im65r = 0, _im65i = 0;
                _im65r = x1i; _im65i = 0;
                double _mul66r = 0, _mul66i = 0;
                c_mul(_ang64r, _ang64i, _im65r, _im65i, &_mul66r, &_mul66i);
                double _add67r = 0, _add67i = 0;
                _add67r = _sub63r + _mul66r; _add67i = _sub63i + _mul66i;
                ang = _add67r;
            } else {
                double _c68r = 0, _c68i = 0;
                _c68r = 4.0; _c68i = 0;
                double _mod69r = 0, _mod69i = 0;
                _mod69r = fmod(j, _c68r); _mod69i = 0;
                double _c70r = 0, _c70i = 0;
                _c70r = 3.0; _c70i = 0;
                if (_mod69r == _c70r) {
                    double _arr71r = 0, _arr71i = 0;
                    { int _idx = (j - 1); _arr71r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr71i = 0; }
                    double _arr72r = 0, _arr72i = 0;
                    { int _idx = (j - 1); _arr72r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr72i = 0; }
                    double _sub73r = 0, _sub73i = 0;
                    _sub73r = _arr71r - _arr72r; _sub73i = _arr71i - _arr72i;
                    double _abs74r = 0, _abs74i = 0;
                    _abs74r = c_abs(_sub73r, _sub73i); _abs74i = 0;
                    double _c75r = 0, _c75i = 0;
                    _c75r = 1.0; _c75i = 0;
                    double _add76r = 0, _add76i = 0;
                    _add76r = _abs74r + _c75r; _add76i = _abs74i + _c75i;
                    double _log77r = 0, _log77i = 0;
                    c_log(_add76r, _add76i, &_log77r, &_log77i);
                    double _c78r = 0, _c78i = 0;
                    _c78r = 2.0; _c78i = 0;
                    double _pow79r = 0, _pow79i = 0;
                    c_mul(j, 0, j, 0, &_pow79r, &_pow79i);
                    double _c80r = 0, _c80i = 0;
                    _c80r = 1.0; _c80i = 0;
                    double _add81r = 0, _add81i = 0;
                    _add81r = _c80r + j; _add81i = _c80i + 0;
                    double _div82r = 0, _div82i = 0;
                    c_div(_pow79r, _pow79i, _add81r, _add81i, &_div82r, &_div82i);
                    double _mul83r = 0, _mul83i = 0;
                    c_mul(_log77r, _log77i, _div82r, _div82i, &_mul83r, &_mul83i);
                    mag = _mul83r;
                    double _c84r = 0, _c84i = 0;
                    _c84r = 2.0; _c84i = 0;
                    double _pow85r = 0, _pow85i = 0;
                    c_mul(j, 0, j, 0, &_pow85r, &_pow85i);
                    double _mul86r = 0, _mul86i = 0;
                    c_mul(_pow85r, _pow85i, M_PI, 0, &_mul86r, &_mul86i);
                    double _arr87r = 0, _arr87i = 0;
                    { int _idx = (j - 1); _arr87r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr87i = 0; }
                    double _mul88r = 0, _mul88i = 0;
                    c_mul(_mul86r, _mul86i, _arr87r, _arr87i, &_mul88r, &_mul88i);
                    double _sin89r = 0, _sin89i = 0;
                    c_sin(_mul88r, _mul88i, &_sin89r, &_sin89i);
                    double _mul90r = 0, _mul90i = 0;
                    c_mul(j, 0, M_PI, 0, &_mul90r, &_mul90i);
                    double _arr91r = 0, _arr91i = 0;
                    { int _idx = (j - 1); _arr91r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr91i = 0; }
                    double _mul92r = 0, _mul92i = 0;
                    c_mul(_mul90r, _mul90i, _arr91r, _arr91i, &_mul92r, &_mul92i);
                    double _cos93r = 0, _cos93i = 0;
                    c_cos(_mul92r, _mul92i, &_cos93r, &_cos93i);
                    double _mul94r = 0, _mul94i = 0;
                    c_mul(_sin89r, _sin89i, _cos93r, _cos93i, &_mul94r, &_mul94i);
                    double _ang95r = 0, _ang95i = 0;
                    _ang95r = c_arg(x1r, x1i); _ang95i = 0;
                    double _ang96r = 0, _ang96i = 0;
                    _ang96r = c_arg(x2r, x2i); _ang96i = 0;
                    double _mul97r = 0, _mul97i = 0;
                    c_mul(_ang95r, _ang95i, _ang96r, _ang96i, &_mul97r, &_mul97i);
                    double _add98r = 0, _add98i = 0;
                    _add98r = _mul94r + _mul97r; _add98i = _mul94i + _mul97i;
                    ang = _add98r;
                } else {
                    double _arr99r = 0, _arr99i = 0;
                    { int _idx = (j - 1); _arr99r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr99i = 0; }
                    double _c100r = 0, _c100i = 0;
                    _c100r = 2.0; _c100i = 0;
                    double _pow101r = 0, _pow101i = 0;
                    c_mul(_arr99r, _arr99i, _arr99r, _arr99i, &_pow101r, &_pow101i);
                    double _arr102r = 0, _arr102i = 0;
                    { int _idx = (j - 1); _arr102r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr102i = 0; }
                    double _c103r = 0, _c103i = 0;
                    _c103r = 2.0; _c103i = 0;
                    double _pow104r = 0, _pow104i = 0;
                    c_mul(_arr102r, _arr102i, _arr102r, _arr102i, &_pow104r, &_pow104i);
                    double _add105r = 0, _add105i = 0;
                    _add105r = _pow101r + _pow104r; _add105i = _pow101i + _pow104i;
                    double _abs106r = 0, _abs106i = 0;
                    _abs106r = c_abs(_add105r, _add105i); _abs106i = 0;
                    double _c107r = 0, _c107i = 0;
                    _c107r = 1.0; _c107i = 0;
                    double _add108r = 0, _add108i = 0;
                    _add108r = _abs106r + _c107r; _add108i = _abs106i + _c107i;
                    double _log109r = 0, _log109i = 0;
                    c_log(_add108r, _add108i, &_log109r, &_log109i);
                    double _sqrt110r = 0, _sqrt110i = 0;
                    c_powr(j, 0, 0.5, &_sqrt110r, &_sqrt110i);
                    double _mul111r = 0, _mul111i = 0;
                    c_mul(_log109r, _log109i, _sqrt110r, _sqrt110i, &_mul111r, &_mul111i);
                    double _c112r = 0, _c112i = 0;
                    _c112r = 1.0; _c112i = 0;
                    double _log113r = 0, _log113i = 0;
                    c_log(j, 0, &_log113r, &_log113i);
                    double _add114r = 0, _add114i = 0;
                    _add114r = _c112r + _log113r; _add114i = _c112i + _log113i;
                    double _mul115r = 0, _mul115i = 0;
                    c_mul(_mul111r, _mul111i, _add114r, _add114i, &_mul115r, &_mul115i);
                    mag = _mul115r;
                    double _mul116r = 0, _mul116i = 0;
                    c_mul(j, 0, M_PI, 0, &_mul116r, &_mul116i);
                    double _arr117r = 0, _arr117i = 0;
                    { int _idx = (j - 1); _arr117r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr117i = 0; }
                    double _mul118r = 0, _mul118i = 0;
                    c_mul(_mul116r, _mul116i, _arr117r, _arr117i, &_mul118r, &_mul118i);
                    double _c119r = 0, _c119i = 0;
                    _c119r = 2.0; _c119i = 0;
                    double _div120r = 0, _div120i = 0;
                    c_div(_mul118r, _mul118i, _c119r, _c119i, &_div120r, &_div120i);
                    double _sin121r = 0, _sin121i = 0;
                    c_sin(_div120r, _div120i, &_sin121r, &_sin121i);
                    double _c122r = 0, _c122i = 0;
                    _c122r = 3.0; _c122i = 0;
                    double _pow123r = 0, _pow123i = 0;
                    c_mul(j, 0, j, 0, &_pow123r, &_pow123i);
                    c_mul(_pow123r, _pow123i, j, 0, &_pow123r, &_pow123i);
                    double _mul124r = 0, _mul124i = 0;
                    c_mul(_pow123r, _pow123i, M_PI, 0, &_mul124r, &_mul124i);
                    double _arr125r = 0, _arr125i = 0;
                    { int _idx = (j - 1); _arr125r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr125i = 0; }
                    double _mul126r = 0, _mul126i = 0;
                    c_mul(_mul124r, _mul124i, _arr125r, _arr125i, &_mul126r, &_mul126i);
                    double _c127r = 0, _c127i = 0;
                    _c127r = 3.0; _c127i = 0;
                    double _div128r = 0, _div128i = 0;
                    c_div(_mul126r, _mul126i, _c127r, _c127i, &_div128r, &_div128i);
                    double _cos129r = 0, _cos129i = 0;
                    c_cos(_div128r, _div128i, &_cos129r, &_cos129i);
                    double _add130r = 0, _add130i = 0;
                    _add130r = _sin121r + _cos129r; _add130i = _sin121i + _cos129i;
                    ang = _add130r;
                }
            }
        }
        double _c131r = 0, _c131i = 0;
        _c131r = 0.0; _c131i = 1.0;
        double _mul132r = 0, _mul132i = 0;
        c_mul(_c131r, _c131i, ang, 0, &_mul132r, &_mul132i);
        double _exp133r = 0, _exp133i = 0;
        c_exp2(_mul132r, _mul132i, &_exp133r, &_exp133i);
        double _mul134r = 0, _mul134i = 0;
        c_mul(mag, 0, _exp133r, _exp133i, &_mul134r, &_mul134i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul134r; cIm[_idx] = _mul134i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_314_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int k = 1; k < (int)(_add3r); k++) {
        double j = k; /* +0i */
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _re6r = 0, _re6i = 0;
        _re6r = x1r; _re6i = 0;
        double _im7r = 0, _im7i = 0;
        _im7r = x2i; _im7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re6r, _re6i, _im7r, _im7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow5r + _mul8r; _add9i = _pow5i + _mul8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 7.0; _c10i = 0;
        double _mod11r = 0, _mod11i = 0;
        _mod11r = fmod(_add9r, _c10r); _mod11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _mod11r + _c12r; _add13i = _mod11i + _c12i;
        double r = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _div16r = 0, _div16i = 0;
        c_div(_mul15r, _mul15i, r, 0, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang14r, _ang14i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = r + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_mul20r, _mul20i, _add22r, _add22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang19r, _ang19i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul18r + _mul25r; _add26i = _mul18i + _mul25i;
        double angle = _add26r; /* +_add26ii */
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x1r, x1i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 0.5; _c28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c28r, _c28i, j, 0, &_mul29r, &_mul29i);
        double _pow30r = 0, _pow30i = 0;
        c_powr(_abs27r, _abs27i, _mul29r, &_pow30r, &_pow30i);
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x2r, x2i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.3; _c32i = 0;
        double _sub33r = 0, _sub33i = 0;
        _sub33r = n - j; _sub33i = 0 - 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _sub33r + _c34r; _add35i = _sub33i + _c34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c32r, _c32i, _add35r, _add35i, &_mul36r, &_mul36i);
        double _pow37r = 0, _pow37i = 0;
        c_powr(_abs31r, _abs31i, _mul36r, &_pow37r, &_pow37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _pow30r + _pow37r; _add38i = _pow30i + _pow37i;
        double magnitude = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(magnitude, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        double _conj43r = 0, _conj43i = 0;
        _conj43r = x1r; _conj43i = -(x1i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(j, 0, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_conj43r, _conj43i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul42r + _mul45r; _add46i = _mul42i + _mul45i;
        double _conj47r = 0, _conj47i = 0;
        _conj47r = x2r; _conj47i = -(x2i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(j, 0, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_conj47r, _conj47i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _sub50r = 0, _sub50i = 0;
        _sub50r = _add46r - _mul49r; _sub50i = _add46i - _mul49i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub50r; cIm[_idx] = _sub50i; } }
        double _cf51r = 0, _cf51i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
        double _cf52r = 0, _cf52i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
        double _abs53r = 0, _abs53i = 0;
        _abs53r = c_abs(_cf52r, _cf52i); _abs53i = 0;
        double _c54r = 0, _c54i = 0;
        _c54r = 1.0; _c54i = 0;
        double _add55r = 0, _add55i = 0;
        _add55r = _abs53r + _c54r; _add55i = _abs53i + _c54i;
        double _log56r = 0, _log56i = 0;
        c_log(_add55r, _add55i, &_log56r, &_log56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_cf51r, _cf51i, _log56r, _log56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _mul57r + 0; _add58i = _mul57i + 0;
        double _add59r = 0, _add59i = 0;
        _add59r = _add58r + j; _add59i = _add58i + 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_315_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re4r, _re4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _re9r = 0, _re9i = 0;
        _re9r = x2r; _re9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_re9r, _re9i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _mul11r; _add12i = _mul8i + _mul11i;
        double r = _add12r; /* +_add12ii */
        double _im13r = 0, _im13i = 0;
        _im13r = x1i; _im13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(j, 0, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_im13r, _im13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _im18r = 0, _im18i = 0;
        _im18r = x2i; _im18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul19r, &_mul19i);
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_mul19r, _mul19i); _abs20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _abs20r; _add21i = 0 + _abs20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_im18r, _im18i, _log22r, _log22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul17r + _mul23r; _add24i = _mul17i + _mul23i;
        double q = _add24r; /* +_add24ii */
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(r, 0); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 5.0; _c26i = 0;
        double _mod27r = 0, _mod27i = 0;
        _mod27r = fmod(j, _c26r); _mod27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _mod27r + _c28r; _add29i = _mod27i + _c28i;
        double _pow30r = 0, _pow30i = 0;
        c_powr(_abs25r, _abs25i, _add29r, &_pow30r, &_pow30i);
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x2r, x2i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 3.0; _c32i = 0;
        double _mod33r = 0, _mod33i = 0;
        _mod33r = fmod(j, _c32r); _mod33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 2.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _mod33r + _c34r; _add35i = _mod33i + _c34i;
        double _pow36r = 0, _pow36i = 0;
        c_powr(_abs31r, _abs31i, _add35r, &_pow36r, &_pow36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _pow30r + _pow36r; _add37i = _pow30i + _pow36i;
        double magnitude = _add37r; /* +_add37ii */
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(q, 0); _ang38i = 0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(j, 0, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang38r, _ang38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 3.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(j, 0, _c42r, _c42i, &_div43r, &_div43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_div43r, _div43i, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_ang41r, _ang41i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _sub46r = 0, _sub46i = 0;
        _sub46r = _mul40r - _mul45r; _sub46i = _mul40i - _mul45i;
        double angle = _sub46r; /* +_sub46ii */
        double _cos47r = 0, _cos47i = 0;
        c_cos(angle, 0, &_cos47r, &_cos47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _sin49r = 0, _sin49i = 0;
        c_sin(angle, 0, &_sin49r, &_sin49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c48r, _c48i, _sin49r, _sin49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _cos47r + _mul50r; _add51i = _cos47i + _mul50i;
        double _mul52r = 0, _mul52i = 0;
        c_mul(magnitude, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_316_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 3.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double k = _add5r; /* +_add5ii */
        double _re6r = 0, _re6i = 0;
        _re6r = x1r; _re6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 8.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_re6r, _re6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _re12r = 0, _re12i = 0;
        _re12r = x2r; _re12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 5.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_re12r, _re12i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul11r + _mul17r; _add18i = _mul11i + _mul17i;
        double r = _add18r; /* +_add18ii */
        double _im19r = 0, _im19i = 0;
        _im19r = x1i; _im19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 7.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_im19r, _im19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _im25r = 0, _im25i = 0;
        _im25r = x2i; _im25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 9.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_im25r, _im25i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _mul24r - _mul30r; _sub31i = _mul24i - _mul30i;
        double im = _sub31r; /* +_sub31ii */
        double _mul32r = 0, _mul32i = 0;
        c_mul(r, 0, im, 0, &_mul32r, &_mul32i);
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_mul32r, _mul32i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _abs33r + _c34r; _add35i = _abs33i + _c34i;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(k, 0, M_PI, 0, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 4.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _c37r + _sin41r; _add42i = _c37i + _sin41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_log36r, _log36i, _add42r, _add42i, &_mul43r, &_mul43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_mul43r, _mul43i, 0, 0, &_mul44r, &_mul44i);
        double _div45r = 0, _div45i = 0;
        c_div(_mul44r, _mul44i, n, 0, &_div45r, &_div45i);
        double mag = _div45r; /* +_div45ii */
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x1r, x1i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(k, 0, M_PI, 0, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 6.0; _c48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(_mul47r, _mul47i, _c48r, _c48i, &_div49r, &_div49i);
        double _cos50r = 0, _cos50i = 0;
        c_cos(_div49r, _div49i, &_cos50r, &_cos50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_ang46r, _ang46i, _cos50r, _cos50i, &_mul51r, &_mul51i);
        double _ang52r = 0, _ang52i = 0;
        _ang52r = c_arg(x2r, x2i); _ang52i = 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(k, 0, M_PI, 0, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 10.0; _c54i = 0;
        double _div55r = 0, _div55i = 0;
        c_div(_mul53r, _mul53i, _c54r, _c54i, &_div55r, &_div55i);
        double _sin56r = 0, _sin56i = 0;
        c_sin(_div55r, _div55i, &_sin56r, &_sin56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_ang52r, _ang52i, _sin56r, _sin56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _mul51r + _mul57r; _add58i = _mul51i + _mul57i;
        double ang = _add58r; /* +_add58ii */
        double _cos59r = 0, _cos59i = 0;
        c_cos(ang, 0, &_cos59r, &_cos59i);
        double _c60r = 0, _c60i = 0;
        _c60r = 0.0; _c60i = 1.0;
        double _sin61r = 0, _sin61i = 0;
        c_sin(ang, 0, &_sin61r, &_sin61i);
        double _mul62r = 0, _mul62i = 0;
        c_mul(_c60r, _c60i, _sin61r, _sin61i, &_mul62r, &_mul62i);
        double _add63r = 0, _add63i = 0;
        _add63r = _cos59r + _mul62r; _add63i = _cos59i + _mul62i;
        double _mul64r = 0, _mul64i = 0;
        c_mul(mag, 0, _add63r, _add63i, &_mul64r, &_mul64i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul64r; cIm[_idx] = _mul64i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_317_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs8r + j; _add9i = _abs8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log10r, _log10i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_mul(j, 0, j, 0, &_pow14r, &_pow14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_pow14r, _pow14i, &_cos15r, &_cos15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _cos15r; _add16i = _mul12i + _cos15i;
        double mag_part = _add16r; /* +_add16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _cos18r = 0, _cos18i = 0;
        c_cos(j, 0, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang17r, _ang17i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _sin21r = 0, _sin21i = 0;
        c_sin(j, 0, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang20r, _ang20i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul19r + _mul22r; _add23i = _mul19i + _mul22i;
        double angle_part = _add23r; /* +_add23ii */
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, angle_part, 0, &_mul25r, &_mul25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag_part, 0, _exp26r, _exp26i, &_mul27r, &_mul27i);
        double _conj28r = 0, _conj28i = 0;
        _conj28r = x1r; _conj28i = -(x1i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_conj28r, _conj28i, 0, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = j + _c30r; _add31i = 0 + _c30i;
        double _div32r = 0, _div32i = 0;
        c_div(_mul29r, _mul29i, _add31r, _add31i, &_div32r, &_div32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul27r + _div32r; _add33i = _mul27i + _div32i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    }
    double _c34r = 0, _c34i = 0;
    _c34r = 1.0; _c34i = 0;
    double _add35r = 0, _add35i = 0;
    _add35r = n + _c34r; _add35i = 0 + _c34i;
    for (int k = 1; k < (int)(_add35r); k++) {
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 0.05; _c37i = 0;
        double _cos38r = 0, _cos38i = 0;
        c_cos(k, 0, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c37r, _c37i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _c36r + _mul39r; _add40i = _c36i + _mul39i;
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 0.03;
        double _sin42r = 0, _sin42i = 0;
        c_sin(k, 0, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c41r, _c41i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _add40r + _mul43r; _add44i = _add40i + _mul43i;
        { double _tr = cRe[(k - 1)]*_add44r - cIm[(k - 1)]*_add44i; cIm[(k - 1)] = cRe[(k - 1)]*_add44i + cIm[(k - 1)]*_add44r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_318_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double rec1 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double imc1 = _im3r; /* +_im3ii */
    double _re4r = 0, _re4i = 0;
    _re4r = x2r; _re4i = 0;
    double rec2 = _re4r; /* +_re4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc2 = _im5r; /* +_im5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, rec1, 0, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, imc2, 0, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _sin9r + _cos11r; _add12i = _sin9i + _cos11i;
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x2r, x2i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang13r, _ang13i, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 0.1; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = j + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_mul15r, _mul15i, _add17r, _add17i, &_div18r, &_div18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _add12r + _div18r; _add19i = _add12i + _div18i;
        double angle = _add19r; /* +_add19ii */
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_abs20r, _abs20i, j, &_pow21r, &_pow21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs22r + j; _add23i = _abs22i + 0;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_pow21r, _pow21i, _log24r, _log24i, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _neg28r = 0, _neg28i = 0;
        _neg28r = -(_c27r); _neg28i = -(_c27i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(_neg28r, _neg28i, j, &_pow29r, &_pow29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 0.5; _c30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_pow29r, _pow29i, _c30r, _c30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _c26r + _mul31r; _add32i = _c26i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_mul25r, _mul25i, _add32r, _add32i, &_mul33r, &_mul33i);
        double magnitude = _mul33r; /* +_mul33ii */
        double _cos34r = 0, _cos34i = 0;
        c_cos(angle, 0, &_cos34r, &_cos34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _sin36r = 0, _sin36i = 0;
        c_sin(angle, 0, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c35r, _c35i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cos34r + _mul37r; _add38i = _cos34i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_319_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re4r, _re4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _re9r = 0, _re9i = 0;
        _re9r = x2r; _re9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_re9r, _re9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul8r + _mul14r; _add15i = _mul8i + _mul14i;
        double r = _add15r; /* +_add15ii */
        double _im16r = 0, _im16i = 0;
        _im16r = x1i; _im16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_im16r, _im16i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _im22r = 0, _im22i = 0;
        _im22r = x2i; _im22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = j + _c23r; _add24i = 0 + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_im22r, _im22i, _log25r, _log25i, &_mul26r, &_mul26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _mul21r - _mul26r; _sub27i = _mul21i - _mul26i;
        double q = _sub27r; /* +_sub27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _add29r = 0, _add29i = 0;
        _add29r = r + _c28r; _add29i = 0 + _c28i;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(_add29r, _add29i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _abs30r + _c31r; _add32i = _abs30i + _c31i;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 4.0; _c35i = 0;
        double _mod36r = 0, _mod36i = 0;
        _mod36r = fmod(j, _c35r); _mod36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _c34r + _mod36r; _add37i = _c34i + _mod36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_log33r, _log33i, _add37r, _add37i, &_mul38r, &_mul38i);
        double magnitude = _mul38r; /* +_mul38ii */
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(q, 0); _ang39i = 0;
        double _sin40r = 0, _sin40i = 0;
        c_sin(j, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang39r, _ang39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x2r, x2i); _ang42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 3.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(j, 0, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang42r, _ang42i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _mul41r + _mul46r; _add47i = _mul41i + _mul46i;
        double angle = _add47r; /* +_add47ii */
        double _cos48r = 0, _cos48i = 0;
        c_cos(angle, 0, &_cos48r, &_cos48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _sin50r = 0, _sin50i = 0;
        c_sin(angle, 0, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c49r, _c49i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _cos48r + _mul51r; _add52i = _cos48i + _mul51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(magnitude, 0, _add52r, _add52i, &_mul53r, &_mul53i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_320_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double _mul7r = 0, _mul7i = 0;
        c_mul(_arr6r, _arr6i, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        double _div10r = 0, _div10i = 0;
        c_div(_mul7r, _mul7i, _add9r, _add9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double mag_part1 = _sin11r; /* +_sin11ii */
        double _arr12r = 0, _arr12i = 0;
        { int _idx = j; _arr12r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr12i = 0; }
        double _mul13r = 0, _mul13i = 0;
        c_mul(_arr12r, _arr12i, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _div16r = 0, _div16i = 0;
        c_div(_mul13r, _mul13i, _add15r, _add15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double mag_part2 = _cos17r; /* +_cos17ii */
        double _add18r = 0, _add18i = 0;
        _add18r = mag_part1 + mag_part2; _add18i = 0 + 0;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x2r, x2i); _abs20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _abs20r; _add21i = _abs19i + _abs20i;
        double _add22r = 0, _add22i = 0;
        _add22r = _add21r + j; _add22i = _add21i + 0;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_add18r, _add18i, _log23r, _log23i, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, n, 0, &_div26r, &_div26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _c25r + _div26r; _add27i = _c25i + _div26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul24r, _mul24i, _add27r, _add27i, &_mul28r, &_mul28i);
        double mag = _mul28r; /* +_mul28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _arr30r = 0, _arr30i = 0;
        { int _idx = j; _arr30r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr30i = 0; }
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _arr30r, _arr30i, &_mul31r, &_mul31i);
        double _div32r = 0, _div32i = 0;
        c_div(_mul31r, _mul31i, n, 0, &_div32r, &_div32i);
        double ang_part1 = _div32r; /* +_div32ii */
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _arr34r = 0, _arr34i = 0;
        { int _idx = j; _arr34r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr34i = 0; }
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang33r, _ang33i, _arr34r, _arr34i, &_mul35r, &_mul35i);
        double _div36r = 0, _div36i = 0;
        c_div(_mul35r, _mul35i, n, 0, &_div36r, &_div36i);
        double ang_part2 = _div36r; /* +_div36ii */
        double _sub37r = 0, _sub37i = 0;
        _sub37r = ang_part1 - ang_part2; _sub37i = 0 - 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 5.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _sub37r + _sin41r; _add42i = _sub37i + _sin41i;
        double angle = _add42r; /* +_add42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, angle, 0, &_mul44r, &_mul44i);
        double _exp45r = 0, _exp45i = 0;
        c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(mag, 0, _exp45r, _exp45i, &_mul46r, &_mul46i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_321_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 1.8; _c6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(j, 0, 1.8, &_pow7r, &_pow7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _abs9r; _add10i = _abs8i + _abs9i;
        double _add11r = 0, _add11i = 0;
        _add11r = _add10r + j; _add11i = _add10i + 0;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _pow7r + _log12r; _add13i = _pow7i + _log12i;
        double _re14r = 0, _re14i = 0;
        _re14r = x1r; _re14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _re14r, _re14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _im17r = 0, _im17i = 0;
        _im17r = x2i; _im17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _im17r, _im17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _sin16r + _cos19r; _add20i = _sin16i + _cos19i;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(_add20r, _add20i); _abs21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_add13r, _add13i, _abs21r, _abs21i, &_mul22r, &_mul22i);
        double mag_part = _mul22r; /* +_mul22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang23r, _ang23i, _log26r, _log26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul27r + _mul32r; _add33i = _mul27i + _mul32i;
        double angle_part = _add33r; /* +_add33ii */
        double _cos34r = 0, _cos34i = 0;
        c_cos(angle_part, 0, &_cos34r, &_cos34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _sin36r = 0, _sin36i = 0;
        c_sin(angle_part, 0, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c35r, _c35i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cos34r + _mul37r; _add38i = _cos34i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag_part, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_322_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double r = _arr6r; /* +_arr6ii */
        double _arr7r = 0, _arr7i = 0;
        { int _idx = j; _arr7r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr7i = 0; }
        double d = _arr7r; /* +_arr7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 1.0;
        double _add9r = 0, _add9i = 0;
        _add9r = r + _c8r; _add9i = 0 + _c8i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_add9r, _add9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin18r, _sin18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _c14r + _mul23r; _add24i = _c14i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log13r, _log13i, _add24r, _add24i, &_mul25r, &_mul25i);
        double mag = _mul25r; /* +_mul25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(d, 0); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 4.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang26r, _ang26i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang32r, _ang32i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul31r + _mul37r; _add38i = _mul31i + _mul37i;
        double angle = _add38r; /* +_add38ii */
        double _cos39r = 0, _cos39i = 0;
        c_cos(angle, 0, &_cos39r, &_cos39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(angle, 0, &_sin40r, &_sin40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_sin40r, _sin40i, _c41r, _c41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cos39r + _mul42r; _add43i = _cos39i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_323_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _re2r = 0, _re2i = 0;
        _re2r = x1r; _re2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(j, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_re2r, _re2i, _add6r, &_pow7r, &_pow7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs8r + j; _add9i = _abs8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_pow7r, _pow7i, _log10r, _log10i, &_mul11r, &_mul11i);
        double _im12r = 0, _im12i = 0;
        _im12r = x1i; _im12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 7.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_im12r, _im12i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul11r + _mul17r; _add18i = _mul11i + _mul17i;
        double mag = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(j, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang19r, _ang19i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 4.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul23r + _mul28r; _add29i = _mul23i + _mul28i;
        double angle = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, angle, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(mag, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_324_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double r = _arr6r; /* +_arr6ii */
        double _arr7r = 0, _arr7i = 0;
        { int _idx = j; _arr7r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr7i = 0; }
        double d = _arr7r; /* +_arr7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(r, 0, r, 0, &_pow9r, &_pow9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_pow9r, _pow9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c15r, _c15i, M_PI, 0, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul16r, _mul16i, r, 0, &_mul17r, &_mul17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul17r, _mul17i, j, 0, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _c14r + _sin19r; _add20i = _c14i + _sin19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log13r, _log13i, _add20r, _add20i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(M_PI, 0, _c23r, _c23i, &_mul24r, &_mul24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul24r, _mul24i, j, 0, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _c22r + _cos26r; _add27i = _c22i + _cos26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul21r, _mul21i, _add27r, _add27i, &_mul28r, &_mul28i);
        double mag = _mul28r; /* +_mul28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(d, 0, _c29r, _c29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = r + _mul30r; _add31i = 0 + _mul30i;
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(_add31r, _add31i); _ang32i = 0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(j, 0, &_sin33r, &_sin33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _add35r = 0, _add35i = 0;
        _add35r = r + _c34r; _add35i = 0 + _c34i;
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(_add35r, _add35i); _abs36i = 0;
        double _log37r = 0, _log37i = 0;
        c_log(_abs36r, _abs36i, &_log37r, &_log37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_sin33r, _sin33i, _log37r, _log37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _ang32r + _mul38r; _add39i = _ang32i + _mul38i;
        double _cos40r = 0, _cos40i = 0;
        c_cos(j, 0, &_cos40r, &_cos40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _sub42r = 0, _sub42i = 0;
        _sub42r = r - _c41r; _sub42i = 0 - _c41i;
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(_sub42r, _sub42i); _ang43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_cos40r, _cos40i, _ang43r, _ang43i, &_mul44r, &_mul44i);
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _add39r - _mul44r; _sub45i = _add39i - _mul44i;
        double ang = _sub45r; /* +_sub45ii */
        double _cos46r = 0, _cos46i = 0;
        c_cos(ang, 0, &_cos46r, &_cos46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(ang, 0, &_sin47r, &_sin47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_sin47r, _sin47i, _c48r, _c48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _cos46r + _mul49r; _add50i = _cos46i + _mul49i;
        double _mul51r = 0, _mul51i = 0;
        c_mul(mag, 0, _add50r, _add50i, &_mul51r, &_mul51i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_325: too complex for auto-transpile, stubbed */
static void poly_325_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_326_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 7.0; _c6i = 0;
        double _mod7r = 0, _mod7i = 0;
        _mod7r = fmod(j, _c6r); _mod7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _mod7r + _c8r; _add9i = _mod7i + _c8i;
        double k = _add9r; /* +_add9ii */
        double _add10r = 0, _add10i = 0;
        _add10r = j + k; _add10i = 0 + 0;
        double r = _add10r; /* +_add10ii */
        double _arr11r = 0, _arr11i = 0;
        { int _idx = j; _arr11r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr11i = 0; }
        double _arr12r = 0, _arr12i = 0;
        { int _idx = j; _arr12r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr12i = 0; }
        double _mul13r = 0, _mul13i = 0;
        c_mul(_arr11r, _arr11i, _arr12r, _arr12i, &_mul13r, &_mul13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_mul13r, _mul13i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 3.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log17r, _log17i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 4.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _add25r = 0, _add25i = 0;
        _add25r = x1r + x2r; _add25i = x1i + x2i;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_add25r, _add25i); _abs26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_cos24r, _cos24i, _abs26r, _abs26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul21r + _mul27r; _add28i = _mul21i + _mul27i;
        double mag = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 6.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang29r, _ang29i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 8.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang35r, _ang35i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul34r + _mul40r; _add41i = _mul34i + _mul40i;
        double _sin42r = 0, _sin42i = 0;
        c_sin(j, 0, &_sin42r, &_sin42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add41r + _sin42r; _add43i = _add41i + _sin42i;
        double angle = _add43r; /* +_add43ii */
        double _cos44r = 0, _cos44i = 0;
        c_cos(angle, 0, &_cos44r, &_cos44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _sin46r = 0, _sin46i = 0;
        c_sin(angle, 0, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c45r, _c45i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _cos44r + _mul47r; _add48i = _cos44i + _mul47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(mag, 0, _add48r, _add48i, &_mul49r, &_mul49i);
        double _conj50r = 0, _conj50i = 0;
        _conj50r = x1r; _conj50i = -(x1i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(j, 0, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_conj50r, _conj50i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul49r + _mul52r; _add53i = _mul49i + _mul52i;
        double _conj54r = 0, _conj54i = 0;
        _conj54r = x2r; _conj54i = -(x2i);
        double _cos55r = 0, _cos55i = 0;
        c_cos(j, 0, &_cos55r, &_cos55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_conj54r, _conj54i, _cos55r, _cos55i, &_mul56r, &_mul56i);
        double _sub57r = 0, _sub57i = 0;
        _sub57r = _add53r - _mul56r; _sub57i = _add53i - _mul56i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub57r; cIm[_idx] = _sub57i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_327_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 4.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _re10r = 0, _re10i = 0;
        _re10r = x1r; _re10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 0.5; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_re10r, _re10i, 0.5, &_pow12r, &_pow12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin9r, _sin9i, _pow12r, _pow12i, &_mul13r, &_mul13i);
        double part1 = _mul13r; /* +_mul13ii */
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _im18r = 0, _im18i = 0;
        _im18r = x2i; _im18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.3; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_im18r, _im18i, 0.3, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_cos17r, _cos17i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double part2 = _mul21r; /* +_mul21ii */
        double _arr22r = 0, _arr22i = 0;
        { int _idx = j; _arr22r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr22i = 0; }
        double _arr23r = 0, _arr23i = 0;
        { int _idx = j; _arr23r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr23i = 0; }
        double _mul24r = 0, _mul24i = 0;
        c_mul(_arr22r, _arr22i, _arr23r, _arr23i, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_mul24r, _mul24i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double part3 = _log28r; /* +_log28ii */
        double _add29r = 0, _add29i = 0;
        _add29r = part1 + part2; _add29i = 0 + 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _add29r + part3; _add30i = _add29i + 0;
        double magnitude = _add30r; /* +_add30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(j, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _cos35r = 0, _cos35i = 0;
        c_cos(j, 0, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang34r, _ang34i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul33r + _mul36r; _add37i = _mul33i + _mul36i;
        double angle = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, angle, 0, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_328_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 5.0; _c6i = 0;
        double _mod7r = 0, _mod7i = 0;
        _mod7r = fmod(j, _c6r); _mod7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _mod7r + _c8r; _add9i = _mod7i + _c8i;
        double k = _add9r; /* +_add9ii */
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_mul(j, 0, j, 0, &_pow11r, &_pow11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(j, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin12r, _sin12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _pow11r + _mul14r; _add15i = _pow11i + _mul14i;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _log19r + _c20r; _add21i = _log19i + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_add15r, _add15i, _add21r, _add21i, &_div22r, &_div22i);
        double r = _div22r; /* +_div22ii */
        double mag_variation = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(n, 0, _c23r, _c23i, &_div24r, &_div24i);
        if (j <= _div24r) {
            double _c25r = 0, _c25i = 0;
            _c25r = 1.0; _c25i = 0;
            double _mul26r = 0, _mul26i = 0;
            c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
            double _c27r = 0, _c27i = 0;
            _c27r = 7.0; _c27i = 0;
            double _div28r = 0, _div28i = 0;
            c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
            double _sin29r = 0, _sin29i = 0;
            c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
            double _add30r = 0, _add30i = 0;
            _add30r = _c25r + _sin29r; _add30i = _c25i + _sin29i;
            double _mul31r = 0, _mul31i = 0;
            c_mul(r, 0, _add30r, _add30i, &_mul31r, &_mul31i);
            mag_variation = _mul31r;
        } else {
            double _c32r = 0, _c32i = 0;
            _c32r = 1.0; _c32i = 0;
            double _mul33r = 0, _mul33i = 0;
            c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
            double _c34r = 0, _c34i = 0;
            _c34r = 5.0; _c34i = 0;
            double _div35r = 0, _div35i = 0;
            c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
            double _cos36r = 0, _cos36i = 0;
            c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
            double _add37r = 0, _add37i = 0;
            _add37r = _c32r + _cos36r; _add37i = _c32i + _cos36i;
            double _mul38r = 0, _mul38i = 0;
            c_mul(r, 0, _add37r, _add37i, &_mul38r, &_mul38i);
            mag_variation = _mul38r;
        }
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _sin40r = 0, _sin40i = 0;
        c_sin(j, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang39r, _ang39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x2r, x2i); _ang42i = 0;
        double _cos43r = 0, _cos43i = 0;
        c_cos(j, 0, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_ang42r, _ang42i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul41r + _mul44r; _add45i = _mul41i + _mul44i;
        double _c46r = 0, _c46i = 0;
        _c46r = 1.0; _c46i = 0;
        double _add47r = 0, _add47i = 0;
        _add47r = mag_variation + _c46r; _add47i = 0 + _c46i;
        double _log48r = 0, _log48i = 0;
        c_log(_add47r, _add47i, &_log48r, &_log48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _add45r + _log48r; _add49i = _add45i + _log48i;
        double angle_variation = _add49r; /* +_add49ii */
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c50r, _c50i, angle_variation, 0, &_mul51r, &_mul51i);
        double _exp52r = 0, _exp52i = 0;
        c_exp2(_mul51r, _mul51i, &_exp52r, &_exp52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(mag_variation, 0, _exp52r, _exp52i, &_mul53r, &_mul53i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_329_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double _arr7r = 0, _arr7i = 0;
        { int _idx = j; _arr7r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr7i = 0; }
        double _add8r = 0, _add8i = 0;
        _add8r = _arr6r + _arr7r; _add8i = _arr6i + _arr7i;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double mag_part1 = _log12r; /* +_log12ii */
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 4.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 3.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_sin16r, _sin16i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double mag_part2 = _mul21r; /* +_mul21ii */
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _c22r + mag_part2; _add23i = _c22i + 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(mag_part1, 0, _add23r, _add23i, &_mul24r, &_mul24i);
        double magnitude = _mul24r; /* +_mul24ii */
        double _conj25r = 0, _conj25i = 0;
        _conj25r = x1r; _conj25i = -(x1i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_conj25r, _conj25i, x2r, x2i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(_mul26r, _mul26i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 5.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _ang27r + _sin31r; _add32i = _ang27i + _sin31i;
        double angle_part1 = _add32r; /* +_add32ii */
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 7.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _arr37r = 0, _arr37i = 0;
        { int _idx = j; _arr37r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr37i = 0; }
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _arr39r = 0, _arr39i = 0;
        { int _idx = j; _arr39r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr39i = 0; }
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c38r, _c38i, _arr39r, _arr39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _arr37r + _mul40r; _add41i = _arr37i + _mul40i;
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(_add41r, _add41i); _ang42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_cos36r, _cos36i, _ang42r, _ang42i, &_mul43r, &_mul43i);
        double angle_part2 = _mul43r; /* +_mul43ii */
        double _add44r = 0, _add44i = 0;
        _add44r = angle_part1 + angle_part2; _add44i = 0 + 0;
        double angle = _add44r; /* +_add44ii */
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, angle, 0, &_mul46r, &_mul46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(magnitude, 0, _exp47r, _exp47i, &_mul48r, &_mul48i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_330_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr6i = 0; }
        double rec = _arr6r; /* +_arr6ii */
        double _arr7r = 0, _arr7i = 0;
        { int _idx = j; _arr7r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr7i = 0; }
        double imc = _arr7r; /* +_arr7ii */
        double _mul8r = 0, _mul8i = 0;
        c_mul(rec, 0, j, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_mul(j, 0, j, 0, &_pow10r, &_pow10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(imc, 0, _pow10r, _pow10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _mul11r; _add12i = _mul8i + _mul11i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_add12r, _add12i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log16r, _log16i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 3.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = rec - imc; _sub26i = 0 - 0;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_sub26r, _sub26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _abs27r + _c28r; _add29i = _abs27i + _c28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_cos25r, _cos25i, _add29r, _add29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul21r + _mul30r; _add31i = _mul21i + _mul30i;
        double mag = _add31r; /* +_add31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang32r, _ang32i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, M_PI, 0, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 8.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_mul39r, _mul39i, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang38r, _ang38i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul37r + _mul43r; _add44i = _mul37i + _mul43i;
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = j + _c45r; _add46i = 0 + _c45i;
        double _log47r = 0, _log47i = 0;
        c_log(_add46r, _add46i, &_log47r, &_log47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _add44r + _log47r; _add48i = _add44i + _log47i;
        double ang = _add48r; /* +_add48ii */
        double _cos49r = 0, _cos49i = 0;
        c_cos(ang, 0, &_cos49r, &_cos49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(ang, 0, &_sin50r, &_sin50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(_sin50r, _sin50i, _c51r, _c51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _cos49r + _mul52r; _add53i = _cos49i + _mul52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(mag, 0, _add53r, _add53i, &_mul54r, &_mul54i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_331_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs2r + j; _add3i = _abs2i + 0;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double mag_part1 = _log4r; /* +_log4ii */
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs5r + n; _add6i = _abs5i + 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _add6r - j; _sub7i = _add6i - 0;
        double _log8r = 0, _log8i = 0;
        c_log(_sub7r, _sub7i, &_log8r, &_log8i);
        double mag_part2 = _log8r; /* +_log8ii */
        double _mul9r = 0, _mul9i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(j, 0, _c11r, _c11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin10r, _sin10i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul9r + _mul14r; _add15i = _mul9i + _mul14i;
        double mag = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(j, 0, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang16r, _ang16i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double angle_part1 = _mul20r; /* +_mul20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 4.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double angle_part2 = _mul25r; /* +_mul25ii */
        double _add26r = 0, _add26i = 0;
        _add26r = angle_part1 + angle_part2; _add26i = 0 + 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 7.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add26r + _sin30r; _add31i = _add26i + _sin30i;
        double angle = _add31r; /* +_add31ii */
        double _cos32r = 0, _cos32i = 0;
        c_cos(angle, 0, &_cos32r, &_cos32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(angle, 0, &_sin33r, &_sin33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sin33r, _sin33i, _c34r, _c34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _cos32r + _mul35r; _add36i = _cos32i + _mul35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(mag, 0, _add36r, _add36i, &_mul37r, &_mul37i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_332: too complex for auto-transpile, stubbed */
static void poly_332_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_333_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _im2r = 0, _im2i = 0;
        _im2r = x1i; _im2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(j, 0, _im2r, _im2i, &_mul3r, &_mul3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_mul3r, _mul3i, &_sin4r, &_sin4i);
        double _re5r = 0, _re5i = 0;
        _re5r = x2r; _re5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _re5r, _re5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _sin4r + _cos7r; _add8i = _sin4i + _cos7i;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x2r, x2i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_ang9r, _ang9i, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _div12r = 0, _div12i = 0;
        c_div(_mul11r, _mul11i, j, 0, &_div12r, &_div12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _add8r + _div12r; _add13i = _add8i + _div12i;
        double angle = _add13r; /* +_add13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.5; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(j, 0, 1.5, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log17r, _log17i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double _neg21r = 0, _neg21i = 0;
        _neg21r = -(j); _neg21i = -(0);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _div25r = 0, _div25i = 0;
        c_div(_neg21r, _neg21i, _add24r, _add24i, &_div25r, &_div25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_div25r, _div25i, &_exp26r, &_exp26i);
        double _sqrt27r = 0, _sqrt27i = 0;
        c_powr(j, 0, 0.5, &_sqrt27r, &_sqrt27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_exp26r, _exp26i, _sqrt27r, _sqrt27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul20r + _mul28r; _add29i = _mul20i + _mul28i;
        double magnitude = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, angle, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(magnitude, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _neg35r = 0, _neg35i = 0;
        _neg35r = -(_c34r); _neg35i = -(_c34i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_neg35r, _neg35i, angle, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = j + _c37r; _add38i = 0 + _c37i;
        double _div39r = 0, _div39i = 0;
        c_div(_mul36r, _mul36i, _add38r, _add38i, &_div39r, &_div39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_div39r, _div39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
        double _conj42r = 0, _conj42i = 0;
        _conj42r = _mul41r; _conj42i = -(_mul41i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul33r + _conj42r; _add43i = _mul33i + _conj42i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_334_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 5.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _div12r + _ang13r; _add14i = _div12i + _ang13i;
        double _cos15r = 0, _cos15i = 0;
        c_cos(_add14r, _add14i, &_cos15r, &_cos15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _sin9r + _cos15r; _add16i = _sin9i + _cos15i;
        double angle_part = _add16r; /* +_add16ii */
        double _arr17r = 0, _arr17i = 0;
        { int _idx = j; _arr17r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr17i = 0; }
        double _arr18r = 0, _arr18i = 0;
        { int _idx = j; _arr18r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr18i = 0; }
        double _add19r = 0, _add19i = 0;
        _add19r = _arr17r + _arr18r; _add19i = _arr17i + _arr18i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_add19r, _add19i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _abs25r; _add26i = _abs24i + _abs25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log23r, _log23i, _add26r, _add26i, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_mul27r, _mul27i, _add29r, _add29i, &_div30r, &_div30i);
        double magnitude_part = _div30r; /* +_div30ii */
        double _arr31r = 0, _arr31i = 0;
        { int _idx = j; _arr31r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr31i = 0; }
        double _c32r = 0, _c32i = 0;
        _c32r = 3.0; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_mul(_arr31r, _arr31i, _arr31r, _arr31i, &_pow33r, &_pow33i);
        c_mul(_pow33r, _pow33i, _arr31r, _arr31i, &_pow33r, &_pow33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 2.0; _c34i = 0;
        double _arr35r = 0, _arr35i = 0;
        { int _idx = j; _arr35r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr35i = 0; }
        double _c36r = 0, _c36i = 0;
        _c36r = 2.0; _c36i = 0;
        double _pow37r = 0, _pow37i = 0;
        c_mul(_arr35r, _arr35i, _arr35r, _arr35i, &_pow37r, &_pow37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c34r, _c34i, _pow37r, _pow37i, &_mul38r, &_mul38i);
        double _sub39r = 0, _sub39i = 0;
        _sub39r = _pow33r - _mul38r; _sub39i = _pow33i - _mul38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, M_PI, 0, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 3.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(_mul40r, _mul40i, _c41r, _c41i, &_div42r, &_div42i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(_div42r, _div42i, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_sub39r, _sub39i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double intricate_term = _mul44r; /* +_mul44ii */
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, angle_part, 0, &_mul46r, &_mul46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(magnitude_part, 0, _exp47r, _exp47i, &_mul48r, &_mul48i);
        double _conj49r = 0, _conj49i = 0;
        _conj49r = x1r; _conj49i = -(x1i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_conj49r, _conj49i, intricate_term, 0, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul48r + _mul50r; _add51i = _mul48i + _mul50i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_335_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double rec_t1 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double imc_t1 = _im3r; /* +_im3ii */
    double _re4r = 0, _re4i = 0;
    _re4r = x2r; _re4i = 0;
    double rec_t2 = _re4r; /* +_re4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_t2 = _im5r; /* +_im5ii */
    for (int j = 0; j < (int)(n); j++) {
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 7.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 5.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin9r, _sin9i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _ang16r, _ang16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul14r + _mul17r; _add18i = _mul14i + _mul17i;
        double angle_part = _add18r; /* +_add18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(j, 0, j, 0, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log22r, _log22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _c26r + j; _add27i = _c26i + 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul25r, _mul25i, _add27r, _add27i, &_div28r, &_div28i);
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x2r, x2i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(j, 0, &_sin31r, &_sin31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _c30r + _sin31r; _add32i = _c30i + _sin31i;
        double _pow33r = 0, _pow33i = 0;
        c_powr(_abs29r, _abs29i, _add32r, &_pow33r, &_pow33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _div28r + _pow33r; _add34i = _div28i + _pow33i;
        double magnitude_part = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _im36r = 0, _im36i = 0;
        _im36r = x1i; _im36i = 0;
        double _re37r = 0, _re37i = 0;
        _re37r = x2r; _re37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_im36r, _im36i, _re37r, _re37i, &_mul38r, &_mul38i);
        double _div39r = 0, _div39i = 0;
        c_div(_mul38r, _mul38i, j, 0, &_div39r, &_div39i);
        double _add40r = 0, _add40i = 0;
        _add40r = angle_part + _div39r; _add40i = 0 + _div39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c35r, _c35i, _add40r, _add40i, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double phase_shift = _exp42r; /* +_exp42ii */
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude_part, 0, phase_shift, 0, &_mul43r, &_mul43i);
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x1r; _conj44i = -(x1i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x2r; _conj45i = -(x2i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_conj44r, _conj44i, _conj45r, _conj45i, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _add48r = 0, _add48i = 0;
        _add48r = j + _c47r; _add48i = 0 + _c47i;
        double _div49r = 0, _div49i = 0;
        c_div(_mul46r, _mul46i, _add48r, _add48i, &_div49r, &_div49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul43r + _div49r; _add50i = _mul43i + _div49i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    for (int k = 0; k < (int)(n); k++) {
        double _c51r = 0, _c51i = 0;
        _c51r = 5.0; _c51i = 0;
        double _mod52r = 0, _mod52i = 0;
        _mod52r = fmod(k, _c51r); _mod52i = 0;
        double _c53r = 0, _c53i = 0;
        _c53r = 0.0; _c53i = 0;
        if (_mod52r == _c53r) {
            double _c54r = 0, _c54i = 0;
            _c54r = 1.0; _c54i = 0;
            double _c55r = 0, _c55i = 0;
            _c55r = 0.5; _c55i = 0;
            double _mul56r = 0, _mul56i = 0;
            c_mul(k, 0, M_PI, 0, &_mul56r, &_mul56i);
            double _c57r = 0, _c57i = 0;
            _c57r = 3.0; _c57i = 0;
            double _div58r = 0, _div58i = 0;
            c_div(_mul56r, _mul56i, _c57r, _c57i, &_div58r, &_div58i);
            double _cos59r = 0, _cos59i = 0;
            c_cos(_div58r, _div58i, &_cos59r, &_cos59i);
            double _mul60r = 0, _mul60i = 0;
            c_mul(_c55r, _c55i, _cos59r, _cos59i, &_mul60r, &_mul60i);
            double _add61r = 0, _add61i = 0;
            _add61r = _c54r + _mul60r; _add61i = _c54i + _mul60i;
            { double _tr = cRe[k]*_add61r - cIm[k]*_add61i; cIm[k] = cRe[k]*_add61i + cIm[k]*_add61r; cRe[k] = _tr; }
        } else {
            double _c62r = 0, _c62i = 0;
            _c62r = 3.0; _c62i = 0;
            double _mod63r = 0, _mod63i = 0;
            _mod63r = fmod(k, _c62r); _mod63i = 0;
            double _c64r = 0, _c64i = 0;
            _c64r = 0.0; _c64i = 0;
            if (_mod63r == _c64r) {
                double _c65r = 0, _c65i = 0;
                _c65r = 1.0; _c65i = 0;
                double _c66r = 0, _c66i = 0;
                _c66r = 0.3; _c66i = 0;
                double _mul67r = 0, _mul67i = 0;
                c_mul(k, 0, M_PI, 0, &_mul67r, &_mul67i);
                double _c68r = 0, _c68i = 0;
                _c68r = 4.0; _c68i = 0;
                double _div69r = 0, _div69i = 0;
                c_div(_mul67r, _mul67i, _c68r, _c68i, &_div69r, &_div69i);
                double _sin70r = 0, _sin70i = 0;
                c_sin(_div69r, _div69i, &_sin70r, &_sin70i);
                double _mul71r = 0, _mul71i = 0;
                c_mul(_c66r, _c66i, _sin70r, _sin70i, &_mul71r, &_mul71i);
                double _add72r = 0, _add72i = 0;
                _add72r = _c65r + _mul71r; _add72i = _c65i + _mul71i;
                { double _tr = cRe[k]*_add72r - cIm[k]*_add72i; cIm[k] = cRe[k]*_add72i + cIm[k]*_add72r; cRe[k] = _tr; }
            } else {
                double _c73r = 0, _c73i = 0;
                _c73r = 1.0; _c73i = 0;
                double _c74r = 0, _c74i = 0;
                _c74r = 0.2; _c74i = 0;
                double _c75r = 0, _c75i = 0;
                _c75r = 1.0; _c75i = 0;
                double _add76r = 0, _add76i = 0;
                _add76r = k + _c75r; _add76i = 0 + _c75i;
                double _log77r = 0, _log77i = 0;
                c_log(_add76r, _add76i, &_log77r, &_log77i);
                double _mul78r = 0, _mul78i = 0;
                c_mul(_c74r, _c74i, _log77r, _log77i, &_mul78r, &_mul78i);
                double _add79r = 0, _add79i = 0;
                _add79r = _c73r + _mul78r; _add79i = _c73i + _mul78i;
                { double _tr = cRe[k]*_add79r - cIm[k]*_add79i; cIm[k] = cRe[k]*_add79i + cIm[k]*_add79r; cRe[k] = _tr; }
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_336_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 7.0; _c2i = 0;
        double _mod3r = 0, _mod3i = 0;
        _mod3r = fmod(j, _c2r); _mod3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _mod3r + _c4r; _add5i = _mod3i + _c4i;
        double k = _add5r; /* +_add5ii */
        double _re6r = 0, _re6i = 0;
        _re6r = x1r; _re6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 6.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_re6r, _re6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _re12r = 0, _re12i = 0;
        _re12r = x2r; _re12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 5.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_re12r, _re12i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul11r + _mul17r; _add18i = _mul11i + _mul17i;
        double r = _add18r; /* +_add18ii */
        double _im19r = 0, _im19i = 0;
        _im19r = x1i; _im19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_im19r, _im19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _im25r = 0, _im25i = 0;
        _im25r = x2i; _im25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_im25r, _im25i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _mul24r - _mul30r; _sub31i = _mul24i - _mul30i;
        double s = _sub31r; /* +_sub31ii */
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x1r, x1i); _abs32i = 0;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(x2r, x2i); _abs33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _abs32r + _abs33r; _add34i = _abs32i + _abs33i;
        double _add35r = 0, _add35i = 0;
        _add35r = _add34r + j; _add35i = _add34i + 0;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(r, 0, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _c37r + _sin38r; _add39i = _c37i + _sin38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_log36r, _log36i, _add39r, _add39i, &_mul40r, &_mul40i);
        double _abs41r = 0, _abs41i = 0;
        _abs41r = c_abs(s, 0); _abs41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _mul40r + _abs41r; _add42i = _mul40i + _abs41i;
        double magnitude = _add42r; /* +_add42ii */
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x1r, x1i); _ang43i = 0;
        double _cos44r = 0, _cos44i = 0;
        c_cos(r, 0, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_ang43r, _ang43i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x2r, x2i); _ang46i = 0;
        double _sin47r = 0, _sin47i = 0;
        c_sin(s, 0, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang46r, _ang46i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul45r + _mul48r; _add49i = _mul45i + _mul48i;
        double angle = _add49r; /* +_add49ii */
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c50r, _c50i, angle, 0, &_mul51r, &_mul51i);
        double _exp52r = 0, _exp52i = 0;
        c_exp2(_mul51r, _mul51i, &_exp52r, &_exp52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(magnitude, 0, _exp52r, _exp52i, &_mul53r, &_mul53i);
        double _conj54r = 0, _conj54i = 0;
        _conj54r = x1r; _conj54i = -(x1i);
        double _pow55r = 0, _pow55i = 0;
        c_powr(_conj54r, _conj54i, k, &_pow55r, &_pow55i);
        double _conj56r = 0, _conj56i = 0;
        _conj56r = x2r; _conj56i = -(x2i);
        double _pow57r = 0, _pow57i = 0;
        c_powr(_conj56r, _conj56i, k, &_pow57r, &_pow57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_pow55r, _pow55i, _pow57r, _pow57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _mul53r + _mul58r; _add59i = _mul53i + _mul58i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_337_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < 35; j++) {
        double _re2r = 0, _re2i = 0;
        _re2r = x1r; _re2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(j, 0, _re2r, _re2i, &_mul3r, &_mul3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_mul3r, _mul3i, &_sin4r, &_sin4i);
        double _im5r = 0, _im5i = 0;
        _im5r = x2i; _im5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _im5r, _im5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _sin4r + _cos7r; _add8i = _sin4i + _cos7i;
        double _re9r = 0, _re9i = 0;
        _re9r = x1r; _re9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _re9r + _c10r; _add11i = _re9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log12r, _log12i, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _add8r + _mul14r; _add15i = _add8i + _mul14i;
        double phase = _add15r; /* +_add15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_abs16r, _abs16i, j, &_pow17r, &_pow17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 35.0; _c19i = 0;
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _c19r - j; _sub20i = _c19i - 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_abs18r, _abs18i, _sub20r, &_pow21r, &_pow21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _pow17r + _pow21r; _add22i = _pow17i + _pow21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 7.0; _c23i = 0;
        double _mod24r = 0, _mod24i = 0;
        _mod24r = fmod(j, _c23r); _mod24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _mod24r + _c25r; _add26i = _mod24i + _c25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_add22r, _add22i, _add26r, _add26i, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_mul27r, _mul27i, _add29r, _add29i, &_div30r, &_div30i);
        double magnitude = _div30r; /* +_div30ii */
        double _cos31r = 0, _cos31i = 0;
        c_cos(phase, 0, &_cos31r, &_cos31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(phase, 0, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c32r, _c32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(magnitude, 0, _add35r, _add35i, &_mul36r, &_mul36i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_338_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int k = 1; k < (int)(_add3r); k++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 5.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_div7r, _div7i, &_cos8r, &_cos8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_re4r, _re4i, _cos8r, _cos8i, &_mul9r, &_mul9i);
        double _re10r = 0, _re10i = 0;
        _re10r = x2r; _re10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(k, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 7.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_re10r, _re10i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul9r + _mul15r; _add16i = _mul9i + _mul15i;
        double real_part = _add16r; /* +_add16ii */
        double _im17r = 0, _im17i = 0;
        _im17r = x1i; _im17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 6.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_im17r, _im17i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _im23r = 0, _im23i = 0;
        _im23r = x2i; _im23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(k, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 8.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_im23r, _im23i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul22r - _mul28r; _sub29i = _mul22i - _mul28i;
        double imag_part = _sub29r; /* +_sub29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(real_part, 0, real_part, 0, &_pow31r, &_pow31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_mul(imag_part, 0, imag_part, 0, &_pow33r, &_pow33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _pow31r + _pow33r; _add34i = _pow31i + _pow33i;
        double _sqrt35r = 0, _sqrt35i = 0;
        c_powr(_add34r, _add34i, 0.5, &_sqrt35r, &_sqrt35i);
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(k, 0); _abs36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
        double _log39r = 0, _log39i = 0;
        c_log(_add38r, _add38i, &_log39r, &_log39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_sqrt35r, _sqrt35i, _log39r, _log39i, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _sin42r = 0, _sin42i = 0;
        c_sin(k, 0, &_sin42r, &_sin42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _c41r + _sin42r; _add43i = _c41i + _sin42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_mul40r, _mul40i, _add43r, _add43i, &_mul44r, &_mul44i);
        double magnitude = _mul44r; /* +_mul44ii */
        double _at245r = 0, _at245i = 0;
        _at245r = atan2(imag_part, real_part); _at245i = 0;
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x1r, x1i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(k, 0, _ang46r, _ang46i, &_mul47r, &_mul47i);
        double _sin48r = 0, _sin48i = 0;
        c_sin(_mul47r, _mul47i, &_sin48r, &_sin48i);
        double _ang49r = 0, _ang49i = 0;
        _ang49r = c_arg(x2r, x2i); _ang49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(k, 0, _ang49r, _ang49i, &_mul50r, &_mul50i);
        double _cos51r = 0, _cos51i = 0;
        c_cos(_mul50r, _mul50i, &_cos51r, &_cos51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_sin48r, _sin48i, _cos51r, _cos51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _at245r + _mul52r; _add53i = _at245i + _mul52i;
        double angle = _add53r; /* +_add53ii */
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 1.0;
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c54r, _c54i, angle, 0, &_mul55r, &_mul55i);
        double _exp56r = 0, _exp56i = 0;
        c_exp2(_mul55r, _mul55i, &_exp56r, &_exp56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(magnitude, 0, _exp56r, _exp56i, &_mul57r, &_mul57i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    }
    double _c58r = 0, _c58i = 0;
    _c58r = 1.0; _c58i = 0;
    double _add59r = 0, _add59i = 0;
    _add59r = n + _c58r; _add59i = 0 + _c58i;
    for (int r = 1; r < (int)(_add59r); r++) {
        double _cf60r = 0, _cf60i = 0;
        { int _idx = (n - r); if (_idx >= 0 && _idx < 36) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
        double _conj61r = 0, _conj61i = 0;
        _conj61r = _cf60r; _conj61i = -(_cf60i);
        double _mul62r = 0, _mul62i = 0;
        c_mul(r, 0, M_PI, 0, &_mul62r, &_mul62i);
        double _c63r = 0, _c63i = 0;
        _c63r = 10.0; _c63i = 0;
        double _div64r = 0, _div64i = 0;
        c_div(_mul62r, _mul62i, _c63r, _c63i, &_div64r, &_div64i);
        double _sin65r = 0, _sin65i = 0;
        c_sin(_div64r, _div64i, &_sin65r, &_sin65i);
        double _mul66r = 0, _mul66i = 0;
        c_mul(_conj61r, _conj61i, _sin65r, _sin65i, &_mul66r, &_mul66i);
        cRe[(r - 1)] += _mul66r; cIm[(r - 1)] += _mul66i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_339: too complex for auto-transpile, stubbed */
static void poly_339_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_340_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs2r + j; _add3i = _abs2i + 0;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _re5r = 0, _re5i = 0;
        _re5r = x2r; _re5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _re5r, _re5i, &_mul6r, &_mul6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_sin7r, _sin7i); _abs8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log4r, _log4i, _abs8r, _abs8i, &_mul9r, &_mul9i);
        double _sqrt10r = 0, _sqrt10i = 0;
        c_powr(j, 0, 0.5, &_sqrt10r, &_sqrt10i);
        double _im11r = 0, _im11i = 0;
        _im11r = x1i; _im11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _im11r, _im11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_cos13r, _cos13i); _abs14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sqrt10r, _sqrt10i, _abs14r, _abs14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul9r + _mul15r; _add16i = _mul9i + _mul15i;
        double mag_part = _add16r; /* +_add16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang17r, _ang17i, j, 0, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(j, 0, &_sin19r, &_sin19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul18r + _sin19r; _add20i = _mul18i + _sin19i;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _add20r + _cos23r; _add24i = _add20i + _cos23i;
        double angle_part = _add24r; /* +_add24ii */
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c25r, _c25i, angle_part, 0, &_mul26r, &_mul26i);
        double _exp27r = 0, _exp27i = 0;
        c_exp2(_mul26r, _mul26i, &_exp27r, &_exp27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_part, 0, _exp27r, _exp27i, &_mul28r, &_mul28i);
        double _conj29r = 0, _conj29i = 0;
        _conj29r = x2r; _conj29i = -(x2i);
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _mod31r = 0, _mod31i = 0;
        _mod31r = fmod(j, _c30r); _mod31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _mod31r + _c32r; _add33i = _mod31i + _c32i;
        double _pow34r = 0, _pow34i = 0;
        c_powr(_conj29r, _conj29i, _add33r, &_pow34r, &_pow34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul28r + _pow34r; _add35i = _mul28i + _pow34i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_341_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_abs2r, _abs2i, _log5r, _log5i, &_mul6r, &_mul6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _sqrt8r = 0, _sqrt8i = 0;
        c_powr(j, 0, 0.5, &_sqrt8r, &_sqrt8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_abs7r, _abs7i, _sqrt8r, _sqrt8i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul6r + _mul9r; _add10i = _mul6i + _mul9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.3; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(j, 0, 1.3, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _c11r + _pow13r; _add14i = _c11i + _pow13i;
        double _div15r = 0, _div15i = 0;
        c_div(_add10r, _add10i, _add14r, _add14i, &_div15r, &_div15i);
        double mag = _div15r; /* +_div15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(j, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(j, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang19r, _ang19i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul18r + _mul23r; _add24i = _mul18i + _mul23i;
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_div26r, _div26i, M_PI, 0, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add24r + _sin28r; _add29i = _add24i + _sin28i;
        double angle = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _c31r = 0, _c31i = 0;
        _c31r = 4.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_div32r, _div32i, M_PI, 0, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 5.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, _c35r, _c35i, &_div36r, &_div36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_div36r, _div36i, M_PI, 0, &_mul37r, &_mul37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_mul37r, _mul37i, &_cos38r, &_cos38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _sin34r + _cos38r; _add39i = _sin34i + _cos38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c30r, _c30i, _add39r, _add39i, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double perturb = _exp41r; /* +_exp41ii */
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, angle, 0, &_mul43r, &_mul43i);
        double _exp44r = 0, _exp44i = 0;
        c_exp2(_mul43r, _mul43i, &_exp44r, &_exp44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag, 0, _exp44r, _exp44i, &_mul45r, &_mul45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_mul45r, _mul45i, perturb, 0, &_mul46r, &_mul46i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_342_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 1; j < 36; j++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 7.0; _c2i = 0;
        double _mod3r = 0, _mod3i = 0;
        _mod3r = fmod(j, _c2r); _mod3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _mod3r + _c4r; _add5i = _mod3i + _c4i;
        double k = _add5r; /* +_add5ii */
        double _re6r = 0, _re6i = 0;
        _re6r = x1r; _re6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_re6r, _re6i, _log9r, _log9i, &_mul10r, &_mul10i);
        double r = _mul10r; /* +_mul10ii */
        double _im11r = 0, _im11i = 0;
        _im11r = x2i; _im11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 5.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_im11r, _im11i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double s = _mul16r; /* +_mul16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang17r, _ang17i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(k, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 4.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul22r + _sin26r; _add27i = _mul22i + _sin26i;
        double theta = _add27r; /* +_add27ii */
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x1r, x1i); _abs28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_abs28r, _abs28i, k, &_pow29r, &_pow29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs30r + j; _add31i = _abs30i + 0;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _pow29r + _log32r; _add33i = _pow29i + _log32i;
        double magnitude = _add33r; /* +_add33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(s, 0, _c34r, _c34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = r + _mul35r; _add36i = 0 + _mul35i;
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, theta, 0, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_add36r, _add36i, _exp39r, _exp39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = x1r + x2r; _add41i = x1i + x2i;
        double _conj42r = 0, _conj42i = 0;
        _conj42r = _add41r; _conj42i = -(_add41i);
        double _pow43r = 0, _pow43i = 0;
        c_powr(_conj42r, _conj42i, k, &_pow43r, &_pow43i);
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x1r, x1i); _ang44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(j, 0, _ang44r, _ang44i, &_mul45r, &_mul45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(_mul45r, _mul45i, &_cos46r, &_cos46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_pow43r, _pow43i, _cos46r, _cos46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul40r + _mul47r; _add48i = _mul40i + _mul47i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_343_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double rec1 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double imc1 = _im3r; /* +_im3ii */
    double _re4r = 0, _re4i = 0;
    _re4r = x2r; _re4i = 0;
    double rec2 = _re4r; /* +_re4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc2 = _im5r; /* +_im5ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _mul8r = 0, _mul8i = 0;
        c_mul(rec1, 0, _add7r, _add7i, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _div11r = 0, _div11i = 0;
        c_div(imc2, 0, _add10r, _add10i, &_div11r, &_div11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _div11r; _add12i = _mul8i + _div11i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_add12r, _add12i); _abs13i = 0;
        double _log14r = 0, _log14i = 0;
        c_log(_abs13r, _abs13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = j + _c15r; _add16i = 0 + _c15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(rec1, 0, _add16r, _add16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _div21r = 0, _div21i = 0;
        c_div(imc2, 0, _add20r, _add20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin18r, _sin18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _log14r + _mul23r; _add24i = _log14i + _mul23i;
        double mag_part = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.5; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(j, 0, 0.5, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang25r, _ang25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _sqrt30r = 0, _sqrt30i = 0;
        c_powr(j, 0, 0.5, &_sqrt30r, &_sqrt30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _sqrt30r, _sqrt30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul28r + _mul31r; _add32i = _mul28i + _mul31i;
        double _re33r = 0, _re33i = 0;
        _re33r = x1r; _re33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _re33r, _re33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add32r + _sin35r; _add36i = _add32i + _sin35i;
        double _im37r = 0, _im37i = 0;
        _im37r = x2i; _im37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, _im37r, _im37i, &_mul38r, &_mul38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_mul38r, _mul38i, &_cos39r, &_cos39i);
        double _sub40r = 0, _sub40i = 0;
        _sub40r = _add36r - _cos39r; _sub40i = _add36i - _cos39i;
        double angle_part = _sub40r; /* +_sub40ii */
        double _cos41r = 0, _cos41i = 0;
        c_cos(angle_part, 0, &_cos41r, &_cos41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(angle_part, 0, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c42r, _c42i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos41r + _mul44r; _add45i = _cos41i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(mag_part, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_344_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr6i = 0; }
        double _arr7r = 0, _arr7i = 0;
        { int _idx = j; _arr7r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(_arr6r, _arr6i, _arr7r, _arr7i, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_mul8r, _mul8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _c13r + _sin17r; _add18i = _c13i + _sin17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log12r, _log12i, _add18r, _add18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 4.0; _c20i = 0;
        double _mod21r = 0, _mod21i = 0;
        _mod21r = fmod(j, _c20r); _mod21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _mod21r + _c22r; _add23i = _mod21i + _c22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul19r, _mul19i, _add23r, _add23i, &_mul24r, &_mul24i);
        double mag = _mul24r; /* +_mul24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 5.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang25r, _ang25i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 7.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang31r, _ang31i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul30r + _mul36r; _add37i = _mul30i + _mul36i;
        double _arr38r = 0, _arr38i = 0;
        { int _idx = j; _arr38r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr38i = 0; }
        double _arr39r = 0, _arr39i = 0;
        { int _idx = j; _arr39r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr39i = 0; }
        double _add40r = 0, _add40i = 0;
        _add40r = _arr38r + _arr39r; _add40i = _arr38i + _arr39i;
        double _abs41r = 0, _abs41i = 0;
        _abs41r = c_abs(_add40r, _add40i); _abs41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = _abs41r + _c42r; _add43i = _abs41i + _c42i;
        double _log44r = 0, _log44i = 0;
        c_log(_add43r, _add43i, &_log44r, &_log44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _add37r + _log44r; _add45i = _add37i + _log44i;
        double ang = _add45r; /* +_add45ii */
        double _cos46r = 0, _cos46i = 0;
        c_cos(ang, 0, &_cos46r, &_cos46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _sin48r = 0, _sin48i = 0;
        c_sin(ang, 0, &_sin48r, &_sin48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c47r, _c47i, _sin48r, _sin48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _cos46r + _mul49r; _add50i = _cos46i + _mul49i;
        double _mul51r = 0, _mul51i = 0;
        c_mul(mag, 0, _add50r, _add50i, &_mul51r, &_mul51i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_345_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_arr6r, _arr6i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log10r, _log10i, 0, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _div14r = 0, _div14i = 0;
        c_div(_mul11r, _mul11i, _add13r, _add13i, &_div14r, &_div14i);
        double mag_part = _div14r; /* +_div14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(j, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(j, 0, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang18r, _ang18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul17r + _mul22r; _add23i = _mul17i + _mul22i;
        double angle_part = _add23r; /* +_add23ii */
        double _cos24r = 0, _cos24i = 0;
        c_cos(angle_part, 0, &_cos24r, &_cos24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(angle_part, 0, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_c25r, _c25i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _cos24r + _mul27r; _add28i = _cos24i + _mul27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(mag_part, 0, _add28r, _add28i, &_mul29r, &_mul29i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
        double _cf30r = 0, _cf30i = 0;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
        double _conj31r = 0, _conj31i = 0;
        _conj31r = _cf30r; _conj31i = -(_cf30i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 4.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_conj31r, _conj31i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        cRe[j] += _mul36r; cIm[j] += _mul36i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_346_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _re2r = 0, _re2i = 0;
        _re2r = x1r; _re2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _div4r = 0, _div4i = 0;
        c_div(j, 0, _c3r, _c3i, &_div4r, &_div4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_div4r, _div4i, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_re2r, _re2i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _re7r = 0, _re7i = 0;
        _re7r = x2r; _re7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 3.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(j, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_div9r, _div9i, &_cos10r, &_cos10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_re7r, _re7i, _cos10r, _cos10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul6r + _mul11r; _add12i = _mul6i + _mul11i;
        double rec_part = _add12r; /* +_add12ii */
        double _im13r = 0, _im13i = 0;
        _im13r = x1i; _im13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 4.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(j, 0, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_im13r, _im13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _im18r = 0, _im18i = 0;
        _im18r = x2i; _im18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(j, 0, _c19r, _c19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_im18r, _im18i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _mul17r - _mul22r; _sub23i = _mul17i - _mul22i;
        double imc_part = _sub23r; /* +_sub23ii */
        double _add24r = 0, _add24i = 0;
        _add24r = rec_part + imc_part; _add24i = 0 + 0;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_add24r, _add24i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.2; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(j, 0, 1.2, &_pow30r, &_pow30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log28r, _log28i, _pow30r, _pow30i, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _c32r + _sin36r; _add37i = _c32i + _sin36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_mul31r, _mul31i, _add37r, _add37i, &_mul38r, &_mul38i);
        double magnitude = _mul38r; /* +_mul38ii */
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 7.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(j, 0, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang39r, _ang39i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 8.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(j, 0, _c45r, _c45i, &_div46r, &_div46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(_div46r, _div46i, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang44r, _ang44i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul43r + _mul48r; _add49i = _mul43i + _mul48i;
        double angle = _add49r; /* +_add49ii */
        double _cos50r = 0, _cos50i = 0;
        c_cos(angle, 0, &_cos50r, &_cos50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _sin52r = 0, _sin52i = 0;
        c_sin(angle, 0, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c51r, _c51i, _sin52r, _sin52i, &_mul53r, &_mul53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _cos50r + _mul53r; _add54i = _cos50i + _mul53i;
        double _mul55r = 0, _mul55i = 0;
        c_mul(magnitude, 0, _add54r, _add54i, &_mul55r, &_mul55i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_347: too complex for auto-transpile, stubbed */
static void poly_347_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_348_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs6r + j; _add7i = _abs6i + 0;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 7.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log8r, _log8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 5.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_cos17r, _cos17i, _abs18r, _abs18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul13r + _mul19r; _add20i = _mul13i + _mul19i;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(j, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang21r, _ang21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _cos25r = 0, _cos25i = 0;
        c_cos(j, 0, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul23r + _mul26r; _add27i = _mul23i + _mul26i;
        double angle = _add27r; /* +_add27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c28r, _c28i, angle, 0, &_mul29r, &_mul29i);
        double _exp30r = 0, _exp30i = 0;
        c_exp2(_mul29r, _mul29i, &_exp30r, &_exp30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(magnitude, 0, _exp30r, _exp30i, &_mul31r, &_mul31i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    for (int k = 0; k < (int)(n); k++) {
        double _re32r = 0, _re32i = 0;
        _re32r = x1r; _re32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 5.0; _c33i = 0;
        double _mod34r = 0, _mod34i = 0;
        _mod34r = fmod(k, _c33r); _mod34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _mod34r + _c35r; _add36i = _mod34i + _c35i;
        double _pow37r = 0, _pow37i = 0;
        c_powr(_re32r, _re32i, _add36r, &_pow37r, &_pow37i);
        double _im38r = 0, _im38i = 0;
        _im38r = x2i; _im38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _mod40r = 0, _mod40i = 0;
        _mod40r = fmod(k, _c39r); _mod40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _mod40r + _c41r; _add42i = _mod40i + _c41i;
        double _pow43r = 0, _pow43i = 0;
        c_powr(_im38r, _im38i, _add42r, &_pow43r, &_pow43i);
        double _sub44r = 0, _sub44i = 0;
        _sub44r = _pow37r - _pow43r; _sub44i = _pow37i - _pow43i;
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _sin46r = 0, _sin46i = 0;
        c_sin(k, 0, &_sin46r, &_sin46i);
        double _cos47r = 0, _cos47i = 0;
        c_cos(k, 0, &_cos47r, &_cos47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _sin46r + _cos47r; _add48i = _sin46i + _cos47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c45r, _c45i, _add48r, _add48i, &_mul49r, &_mul49i);
        double _exp50r = 0, _exp50i = 0;
        c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_sub44r, _sub44i, _exp50r, _exp50i, &_mul51r, &_mul51i);
        cRe[k] += _mul51r; cIm[k] += _mul51i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_349: too complex for auto-transpile, stubbed */
static void poly_349_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_350_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < 35; j++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs2r + j; _add3i = _abs2i + 0;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 6.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log4r, _log4i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double mag_part1 = _mul9r; /* +_mul9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs10r + j; _add11i = _abs10i + 0;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 8.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log12r, _log12i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double mag_part2 = _mul17r; /* +_mul17ii */
        double _add18r = 0, _add18i = 0;
        _add18r = mag_part1 + mag_part2; _add18i = 0 + 0;
        double _sqrt19r = 0, _sqrt19i = 0;
        c_powr(j, 0, 0.5, &_sqrt19r, &_sqrt19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add18r + _sqrt19r; _add20i = _add18i + _sqrt19i;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double angle_part1 = _mul25r; /* +_mul25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 4.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double angle_part2 = _mul30r; /* +_mul30ii */
        double _add31r = 0, _add31i = 0;
        _add31r = angle_part1 + angle_part2; _add31i = 0 + 0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(j, 0, &_sin32r, &_sin32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(j, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_sin32r, _sin32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add31r + _mul36r; _add37i = _add31i + _mul36i;
        double angle = _add37r; /* +_add37ii */
        double _cos38r = 0, _cos38i = 0;
        c_cos(angle, 0, &_cos38r, &_cos38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _sin40r = 0, _sin40i = 0;
        c_sin(angle, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c39r, _c39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos38r + _mul41r; _add42i = _cos38i + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_351_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double rec1 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double imc1 = _im3r; /* +_im3ii */
    double _re4r = 0, _re4i = 0;
    _re4r = x2r; _re4i = 0;
    double rec2 = _re4r; /* +_re4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc2 = _im5r; /* +_im5ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 1.5; _c6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(j, 0, 1.5, &_pow7r, &_pow7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(rec1, 0, _pow7r, _pow7i, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _div11r = 0, _div11i = 0;
        c_div(imc2, 0, _add10r, _add10i, &_div11r, &_div11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _div11r; _add12i = _mul8i + _div11i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_add12r, _add12i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 4.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _c17r + _sin21r; _add22i = _c17i + _sin21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log16r, _log16i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag_part = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul28r + _mul33r; _add34i = _mul28i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 7.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add34r + _sin38r; _add39i = _add34i + _sin38i;
        double angle_part = _add39r; /* +_add39ii */
        double _cos40r = 0, _cos40i = 0;
        c_cos(angle_part, 0, &_cos40r, &_cos40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _sin42r = 0, _sin42i = 0;
        c_sin(angle_part, 0, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c41r, _c41i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _cos40r + _mul43r; _add44i = _cos40i + _mul43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag_part, 0, _add44r, _add44i, &_mul45r, &_mul45i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_352_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double sum_real = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double sum_imag = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        for (int k = 1; k < (int)(_add5r); k++) {
            double _re6r = 0, _re6i = 0;
            _re6r = x1r; _re6i = 0;
            double _pow7r = 0, _pow7i = 0;
            c_powr(_re6r, _re6i, k, &_pow7r, &_pow7i);
            double _mul8r = 0, _mul8i = 0;
            c_mul(k, 0, M_PI, 0, &_mul8r, &_mul8i);
            double _div9r = 0, _div9i = 0;
            c_div(_mul8r, _mul8i, j, 0, &_div9r, &_div9i);
            double _cos10r = 0, _cos10i = 0;
            c_cos(_div9r, _div9i, &_cos10r, &_cos10i);
            double _mul11r = 0, _mul11i = 0;
            c_mul(_pow7r, _pow7i, _cos10r, _cos10i, &_mul11r, &_mul11i);
            sum_real += _mul11r;
            double _im12r = 0, _im12i = 0;
            _im12r = x2i; _im12i = 0;
            double _pow13r = 0, _pow13i = 0;
            c_powr(_im12r, _im12i, k, &_pow13r, &_pow13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(k, 0, M_PI, 0, &_mul14r, &_mul14i);
            double _div15r = 0, _div15i = 0;
            c_div(_mul14r, _mul14i, j, 0, &_div15r, &_div15i);
            double _sin16r = 0, _sin16i = 0;
            c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_pow13r, _pow13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
            sum_imag += _mul17r;
        }
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(sum_real, 0); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.5; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_abs22r, _abs22i, 0.5, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log21r, _log21i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(sum_imag, 0); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.3; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_abs30r, _abs30i, 0.3, &_pow32r, &_pow32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_log29r, _log29i, _pow32r, _pow32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul25r + _mul33r; _add34i = _mul25i + _mul33i;
        double mag = _add34r; /* +_add34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x1r, x1i); _ang35i = 0;
        double _sin36r = 0, _sin36i = 0;
        c_sin(j, 0, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang35r, _ang35i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 2.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(j, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang38r, _ang38i, _cos41r, _cos41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul37r + _mul42r; _add43i = _mul37i + _mul42i;
        double angle = _add43r; /* +_add43ii */
        double _cos44r = 0, _cos44i = 0;
        c_cos(angle, 0, &_cos44r, &_cos44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _sin46r = 0, _sin46i = 0;
        c_sin(angle, 0, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c45r, _c45i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _cos44r + _mul47r; _add48i = _cos44i + _mul47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(mag, 0, _add48r, _add48i, &_mul49r, &_mul49i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    }
    for (int r = 0; r < (int)(n); r++) {
        double _c50r = 0, _c50i = 0;
        _c50r = 1.0; _c50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 0.1; _c51i = 0;
        double _sin52r = 0, _sin52i = 0;
        c_sin(r, 0, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c51r, _c51i, _sin52r, _sin52i, &_mul53r, &_mul53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _c50r + _mul53r; _add54i = _c50i + _mul53i;
        double _c55r = 0, _c55i = 0;
        _c55r = 0.0; _c55i = 0.1;
        double _cos56r = 0, _cos56i = 0;
        c_cos(r, 0, &_cos56r, &_cos56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_c55r, _c55i, _cos56r, _cos56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _add54r + _mul57r; _add58i = _add54i + _mul57i;
        { double _tr = cRe[r]*_add58r - cIm[r]*_add58i; cIm[r] = cRe[r]*_add58i + cIm[r]*_add58r; cRe[r] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_353_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double _arr7r = 0, _arr7i = 0;
        { int _idx = j; _arr7r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr7i = 0; }
        double _add8r = 0, _add8i = 0;
        _add8r = _arr6r + _arr7r; _add8i = _arr6i + _arr7i;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(j, 0, &_sin14r, &_sin14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c13r + _sin14r; _add15i = _c13i + _sin14i;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(j, 0, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _add15r + _cos18r; _add19i = _add15i + _cos18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log12r, _log12i, _add19r, _add19i, &_mul20r, &_mul20i);
        double mag_variation = _mul20r; /* +_mul20ii */
        double _arr21r = 0, _arr21i = 0;
        { int _idx = j; _arr21r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr21i = 0; }
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _arr23r = 0, _arr23i = 0;
        { int _idx = j; _arr23r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr23i = 0; }
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c22r, _c22i, _arr23r, _arr23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _arr21r + _mul24r; _add25i = _arr21i + _mul24i;
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(_add25r, _add25i); _ang26i = 0;
        double _arr27r = 0, _arr27i = 0;
        { int _idx = j; _arr27r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr27i = 0; }
        double _mul28r = 0, _mul28i = 0;
        c_mul(_arr27r, _arr27i, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = j + _c29r; _add30i = 0 + _c29i;
        double _div31r = 0, _div31i = 0;
        c_div(_mul28r, _mul28i, _add30r, _add30i, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _ang26r + _sin32r; _add33i = _ang26i + _sin32i;
        double _arr34r = 0, _arr34i = 0;
        { int _idx = j; _arr34r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr34i = 0; }
        double _mul35r = 0, _mul35i = 0;
        c_mul(_arr34r, _arr34i, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = j + _c36r; _add37i = 0 + _c36i;
        double _div38r = 0, _div38i = 0;
        c_div(_mul35r, _mul35i, _add37r, _add37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _sub40r = 0, _sub40i = 0;
        _sub40r = _add33r - _cos39r; _sub40i = _add33i - _cos39i;
        double angle_variation = _sub40r; /* +_sub40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle_variation, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag_variation, 0, _exp43r, _exp43i, &_mul44r, &_mul44i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_354_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 5.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin9r, _sin9i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double part1 = _mul13r; /* +_mul13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs14r + j; _add15i = _abs14i + 0;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 3.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log16r, _log16i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double part2 = _mul21r; /* +_mul21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 4.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 6.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _cos25r + _sin29r; _add30i = _cos25i + _sin29i;
        double part3 = _add30r; /* +_add30ii */
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x1r, x1i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.5; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, j, 0, &_mul33r, &_mul33i);
        double _pow34r = 0, _pow34i = 0;
        c_powr(_abs31r, _abs31i, _mul33r, &_pow34r, &_pow34i);
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(j, 0); _abs35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _abs35r + _c36r; _add37i = _abs35i + _c36i;
        double _log38r = 0, _log38i = 0;
        c_log(_add37r, _add37i, &_log38r, &_log38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 7.0; _c39i = 0;
        double _mod40r = 0, _mod40i = 0;
        _mod40r = fmod(j, _c39r); _mod40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _mod40r + _c41r; _add42i = _mod40i + _c41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_log38r, _log38i, _add42r, _add42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _pow34r + _mul43r; _add44i = _pow34i + _mul43i;
        double magnitude = _add44r; /* +_add44ii */
        double _add45r = 0, _add45i = 0;
        _add45r = part1 + part2; _add45i = 0 + 0;
        double _add46r = 0, _add46i = 0;
        _add46r = _add45r + part3; _add46i = _add45i + 0;
        double angle = _add46r; /* +_add46ii */
        double _cos47r = 0, _cos47i = 0;
        c_cos(angle, 0, &_cos47r, &_cos47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _sin49r = 0, _sin49i = 0;
        c_sin(angle, 0, &_sin49r, &_sin49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c48r, _c48i, _sin49r, _sin49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _cos47r + _mul50r; _add51i = _cos47i + _mul50i;
        double _mul52r = 0, _mul52i = 0;
        c_mul(magnitude, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_355_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < 35; j++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double mag = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double angle = _c3r; /* +_c3ii */
        for (int k = 1; k < 36; k++) {
            double _re4r = 0, _re4i = 0;
            _re4r = x1r; _re4i = 0;
            double _pow5r = 0, _pow5i = 0;
            c_powr(_re4r, _re4i, k, &_pow5r, &_pow5i);
            double _abs6r = 0, _abs6i = 0;
            _abs6r = c_abs(x2r, x2i); _abs6i = 0;
            double _add7r = 0, _add7i = 0;
            _add7r = _abs6r + k; _add7i = _abs6i + 0;
            double _log8r = 0, _log8i = 0;
            c_log(_add7r, _add7i, &_log8r, &_log8i);
            double _mul9r = 0, _mul9i = 0;
            c_mul(_pow5r, _pow5i, _log8r, _log8i, &_mul9r, &_mul9i);
            double _c10r = 0, _c10i = 0;
            _c10r = 1.0; _c10i = 0;
            double _c11r = 0, _c11i = 0;
            _c11r = 2.0; _c11i = 0;
            double _pow12r = 0, _pow12i = 0;
            c_mul(k, 0, k, 0, &_pow12r, &_pow12i);
            double _add13r = 0, _add13i = 0;
            _add13r = _c10r + _pow12r; _add13i = _c10i + _pow12i;
            double _div14r = 0, _div14i = 0;
            c_div(_mul9r, _mul9i, _add13r, _add13i, &_div14r, &_div14i);
            mag += _div14r;
            double _ang15r = 0, _ang15i = 0;
            _ang15r = c_arg(x1r, x1i); _ang15i = 0;
            double _mul16r = 0, _mul16i = 0;
            c_mul(k, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
            double _sin17r = 0, _sin17i = 0;
            c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
            double _ang18r = 0, _ang18i = 0;
            _ang18r = c_arg(x2r, x2i); _ang18i = 0;
            double _mul19r = 0, _mul19i = 0;
            c_mul(k, 0, _ang18r, _ang18i, &_mul19r, &_mul19i);
            double _cos20r = 0, _cos20i = 0;
            c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(_sin17r, _sin17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
            angle += _mul21r;
        }
        double _cos22r = 0, _cos22i = 0;
        c_cos(angle, 0, &_cos22r, &_cos22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _sin24r = 0, _sin24i = 0;
        c_sin(angle, 0, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c23r, _c23i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _cos22r + _mul25r; _add26i = _cos22i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag, 0, _add26r, _add26i, &_mul27r, &_mul27i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int r = 1; r < 36; r++) {
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _re29r = 0, _re29i = 0;
        _re29r = x1r; _re29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_re29r, _re29i, r, 0, &_mul30r, &_mul30i);
        double _im31r = 0, _im31i = 0;
        _im31r = x2i; _im31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = r + _c32r; _add33i = 0 + _c32i;
        double _div34r = 0, _div34i = 0;
        c_div(_im31r, _im31i, _add33r, _add33i, &_div34r, &_div34i);
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _mul30r - _div34r; _sub35i = _mul30i - _div34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c28r, _c28i, _sub35r, _sub35i, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        { double _tr = cRe[(r - 1)]*_exp37r - cIm[(r - 1)]*_exp37i; cIm[(r - 1)] = cRe[(r - 1)]*_exp37i + cIm[(r - 1)]*_exp37r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_356_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < 35; j++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double temp = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        for (int k = 1; k < (int)(_add4r); k++) {
            double _re5r = 0, _re5i = 0;
            _re5r = x1r; _re5i = 0;
            double _pow6r = 0, _pow6i = 0;
            c_powr(_re5r, _re5i, k, &_pow6r, &_pow6i);
            double _ang7r = 0, _ang7i = 0;
            _ang7r = c_arg(x2r, x2i); _ang7i = 0;
            double _mul8r = 0, _mul8i = 0;
            c_mul(k, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
            double _sin9r = 0, _sin9i = 0;
            c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(_pow6r, _pow6i, _sin9r, _sin9i, &_mul10r, &_mul10i);
            double _im11r = 0, _im11i = 0;
            _im11r = x2i; _im11i = 0;
            double _pow12r = 0, _pow12i = 0;
            c_powr(_im11r, _im11i, k, &_pow12r, &_pow12i);
            double _ang13r = 0, _ang13i = 0;
            _ang13r = c_arg(x1r, x1i); _ang13i = 0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(k, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
            double _cos15r = 0, _cos15i = 0;
            c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_pow12r, _pow12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
            double _add17r = 0, _add17i = 0;
            _add17r = _mul10r + _mul16r; _add17i = _mul10i + _mul16i;
            temp += _add17r;
        }
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _abs19r; _add20i = _abs18i + _abs19i;
        double _add21r = 0, _add21i = 0;
        _add21r = _add20r + j; _add21i = _add20i + 0;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log22r, _log22i, temp, 0, &_mul23r, &_mul23i);
        double magnitude = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _cos25r = 0, _cos25i = 0;
        c_cos(j, 0, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(j, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang27r, _ang27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul26r + _mul29r; _add30i = _mul26i + _mul29i;
        double angle = _add30r; /* +_add30ii */
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, angle, 0, &_mul32r, &_mul32i);
        double _exp33r = 0, _exp33i = 0;
        c_exp2(_mul32r, _mul32i, &_exp33r, &_exp33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(magnitude, 0, _exp33r, _exp33i, &_mul34r, &_mul34i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    for (int r = 0; r < 35; r++) {
        double _re35r = 0, _re35i = 0;
        _re35r = x1r; _re35i = 0;
        double _re36r = 0, _re36i = 0;
        _re36r = x2r; _re36i = 0;
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _re35r - _re36r; _sub37i = _re35i - _re36i;
        double _sin38r = 0, _sin38i = 0;
        c_sin(r, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_sub37r, _sub37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _im40r = 0, _im40i = 0;
        _im40r = x1i; _im40i = 0;
        double _im41r = 0, _im41i = 0;
        _im41r = x2i; _im41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _im40r + _im41r; _add42i = _im40i + _im41i;
        double _cos43r = 0, _cos43i = 0;
        c_cos(r, 0, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_add42r, _add42i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul39r + _mul44r; _add45i = _mul39i + _mul44i;
        cRe[r] += _add45r; cIm[r] += _add45i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_357_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs2r + j; _add3i = _abs2i + 0;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(j, 0, _c5r, _c5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log4r, _log4i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _sqrt10r = 0, _sqrt10i = 0;
        c_powr(_abs9r, _abs9i, 0.5, &_sqrt10r, &_sqrt10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 3.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(j, 0, _c11r, _c11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sqrt10r, _sqrt10i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul8r + _mul14r; _add15i = _mul8i + _mul14i;
        double magnitude = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang16r, _ang16i, j, 0, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _sub19r = 0, _sub19i = 0;
        _sub19r = n - j; _sub19i = 0 - 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _sub19r, _sub19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
        double angle = _add21r; /* +_add21ii */
        double _cos22r = 0, _cos22i = 0;
        c_cos(angle, 0, &_cos22r, &_cos22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(angle, 0, &_sin23r, &_sin23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin23r, _sin23i, _c24r, _c24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _cos22r + _mul25r; _add26i = _cos22i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(magnitude, 0, _add26r, _add26i, &_mul27r, &_mul27i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int k = 0; k < (int)(n); k++) {
        double _re28r = 0, _re28i = 0;
        _re28r = x1r; _re28i = 0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(k, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_re28r, _re28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _im31r = 0, _im31i = 0;
        _im31r = x2i; _im31i = 0;
        double _cos32r = 0, _cos32i = 0;
        c_cos(k, 0, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_im31r, _im31i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul30r - _mul33r; _sub34i = _mul30i - _mul33i;
        cRe[k] += _sub34r; cIm[k] += _sub34i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_358_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x1r; _re3i = 0;
    double _sub4r = 0, _sub4i = 0;
    _sub4r = _re2r - _re3r; _sub4i = _re2i - _re3i;
    double real_diff = _sub4r; /* +_sub4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double _im6r = 0, _im6i = 0;
    _im6r = x1i; _im6i = 0;
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _im5r - _im6r; _sub7i = _im5i - _im6i;
    double im_diff = _sub7r; /* +_sub7ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(n, 0, _c9r, _c9i, &_div10r, &_div10i);
        double _sub11r = 0, _sub11i = 0;
        _sub11r = j - _div10r; _sub11i = 0 - _div10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_sub11r, _sub11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_abs12r, _abs12i, _log15r, _log15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _c8r + _mul16r; _add17i = _c8i + _mul16i;
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, n, 0, &_div19r, &_div19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_div19r, _div19i, _ang20r, _ang20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _ang18r + _mul21r; _add22i = _ang18i + _mul21i;
        double _sin23r = 0, _sin23i = 0;
        c_sin(j, 0, &_sin23r, &_sin23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(j, 0, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_sin23r, _sin23i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _add22r + _mul27r; _add28i = _add22i + _mul27i;
        double angle = _add28r; /* +_add28ii */
        for (int k = 1; k < 4; k++) {
            double _c29r = 0, _c29i = 0;
            _c29r = 1.0; _c29i = 0;
            double _c30r = 0, _c30i = 0;
            _c30r = 0.1; _c30i = 0;
            double _mul31r = 0, _mul31i = 0;
            c_mul(_c30r, _c30i, k, 0, &_mul31r, &_mul31i);
            double _mul32r = 0, _mul32i = 0;
            c_mul(j, 0, k, 0, &_mul32r, &_mul32i);
            double _div33r = 0, _div33i = 0;
            c_div(_mul32r, _mul32i, n, 0, &_div33r, &_div33i);
            double _sin34r = 0, _sin34i = 0;
            c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
            double _mul35r = 0, _mul35i = 0;
            c_mul(_mul31r, _mul31i, _sin34r, _sin34i, &_mul35r, &_mul35i);
            double _add36r = 0, _add36i = 0;
            _add36r = _c29r + _mul35r; _add36i = _c29i + _mul35i;
            mag *= _add36r;
            double _c37r = 0, _c37i = 0;
            _c37r = 0.5; _c37i = 0;
            double _mul38r = 0, _mul38i = 0;
            c_mul(_c37r, _c37i, k, 0, &_mul38r, &_mul38i);
            double _mul39r = 0, _mul39i = 0;
            c_mul(j, 0, k, 0, &_mul39r, &_mul39i);
            double _div40r = 0, _div40i = 0;
            c_div(_mul39r, _mul39i, n, 0, &_div40r, &_div40i);
            double _cos41r = 0, _cos41i = 0;
            c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
            double _mul42r = 0, _mul42i = 0;
            c_mul(_mul38r, _mul38i, _cos41r, _cos41i, &_mul42r, &_mul42i);
            angle += _mul42r;
            for (int r = 1; r < 3; r++) {
                double _c43r = 0, _c43i = 0;
                _c43r = 0.05; _c43i = 0;
                double _mul44r = 0, _mul44i = 0;
                c_mul(_c43r, _c43i, r, 0, &_mul44r, &_mul44i);
                double _add45r = 0, _add45i = 0;
                _add45r = j + r; _add45i = 0 + 0;
                double _log46r = 0, _log46i = 0;
                c_log(_add45r, _add45i, &_log46r, &_log46i);
                double _mul47r = 0, _mul47i = 0;
                c_mul(_mul44r, _mul44i, _log46r, _log46i, &_mul47r, &_mul47i);
                mag += _mul47r;
                double _c48r = 0, _c48i = 0;
                _c48r = 0.3; _c48i = 0;
                double _mul49r = 0, _mul49i = 0;
                c_mul(_c48r, _c48i, r, 0, &_mul49r, &_mul49i);
                double _mul50r = 0, _mul50i = 0;
                c_mul(j, 0, r, 0, &_mul50r, &_mul50i);
                double _div51r = 0, _div51i = 0;
                c_div(_mul50r, _mul50i, n, 0, &_div51r, &_div51i);
                double _sin52r = 0, _sin52i = 0;
                c_sin(_div51r, _div51i, &_sin52r, &_sin52i);
                double _mul53r = 0, _mul53i = 0;
                c_mul(_mul49r, _mul49i, _sin52r, _sin52i, &_mul53r, &_mul53i);
                angle += _mul53r;
            }
        }
        double _cos54r = 0, _cos54i = 0;
        c_cos(angle, 0, &_cos54r, &_cos54i);
        double _sin55r = 0, _sin55i = 0;
        c_sin(angle, 0, &_sin55r, &_sin55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 0.0; _c56i = 1.0;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_sin55r, _sin55i, _c56r, _c56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _cos54r + _mul57r; _add58i = _cos54i + _mul57i;
        double _mul59r = 0, _mul59i = 0;
        c_mul(mag, 0, _add58r, _add58i, &_mul59r, &_mul59i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_359_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _conj2r = 0, _conj2i = 0;
    _conj2r = x1r; _conj2i = -(x1i);
    double _conj3r = 0, _conj3i = 0;
    _conj3r = x2r; _conj3i = -(x2i);
    double _add4r = 0, _add4i = 0;
    _add4r = _conj2r + _conj3r; _add4i = _conj2i + _conj3i;
    double t_conj = _add4r; /* +_add4ii */
    for (int j = 0; j < (int)(n); j++) {
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 7.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_re5r, _re5i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _re11r = 0, _re11i = 0;
        _re11r = x2r; _re11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 5.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_re11r, _re11i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul10r + _mul16r; _add17i = _mul10i + _mul16i;
        double real_part = _add17r; /* +_add17ii */
        double _im18r = 0, _im18i = 0;
        _im18r = x1i; _im18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 6.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_im18r, _im18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _im24r = 0, _im24i = 0;
        _im24r = x2i; _im24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 4.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_im24r, _im24i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul23r - _mul29r; _sub30i = _mul23i - _mul29i;
        double imag_part = _sub30r; /* +_sub30ii */
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(t_conj, 0); _abs31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _abs31r + j; _add32i = _abs31i + 0;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(real_part, 0); _abs34i = 0;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(imag_part, 0); _abs35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _abs34r + _abs35r; _add36i = _abs34i + _abs35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_log33r, _log33i, _add36r, _add36i, &_mul37r, &_mul37i);
        double magnitude = _mul37r; /* +_mul37ii */
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(t_conj, 0); _ang39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 3.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(j, 0, _c40r, _c40i, &_div41r, &_div41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_div41r, _div41i, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang39r, _ang39i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _ang38r + _mul43r; _add44i = _ang38i + _mul43i;
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x2r, x2i); _ang45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 4.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(j, 0, _c46r, _c46i, &_div47r, &_div47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_div47r, _div47i, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_ang45r, _ang45i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _sub50r = 0, _sub50i = 0;
        _sub50r = _add44r - _mul49r; _sub50i = _add44i - _mul49i;
        double angle = _sub50r; /* +_sub50ii */
        double _cos51r = 0, _cos51i = 0;
        c_cos(angle, 0, &_cos51r, &_cos51i);
        double _sin52r = 0, _sin52i = 0;
        c_sin(angle, 0, &_sin52r, &_sin52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 0.0; _c53i = 1.0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_sin52r, _sin52i, _c53r, _c53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _cos51r + _mul54r; _add55i = _cos51i + _mul54i;
        double _mul56r = 0, _mul56i = 0;
        c_mul(magnitude, 0, _add55r, _add55i, &_mul56r, &_mul56i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_360_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double _arr7r = 0, _arr7i = 0;
        { int _idx = j; _arr7r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(_arr6r, _arr6i, _arr7r, _arr7i, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_mul8r, _mul8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(j, 0, &_sin13r, &_sin13i);
        double _pow14r = 0, _pow14i = 0;
        c_powr(j, 0, _sin13r, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log12r, _log12i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _cos17r = 0, _cos17i = 0;
        c_cos(j, 0, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _c16r + _cos17r; _add18i = _c16i + _cos17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul15r, _mul15i, _add18r, _add18i, &_mul19r, &_mul19i);
        double magnitude = _mul19r; /* +_mul19ii */
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c20r, _c20i, M_PI, 0, &_mul21r, &_mul21i);
        double _arr22r = 0, _arr22i = 0;
        { int _idx = j; _arr22r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr22i = 0; }
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul21r, _mul21i, _arr22r, _arr22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c25r, _c25i, M_PI, 0, &_mul26r, &_mul26i);
        double _arr27r = 0, _arr27i = 0;
        { int _idx = j; _arr27r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr27i = 0; }
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul26r, _mul26i, _arr27r, _arr27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _sin24r + _cos29r; _add30i = _sin24i + _cos29i;
        double _arr31r = 0, _arr31i = 0;
        { int _idx = j; _arr31r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr31i = 0; }
        double _arr32r = 0, _arr32i = 0;
        { int _idx = j; _arr32r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr32i = 0; }
        double _add33r = 0, _add33i = 0;
        _add33r = _arr31r + _arr32r; _add33i = _arr31i + _arr32i;
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(_add33r, _add33i); _abs34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _abs34r + _c35r; _add36i = _abs34i + _c35i;
        double _log37r = 0, _log37i = 0;
        c_log(_add36r, _add36i, &_log37r, &_log37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add30r + _log37r; _add38i = _add30i + _log37i;
        double angle = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(magnitude, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_361_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double re1 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double im1 = _im3r; /* +_im3ii */
    double _re4r = 0, _re4i = 0;
    _re4r = x2r; _re4i = 0;
    double re2 = _re4r; /* +_re4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double im2 = _im5r; /* +_im5ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _mod7r = 0, _mod7i = 0;
        _mod7r = fmod(j, _c6r); _mod7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, re1, 0, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _mul10r; _add11i = _abs9i + _mul10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, im2, 0, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c13r + _sin15r; _add16i = _c13i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log12r, _log12i, _add16r, _add16i, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, im1, 0, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _mul19r; _add20i = _abs18i + _mul19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, re2, 0, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _c22r + _cos24r; _add25i = _c22i + _cos24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log21r, _log21i, _add25r, _add25i, &_mul26r, &_mul26i);
        double _tern27r = 0, _tern27i = 0;
        if (_mod7r == _c8r) { _tern27r = _mul17r; _tern27i = _mul17i; }
        else { _tern27r = _mul26r; _tern27i = _mul26i; }
        double mag = _tern27r; /* +_tern27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(n, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_mul30r, _mul30i, re1, 0, &_mul31r, &_mul31i);
        double _div32r = 0, _div32i = 0;
        c_div(_mul31r, _mul31i, n, 0, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_mul34r, _mul34i, im2, 0, &_mul35r, &_mul35i);
        double _div36r = 0, _div36i = 0;
        c_div(_mul35r, _mul35i, n, 0, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sin33r + _cos37r; _add38i = _sin33i + _cos37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, M_PI, 0, &_mul39r, &_mul39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_mul39r, _mul39i, re2, 0, &_mul40r, &_mul40i);
        double _div41r = 0, _div41i = 0;
        c_div(_mul40r, _mul40i, n, 0, &_div41r, &_div41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_div41r, _div41i, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(j, 0, M_PI, 0, &_mul43r, &_mul43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_mul43r, _mul43i, im1, 0, &_mul44r, &_mul44i);
        double _div45r = 0, _div45i = 0;
        c_div(_mul44r, _mul44i, n, 0, &_div45r, &_div45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(_div45r, _div45i, &_cos46r, &_cos46i);
        double _sub47r = 0, _sub47i = 0;
        _sub47r = _sin42r - _cos46r; _sub47i = _sin42i - _cos46i;
        double _tern48r = 0, _tern48i = 0;
        if (j <= _div29r) { _tern48r = _add38r; _tern48i = _add38i; }
        else { _tern48r = _sub47r; _tern48i = _sub47i; }
        double angle = _tern48r; /* +_tern48ii */
        double _cos49r = 0, _cos49i = 0;
        c_cos(angle, 0, &_cos49r, &_cos49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _sin51r = 0, _sin51i = 0;
        c_sin(angle, 0, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c50r, _c50i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _cos49r + _mul52r; _add53i = _cos49i + _mul52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(mag, 0, _add53r, _add53i, &_mul54r, &_mul54i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_362_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 4.0; _c2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(j, 0, _c2r, _c2i, &_mul3r, &_mul3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 5.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _mul3r + _c4r; _add5i = _mul3i + _c4i;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(_add5r, n); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double k = _add8r; /* +_add8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 6.0; _c9i = 0;
        double _fdiv10r = 0, _fdiv10i = 0;
        c_div(j, 0, _c9r, _c9i, &_fdiv10r, &_fdiv10i);
        _fdiv10r = floor(_fdiv10r); _fdiv10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _fdiv10r + _c11r; _add12i = _fdiv10i + _c11i;
        double r = _add12r; /* +_add12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(j, 0, j, 0, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _abs13r + _pow15r; _add16i = _abs13i + _pow15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(k, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _div19r + _div21r; _add22i = _div19i + _div21i;
        double _sin23r = 0, _sin23i = 0;
        c_sin(_add22r, _add22i, &_sin23r, &_sin23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_sin23r, _sin23i); _abs24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log17r, _log17i, _abs24r, _abs24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = j + r; _add26i = 0 + 0;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul25r + _log27r; _add28i = _mul25i + _log27i;
        double mag = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = k + _c30r; _add31i = 0 + _c30i;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _add31r, _add31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang29r, _ang29i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 2.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = r + _c36r; _add37i = 0 + _c36i;
        double _div38r = 0, _div38i = 0;
        c_div(j, 0, _add37r, _add37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang35r, _ang35i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul34r + _mul40r; _add41i = _mul34i + _mul40i;
        double _re42r = 0, _re42i = 0;
        _re42r = x1r; _re42i = 0;
        double _im43r = 0, _im43i = 0;
        _im43r = x2i; _im43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_re42r, _re42i, _im43r, _im43i, &_mul44r, &_mul44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = j + _c45r; _add46i = 0 + _c45i;
        double _div47r = 0, _div47i = 0;
        c_div(_mul44r, _mul44i, _add46r, _add46i, &_div47r, &_div47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _add41r + _div47r; _add48i = _add41i + _div47i;
        double angle = _add48r; /* +_add48ii */
        double _cos49r = 0, _cos49i = 0;
        c_cos(angle, 0, &_cos49r, &_cos49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _sin51r = 0, _sin51i = 0;
        c_sin(angle, 0, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c50r, _c50i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _cos49r + _mul52r; _add53i = _cos49i + _mul52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(mag, 0, _add53r, _add53i, &_mul54r, &_mul54i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_363_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x1r, x1i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x2r, x2i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin10r + _cos13r; _add14i = _sin10i + _cos13i;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(j, 0, j, 0, &_pow16r, &_pow16i);
        double _div17r = 0, _div17i = 0;
        c_div(_pow16r, _pow16i, n, 0, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _add14r + _sin18r; _add19i = _add14i + _sin18i;
        double angle = _add19r; /* +_add19ii */
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log23r, _log23i, j, 0, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.5; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_abs25r, _abs25i, 0.5, &_pow27r, &_pow27i);
        double _sqrt28r = 0, _sqrt28i = 0;
        c_powr(j, 0, 0.5, &_sqrt28r, &_sqrt28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_pow27r, _pow27i, _sqrt28r, _sqrt28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul24r + _mul29r; _add30i = _mul24i + _mul29i;
        double magnitude = _add30r; /* +_add30ii */
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, angle, 0, &_mul32r, &_mul32i);
        double _exp33r = 0, _exp33i = 0;
        c_exp2(_mul32r, _mul32i, &_exp33r, &_exp33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(magnitude, 0, _exp33r, _exp33i, &_mul34r, &_mul34i);
        double _conj35r = 0, _conj35i = 0;
        _conj35r = x1r; _conj35i = -(x1i);
        double _conj36r = 0, _conj36i = 0;
        _conj36r = x2r; _conj36i = -(x2i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_conj35r, _conj35i, _conj36r, _conj36i, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = j + _c38r; _add39i = 0 + _c38i;
        double _div40r = 0, _div40i = 0;
        c_div(_mul37r, _mul37i, _add39r, _add39i, &_div40r, &_div40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul34r + _div40r; _add41i = _mul34i + _div40i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    }
    double _c42r = 0, _c42i = 0;
    _c42r = 2.0; _c42i = 0;
    double _fdiv43r = 0, _fdiv43i = 0;
    c_div(n, 0, _c42r, _c42i, &_fdiv43r, &_fdiv43i);
    _fdiv43r = floor(_fdiv43r); _fdiv43i = 0;
    double _c44r = 0, _c44i = 0;
    _c44r = 1.0; _c44i = 0;
    double _add45r = 0, _add45i = 0;
    _add45r = _fdiv43r + _c44r; _add45i = _fdiv43i + _c44i;
    for (int k = 1; k < (int)(_add45r); k++) {
        double _re46r = 0, _re46i = 0;
        _re46r = x1r; _re46i = 0;
        double _re47r = 0, _re47i = 0;
        _re47r = x2r; _re47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_re46r, _re46i, _re47r, _re47i, &_mul48r, &_mul48i);
        double _im49r = 0, _im49i = 0;
        _im49r = x1i; _im49i = 0;
        double _im50r = 0, _im50i = 0;
        _im50r = x2i; _im50i = 0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_im49r, _im49i, _im50r, _im50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul48r + _mul51r; _add52i = _mul48i + _mul51i;
        double _sin53r = 0, _sin53i = 0;
        c_sin(k, 0, &_sin53r, &_sin53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_add52r, _add52i, _sin53r, _sin53i, &_mul54r, &_mul54i);
        cRe[(k - 1)] += _mul54r; cIm[(k - 1)] += _mul54i;
    }
    double _c55r = 0, _c55i = 0;
    _c55r = 4.0; _c55i = 0;
    double _sub56r = 0, _sub56i = 0;
    _sub56r = n - _c55r; _sub56i = 0 - _c55i;
    double _c57r = 0, _c57i = 0;
    _c57r = 1.0; _c57i = 0;
    double _add58r = 0, _add58i = 0;
    _add58r = n + _c57r; _add58i = 0 + _c57i;
    for (int r = (int)(_sub56r); r < (int)(_add58r); r++) {
        double _abs59r = 0, _abs59i = 0;
        _abs59r = c_abs(x1r, x1i); _abs59i = 0;
        double _abs60r = 0, _abs60i = 0;
        _abs60r = c_abs(x2r, x2i); _abs60i = 0;
        double _prod61r = 0, _prod61i = 0;
        c_mul(_abs59r, _abs59i, _abs60r, _abs60i, &_prod61r, &_prod61i);
        double _prod62r = 0, _prod62i = 0;
        c_mul(_prod61r, _prod61i, r, 0, &_prod62r, &_prod62i);
        double _cos63r = 0, _cos63i = 0;
        c_cos(r, 0, &_cos63r, &_cos63i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(_prod62r, _prod62i, _cos63r, _cos63i, &_mul64r, &_mul64i);
        cRe[(r - 1)] += _mul64r; cIm[(r - 1)] += _mul64i;
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_364_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr8i = 0; }
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (j - 1); _arr9r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr9i = 0; }
        double _add10r = 0, _add10i = 0;
        _add10r = _arr8r + _arr9r; _add10i = _arr8i + _arr9i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(j, 0, &_sin15r, &_sin15i);
        double _pow16r = 0, _pow16i = 0;
        c_powr(j, 0, _sin15r, &_pow16r, &_pow16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log14r, _log14i, _pow16r, _pow16i, &_mul17r, &_mul17i);
        double _sqrt18r = 0, _sqrt18i = 0;
        c_powr(j, 0, 0.5, &_sqrt18r, &_sqrt18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_cos22r, _cos22i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sqrt18r, _sqrt18i, _abs23r, _abs23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul17r + _mul24r; _add25i = _mul17i + _mul24i;
        double mag = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(j, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _cos30r = 0, _cos30i = 0;
        c_cos(j, 0, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul28r + _mul31r; _add32i = _mul28i + _mul31i;
        double ang = _add32r; /* +_add32ii */
        double _cos33r = 0, _cos33i = 0;
        c_cos(ang, 0, &_cos33r, &_cos33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _sin35r = 0, _sin35i = 0;
        c_sin(ang, 0, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c34r, _c34i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _cos33r + _mul36r; _add37i = _cos33i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(mag, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_365: too complex for auto-transpile, stubbed */
static void poly_365_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_366_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _sin2r = 0, _sin2i = 0;
        c_sin(j, 0, &_sin2r, &_sin2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(_re1r, _re1i, _sin2r, _sin2i, &_mul3r, &_mul3i);
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_pow6r, _pow6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re4r, _re4i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul3r + _mul8r; _add9i = _mul3i + _mul8i;
        double r = _add9r; /* +_add9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _cos11r = 0, _cos11i = 0;
        c_cos(j, 0, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_im10r, _im10i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _im13r = 0, _im13i = 0;
        _im13r = x2i; _im13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(j, 0, j, 0, &_pow15r, &_pow15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_pow15r, _pow15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_im13r, _im13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = _mul12r - _mul17r; _sub18i = _mul12i - _mul17i;
        double im = _sub18r; /* +_sub18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs19r + j; _add20i = _abs19i + 0;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _sqrt22r = 0, _sqrt22i = 0;
        c_powr(j, 0, 0.5, &_sqrt22r, &_sqrt22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log21r, _log21i, _sqrt22r, _sqrt22i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 5.0; _c24i = 0;
        double _mod25r = 0, _mod25i = 0;
        _mod25r = fmod(j, _c24r); _mod25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _mod25r + _c26r; _add27i = _mod25i + _c26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul23r, _mul23i, _add27r, _add27i, &_mul28r, &_mul28i);
        double mag = _mul28r; /* +_mul28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _ang29r + _ang30r; _add31i = _ang29i + _ang30i;
        double _sin32r = 0, _sin32i = 0;
        c_sin(j, 0, &_sin32r, &_sin32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(j, 0, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin32r, _sin32i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add31r + _mul34r; _add35i = _add31i + _mul34i;
        double angle = _add35r; /* +_add35ii */
        double _cos36r = 0, _cos36i = 0;
        c_cos(angle, 0, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(r, 0, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(angle, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(im, 0, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _cplx40r = 0, _cplx40i = 0;
        _cplx40r = _mul37r; _cplx40i = _mul39r;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_cplx40r, _cplx40i, mag, 0, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int k = 1; k < 36; k++) {
        double _cf42r = 0, _cf42i = 0;
        { int _idx = ((k + 3) % 35); if (_idx >= 0 && _idx < 35) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
        double _conj43r = 0, _conj43i = 0;
        _conj43r = _cf42r; _conj43i = -(_cf42i);
        double _c44r = 0, _c44i = 0;
        _c44r = 2.0; _c44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(k, 0, _c44r, _c44i, &_div45r, &_div45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_div45r, _div45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_conj43r, _conj43i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(k, 0, &_cos48r, &_cos48i);
        double _sub49r = 0, _sub49i = 0;
        _sub49r = _mul47r - _cos48r; _sub49i = _mul47i - _cos48i;
        cRe[(k - 1)] += _sub49r; cIm[(k - 1)] += _sub49i;
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_367_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 4.0; _c4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _c4r, _c4i, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mul5r + _c6r; _add7i = _mul5i + _c6i;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(_add7r, n); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
        double k = _add10r; /* +_add10ii */
        double _c11r = 0, _c11i = 0;
        _c11r = 3.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _c11r; _add12i = 0 + _c11i;
        double _mod13r = 0, _mod13i = 0;
        _mod13r = fmod(_add12r, n); _mod13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _mod13r + _c14r; _add15i = _mod13i + _c14i;
        double r = _add15r; /* +_add15ii */
        double _re16r = 0, _re16i = 0;
        _re16r = x1r; _re16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(j, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_re16r, _re16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _re19r = 0, _re19i = 0;
        _re19r = x2r; _re19i = 0;
        double _cos20r = 0, _cos20i = 0;
        c_cos(k, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_re19r, _re19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
        double rec = _add22r; /* +_add22ii */
        double _im23r = 0, _im23i = 0;
        _im23r = x1i; _im23i = 0;
        double _cos24r = 0, _cos24i = 0;
        c_cos(r, 0, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_im23r, _im23i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _im26r = 0, _im26i = 0;
        _im26r = x2i; _im26i = 0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(k, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_im26r, _im26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul25r - _mul28r; _sub29i = _mul25i - _mul28i;
        double imc = _sub29r; /* +_sub29ii */
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _abs30r + _c31r; _add32i = _abs30i + _c31i;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(rec, 0); _abs34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_log33r, _log33i, _abs34r, _abs34i, &_mul35r, &_mul35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(j, 0, &_sin36r, &_sin36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(r, 0, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_sin36r, _sin36i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul35r + _mul38r; _add39i = _mul35i + _mul38i;
        double _re40r = 0, _re40i = 0;
        _re40r = x1r; _re40i = 0;
        double _im41r = 0, _im41i = 0;
        _im41r = x2i; _im41i = 0;
        double _prod42r = 0, _prod42i = 0;
        c_mul(_re40r, _re40i, _im41r, _im41i, &_prod42r, &_prod42i);
        double _prod43r = 0, _prod43i = 0;
        c_mul(_prod42r, _prod42i, j, 0, &_prod43r, &_prod43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _add39r + _prod43r; _add44i = _add39i + _prod43i;
        double mag = _add44r; /* +_add44ii */
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x1r, x1i); _ang45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang45r, _ang45i, k, 0, &_mul46r, &_mul46i);
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x2r, x2i); _ang47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang47r, _ang47i, r, 0, &_mul48r, &_mul48i);
        double _sub49r = 0, _sub49i = 0;
        _sub49r = _mul46r - _mul48r; _sub49i = _mul46i - _mul48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(j, 0, M_PI, 0, &_mul50r, &_mul50i);
        double _div51r = 0, _div51i = 0;
        c_div(_mul50r, _mul50i, n, 0, &_div51r, &_div51i);
        double _sin52r = 0, _sin52i = 0;
        c_sin(_div51r, _div51i, &_sin52r, &_sin52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _sub49r + _sin52r; _add53i = _sub49i + _sin52i;
        double angle = _add53r; /* +_add53ii */
        double _cos54r = 0, _cos54i = 0;
        c_cos(angle, 0, &_cos54r, &_cos54i);
        double _c55r = 0, _c55i = 0;
        _c55r = 0.0; _c55i = 1.0;
        double _sin56r = 0, _sin56i = 0;
        c_sin(angle, 0, &_sin56r, &_sin56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_c55r, _c55i, _sin56r, _sin56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _cos54r + _mul57r; _add58i = _cos54i + _mul57i;
        double _mul59r = 0, _mul59i = 0;
        c_mul(mag, 0, _add58r, _add58i, &_mul59r, &_mul59i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_368_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + j; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x2r, x2i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _c7r + _sin10r; _add11i = _c7i + _sin10i;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log6r, _log6i, _add11r, _add11i, &_mul12r, &_mul12i);
        double mag = _mul12r; /* +_mul12ii */
        double _re13r = 0, _re13i = 0;
        _re13r = x1r; _re13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _re13r, _re13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _im16r, _im16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_mul(_sin18r, _sin18i, _sin18r, _sin18i, &_pow20r, &_pow20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _cos15r + _pow20r; _add21i = _cos15i + _pow20i;
        double angle = _add21r; /* +_add21ii */
        double _cos22r = 0, _cos22i = 0;
        c_cos(angle, 0, &_cos22r, &_cos22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _sin24r = 0, _sin24i = 0;
        c_sin(angle, 0, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c23r, _c23i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _cos22r + _mul25r; _add26i = _cos22i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag, 0, _add26r, _add26i, &_mul27r, &_mul27i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    double _c28r = 0, _c28i = 0;
    _c28r = 1.0; _c28i = 0;
    double _add29r = 0, _add29i = 0;
    _add29r = n + _c28r; _add29i = 0 + _c28i;
    for (int k = 1; k < (int)(_add29r); k++) {
        double _re30r = 0, _re30i = 0;
        _re30r = x1r; _re30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(_re30r, _re30i, k, &_pow31r, &_pow31i);
        double _im32r = 0, _im32i = 0;
        _im32r = x2i; _im32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_powr(_im32r, _im32i, k, &_pow33r, &_pow33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _pow31r - _pow33r; _sub34i = _pow31i - _pow33i;
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(k, 0, x2r, x2i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = x1r + _mul36r; _add37i = x1i + _mul36i;
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(_add37r, _add37i); _ang38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c35r, _c35i, _ang38r, _ang38i, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_sub34r, _sub34i, _exp40r, _exp40i, &_mul41r, &_mul41i);
        cRe[(k - 1)] += _mul41r; cIm[(k - 1)] += _mul41i;
    }
    double _c42r = 0, _c42i = 0;
    _c42r = 1.0; _c42i = 0;
    double _add43r = 0, _add43i = 0;
    _add43r = n + _c42r; _add43i = 0 + _c42i;
    for (int r = 1; r < (int)(_add43r); r++) {
        double _mul44r = 0, _mul44i = 0;
        c_mul(r, 0, x2r, x2i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = x1r + _mul44r; _add45i = x1i + _mul44i;
        double _abs46r = 0, _abs46i = 0;
        _abs46r = c_abs(_add45r, _add45i); _abs46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 10.0; _c48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(r, 0, _c48r, _c48i, &_div49r, &_div49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _c47r + _div49r; _add50i = _c47i + _div49i;
        double _pow51r = 0, _pow51i = 0;
        c_powr(_abs46r, _abs46i, _add50r, &_pow51r, &_pow51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul52r, &_mul52i);
        double _ang53r = 0, _ang53i = 0;
        _ang53r = c_arg(_mul52r, _mul52i); _ang53i = 0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(r, 0, _ang53r, _ang53i, &_mul54r, &_mul54i);
        double _cos55r = 0, _cos55i = 0;
        c_cos(_mul54r, _mul54i, &_cos55r, &_cos55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_pow51r, _pow51i, _cos55r, _cos55i, &_mul56r, &_mul56i);
        { double _tr = cRe[(r - 1)]*_mul56r - cIm[(r - 1)]*_mul56i; cIm[(r - 1)] = cRe[(r - 1)]*_mul56i + cIm[(r - 1)]*_mul56r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_369_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _c1r, _c1i, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mul2r + _c3r; _add4i = _mul2i + _c3i;
        double _c5r = 0, _c5i = 0;
        _c5r = 10.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(_add4r, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double k = _add8r; /* +_add8ii */
        double _re9r = 0, _re9i = 0;
        _re9r = x1r; _re9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 4.0; _c10i = 0;
        double _mod11r = 0, _mod11i = 0;
        _mod11r = fmod(j, _c10r); _mod11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _mod11r + _c12r; _add13i = _mod11i + _c12i;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_re9r, _re9i, _add13r, &_pow14r, &_pow14i);
        double _re15r = 0, _re15i = 0;
        _re15r = x2r; _re15i = 0;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs16r + j; _add17i = _abs16i + 0;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_re15r, _re15i, _log18r, _log18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _pow14r + _mul19r; _add20i = _pow14i + _mul19i;
        double r = _add20r; /* +_add20ii */
        double _im21r = 0, _im21i = 0;
        _im21r = x2i; _im21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(k, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin24r, _sin24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _im21r + _mul28r; _add29i = _im21i + _mul28i;
        double im_part = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang30r, _ang30i, j, 0, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang32r, _ang32i, k, 0, &_mul33r, &_mul33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul31r - _mul33r; _sub34i = _mul31i - _mul33i;
        double _sin35r = 0, _sin35i = 0;
        c_sin(j, 0, &_sin35r, &_sin35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(k, 0, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sin35r, _sin35i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sub34r + _mul37r; _add38i = _sub34i + _mul37i;
        double angle = _add38r; /* +_add38ii */
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(x1r, x1i); _abs39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 5.0; _c41i = 0;
        double _mod42r = 0, _mod42i = 0;
        _mod42r = fmod(j, _c41r); _mod42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = _c40r + _mod42r; _add43i = _c40i + _mod42i;
        double _pow44r = 0, _pow44i = 0;
        c_powr(_abs39r, _abs39i, _add43r, &_pow44r, &_pow44i);
        double _abs45r = 0, _abs45i = 0;
        _abs45r = c_abs(x2r, x2i); _abs45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 2.0; _c46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 3.0; _c47i = 0;
        double _mod48r = 0, _mod48i = 0;
        _mod48r = fmod(k, _c47r); _mod48i = 0;
        double _add49r = 0, _add49i = 0;
        _add49r = _c46r + _mod48r; _add49i = _c46i + _mod48i;
        double _pow50r = 0, _pow50i = 0;
        c_powr(_abs45r, _abs45i, _add49r, &_pow50r, &_pow50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _pow44r + _pow50r; _add51i = _pow44i + _pow50i;
        double magnitude = _add51r; /* +_add51ii */
        double _cos52r = 0, _cos52i = 0;
        c_cos(angle, 0, &_cos52r, &_cos52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 0.0; _c53i = 1.0;
        double _sin54r = 0, _sin54i = 0;
        c_sin(angle, 0, &_sin54r, &_sin54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c53r, _c53i, _sin54r, _sin54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _cos52r + _mul55r; _add56i = _cos52i + _mul55i;
        double _mul57r = 0, _mul57i = 0;
        c_mul(magnitude, 0, _add56r, _add56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = x1r + x2r; _add58i = x1i + x2i;
        double _conj59r = 0, _conj59i = 0;
        _conj59r = _add58r; _conj59i = -(_add58i);
        double _pow60r = 0, _pow60i = 0;
        c_powr(_conj59r, _conj59i, k, &_pow60r, &_pow60i);
        double _add61r = 0, _add61i = 0;
        _add61r = _mul57r + _pow60r; _add61i = _mul57i + _pow60i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add61r; cIm[_idx] = _add61i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_370: too complex for auto-transpile, stubbed */
static void poly_370_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_371_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + j; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _sqrt7r = 0, _sqrt7i = 0;
        c_powr(j, 0, 0.5, &_sqrt7r, &_sqrt7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log6r, _log6i, _sqrt7r, _sqrt7i, &_mul8r, &_mul8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_sin11r, _sin11i, _sin11r, _sin11i, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _pow13r; _add14i = _mul8i + _pow13i;
        double mag_part = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _cos16r = 0, _cos16i = 0;
        c_cos(j, 0, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _sin19r = 0, _sin19i = 0;
        c_sin(j, 0, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
        double angle_part = _add21r; /* +_add21ii */
        double _cos22r = 0, _cos22i = 0;
        c_cos(angle_part, 0, &_cos22r, &_cos22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _sin24r = 0, _sin24i = 0;
        c_sin(angle_part, 0, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c23r, _c23i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _cos22r + _mul25r; _add26i = _cos22i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag_part, 0, _add26r, _add26i, &_mul27r, &_mul27i);
        double _conj28r = 0, _conj28i = 0;
        _conj28r = x1r; _conj28i = -(x1i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(_conj28r, _conj28i, j, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul27r + _pow29r; _add30i = _mul27i + _pow29i;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x2r, x2i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _abs31r + _c32r; _add33i = _abs31i + _c32i;
        double _log34r = 0, _log34i = 0;
        c_log(_add33r, _add33i, &_log34r, &_log34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(j, 0, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_log34r, _log34i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _add30r - _mul36r; _sub37i = _add30i - _mul36i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub37r; cIm[_idx] = _sub37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_372_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int k = 1; k < (int)(_add7r); k++) {
        double _mul8r = 0, _mul8i = 0;
        c_mul(x1r, x1i, k, 0, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_mul8r, _mul8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang13r, _ang13i, k, 0, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log12r, _log12i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double mag1 = _mul16r; /* +_mul16ii */
        double _div17r = 0, _div17i = 0;
        c_div(x2r, x2i, k, 0, &_div17r, &_div17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_div17r, _div17i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = k + _c23r; _add24i = 0 + _c23i;
        double _div25r = 0, _div25i = 0;
        c_div(_ang22r, _ang22i, _add24r, _add24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log21r, _log21i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double mag2 = _mul27r; /* +_mul27ii */
        double _add28r = 0, _add28i = 0;
        _add28r = mag1 + mag2; _add28i = 0 + 0;
        double mag = _add28r; /* +_add28ii */
        double _arr29r = 0, _arr29i = 0;
        { int _idx = (k - 1); _arr29r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr29i = 0; }
        double _mul30r = 0, _mul30i = 0;
        c_mul(_arr29r, _arr29i, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = k + _c31r; _add32i = 0 + _c31i;
        double _div33r = 0, _div33i = 0;
        c_div(_mul30r, _mul30i, _add32r, _add32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _arr35r = 0, _arr35i = 0;
        { int _idx = (k - 1); _arr35r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr35i = 0; }
        double _mul36r = 0, _mul36i = 0;
        c_mul(_arr35r, _arr35i, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 3.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = k + _c37r; _add38i = 0 + _c37i;
        double _div39r = 0, _div39i = 0;
        c_div(_mul36r, _mul36i, _add38r, _add38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _sin34r + _cos40r; _add41i = _sin34i + _cos40i;
        double angle = _add41r; /* +_add41ii */
        double _cos42r = 0, _cos42i = 0;
        c_cos(angle, 0, &_cos42r, &_cos42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _sin44r = 0, _sin44i = 0;
        c_sin(angle, 0, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c43r, _c43i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _cos42r + _mul45r; _add46i = _cos42i + _mul45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(mag, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_373_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re1r + (_re2r - _re1r) * _li / 34.0;
    }
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im3r + (_im4r - _im3r) * _li / 34.0;
    }
    for (int j = 1; j < 36; j++) {
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs5r + j; _add6i = _abs5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 7.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log7r, _log7i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 11.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _re17r = 0, _re17i = 0;
        _re17r = x2r; _re17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_cos16r, _cos16i, _re17r, _re17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul12r + _mul18r; _add19i = _mul12i + _mul18i;
        double mag_part = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang21r, _ang21i, j, 0, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _ang20r + _mul22r; _add23i = _ang20i + _mul22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 13.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _add23r + _sin27r; _add28i = _add23i + _sin27i;
        double ang_part = _add28r; /* +_add28ii */
        double _im29r = 0, _im29i = 0;
        _im29r = x1i; _im29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_im29r, _im29i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = mag_part + _mul34r; _add35i = 0 + _mul34i;
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, ang_part, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_add35r, _add35i, _exp38r, _exp38i, &_mul39r, &_mul39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x2r; _conj40i = -(x2i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, M_PI, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 17.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(_mul41r, _mul41i, _c42r, _c42i, &_div43r, &_div43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_div43r, _div43i, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_conj40r, _conj40i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul39r + _mul45r; _add46i = _mul39i + _mul45i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_374_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _re5r = 0, _re5i = 0;
        _re5r = x2r; _re5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_re5r, _re5i, j, 0, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _re4r + _mul6r; _add7i = _re4i + _mul6i;
        double r = _add7r; /* +_add7ii */
        double _im8r = 0, _im8i = 0;
        _im8r = x1i; _im8i = 0;
        double _im9r = 0, _im9i = 0;
        _im9r = x2i; _im9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_im9r, _im9i, j, 0, &_mul10r, &_mul10i);
        double _sub11r = 0, _sub11i = 0;
        _sub11r = _im8r - _mul10r; _sub11i = _im8i - _mul10i;
        double im = _sub11r; /* +_sub11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_mul(j, 0, j, 0, &_pow14r, &_pow14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _abs12r + _pow14r; _add15i = _abs12i + _pow14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 7.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_sin21r, _sin21i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _c17r + _mul26r; _add27i = _c17i + _mul26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log16r, _log16i, _add27r, _add27i, &_mul28r, &_mul28i);
        double mag = _mul28r; /* +_mul28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _cos30r = 0, _cos30i = 0;
        c_cos(j, 0, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(j, 0, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang32r, _ang32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
        double angle = _add35r; /* +_add35ii */
        double _cos36r = 0, _cos36i = 0;
        c_cos(angle, 0, &_cos36r, &_cos36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(angle, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c37r, _c37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _cos36r + _mul39r; _add40i = _cos36i + _mul39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(mag, 0, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_375_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(j, 0, j, 0, &_pow3r, &_pow3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(_re1r, _re1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
        double _re5r = 0, _re5i = 0;
        _re5r = x2r; _re5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _div8r = 0, _div8i = 0;
        c_div(_re5r, _re5i, _add7r, _add7i, &_div8r, &_div8i);
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _mul4r - _div8r; _sub9i = _mul4i - _div8i;
        double r = _sub9r; /* +_sub9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x2i; _im10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _abs11r; _add12i = 0 + _abs11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_im10r, _im10i, _log13r, _log13i, &_mul14r, &_mul14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, _ang18r, _ang18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_sin17r, _sin17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul14r + _mul21r; _add22i = _mul14i + _mul21i;
        double im = _add22r; /* +_add22ii */
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, im, 0, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = r + _mul24r; _add25i = 0 + _mul24i;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _re27r = 0, _re27i = 0;
        _re27r = x1r; _re27i = 0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(j, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_re27r, _re27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _im30r = 0, _im30i = 0;
        _im30r = x2i; _im30i = 0;
        double _cos31r = 0, _cos31i = 0;
        c_cos(j, 0, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_im30r, _im30i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul29r + _mul32r; _add33i = _mul29i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c26r, _c26i, _add33r, _add33i, &_mul34r, &_mul34i);
        double _exp35r = 0, _exp35i = 0;
        c_exp2(_mul34r, _mul34i, &_exp35r, &_exp35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_add25r, _add25i, _exp35r, _exp35i, &_mul36r, &_mul36i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_376_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double mag = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double angle = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        for (int k = 1; k < (int)(_add4r); k++) {
            double _re5r = 0, _re5i = 0;
            _re5r = x1r; _re5i = 0;
            double _c6r = 0, _c6i = 0;
            _c6r = 1.0; _c6i = 0;
            double _add7r = 0, _add7i = 0;
            _add7r = k + _c6r; _add7i = 0 + _c6i;
            double _log8r = 0, _log8i = 0;
            c_log(_add7r, _add7i, &_log8r, &_log8i);
            double _mul9r = 0, _mul9i = 0;
            c_mul(_re5r, _re5i, _log8r, _log8i, &_mul9r, &_mul9i);
            double _c10r = 0, _c10i = 0;
            _c10r = 0.5; _c10i = 0;
            double _pow11r = 0, _pow11i = 0;
            c_powr(k, 0, 0.5, &_pow11r, &_pow11i);
            double _div12r = 0, _div12i = 0;
            c_div(_mul9r, _mul9i, _pow11r, _pow11i, &_div12r, &_div12i);
            mag += _div12r;
            double _ang13r = 0, _ang13i = 0;
            _ang13r = c_arg(x2r, x2i); _ang13i = 0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(k, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
            double _sin15r = 0, _sin15i = 0;
            c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
            double _re16r = 0, _re16i = 0;
            _re16r = x1r; _re16i = 0;
            double _mul17r = 0, _mul17i = 0;
            c_mul(k, 0, _re16r, _re16i, &_mul17r, &_mul17i);
            double _cos18r = 0, _cos18i = 0;
            c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
            double _add19r = 0, _add19i = 0;
            _add19r = _sin15r + _cos18r; _add19i = _sin15i + _cos18i;
            angle += _add19r;
        }
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c20r, _c20i, angle, 0, &_mul21r, &_mul21i);
        double _exp22r = 0, _exp22i = 0;
        c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(mag, 0, _exp22r, _exp22i, &_mul23r, &_mul23i);
        double _conj24r = 0, _conj24i = 0;
        _conj24r = x1r; _conj24i = -(x1i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(j, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_conj24r, _conj24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul23r + _mul26r; _add27i = _mul23i + _mul26i;
        double _conj28r = 0, _conj28i = 0;
        _conj28r = x2r; _conj28i = -(x2i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(j, 0, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_conj28r, _conj28i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _add27r - _mul30r; _sub31i = _add27i - _mul30i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub31r; cIm[_idx] = _sub31i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_377_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x1r, x1i); _ang4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_ang4r, _ang4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x2r, x2i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_sin9r, _sin9i, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _div14r = 0, _div14i = 0;
        c_div(_mul11r, _mul11i, _add13r, _add13i, &_div14r, &_div14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul8r + _div14r; _add15i = _mul8i + _div14i;
        double angle = _add15r; /* +_add15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _sqrt17r = 0, _sqrt17i = 0;
        c_powr(j, 0, 0.5, &_sqrt17r, &_sqrt17i);
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs16r, _abs16i, _sqrt17r, &_pow18r, &_pow18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_c21r, _c21i, j, 0, &_div22r, &_div22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _c20r + _div22r; _add23i = _c20i + _div22i;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_abs19r, _abs19i, _add23r, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _pow18r + _pow24r; _add25i = _pow18i + _pow24i;
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(n, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = j - _div27r; _sub28i = 0 - _div27i;
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(_sub28r, _sub28i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs29r + _c30r; _add31i = _abs29i + _c30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _add25r + _log32r; _add33i = _add25i + _log32i;
        double magnitude = _add33r; /* +_add33ii */
        double _cos34r = 0, _cos34i = 0;
        c_cos(angle, 0, &_cos34r, &_cos34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _sin36r = 0, _sin36i = 0;
        c_sin(angle, 0, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c35r, _c35i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cos34r + _mul37r; _add38i = _cos34i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x1r; _conj40i = -(x1i);
        double _re41r = 0, _re41i = 0;
        _re41r = x2r; _re41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj40r, _conj40i, _re41r, _re41i, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = j + _c43r; _add44i = 0 + _c43i;
        double _div45r = 0, _div45i = 0;
        c_div(_mul42r, _mul42i, _add44r, _add44i, &_div45r, &_div45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul39r + _div45r; _add46i = _mul39i + _div45i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_378_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 1.0;
        double _c9r = 0, _c9i = 0;
        _c9r = 5.0; _c9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_c9r, _c9i, M_PI, 0, &_mul10r, &_mul10i);
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr11i = 0; }
        double _mul12r = 0, _mul12i = 0;
        c_mul(_mul10r, _mul10i, _arr11r, _arr11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_c8r, _c8i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _exp15r = 0, _exp15i = 0;
        c_exp2(_mul14r, _mul14i, &_exp15r, &_exp15i);
        double term1 = _exp15r; /* +_exp15ii */
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c17r, _c17i, M_PI, 0, &_mul18r, &_mul18i);
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr19i = 0; }
        double _mul20r = 0, _mul20i = 0;
        c_mul(_mul18r, _mul18i, _arr19r, _arr19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c16r, _c16i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _exp23r = 0, _exp23i = 0;
        c_exp2(_mul22r, _mul22i, &_exp23r, &_exp23i);
        double term2 = _exp23r; /* +_exp23ii */
        double _arr24r = 0, _arr24i = 0;
        { int _idx = (j - 1); _arr24r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr24i = 0; }
        double _arr25r = 0, _arr25i = 0;
        { int _idx = (j - 1); _arr25r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr25i = 0; }
        double _mul26r = 0, _mul26i = 0;
        c_mul(_arr24r, _arr24i, _arr25r, _arr25i, &_mul26r, &_mul26i);
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_mul26r, _mul26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _abs27r + _c28r; _add29i = _abs27i + _c28i;
        double _log30r = 0, _log30i = 0;
        c_log(_add29r, _add29i, &_log30r, &_log30i);
        double term3 = _log30r; /* +_log30ii */
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(j, 0, j, 0, &_pow32r, &_pow32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(term3, 0, _pow32r, _pow32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul33r + 0; _add34i = _mul33i + 0;
        double mag = _add34r; /* +_add34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(term1, 0); _ang35i = 0;
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(term2, 0); _ang36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _ang35r + _ang36r; _add37i = _ang35i + _ang36i;
        double _add38r = 0, _add38i = 0;
        _add38r = x1r + x2r; _add38i = x1i + x2i;
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(_add38r, _add38i); _ang39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, _ang39r, _ang39i, &_mul40r, &_mul40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_mul40r, _mul40i, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _add37r + _sin41r; _add42i = _add37i + _sin41i;
        double angle = _add42r; /* +_add42ii */
        double _cos43r = 0, _cos43i = 0;
        c_cos(angle, 0, &_cos43r, &_cos43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _sin45r = 0, _sin45i = 0;
        c_sin(angle, 0, &_sin45r, &_sin45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c44r, _c44i, _sin45r, _sin45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _cos43r + _mul46r; _add47i = _cos43i + _mul46i;
        double _mul48r = 0, _mul48i = 0;
        c_mul(mag, 0, _add47r, _add47i, &_mul48r, &_mul48i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_379_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + j; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 6.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _c7r + _sin11r; _add12i = _c7i + _sin11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log6r, _log6i, _add12r, _add12i, &_mul13r, &_mul13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 4.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_cos17r, _cos17i, _abs18r, _abs18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul13r + _mul19r; _add20i = _mul13i + _mul19i;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 5.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
        double angle = _add31r; /* +_add31ii */
        double _cos32r = 0, _cos32i = 0;
        c_cos(angle, 0, &_cos32r, &_cos32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _sin34r = 0, _sin34i = 0;
        c_sin(angle, 0, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c33r, _c33i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _cos32r + _mul35r; _add36i = _cos32i + _mul35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(magnitude, 0, _add36r, _add36i, &_mul37r, &_mul37i);
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x1r; _conj38i = -(x1i);
        double _c39r = 0, _c39i = 0;
        _c39r = 7.0; _c39i = 0;
        double _mod40r = 0, _mod40i = 0;
        _mod40r = fmod(j, _c39r); _mod40i = 0;
        double _pow41r = 0, _pow41i = 0;
        c_powr(_conj38r, _conj38i, _mod40r, &_pow41r, &_pow41i);
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x2r, x2i); _ang42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(j, 0, _ang42r, _ang42i, &_mul43r, &_mul43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_mul43r, _mul43i, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_pow41r, _pow41i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul37r + _mul45r; _add46i = _mul37i + _mul45i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_380_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr8i = 0; }
        double r = _arr8r; /* +_arr8ii */
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (j - 1); _arr9r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr9i = 0; }
        double m = _arr9r; /* +_arr9ii */
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_mul(r, 0, r, 0, &_pow11r, &_pow11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(m, 0, m, 0, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _pow11r + _pow13r; _add14i = _pow11i + _pow13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(_add14r, _add14i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _sin21r = 0, _sin21i = 0;
        c_sin(r, 0, &_sin21r, &_sin21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(m, 0, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _sin21r + _cos22r; _add23i = _sin21i + _cos22i;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_add20r, _add20i, _add23r, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log18r, _log18i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double mag = _mul25r; /* +_mul25ii */
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, r, 0, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, m, 0, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _sin27r + _cos29r; _add30i = _sin27i + _cos29i;
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(m, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _add30r + _mul33r; _add34i = _add30i + _mul33i;
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _cos36r = 0, _cos36i = 0;
        c_cos(r, 0, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang35r, _ang35i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _add34r - _mul37r; _sub38i = _add34i - _mul37i;
        double angle = _sub38r; /* +_sub38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(mag, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_381_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double rec_t1 = _re2r; /* +_re2ii */
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_t2 = _re3r; /* +_re3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double imc_t1 = _im4r; /* +_im4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_t2 = _im5r; /* +_im5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 7.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 5.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sin11r, _sin11i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double phase = _mul16r; /* +_mul16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(j, 0, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log20r, _log20i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _c24r; _add25i = _abs23i + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log26r, _log26i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul22r + _mul28r; _add29i = _mul22i + _mul28i;
        double magnitude = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.5; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_powr(j, 0, 0.5, &_pow32r, &_pow32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang30r, _ang30i, _pow32r, _pow32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = j + _c35r; _add36i = 0 + _c35i;
        double _div37r = 0, _div37i = 0;
        c_div(_ang34r, _ang34i, _add36r, _add36i, &_div37r, &_div37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul33r + _div37r; _add38i = _mul33i + _div37i;
        double angle = _add38r; /* +_add38ii */
        double _add39r = 0, _add39i = 0;
        _add39r = angle + phase; _add39i = 0 + 0;
        double _cos40r = 0, _cos40i = 0;
        c_cos(_add39r, _add39i, &_cos40r, &_cos40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _sub42r = 0, _sub42i = 0;
        _sub42r = angle - phase; _sub42i = 0 - 0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(_sub42r, _sub42i, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c41r, _c41i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos40r + _mul44r; _add45i = _cos40i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        double _conj47r = 0, _conj47i = 0;
        _conj47r = x1r; _conj47i = -(x1i);
        double _pow48r = 0, _pow48i = 0;
        c_powr(_conj47r, _conj47i, j, &_pow48r, &_pow48i);
        double _re49r = 0, _re49i = 0;
        _re49r = x2r; _re49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_pow48r, _pow48i, _re49r, _re49i, &_mul50r, &_mul50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 2.0; _c51i = 0;
        double _add52r = 0, _add52i = 0;
        _add52r = j + _c51r; _add52i = 0 + _c51i;
        double _div53r = 0, _div53i = 0;
        c_div(_mul50r, _mul50i, _add52r, _add52i, &_div53r, &_div53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _mul46r + _div53r; _add54i = _mul46i + _div53i;
        double _add55r = 0, _add55i = 0;
        _add55r = x1r + x2r; _add55i = x1i + x2i;
        double _im56r = 0, _im56i = 0;
        _im56r = _add55i; _im56i = 0;
        double _ang57r = 0, _ang57i = 0;
        _ang57r = c_arg(x1r, x1i); _ang57i = 0;
        double _mul58r = 0, _mul58i = 0;
        c_mul(j, 0, _ang57r, _ang57i, &_mul58r, &_mul58i);
        double _cos59r = 0, _cos59i = 0;
        c_cos(_mul58r, _mul58i, &_cos59r, &_cos59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_im56r, _im56i, _cos59r, _cos59i, &_mul60r, &_mul60i);
        double _ang61r = 0, _ang61i = 0;
        _ang61r = c_arg(x2r, x2i); _ang61i = 0;
        double _mul62r = 0, _mul62i = 0;
        c_mul(j, 0, _ang61r, _ang61i, &_mul62r, &_mul62i);
        double _sin63r = 0, _sin63i = 0;
        c_sin(_mul62r, _mul62i, &_sin63r, &_sin63i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(_mul60r, _mul60i, _sin63r, _sin63i, &_mul64r, &_mul64i);
        double _add65r = 0, _add65i = 0;
        _add65r = _add54r + _mul64r; _add65i = _add54i + _mul64i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add65r; cIm[_idx] = _add65i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_382_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double coeff_real = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        double coeff_imag = _c5r; /* +_c5ii */
        for (int k = 1; k < 6; k++) {
            double _re6r = 0, _re6i = 0;
            _re6r = x1r; _re6i = 0;
            double _pow7r = 0, _pow7i = 0;
            c_powr(_re6r, _re6i, k, &_pow7r, &_pow7i);
            double _mul8r = 0, _mul8i = 0;
            c_mul(j, 0, k, 0, &_mul8r, &_mul8i);
            double _mul9r = 0, _mul9i = 0;
            c_mul(_mul8r, _mul8i, M_PI, 0, &_mul9r, &_mul9i);
            double _c10r = 0, _c10i = 0;
            _c10r = 4.0; _c10i = 0;
            double _div11r = 0, _div11i = 0;
            c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
            double _sin12r = 0, _sin12i = 0;
            c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_pow7r, _pow7i, _sin12r, _sin12i, &_mul13r, &_mul13i);
            double _re14r = 0, _re14i = 0;
            _re14r = x2r; _re14i = 0;
            double _c15r = 0, _c15i = 0;
            _c15r = 2.0; _c15i = 0;
            double _div16r = 0, _div16i = 0;
            c_div(k, 0, _c15r, _c15i, &_div16r, &_div16i);
            double _pow17r = 0, _pow17i = 0;
            c_powr(_re14r, _re14i, _div16r, &_pow17r, &_pow17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(j, 0, k, 0, &_mul18r, &_mul18i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(_mul18r, _mul18i, M_PI, 0, &_mul19r, &_mul19i);
            double _c20r = 0, _c20i = 0;
            _c20r = 3.0; _c20i = 0;
            double _div21r = 0, _div21i = 0;
            c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
            double _cos22r = 0, _cos22i = 0;
            c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
            double _mul23r = 0, _mul23i = 0;
            c_mul(_pow17r, _pow17i, _cos22r, _cos22i, &_mul23r, &_mul23i);
            double _add24r = 0, _add24i = 0;
            _add24r = _mul13r + _mul23r; _add24i = _mul13i + _mul23i;
            coeff_real += _add24r;
            double _im25r = 0, _im25i = 0;
            _im25r = x1i; _im25i = 0;
            double _pow26r = 0, _pow26i = 0;
            c_powr(_im25r, _im25i, k, &_pow26r, &_pow26i);
            double _mul27r = 0, _mul27i = 0;
            c_mul(j, 0, k, 0, &_mul27r, &_mul27i);
            double _mul28r = 0, _mul28i = 0;
            c_mul(_mul27r, _mul27i, M_PI, 0, &_mul28r, &_mul28i);
            double _c29r = 0, _c29i = 0;
            _c29r = 5.0; _c29i = 0;
            double _div30r = 0, _div30i = 0;
            c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
            double _cos31r = 0, _cos31i = 0;
            c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
            double _mul32r = 0, _mul32i = 0;
            c_mul(_pow26r, _pow26i, _cos31r, _cos31i, &_mul32r, &_mul32i);
            double _im33r = 0, _im33i = 0;
            _im33r = x2i; _im33i = 0;
            double _c34r = 0, _c34i = 0;
            _c34r = 2.0; _c34i = 0;
            double _div35r = 0, _div35i = 0;
            c_div(k, 0, _c34r, _c34i, &_div35r, &_div35i);
            double _pow36r = 0, _pow36i = 0;
            c_powr(_im33r, _im33i, _div35r, &_pow36r, &_pow36i);
            double _mul37r = 0, _mul37i = 0;
            c_mul(j, 0, k, 0, &_mul37r, &_mul37i);
            double _mul38r = 0, _mul38i = 0;
            c_mul(_mul37r, _mul37i, M_PI, 0, &_mul38r, &_mul38i);
            double _c39r = 0, _c39i = 0;
            _c39r = 6.0; _c39i = 0;
            double _div40r = 0, _div40i = 0;
            c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
            double _sin41r = 0, _sin41i = 0;
            c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
            double _mul42r = 0, _mul42i = 0;
            c_mul(_pow36r, _pow36i, _sin41r, _sin41i, &_mul42r, &_mul42i);
            double _sub43r = 0, _sub43i = 0;
            _sub43r = _mul32r - _mul42r; _sub43i = _mul32i - _mul42i;
            coeff_imag += _sub43r;
        }
        for (int r = 1; r < 4; r++) {
            double _add44r = 0, _add44i = 0;
            _add44r = x1r + r; _add44i = x1i + 0;
            double _abs45r = 0, _abs45i = 0;
            _abs45r = c_abs(_add44r, _add44i); _abs45i = 0;
            double _c46r = 0, _c46i = 0;
            _c46r = 1.0; _c46i = 0;
            double _add47r = 0, _add47i = 0;
            _add47r = _abs45r + _c46r; _add47i = _abs45i + _c46i;
            double _log48r = 0, _log48i = 0;
            c_log(_add47r, _add47i, &_log48r, &_log48i);
            double _mul49r = 0, _mul49i = 0;
            c_mul(j, 0, r, 0, &_mul49r, &_mul49i);
            double _mul50r = 0, _mul50i = 0;
            c_mul(_mul49r, _mul49i, M_PI, 0, &_mul50r, &_mul50i);
            double _c51r = 0, _c51i = 0;
            _c51r = 7.0; _c51i = 0;
            double _div52r = 0, _div52i = 0;
            c_div(_mul50r, _mul50i, _c51r, _c51i, &_div52r, &_div52i);
            double _sin53r = 0, _sin53i = 0;
            c_sin(_div52r, _div52i, &_sin53r, &_sin53i);
            double _mul54r = 0, _mul54i = 0;
            c_mul(_log48r, _log48i, _sin53r, _sin53i, &_mul54r, &_mul54i);
            double _add55r = 0, _add55i = 0;
            _add55r = x1r + r; _add55i = x1i + 0;
            double _ang56r = 0, _ang56i = 0;
            _ang56r = c_arg(_add55r, _add55i); _ang56i = 0;
            double _mul57r = 0, _mul57i = 0;
            c_mul(_mul54r, _mul54i, _ang56r, _ang56i, &_mul57r, &_mul57i);
            coeff_real += _mul57r;
            double _sub58r = 0, _sub58i = 0;
            _sub58r = x2r - r; _sub58i = x2i - 0;
            double _abs59r = 0, _abs59i = 0;
            _abs59r = c_abs(_sub58r, _sub58i); _abs59i = 0;
            double _c60r = 0, _c60i = 0;
            _c60r = 1.0; _c60i = 0;
            double _add61r = 0, _add61i = 0;
            _add61r = _abs59r + _c60r; _add61i = _abs59i + _c60i;
            double _log62r = 0, _log62i = 0;
            c_log(_add61r, _add61i, &_log62r, &_log62i);
            double _mul63r = 0, _mul63i = 0;
            c_mul(j, 0, r, 0, &_mul63r, &_mul63i);
            double _mul64r = 0, _mul64i = 0;
            c_mul(_mul63r, _mul63i, M_PI, 0, &_mul64r, &_mul64i);
            double _c65r = 0, _c65i = 0;
            _c65r = 8.0; _c65i = 0;
            double _div66r = 0, _div66i = 0;
            c_div(_mul64r, _mul64i, _c65r, _c65i, &_div66r, &_div66i);
            double _cos67r = 0, _cos67i = 0;
            c_cos(_div66r, _div66i, &_cos67r, &_cos67i);
            double _mul68r = 0, _mul68i = 0;
            c_mul(_log62r, _log62i, _cos67r, _cos67i, &_mul68r, &_mul68i);
            double _sub69r = 0, _sub69i = 0;
            _sub69r = x2r - r; _sub69i = x2i - 0;
            double _ang70r = 0, _ang70i = 0;
            _ang70r = c_arg(_sub69r, _sub69i); _ang70i = 0;
            double _mul71r = 0, _mul71i = 0;
            c_mul(_mul68r, _mul68i, _ang70r, _ang70i, &_mul71r, &_mul71i);
            coeff_imag += _mul71r;
        }
        double _c72r = 0, _c72i = 0;
        _c72r = 0.0; _c72i = 1.0;
        double _mul73r = 0, _mul73i = 0;
        c_mul(_c72r, _c72i, coeff_imag, 0, &_mul73r, &_mul73i);
        double _add74r = 0, _add74i = 0;
        _add74r = coeff_real + _mul73r; _add74i = 0 + _mul73i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add74r; cIm[_idx] = _add74i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_383_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + j; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _re7r = 0, _re7i = 0;
        _re7r = x2r; _re7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _re7r, _re7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log6r, _log6i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _c11r; _add12i = 0 + _c11i;
        double _div13r = 0, _div13i = 0;
        c_div(0, 0, _add12r, _add12i, &_div13r, &_div13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul10r + _div13r; _add14i = _mul10i + _div13i;
        double mag_part = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _im16r, _im16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang15r, _ang15i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin20r, _sin20i, _ang21r, _ang21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul19r + _mul22r; _add23i = _mul19i + _mul22i;
        double angle_part = _add23r; /* +_add23ii */
        double _cos24r = 0, _cos24i = 0;
        c_cos(angle_part, 0, &_cos24r, &_cos24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(angle_part, 0, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_c25r, _c25i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _cos24r + _mul27r; _add28i = _cos24i + _mul27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(mag_part, 0, _add28r, _add28i, &_mul29r, &_mul29i);
        double _conj30r = 0, _conj30i = 0;
        _conj30r = x1r; _conj30i = -(x1i);
        double _pow31r = 0, _pow31i = 0;
        c_powr(_conj30r, _conj30i, j, &_pow31r, &_pow31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul29r + _pow31r; _add32i = _mul29i + _pow31i;
        double _conj33r = 0, _conj33i = 0;
        _conj33r = x2r; _conj33i = -(x2i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = n - j; _sub34i = 0 - 0;
        double _pow35r = 0, _pow35i = 0;
        c_powr(_conj33r, _conj33i, _sub34r, &_pow35r, &_pow35i);
        double _sub36r = 0, _sub36i = 0;
        _sub36r = _add32r - _pow35r; _sub36i = _add32i - _pow35i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_384_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _re2r = 0, _re2i = 0;
        _re2r = x2r; _re2i = 0;
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _sub4r = 0, _sub4i = 0;
        _sub4r = _re2r - _re3r; _sub4i = _re2i - _re3i;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_sub4r, _sub4i, j, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 35.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _re1r + _div7r; _add8i = _re1i + _div7i;
        double r = _add8r; /* +_add8ii */
        double _im9r = 0, _im9i = 0;
        _im9r = x1i; _im9i = 0;
        double _im10r = 0, _im10i = 0;
        _im10r = x2i; _im10i = 0;
        double _im11r = 0, _im11i = 0;
        _im11r = x1i; _im11i = 0;
        double _sub12r = 0, _sub12i = 0;
        _sub12r = _im10r - _im11r; _sub12i = _im10i - _im11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sub12r, _sub12i, j, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 35.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _im9r + _div15r; _add16i = _im9i + _div15i;
        double im = _add16r; /* +_add16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs17r + j; _add18i = _abs17i + 0;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(r, 0, j, 0, &_mul20r, &_mul20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_mul20r, _mul20i, &_sin21r, &_sin21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_sin21r, _sin21i); _abs22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log19r, _log19i, _abs22r, _abs22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul23r + 0; _add24i = _mul23i + 0;
        double magnitude = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _cos26r = 0, _cos26i = 0;
        c_cos(j, 0, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(im, 0, j, 0, &_mul29r, &_mul29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_mul29r, _mul29i, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang28r, _ang28i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul27r + _mul31r; _add32i = _mul27i + _mul31i;
        double angle = _add32r; /* +_add32ii */
        double _cos33r = 0, _cos33i = 0;
        c_cos(angle, 0, &_cos33r, &_cos33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _sin35r = 0, _sin35i = 0;
        c_sin(angle, 0, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c34r, _c34i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _cos33r + _mul36r; _add37i = _cos33i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(magnitude, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_385_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int k = 1; k < (int)(_add3r); k++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_powr(_re4r, _re4i, k, &_pow5r, &_pow5i);
        double _re6r = 0, _re6i = 0;
        _re6r = x2r; _re6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = n - k; _sub7i = 0 - 0;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_re6r, _re6i, _sub7r, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow5r + _pow8r; _add9i = _pow5i + _pow8i;
        double real_part = _add9r; /* +_add9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 5.0; _c11i = 0;
        double _mod12r = 0, _mod12i = 0;
        _mod12r = fmod(k, _c11r); _mod12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _mod12r + _c13r; _add14i = _mod12i + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_im10r, _im10i, _add14r, &_pow15r, &_pow15i);
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _fdiv18r = 0, _fdiv18i = 0;
        c_div(k, 0, _c17r, _c17i, &_fdiv18r, &_fdiv18i);
        _fdiv18r = floor(_fdiv18r); _fdiv18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _fdiv18r + _c19r; _add20i = _fdiv18i + _c19i;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_im16r, _im16i, _add20r, &_pow21r, &_pow21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _pow15r - _pow21r; _sub22i = _pow15i - _pow21i;
        double imag_part = _sub22r; /* +_sub22ii */
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x1r, x1i); _abs23i = 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _abs24r; _add25i = _abs23i + _abs24i;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_add25r, _add25i, k, &_pow26r, &_pow26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = k + _c27r; _add28i = 0 + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow26r, _pow26i, _log29r, _log29i, &_mul30r, &_mul30i);
        double magnitude = _mul30r; /* +_mul30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(k, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _cos35r = 0, _cos35i = 0;
        c_cos(k, 0, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang34r, _ang34i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul33r + _mul36r; _add37i = _mul33i + _mul36i;
        double angle = _add37r; /* +_add37ii */
        double _cos38r = 0, _cos38i = 0;
        c_cos(angle, 0, &_cos38r, &_cos38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _sin40r = 0, _sin40i = 0;
        c_sin(angle, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c39r, _c39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos38r + _mul41r; _add42i = _cos38i + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_386_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _sin2r = 0, _sin2i = 0;
        c_sin(j, 0, &_sin2r, &_sin2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(_re1r, _re1i, _sin2r, _sin2i, &_mul3r, &_mul3i);
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(j, 0, _c5r, _c5i, &_div6r, &_div6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_div6r, _div6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re4r, _re4i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul3r + _mul8r; _add9i = _mul3i + _mul8i;
        double real_part = _add9r; /* +_add9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _cos11r = 0, _cos11i = 0;
        c_cos(j, 0, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_im10r, _im10i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _im13r = 0, _im13i = 0;
        _im13r = x2i; _im13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(j, 0, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_im13r, _im13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = _mul12r - _mul17r; _sub18i = _mul12i - _mul17i;
        double imag_part = _sub18r; /* +_sub18ii */
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_mul(real_part, 0, real_part, 0, &_pow20r, &_pow20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_mul(imag_part, 0, imag_part, 0, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _pow20r + _pow22r; _add23i = _pow20i + _pow22i;
        double _sqrt24r = 0, _sqrt24i = 0;
        c_powr(_add23r, _add23i, 0.5, &_sqrt24r, &_sqrt24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _abs25r; _add26i = 0 + _abs25i;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x2r, x2i); _abs27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _add26r + _abs27r; _add28i = _add26i + _abs27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_sqrt24r, _sqrt24i, _log29r, _log29i, &_mul30r, &_mul30i);
        double magnitude = _mul30r; /* +_mul30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _sqrt32r = 0, _sqrt32i = 0;
        c_powr(j, 0, 0.5, &_sqrt32r, &_sqrt32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _sqrt32r, _sqrt32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _cos35r = 0, _cos35i = 0;
        c_cos(j, 0, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang34r, _ang34i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul33r + _mul36r; _add37i = _mul33i + _mul36i;
        double angle = _add37r; /* +_add37ii */
        double _cos38r = 0, _cos38i = 0;
        c_cos(angle, 0, &_cos38r, &_cos38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _sin40r = 0, _sin40i = 0;
        c_sin(angle, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c39r, _c39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos38r + _mul41r; _add42i = _cos38i + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_387_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int k = 1; k < (int)(_add3r); k++) {
        double j = k; /* +0i */
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + k; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(j, 0, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log6r, _log6i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(j, 0, &_cos9r, &_cos9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_cos9r, _cos9i, _log13r, _log13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul8r + _mul14r; _add15i = _mul8i + _mul14i;
        double mag_part = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 0.5; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(j, 0, 0.5, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang16r, _ang16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _log23r, _log23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul19r + _mul24r; _add25i = _mul19i + _mul24i;
        double _re26r = 0, _re26i = 0;
        _re26r = x1r; _re26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _re26r, _re26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add25r + _sin28r; _add29i = _add25i + _sin28i;
        double _im30r = 0, _im30i = 0;
        _im30r = x2i; _im30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _im30r, _im30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _add29r - _cos32r; _sub33i = _add29i - _cos32i;
        double angle_part = _sub33r; /* +_sub33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, angle_part, 0, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(mag_part, 0, _exp36r, _exp36i, &_mul37r, &_mul37i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_388_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_re4r, _re4i, j, 0, &_mul5r, &_mul5i);
        double _re6r = 0, _re6i = 0;
        _re6r = x2r; _re6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _div9r = 0, _div9i = 0;
        c_div(_re6r, _re6i, _add8r, _add8i, &_div9r, &_div9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul5r + _div9r; _add10i = _mul5i + _div9i;
        double r_part = _add10r; /* +_add10ii */
        double _im11r = 0, _im11i = 0;
        _im11r = x1i; _im11i = 0;
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_im11r, _im11i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _im14r = 0, _im14i = 0;
        _im14r = x2i; _im14i = 0;
        double _cos15r = 0, _cos15i = 0;
        c_cos(j, 0, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_im14r, _im14i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul13r + _mul16r; _add17i = _mul13i + _mul16i;
        double i_part = _add17r; /* +_add17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs18r + j; _add19i = _abs18i + 0;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 5.0; _c21i = 0;
        double _mod22r = 0, _mod22i = 0;
        _mod22r = fmod(j, _c21r); _mod22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _mod22r + _c23r; _add24i = _mod22i + _c23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log20r, _log20i, _add24r, _add24i, &_mul25r, &_mul25i);
        double mag = _mul25r; /* +_mul25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 4.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul30r + _mul35r; _add36i = _mul30i + _mul35i;
        double angle = _add36r; /* +_add36ii */
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, i_part, 0, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = r_part + _mul38r; _add39i = 0 + _mul38i;
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, angle, 0, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_add39r, _add39i, _exp42r, _exp42i, &_mul43r, &_mul43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_mul43r, _mul43i, mag, 0, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_389_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double coeff_real = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        double coeff_imag = _c5r; /* +_c5ii */
        for (int k = 1; k < 6; k++) {
            double _re6r = 0, _re6i = 0;
            _re6r = x1r; _re6i = 0;
            double _pow7r = 0, _pow7i = 0;
            c_powr(_re6r, _re6i, k, &_pow7r, &_pow7i);
            double _ang8r = 0, _ang8i = 0;
            _ang8r = c_arg(x2r, x2i); _ang8i = 0;
            double _mul9r = 0, _mul9i = 0;
            c_mul(k, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
            double _add10r = 0, _add10i = 0;
            _add10r = _mul9r + j; _add10i = _mul9i + 0;
            double _cos11r = 0, _cos11i = 0;
            c_cos(_add10r, _add10i, &_cos11r, &_cos11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_pow7r, _pow7i, _cos11r, _cos11i, &_mul12r, &_mul12i);
            coeff_real += _mul12r;
            double _im13r = 0, _im13i = 0;
            _im13r = x2i; _im13i = 0;
            double _pow14r = 0, _pow14i = 0;
            c_powr(_im13r, _im13i, k, &_pow14r, &_pow14i);
            double _ang15r = 0, _ang15i = 0;
            _ang15r = c_arg(x1r, x1i); _ang15i = 0;
            double _mul16r = 0, _mul16i = 0;
            c_mul(k, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
            double _add17r = 0, _add17i = 0;
            _add17r = _mul16r + j; _add17i = _mul16i + 0;
            double _sin18r = 0, _sin18i = 0;
            c_sin(_add17r, _add17i, &_sin18r, &_sin18i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(_pow14r, _pow14i, _sin18r, _sin18i, &_mul19r, &_mul19i);
            coeff_imag += _mul19r;
        }
        for (int r = 1; r < 4; r++) {
            double _add20r = 0, _add20i = 0;
            _add20r = x1r + x2r; _add20i = x1i + x2i;
            double _abs21r = 0, _abs21i = 0;
            _abs21r = c_abs(_add20r, _add20i); _abs21i = 0;
            double _c22r = 0, _c22i = 0;
            _c22r = 1.0; _c22i = 0;
            double _add23r = 0, _add23i = 0;
            _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
            double _log24r = 0, _log24i = 0;
            c_log(_add23r, _add23i, &_log24r, &_log24i);
            double _re25r = 0, _re25i = 0;
            _re25r = x1r; _re25i = 0;
            double _mul26r = 0, _mul26i = 0;
            c_mul(_log24r, _log24i, _re25r, _re25i, &_mul26r, &_mul26i);
            double _div27r = 0, _div27i = 0;
            c_div(_mul26r, _mul26i, r, 0, &_div27r, &_div27i);
            coeff_real += _div27r;
            double _sub28r = 0, _sub28i = 0;
            _sub28r = x1r - x2r; _sub28i = x1i - x2i;
            double _abs29r = 0, _abs29i = 0;
            _abs29r = c_abs(_sub28r, _sub28i); _abs29i = 0;
            double _c30r = 0, _c30i = 0;
            _c30r = 1.0; _c30i = 0;
            double _add31r = 0, _add31i = 0;
            _add31r = _abs29r + _c30r; _add31i = _abs29i + _c30i;
            double _log32r = 0, _log32i = 0;
            c_log(_add31r, _add31i, &_log32r, &_log32i);
            double _im33r = 0, _im33i = 0;
            _im33r = x2i; _im33i = 0;
            double _mul34r = 0, _mul34i = 0;
            c_mul(_log32r, _log32i, _im33r, _im33i, &_mul34r, &_mul34i);
            double _div35r = 0, _div35i = 0;
            c_div(_mul34r, _mul34i, r, 0, &_div35r, &_div35i);
            coeff_imag += _div35r;
        }
        double _c36r = 0, _c36i = 0;
        _c36r = 2.0; _c36i = 0;
        double _pow37r = 0, _pow37i = 0;
        c_mul(coeff_real, 0, coeff_real, 0, &_pow37r, &_pow37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 2.0; _c38i = 0;
        double _pow39r = 0, _pow39i = 0;
        c_mul(coeff_imag, 0, coeff_imag, 0, &_pow39r, &_pow39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _pow37r + _pow39r; _add40i = _pow37i + _pow39i;
        double _sqrt41r = 0, _sqrt41i = 0;
        c_powr(_add40r, _add40i, 0.5, &_sqrt41r, &_sqrt41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(j, 0, n, 0, &_div43r, &_div43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _c42r + _div43r; _add44i = _c42i + _div43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_sqrt41r, _sqrt41i, _add44r, _add44i, &_mul45r, &_mul45i);
        double magnitude = _mul45r; /* +_mul45ii */
        double _at246r = 0, _at246i = 0;
        _at246r = atan2(coeff_imag, coeff_real); _at246i = 0;
        double _sin47r = 0, _sin47i = 0;
        c_sin(j, 0, &_sin47r, &_sin47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 2.0; _c48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(j, 0, _c48r, _c48i, &_div49r, &_div49i);
        double _cos50r = 0, _cos50i = 0;
        c_cos(_div49r, _div49i, &_cos50r, &_cos50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_sin47r, _sin47i, _cos50r, _cos50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _at246r + _mul51r; _add52i = _at246i + _mul51i;
        double angle = _add52r; /* +_add52ii */
        double _cos53r = 0, _cos53i = 0;
        c_cos(angle, 0, &_cos53r, &_cos53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 1.0;
        double _sin55r = 0, _sin55i = 0;
        c_sin(angle, 0, &_sin55r, &_sin55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c54r, _c54i, _sin55r, _sin55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _cos53r + _mul56r; _add57i = _cos53i + _mul56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(magnitude, 0, _add57r, _add57i, &_mul58r, &_mul58i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_390: too complex for auto-transpile, stubbed */
static void poly_390_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_391: too complex for auto-transpile, stubbed */
static void poly_391_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_392_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double mag = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        double ang = _c5r; /* +_c5ii */
        for (int k = 1; k < 6; k++) {
            double _re6r = 0, _re6i = 0;
            _re6r = x1r; _re6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 1.0; _c7i = 0;
            double _add8r = 0, _add8i = 0;
            _add8r = k + _c7r; _add8i = 0 + _c7i;
            double _log9r = 0, _log9i = 0;
            c_log(_add8r, _add8i, &_log9r, &_log9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(_re6r, _re6i, _log9r, _log9i, &_mul10r, &_mul10i);
            double _mul11r = 0, _mul11i = 0;
            c_mul(j, 0, k, 0, &_mul11r, &_mul11i);
            double _sin12r = 0, _sin12i = 0;
            c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_mul10r, _mul10i, _sin12r, _sin12i, &_mul13r, &_mul13i);
            double term1 = _mul13r; /* +_mul13ii */
            double _im14r = 0, _im14i = 0;
            _im14r = x2i; _im14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = j + k; _add15i = 0 + 0;
            double _cos16r = 0, _cos16i = 0;
            c_cos(_add15r, _add15i, &_cos16r, &_cos16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_im14r, _im14i, _cos16r, _cos16i, &_mul17r, &_mul17i);
            double term2 = _mul17r; /* +_mul17ii */
            double _c18r = 0, _c18i = 0;
            _c18r = 2.0; _c18i = 0;
            double _pow19r = 0, _pow19i = 0;
            c_mul(term1, 0, term1, 0, &_pow19r, &_pow19i);
            double _c20r = 0, _c20i = 0;
            _c20r = 2.0; _c20i = 0;
            double _pow21r = 0, _pow21i = 0;
            c_mul(term2, 0, term2, 0, &_pow21r, &_pow21i);
            double _add22r = 0, _add22i = 0;
            _add22r = _pow19r + _pow21r; _add22i = _pow19i + _pow21i;
            mag += _add22r;
            double _ang23r = 0, _ang23i = 0;
            _ang23r = c_arg(x1r, x1i); _ang23i = 0;
            double _sin24r = 0, _sin24i = 0;
            c_sin(k, 0, &_sin24r, &_sin24i);
            double _mul25r = 0, _mul25i = 0;
            c_mul(_ang23r, _ang23i, _sin24r, _sin24i, &_mul25r, &_mul25i);
            double _ang26r = 0, _ang26i = 0;
            _ang26r = c_arg(x2r, x2i); _ang26i = 0;
            double _cos27r = 0, _cos27i = 0;
            c_cos(k, 0, &_cos27r, &_cos27i);
            double _mul28r = 0, _mul28i = 0;
            c_mul(_ang26r, _ang26i, _cos27r, _cos27i, &_mul28r, &_mul28i);
            double _sub29r = 0, _sub29i = 0;
            _sub29r = _mul25r - _mul28r; _sub29i = _mul25i - _mul28i;
            ang += _sub29r;
        }
        for (int r = 1; r < 4; r++) {
            double _add30r = 0, _add30i = 0;
            _add30r = x1r + r; _add30i = x1i + 0;
            double _abs31r = 0, _abs31i = 0;
            _abs31r = c_abs(_add30r, _add30i); _abs31i = 0;
            double _sqrt32r = 0, _sqrt32i = 0;
            c_powr(r, 0, 0.5, &_sqrt32r, &_sqrt32i);
            double _mul33r = 0, _mul33i = 0;
            c_mul(_abs31r, _abs31i, _sqrt32r, _sqrt32i, &_mul33r, &_mul33i);
            double _add34r = 0, _add34i = 0;
            _add34r = j + r; _add34i = 0 + 0;
            double _div35r = 0, _div35i = 0;
            c_div(_mul33r, _mul33i, _add34r, _add34i, &_div35r, &_div35i);
            mag += _div35r;
            double _mul36r = 0, _mul36i = 0;
            c_mul(r, 0, M_PI, 0, &_mul36r, &_mul36i);
            double _div37r = 0, _div37i = 0;
            c_div(_mul36r, _mul36i, j, 0, &_div37r, &_div37i);
            double _sin38r = 0, _sin38i = 0;
            c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
            double _cos39r = 0, _cos39i = 0;
            c_cos(r, 0, &_cos39r, &_cos39i);
            double _mul40r = 0, _mul40i = 0;
            c_mul(_sin38r, _sin38i, _cos39r, _cos39i, &_mul40r, &_mul40i);
            ang += _mul40r;
        }
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(j, 0, n, 0, &_div42r, &_div42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _c41r + _div42r; _add43i = _c41i + _div42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        double varied_mag = _mul44r; /* +_mul44ii */
        double _abs45r = 0, _abs45i = 0;
        _abs45r = c_abs(x1r, x1i); _abs45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 1.0; _c46i = 0;
        double _add47r = 0, _add47i = 0;
        _add47r = _abs45r + _c46r; _add47i = _abs45i + _c46i;
        double _log48r = 0, _log48i = 0;
        c_log(_add47r, _add47i, &_log48r, &_log48i);
        double _add49r = 0, _add49i = 0;
        _add49r = ang + _log48r; _add49i = 0 + _log48i;
        double _abs50r = 0, _abs50i = 0;
        _abs50r = c_abs(x2r, x2i); _abs50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 1.0; _c51i = 0;
        double _add52r = 0, _add52i = 0;
        _add52r = _abs50r + _c51r; _add52i = _abs50i + _c51i;
        double _log53r = 0, _log53i = 0;
        c_log(_add52r, _add52i, &_log53r, &_log53i);
        double _sub54r = 0, _sub54i = 0;
        _sub54r = _add49r - _log53r; _sub54i = _add49i - _log53i;
        double varied_ang = _sub54r; /* +_sub54ii */
        double _cos55r = 0, _cos55i = 0;
        c_cos(varied_ang, 0, &_cos55r, &_cos55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 0.0; _c56i = 1.0;
        double _sin57r = 0, _sin57i = 0;
        c_sin(varied_ang, 0, &_sin57r, &_sin57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_c56r, _c56i, _sin57r, _sin57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _cos55r + _mul58r; _add59i = _cos55i + _mul58i;
        double _mul60r = 0, _mul60i = 0;
        c_mul(varied_mag, 0, _add59r, _add59i, &_mul60r, &_mul60i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul60r; cIm[_idx] = _mul60i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_393_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double mag = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double angle = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        for (int k = 1; k < (int)(_add4r); k++) {
            double _abs5r = 0, _abs5i = 0;
            _abs5r = c_abs(x1r, x1i); _abs5i = 0;
            double _add6r = 0, _add6i = 0;
            _add6r = _abs5r + k; _add6i = _abs5i + 0;
            double _log7r = 0, _log7i = 0;
            c_log(_add6r, _add6i, &_log7r, &_log7i);
            double _re8r = 0, _re8i = 0;
            _re8r = x2r; _re8i = 0;
            double _mul9r = 0, _mul9i = 0;
            c_mul(k, 0, _re8r, _re8i, &_mul9r, &_mul9i);
            double _sin10r = 0, _sin10i = 0;
            c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
            double _mul11r = 0, _mul11i = 0;
            c_mul(_log7r, _log7i, _sin10r, _sin10i, &_mul11r, &_mul11i);
            double _c12r = 0, _c12i = 0;
            _c12r = 1.0; _c12i = 0;
            double _add13r = 0, _add13i = 0;
            _add13r = k + _c12r; _add13i = 0 + _c12i;
            double _div14r = 0, _div14i = 0;
            c_div(_mul11r, _mul11i, _add13r, _add13i, &_div14r, &_div14i);
            mag += _div14r;
            double _ang15r = 0, _ang15i = 0;
            _ang15r = c_arg(x1r, x1i); _ang15i = 0;
            double _pow16r = 0, _pow16i = 0;
            c_powr(_ang15r, _ang15i, k, &_pow16r, &_pow16i);
            double _im17r = 0, _im17i = 0;
            _im17r = x2i; _im17i = 0;
            double _mul18r = 0, _mul18i = 0;
            c_mul(k, 0, _im17r, _im17i, &_mul18r, &_mul18i);
            double _cos19r = 0, _cos19i = 0;
            c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
            double _mul20r = 0, _mul20i = 0;
            c_mul(_pow16r, _pow16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
            angle += _mul20r;
        }
        double _cos21r = 0, _cos21i = 0;
        c_cos(angle, 0, &_cos21r, &_cos21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(angle, 0, &_sin22r, &_sin22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sin22r, _sin22i, _c23r, _c23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _cos21r + _mul24r; _add25i = _cos21i + _mul24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag, 0, _add25r, _add25i, &_mul26r, &_mul26i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_394_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double k = j; /* +0i */
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _sin2r = 0, _sin2i = 0;
        c_sin(k, 0, &_sin2r, &_sin2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(_re1r, _re1i, _sin2r, _sin2i, &_mul3r, &_mul3i);
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(k, 0, k, 0, &_pow6r, &_pow6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_pow6r, _pow6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re4r, _re4i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul3r + _mul8r; _add9i = _mul3i + _mul8i;
        double r = _add9r; /* +_add9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 3.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(k, 0, _c11r, _c11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_im10r, _im10i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _im15r = 0, _im15i = 0;
        _im15r = x2i; _im15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 4.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(k, 0, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_im15r, _im15i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _mul14r - _mul19r; _sub20i = _mul14i - _mul19i;
        double im = _sub20r; /* +_sub20ii */
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs21r + _abs22r; _add23i = _abs21i + _abs22i;
        double _add24r = 0, _add24i = 0;
        _add24r = _add23r + k; _add24i = _add23i + 0;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(k, 0, &_sin27r, &_sin27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(k, 0, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin27r, _sin27i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _c26r + _mul29r; _add30i = _c26i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log25r, _log25i, _add30r, _add30i, &_mul31r, &_mul31i);
        double mag = _mul31r; /* +_mul31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 5.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(k, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 7.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(k, 0, _c38r, _c38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang37r, _ang37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul36r + _mul41r; _add42i = _mul36i + _mul41i;
        double ang = _add42r; /* +_add42ii */
        double _cos43r = 0, _cos43i = 0;
        c_cos(ang, 0, &_cos43r, &_cos43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _sin45r = 0, _sin45i = 0;
        c_sin(ang, 0, &_sin45r, &_sin45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c44r, _c44i, _sin45r, _sin45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _cos43r + _mul46r; _add47i = _cos43i + _mul46i;
        double _mul48r = 0, _mul48i = 0;
        c_mul(mag, 0, _add47r, _add47i, &_mul48r, &_mul48i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_395_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_arr8r, _arr8i, x1r, x1i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_mul9r, _mul9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double mag_part1 = _log13r; /* +_log13ii */
        double _arr14r = 0, _arr14i = 0;
        { int _idx = (j - 1); _arr14r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr14i = 0; }
        double _cos15r = 0, _cos15i = 0;
        c_cos(_arr14r, _arr14i, &_cos15r, &_cos15i);
        double _arr16r = 0, _arr16i = 0;
        { int _idx = (j - 1); _arr16r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr16i = 0; }
        double _sin17r = 0, _sin17i = 0;
        c_sin(_arr16r, _arr16i, &_sin17r, &_sin17i);
        double _sum18r = 0, _sum18i = 0;
        _sum18r = _cos15r + _sin17r; _sum18i = _cos15i + _sin17i;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.5; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(j, 0, 0.5, &_pow20r, &_pow20i);
        double _div21r = 0, _div21i = 0;
        c_div(_sum18r, _sum18i, _pow20r, _pow20i, &_div21r, &_div21i);
        double mag_part2 = _div21r; /* +_div21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 7.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _c23r + _sin27r; _add28i = _c23i + _sin27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_mul22r, _mul22i, _add28r, _add28i, &_mul29r, &_mul29i);
        double mag = _mul29r; /* +_mul29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double ang_part1 = _mul34r; /* +_mul34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang35r, _ang35i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double ang_part2 = _mul39r; /* +_mul39ii */
        double _add40r = 0, _add40i = 0;
        _add40r = ang_part1 + ang_part2; _add40i = 0 + 0;
        double _abs41r = 0, _abs41i = 0;
        _abs41r = c_abs(j, 0); _abs41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = _abs41r + _c42r; _add43i = _abs41i + _c42i;
        double _log44r = 0, _log44i = 0;
        c_log(_add43r, _add43i, &_log44r, &_log44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _add40r + _log44r; _add45i = _add40i + _log44i;
        double ang = _add45r; /* +_add45ii */
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c46r, _c46i, ang, 0, &_mul47r, &_mul47i);
        double _exp48r = 0, _exp48i = 0;
        c_exp2(_mul47r, _mul47i, &_exp48r, &_exp48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(mag, 0, _exp48r, _exp48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = x1r + x2r; _add50i = x1i + x2i;
        double _conj51r = 0, _conj51i = 0;
        _conj51r = _add50r; _conj51i = -(_add50i);
        double _sin52r = 0, _sin52i = 0;
        c_sin(j, 0, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_conj51r, _conj51i, _sin52r, _sin52i, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 1.0; _c54i = 0;
        double _add55r = 0, _add55i = 0;
        _add55r = j + _c54r; _add55i = 0 + _c54i;
        double _div56r = 0, _div56i = 0;
        c_div(_mul53r, _mul53i, _add55r, _add55i, &_div56r, &_div56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _mul49r + _div56r; _add57i = _mul49i + _div56i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add57r; cIm[_idx] = _add57i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_396_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re4r, _re4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _im9r = 0, _im9i = 0;
        _im9r = x2i; _im9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 8.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_im9r, _im9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul8r + _mul14r; _add15i = _mul8i + _mul14i;
        double r_part = _add15r; /* +_add15ii */
        double _re16r = 0, _re16i = 0;
        _re16r = x2r; _re16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 7.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_re16r, _re16i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _im22r = 0, _im22i = 0;
        _im22r = x1i; _im22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 5.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_im22r, _im22i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul21r - _mul27r; _sub28i = _mul21i - _mul27i;
        double i_part = _sub28r; /* +_sub28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_mul(r_part, 0, r_part, 0, &_pow30r, &_pow30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(i_part, 0, i_part, 0, &_pow32r, &_pow32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _pow30r + _pow32r; _add33i = _pow30i + _pow32i;
        double _sqrt34r = 0, _sqrt34i = 0;
        c_powr(_add33r, _add33i, 0.5, &_sqrt34r, &_sqrt34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = j + _c35r; _add36i = 0 + _c35i;
        double _div37r = 0, _div37i = 0;
        c_div(0, 0, _add36r, _add36i, &_div37r, &_div37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sqrt34r + _div37r; _add38i = _sqrt34i + _div37i;
        double mag = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, i_part, 0, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = r_part + _mul40r; _add41i = 0 + _mul40i;
        double temp = _add41r; /* +_add41ii */
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(temp, 0); _ang42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(j, 0, M_PI, 0, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 6.0; _c44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(_mul43r, _mul43i, _c44r, _c44i, &_div45r, &_div45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(_div45r, _div45i, &_cos46r, &_cos46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _ang42r + _cos46r; _add47i = _ang42i + _cos46i;
        double theta = _add47r; /* +_add47ii */
        double _cos48r = 0, _cos48i = 0;
        c_cos(theta, 0, &_cos48r, &_cos48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _sin50r = 0, _sin50i = 0;
        c_sin(theta, 0, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c49r, _c49i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _cos48r + _mul51r; _add52i = _cos48i + _mul51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(mag, 0, _add52r, _add52i, &_mul53r, &_mul53i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_397_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(n, 0, _c9r, _c9i, &_div10r, &_div10i);
        double _sub11r = 0, _sub11i = 0;
        _sub11r = j - _div10r; _sub11i = 0 - _div10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_sub11r, _sub11i); _abs12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs8r, _abs8i, _abs12r, &_pow13r, &_pow13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(n, 0, _c15r, _c15i, &_div16r, &_div16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = j - _div16r; _sub17i = 0 - _div16i;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_sub17r, _sub17i); _abs18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_abs14r, _abs14i, _abs18r, &_pow19r, &_pow19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _pow13r + _pow19r; _add20i = _pow13i + _pow19i;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _add20r + _c21r; _add22i = _add20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double magnitude_part = _log23r; /* +_log23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, _ang24r, _ang24i, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, _ang27r, _ang27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_sin26r, _sin26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _im31r = 0, _im31i = 0;
        _im31r = x1i; _im31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_im31r, _im31i, M_PI, 0, &_mul32r, &_mul32i);
        double _div33r = 0, _div33i = 0;
        c_div(_mul32r, _mul32i, j, 0, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul30r + _sin34r; _add35i = _mul30i + _sin34i;
        double angle_part = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _mod37r = 0, _mod37i = 0;
        _mod37r = fmod(j, _c36r); _mod37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 0;
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang39r, _ang39i, j, 0, &_mul40r, &_mul40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_mul40r, _mul40i, &_cos41r, &_cos41i);
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x2r, x2i); _ang42i = 0;
        double _sub43r = 0, _sub43i = 0;
        _sub43r = n - j; _sub43i = 0 - 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_ang42r, _ang42i, _sub43r, _sub43i, &_mul44r, &_mul44i);
        double _sin45r = 0, _sin45i = 0;
        c_sin(_mul44r, _mul44i, &_sin45r, &_sin45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_cos41r, _cos41i, _sin45r, _sin45i, &_mul46r, &_mul46i);
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x1r, x1i); _ang47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 1.0; _c48i = 0;
        double _add49r = 0, _add49i = 0;
        _add49r = j + _c48r; _add49i = 0 + _c48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_ang47r, _ang47i, _add49r, _add49i, &_mul50r, &_mul50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(_mul50r, _mul50i, &_sin51r, &_sin51i);
        double _ang52r = 0, _ang52i = 0;
        _ang52r = c_arg(x2r, x2i); _ang52i = 0;
        double _c53r = 0, _c53i = 0;
        _c53r = 2.0; _c53i = 0;
        double _add54r = 0, _add54i = 0;
        _add54r = j + _c53r; _add54i = 0 + _c53i;
        double _mul55r = 0, _mul55i = 0;
        c_mul(_ang52r, _ang52i, _add54r, _add54i, &_mul55r, &_mul55i);
        double _cos56r = 0, _cos56i = 0;
        c_cos(_mul55r, _mul55i, &_cos56r, &_cos56i);
        double _sub57r = 0, _sub57i = 0;
        _sub57r = _sin51r - _cos56r; _sub57i = _sin51i - _cos56i;
        double _tern58r = 0, _tern58i = 0;
        if (_mod37r == _c38r) { _tern58r = _mul46r; _tern58i = _mul46i; }
        else { _tern58r = _sub57r; _tern58i = _sub57i; }
        double variation = _tern58r; /* +_tern58ii */
        double _c59r = 0, _c59i = 0;
        _c59r = 0.0; _c59i = 1.0;
        double _add60r = 0, _add60i = 0;
        _add60r = angle_part + variation; _add60i = 0 + 0;
        double _mul61r = 0, _mul61i = 0;
        c_mul(_c59r, _c59i, _add60r, _add60i, &_mul61r, &_mul61i);
        double _exp62r = 0, _exp62i = 0;
        c_exp2(_mul61r, _mul61i, &_exp62r, &_exp62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(magnitude_part, 0, _exp62r, _exp62i, &_mul63r, &_mul63i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul63r; cIm[_idx] = _mul63i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_398_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _re2r = 0, _re2i = 0;
        _re2r = x2r; _re2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(_re2r, _re2i, j, 0, &_mul3r, &_mul3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 35.0; _c4i = 0;
        double _div5r = 0, _div5i = 0;
        c_div(_mul3r, _mul3i, _c4r, _c4i, &_div5r, &_div5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _re1r + _div5r; _add6i = _re1i + _div5i;
        double r = _add6r; /* +_add6ii */
        double _im7r = 0, _im7i = 0;
        _im7r = x1i; _im7i = 0;
        double _im8r = 0, _im8i = 0;
        _im8r = x2i; _im8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_im8r, _im8i, j, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 35.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sub12r = 0, _sub12i = 0;
        _sub12r = _im7r - _div11r; _sub12i = _im7i - _div11i;
        double d = _sub12r; /* +_sub12ii */
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 1.0;
        double _add14r = 0, _add14i = 0;
        _add14r = r + _c13r; _add14i = 0 + _c13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(_add14r, _add14i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 4.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_div21r, _div21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log18r, _log18i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 6.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul23r + _cos27r; _add28i = _mul23i + _cos27i;
        double mag = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(d, 0); _ang29i = 0;
        double _cos30r = 0, _cos30i = 0;
        c_cos(j, 0, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(j, 0, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang32r, _ang32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
        double ang = _add35r; /* +_add35ii */
        double _cos36r = 0, _cos36i = 0;
        c_cos(ang, 0, &_cos36r, &_cos36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(ang, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c37r, _c37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _cos36r + _mul39r; _add40i = _cos36i + _mul39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(mag, 0, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_399_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 7.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mod2r + _c3r; _add4i = _mod2i + _c3i;
        double k = _add4r; /* +_add4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 5.0; _c5i = 0;
        double _fdiv6r = 0, _fdiv6i = 0;
        c_div(j, 0, _c5r, _c5i, &_fdiv6r, &_fdiv6i);
        _fdiv6r = floor(_fdiv6r); _fdiv6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _fdiv6r + _c7r; _add8i = _fdiv6i + _c7i;
        double r = _add8r; /* +_add8ii */
        double _re9r = 0, _re9i = 0;
        _re9r = x1r; _re9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_re9r, _re9i, k, &_pow10r, &_pow10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(k, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_pow10r, _pow10i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double term1 = _mul15r; /* +_mul15ii */
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_im16r, _im16i, r, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(r, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 4.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_pow17r, _pow17i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double term2 = _mul22r; /* +_mul22ii */
        double _add23r = 0, _add23i = 0;
        _add23r = term1 + term2; _add23i = 0 + 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _abs25r; _add26i = _abs24i + _abs25i;
        double _add27r = 0, _add27i = 0;
        _add27r = _add26r + j; _add27i = _add26i + 0;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add23r + _log28r; _add29i = _add23i + _log28i;
        double magnitude = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang30r, _ang30i, r, 0, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_ang32r, _ang32i, k, 0, &_div33r, &_div33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul31r - _div33r; _sub34i = _mul31i - _div33i;
        double _sin35r = 0, _sin35i = 0;
        c_sin(j, 0, &_sin35r, &_sin35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(j, 0, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sin35r, _sin35i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sub34r + _mul37r; _add38i = _sub34i + _mul37i;
        double angle = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(magnitude, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul43r, &_mul43i);
        double _conj44r = 0, _conj44i = 0;
        _conj44r = _mul43r; _conj44i = -(_mul43i);
        double _abs45r = 0, _abs45i = 0;
        _abs45r = c_abs(x1r, x1i); _abs45i = 0;
        double _abs46r = 0, _abs46i = 0;
        _abs46r = c_abs(x2r, x2i); _abs46i = 0;
        double _add47r = 0, _add47i = 0;
        _add47r = _abs45r + _abs46r; _add47i = _abs45i + _abs46i;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_conj44r, _conj44i, _add47r, _add47i, &_mul48r, &_mul48i);
        double _div49r = 0, _div49i = 0;
        c_div(_mul48r, _mul48i, j, 0, &_div49r, &_div49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul42r + _div49r; _add50i = _mul42i + _div49i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
        double _c51r = 0, _c51i = 0;
        _c51r = 4.0; _c51i = 0;
        double _mod52r = 0, _mod52i = 0;
        _mod52r = fmod(j, _c51r); _mod52i = 0;
        double _c53r = 0, _c53i = 0;
        _c53r = 0.0; _c53i = 0;
        if (_mod52r == _c53r) {
            double _mul54r = 0, _mul54i = 0;
            c_mul(j, 0, M_PI, 0, &_mul54r, &_mul54i);
            double _c55r = 0, _c55i = 0;
            _c55r = 5.0; _c55i = 0;
            double _div56r = 0, _div56i = 0;
            c_div(_mul54r, _mul54i, _c55r, _c55i, &_div56r, &_div56i);
            double _sin57r = 0, _sin57i = 0;
            c_sin(_div56r, _div56i, &_sin57r, &_sin57i);
            double _mul58r = 0, _mul58i = 0;
            c_mul(j, 0, M_PI, 0, &_mul58r, &_mul58i);
            double _c59r = 0, _c59i = 0;
            _c59r = 6.0; _c59i = 0;
            double _div60r = 0, _div60i = 0;
            c_div(_mul58r, _mul58i, _c59r, _c59i, &_div60r, &_div60i);
            double _cos61r = 0, _cos61i = 0;
            c_cos(_div60r, _div60i, &_cos61r, &_cos61i);
            double _add62r = 0, _add62i = 0;
            _add62r = _sin57r + _cos61r; _add62i = _sin57i + _cos61i;
            { double _tr = cRe[(j - 1)]*_add62r - cIm[(j - 1)]*_add62i; cIm[(j - 1)] = cRe[(j - 1)]*_add62i + cIm[(j - 1)]*_add62r; cRe[(j - 1)] = _tr; }
        }
        double _c63r = 0, _c63i = 0;
        _c63r = 6.0; _c63i = 0;
        double _mod64r = 0, _mod64i = 0;
        _mod64r = fmod(j, _c63r); _mod64i = 0;
        double _c65r = 0, _c65i = 0;
        _c65r = 0.0; _c65i = 0;
        if (_mod64r == _c65r) {
            double _re66r = 0, _re66i = 0;
            _re66r = x1r; _re66i = 0;
            double _c67r = 0, _c67i = 0;
            _c67r = 2.0; _c67i = 0;
            double _pow68r = 0, _pow68i = 0;
            c_mul(_re66r, _re66i, _re66r, _re66i, &_pow68r, &_pow68i);
            double _im69r = 0, _im69i = 0;
            _im69r = x2i; _im69i = 0;
            double _c70r = 0, _c70i = 0;
            _c70r = 2.0; _c70i = 0;
            double _pow71r = 0, _pow71i = 0;
            c_mul(_im69r, _im69i, _im69r, _im69i, &_pow71r, &_pow71i);
            double _sub72r = 0, _sub72i = 0;
            _sub72r = _pow68r - _pow71r; _sub72i = _pow68i - _pow71i;
            cRe[(j - 1)] += _sub72r; cIm[(j - 1)] += _sub72i;
        }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_400_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re2r + (_re3r - _re2r) * _li / 34.0;
    }
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im4r + (_im5r - _im4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_arr8r, _arr8i, M_PI, 0, &_mul9r, &_mul9i);
        double _div10r = 0, _div10i = 0;
        c_div(_mul9r, _mul9i, n, 0, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _arr12r = 0, _arr12i = 0;
        { int _idx = (j - 1); _arr12r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr12i = 0; }
        double _mul13r = 0, _mul13i = 0;
        c_mul(_arr12r, _arr12i, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_c14r, _c14i, n, 0, &_mul15r, &_mul15i);
        double _div16r = 0, _div16i = 0;
        c_div(_mul13r, _mul13i, _mul15r, _mul15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sin11r, _sin11i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double term1 = _mul18r; /* +_mul18ii */
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr19i = 0; }
        double _arr20r = 0, _arr20i = 0;
        { int _idx = (j - 1); _arr20r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr20i = 0; }
        double _add21r = 0, _add21i = 0;
        _add21r = _arr19r + _arr20r; _add21i = _arr19i + _arr20i;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_add21r, _add21i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double term2 = _log25r; /* +_log25ii */
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_abs26r, _abs26i, j, &_pow27r, &_pow27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _c28r + j; _add29i = _c28i + 0;
        double _div30r = 0, _div30i = 0;
        c_div(_pow27r, _pow27i, _add29r, _add29i, &_div30r, &_div30i);
        double term3 = _div30r; /* +_div30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang31r, _ang31i, j, 0, &_mul32r, &_mul32i);
        double term4 = _mul32r; /* +_mul32ii */
        double _add33r = 0, _add33i = 0;
        _add33r = term1 + term2; _add33i = 0 + 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _add33r + term3; _add34i = _add33i + 0;
        double magnitude = _add34r; /* +_add34ii */
        double _arr35r = 0, _arr35i = 0;
        { int _idx = (j - 1); _arr35r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr35i = 0; }
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, _arr35r, _arr35i, &_mul36r, &_mul36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = term4 + _sin37r; _add38i = 0 + _sin37i;
        double _arr39r = 0, _arr39i = 0;
        { int _idx = (j - 1); _arr39r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr39i = 0; }
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, _arr39r, _arr39i, &_mul40r, &_mul40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_mul40r, _mul40i, &_cos41r, &_cos41i);
        double _sub42r = 0, _sub42i = 0;
        _sub42r = _add38r - _cos41r; _sub42i = _add38i - _cos41i;
        double angle = _sub42r; /* +_sub42ii */
        double _cos43r = 0, _cos43i = 0;
        c_cos(angle, 0, &_cos43r, &_cos43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _sin45r = 0, _sin45i = 0;
        c_sin(angle, 0, &_sin45r, &_sin45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c44r, _c44i, _sin45r, _sin45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _cos43r + _mul46r; _add47i = _cos43i + _mul46i;
        double _mul48r = 0, _mul48i = 0;
        c_mul(magnitude, 0, _add47r, _add47i, &_mul48r, &_mul48i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
