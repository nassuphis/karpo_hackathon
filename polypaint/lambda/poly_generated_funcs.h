/* AUTO-GENERATED from poly100.py — do not edit manually */
/* 100 coefficient functions */

static void poly_1_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 37; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _div2r = 0, _div2i = 0;
        c_div(i, 0, _c1r, _c1i, &_div2r, &_div2i);
        double _pow3r = 0, _pow3i = 0;
        c_powr(x1r, x1i, _div2r, &_pow3r, &_pow3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_pow3r, _pow3i, &_sin4r, &_sin4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 3.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(i, 0, _c5r, _c5i, &_div6r, &_div6i);
        double _pow7r = 0, _pow7i = 0;
        c_powr(x2r, x2i, _div6r, &_pow7r, &_pow7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_pow7r, _pow7i, &_cos8r, &_cos8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_sin4r, _sin4i, _cos8r, _cos8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_mul(i, 0, i, 0, &_pow11r, &_pow11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_pow11r, _pow11i, x1r, x1i, &_mul12r, &_mul12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_mul12r, _mul12i, x2r, x2i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul9r + _mul13r; _add14i = _mul9i + _mul13i;
        double _add15r = 0, _add15i = 0;
        _add15r = x1r + x2r; _add15i = x1i + x2i;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_add15r, _add15i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log19r, _log19i, _c20r, _c20i, &_mul21r, &_mul21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul21r, _mul21i, i, 0, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _add14r + _mul22r; _add23i = _add14i + _mul22i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    }
    double _mul24r = 0, _mul24i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul24r, &_mul24i);
    double _cf25r = 0, _cf25i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 36) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
    double _re26r = 0, _re26i = 0;
    _re26r = _cf25r; _re26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_mul24r, _mul24i, _re26r, _re26i, &_mul27r, &_mul27i);
    double _cf28r = 0, _cf28i = 0;
    { int _idx = 18; if (_idx >= 0 && _idx < 36) { _cf28r = cRe[_idx]; _cf28i = cIm[_idx]; } }
    double _im29r = 0, _im29i = 0;
    _im29r = _cf28i; _im29i = 0;
    double _c30r = 0, _c30i = 0;
    _c30r = 3.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow31r, &_pow31i);
    c_mul(_pow31r, _pow31i, x1r, x1i, &_pow31r, &_pow31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_im29r, _im29i, _pow31r, _pow31i, &_mul32r, &_mul32i);
    double _add33r = 0, _add33i = 0;
    _add33r = _mul27r + _mul32r; _add33i = _mul27i + _mul32i;
    { int _idx = 10; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    double _cf34r = 0, _cf34i = 0;
    { int _idx = 10; if (_idx >= 0 && _idx < 36) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
    double _mul35r = 0, _mul35i = 0;
    c_mul(x2r, x2i, _cf34r, _cf34i, &_mul35r, &_mul35i);
    double _cf36r = 0, _cf36i = 0;
    { int _idx = 34; if (_idx >= 0 && _idx < 36) { _cf36r = cRe[_idx]; _cf36i = cIm[_idx]; } }
    double _re37r = 0, _re37i = 0;
    _re37r = _cf36r; _re37i = 0;
    double _c38r = 0, _c38i = 0;
    _c38r = 3.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow39r, &_pow39i);
    c_mul(_pow39r, _pow39i, x1r, x1i, &_pow39r, &_pow39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_re37r, _re37i, _pow39r, _pow39i, &_mul40r, &_mul40i);
    double _add41r = 0, _add41i = 0;
    _add41r = _mul35r + _mul40r; _add41i = _mul35i + _mul40i;
    { int _idx = 21; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    double _cf42r = 0, _cf42i = 0;
    { int _idx = 21; if (_idx >= 0 && _idx < 36) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
    double _cf43r = 0, _cf43i = 0;
    { int _idx = 16; if (_idx >= 0 && _idx < 36) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
    double _re44r = 0, _re44i = 0;
    _re44r = _cf43r; _re44i = 0;
    double _c45r = 0, _c45i = 0;
    _c45r = 2.0; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow46r, &_pow46i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_re44r, _re44i, _pow46r, _pow46i, &_mul47r, &_mul47i);
    double _sub48r = 0, _sub48i = 0;
    _sub48r = _cf42r - _mul47r; _sub48i = _cf42i - _mul47i;
    { int _idx = 32; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub48r; cIm[_idx] = _sub48i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_2_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int k = 2; k < 37; k++) {
        double _cf2r = 0, _cf2i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 36) { _cf2r = cRe[_idx]; _cf2i = cIm[_idx]; } }
        double _mul3r = 0, _mul3i = 0;
        c_mul(k, 0, _cf2r, _cf2i, &_mul3r, &_mul3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_mul3r, _mul3i, &_sin4r, &_sin4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, x1r, x1i, &_mul5r, &_mul5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_mul5r, _mul5i, &_cos6r, &_cos6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _sin4r + _cos6r; _add7i = _sin4i + _cos6i;
        double _mul8r = 0, _mul8i = 0;
        c_mul(k, 0, x2r, x2i, &_mul8r, &_mul8i);
        double _re9r = 0, _re9i = 0;
        _re9r = _mul8r; _re9i = 0;
        double _cf10r = 0, _cf10i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 36) { _cf10r = cRe[_idx]; _cf10i = cIm[_idx]; } }
        double _mul11r = 0, _mul11i = 0;
        c_mul(k, 0, _cf10r, _cf10i, &_mul11r, &_mul11i);
        double _im12r = 0, _im12i = 0;
        _im12r = _mul11i; _im12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_re9r, _re9i, _im12r, _im12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add7r + _mul13r; _add14i = _add7i + _mul13i;
        double v = _add14r; /* +_add14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(v, 0); _abs15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(v, 0, _abs15r, _abs15i, &_div16r, &_div16i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div16r; cIm[_idx] = _div16i; } }
    }
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
    double _re19r = 0, _re19i = 0;
    _re19r = x1r; _re19i = 0;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_re19r, _re19i, x2r, x2i, &_mul20r, &_mul20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _pow18r + _mul20r; _add21i = _pow18i + _mul20i;
    double _c22r = 0, _c22i = 0;
    _c22r = 2.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow23r, &_pow23i);
    double _im24r = 0, _im24i = 0;
    _im24r = _pow23i; _im24i = 0;
    double _sub25r = 0, _sub25i = 0;
    _sub25r = _add21r - _im24r; _sub25i = _add21i - _im24i;
    { int _idx = 17; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub25r; cIm[_idx] = _sub25i; } }
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _add27r = 0, _add27i = 0;
    _add27r = x1r + x2r; _add27i = x1i + x2i;
    double _mul28r = 0, _mul28i = 0;
    c_mul(_c26r, _c26i, _add27r, _add27i, &_mul28r, &_mul28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul29r, &_mul29i);
    double _re30r = 0, _re30i = 0;
    _re30r = _mul29r; _re30i = 0;
    double _sub31r = 0, _sub31i = 0;
    _sub31r = _mul28r - _re30r; _sub31i = _mul28i - _re30i;
    double _re32r = 0, _re32i = 0;
    _re32r = x1r; _re32i = 0;
    double _sin33r = 0, _sin33i = 0;
    c_sin(_re32r, _re32i, &_sin33r, &_sin33i);
    double _im34r = 0, _im34i = 0;
    _im34r = x2i; _im34i = 0;
    double _cos35r = 0, _cos35i = 0;
    c_cos(_im34r, _im34i, &_cos35r, &_cos35i);
    double _mul36r = 0, _mul36i = 0;
    c_mul(_sin33r, _sin33i, _cos35r, _cos35i, &_mul36r, &_mul36i);
    double _add37r = 0, _add37i = 0;
    _add37r = _sub31r + _mul36r; _add37i = _sub31i + _mul36i;
    { int _idx = 31; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    double _cf38r = 0, _cf38i = 0;
    { int _idx = 17; if (_idx >= 0 && _idx < 36) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
    double _cf39r = 0, _cf39i = 0;
    { int _idx = 31; if (_idx >= 0 && _idx < 36) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
    double _mul40r = 0, _mul40i = 0;
    c_mul(_cf38r, _cf38i, _cf39r, _cf39i, &_mul40r, &_mul40i);
    double _mul41r = 0, _mul41i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul41r, &_mul41i);
    double _re42r = 0, _re42i = 0;
    _re42r = _mul41r; _re42i = 0;
    double _sin43r = 0, _sin43i = 0;
    c_sin(_re42r, _re42i, &_sin43r, &_sin43i);
    double _add44r = 0, _add44i = 0;
    _add44r = _mul40r + _sin43r; _add44i = _mul40i + _sin43i;
    double _mul45r = 0, _mul45i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul45r, &_mul45i);
    double _im46r = 0, _im46i = 0;
    _im46r = _mul45i; _im46i = 0;
    double _cos47r = 0, _cos47i = 0;
    c_cos(_im46r, _im46i, &_cos47r, &_cos47i);
    double _sub48r = 0, _sub48i = 0;
    _sub48r = _add44r - _cos47r; _sub48i = _add44i - _cos47i;
    { int _idx = 35; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub48r; cIm[_idx] = _sub48i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_3_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 2; k < 37; k++) {
        double _pow1r = 0, _pow1i = 0;
        c_powr(x1r, x1i, k, &_pow1r, &_pow1i);
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _div3r = 0, _div3i = 0;
        c_div(_pow1r, _pow1i, _c2r, _c2i, &_div3r, &_div3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = k - _c4r; _sub5i = 0 - _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powr(x2r, x2i, _sub5r, &_pow6r, &_pow6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_pow6r, _pow6i, &_cos7r, &_cos7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _div3r + _cos7r; _add8i = _div3i + _cos7i;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _add8r + _log12r; _add13i = _add8i + _log12i;
        double v = _add13r; /* +_add13ii */
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _mod15r = 0, _mod15i = 0;
        _mod15r = fmod(k, _c14r); _mod15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 0;
        if (_mod15r == _c16r) {
            double _re17r = 0, _re17i = 0;
            _re17r = v; _re17i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _re17r; cIm[_idx] = _re17i; } }
        } else {
            double _im18r = 0, _im18i = 0;
            _im18r = 0; _im18i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _im18r; cIm[_idx] = _im18i; } }
        }
    }
    double _c19r = 0, _c19i = 0;
    _c19r = 1.0; _c19i = 0;
    double _mul20r = 0, _mul20i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul20r, &_mul20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _c19r + _mul20r; _add21i = _c19i + _mul20i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    double _cf22r = 0, _cf22i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 36) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
    double _cf23r = 0, _cf23i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 36) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
    double _mul24r = 0, _mul24i = 0;
    c_mul(_cf22r, _cf22i, _cf23r, _cf23i, &_mul24r, &_mul24i);
    double _cf25r = 0, _cf25i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 36) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
    double _cf26r = 0, _cf26i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 36) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
    double _mul27r = 0, _mul27i = 0;
    c_mul(_cf25r, _cf25i, _cf26r, _cf26i, &_mul27r, &_mul27i);
    double _sub28r = 0, _sub28i = 0;
    _sub28r = _mul24r - _mul27r; _sub28i = _mul24i - _mul27i;
    { int _idx = 17; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub28r; cIm[_idx] = _sub28i; } }
    double _cf29r = 0, _cf29i = 0;
    { int _idx = 11; if (_idx >= 0 && _idx < 36) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
    double _cf30r = 0, _cf30i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 36) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
    double _div31r = 0, _div31i = 0;
    c_div(_cf29r, _cf29i, _cf30r, _cf30i, &_div31r, &_div31i);
    double _cf32r = 0, _cf32i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 36) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
    double _cf33r = 0, _cf33i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 36) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
    double _mul34r = 0, _mul34i = 0;
    c_mul(_cf32r, _cf32i, _cf33r, _cf33i, &_mul34r, &_mul34i);
    double _add35r = 0, _add35i = 0;
    _add35r = _div31r + _mul34r; _add35i = _div31i + _mul34i;
    { int _idx = 23; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    double _sum36r = 0, _sum36i = 0;
    _sum36r = 0; _sum36i = 0;
    for (int _si = 15; _si < 20; _si++) { _sum36r += cRe[_si]; _sum36i += cIm[_si]; }
    { int _idx = 35; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sum36r; cIm[_idx] = _sum36i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_4_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 37; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 36.0; _c1i = 0;
        double _sub2r = 0, _sub2i = 0;
        _sub2r = _c1r - k; _sub2i = _c1i - 0;
        double _pow3r = 0, _pow3i = 0;
        c_powr(x1r, x1i, _sub2r, &_pow3r, &_pow3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 36.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - k; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(x2r, x2i, _sub5r, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow3r + _pow6r; _add7i = _pow3i + _pow6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 1.0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(k, 0, _c8r, _c8i, &_mul9r, &_mul9i);
        double _div10r = 0, _div10i = 0;
        c_div(_add7r, _add7i, _mul9r, _mul9i, &_div10r, &_div10i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div10r; cIm[_idx] = _div10i; } }
    }
    double _mul11r = 0, _mul11i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul11r, &_mul11i);
    double _abs12r = 0, _abs12i = 0;
    _abs12r = c_abs(x1r, x1i); _abs12i = 0;
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _add14r = 0, _add14i = 0;
    _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
    double _log15r = 0, _log15i = 0;
    c_log(_add14r, _add14i, &_log15r, &_log15i);
    double _add16r = 0, _add16i = 0;
    _add16r = _mul11r + _log15r; _add16i = _mul11i + _log15i;
    double _sin17r = 0, _sin17i = 0;
    c_sin(x2r, x2i, &_sin17r, &_sin17i);
    double _sub18r = 0, _sub18i = 0;
    _sub18r = _add16r - _sin17r; _sub18i = _add16i - _sin17i;
    { int _idx = 16; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub18r; cIm[_idx] = _sub18i; } }
    double _re19r = 0, _re19i = 0;
    _re19r = x1r; _re19i = 0;
    double _im20r = 0, _im20i = 0;
    _im20r = x1i; _im20i = 0;
    double _sub21r = 0, _sub21i = 0;
    _sub21r = _re19r - _im20r; _sub21i = _re19i - _im20i;
    double _c22r = 0, _c22i = 0;
    _c22r = 0.0; _c22i = 1.0;
    double _re23r = 0, _re23i = 0;
    _re23r = x2r; _re23i = 0;
    double _im24r = 0, _im24i = 0;
    _im24r = x2i; _im24i = 0;
    double _add25r = 0, _add25i = 0;
    _add25r = _re23r + _im24r; _add25i = _re23i + _im24i;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c22r, _c22i, _add25r, _add25i, &_mul26r, &_mul26i);
    double _add27r = 0, _add27i = 0;
    _add27r = _sub21r + _mul26r; _add27i = _sub21i + _mul26i;
    { int _idx = 24; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(x1r, x1i); _abs28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_abs28r, _abs28i, _abs28r, _abs28i, &_pow30r, &_pow30i);
    double _abs31r = 0, _abs31i = 0;
    _abs31r = c_abs(x2r, x2i); _abs31i = 0;
    double _c32r = 0, _c32i = 0;
    _c32r = 2.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(_abs31r, _abs31i, _abs31r, _abs31i, &_pow33r, &_pow33i);
    double _sub34r = 0, _sub34i = 0;
    _sub34r = _pow30r - _pow33r; _sub34i = _pow30i - _pow33i;
    double _c35r = 0, _c35i = 0;
    _c35r = 0.0; _c35i = 1.0;
    double _ang36r = 0, _ang36i = 0;
    _ang36r = c_arg(x1r, x1i); _ang36i = 0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c35r, _c35i, _ang36r, _ang36i, &_mul37r, &_mul37i);
    double _ang38r = 0, _ang38i = 0;
    _ang38r = c_arg(x2r, x2i); _ang38i = 0;
    double _mul39r = 0, _mul39i = 0;
    c_mul(_mul37r, _mul37i, _ang38r, _ang38i, &_mul39r, &_mul39i);
    double _add40r = 0, _add40i = 0;
    _add40r = _sub34r + _mul39r; _add40i = _sub34i + _mul39i;
    { int _idx = 29; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    double _mul41r = 0, _mul41i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul41r, &_mul41i);
    double _conj42r = 0, _conj42i = 0;
    _conj42r = _mul41r; _conj42i = -(_mul41i);
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(_conj42r, _conj42i, _conj42r, _conj42i, &_pow44r, &_pow44i);
    double _add45r = 0, _add45i = 0;
    _add45r = x1r + x2r; _add45i = x1i + x2i;
    double _sin46r = 0, _sin46i = 0;
    c_sin(_add45r, _add45i, &_sin46r, &_sin46i);
    double _sub47r = 0, _sub47i = 0;
    _sub47r = _pow44r - _sin46r; _sub47i = _pow44i - _sin46i;
    { int _idx = 35; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub47r; cIm[_idx] = _sub47i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_5_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double p[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0};
    for (int k = 1; k < 17; k++) {
        double _arr1r = 0, _arr1i = 0;
        { int _idx = (k - 1); _arr1r = (_idx >= 0 && _idx < 16) ? p[_idx] : 0.0; _arr1i = 0; }
        double _mul2r = 0, _mul2i = 0;
        c_mul(_arr1r, _arr1i, x1r, x1i, &_mul2r, &_mul2i);
        double _sin3r = 0, _sin3i = 0;
        c_sin(_mul2r, _mul2i, &_sin3r, &_sin3i);
        double _arr4r = 0, _arr4i = 0;
        { int _idx = (k - 1); _arr4r = (_idx >= 0 && _idx < 16) ? p[_idx] : 0.0; _arr4i = 0; }
        double _mul5r = 0, _mul5i = 0;
        c_mul(_arr4r, _arr4i, x2r, x2i, &_mul5r, &_mul5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_mul5r, _mul5i, &_cos6r, &_cos6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _sin3r + _cos6r; _add7i = _sin3i + _cos6i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    }
    for (int k = 17; k < 33; k++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (k - 17); _arr8r = (_idx >= 0 && _idx < 16) ? p[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_arr8r, _arr8i, x1r, x1i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul9r + x2r; _add10i = _mul9i + x2i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
        double _log12r = 0, _log12i = 0;
        c_log(_abs11r, _abs11i, &_log12r, &_log12i);
        double _add13r = 0, _add13i = 0;
        _add13r = x1r + x2r; _add13i = x1i + x2i;
        double _div14r = 0, _div14i = 0;
        c_div(_log12r, _log12i, _add13r, _add13i, &_div14r, &_div14i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div14r; cIm[_idx] = _div14i; } }
    }
    double _prod15r = 0, _prod15i = 0;
    /* WARNING: unhandled np.prod */
    double _mul16r = 0, _mul16i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul16r, &_mul16i);
    double _div17r = 0, _div17i = 0;
    c_div(_prod15r, _prod15i, _mul16r, _mul16i, &_div17r, &_div17i);
    { int _idx = 32; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div17r; cIm[_idx] = _div17i; } }
    double _sum18r = 0, _sum18i = 0;
    /* WARNING: unhandled np.sum */
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow20r, &_pow20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = _sum18r - _pow20r; _sub21i = _sum18i - _pow20i;
    double _c22r = 0, _c22i = 0;
    _c22r = 2.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow23r, &_pow23i);
    double _add24r = 0, _add24i = 0;
    _add24r = _sub21r + _pow23r; _add24i = _sub21i + _pow23i;
    { int _idx = 33; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add24r; cIm[_idx] = _add24i; } }
    double _arr25r = 0, _arr25i = 0;
    { int _idx = 8; _arr25r = (_idx >= 0 && _idx < 16) ? p[_idx] : 0.0; _arr25i = 0; }
    double _arr26r = 0, _arr26i = 0;
    { int _idx = 9; _arr26r = (_idx >= 0 && _idx < 16) ? p[_idx] : 0.0; _arr26i = 0; }
    double _mul27r = 0, _mul27i = 0;
    c_mul(_arr25r, _arr25i, _arr26r, _arr26i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = x1r + x2r; _add28i = x1i + x2i;
    double _mul29r = 0, _mul29i = 0;
    c_mul(_mul27r, _mul27i, _add28r, _add28i, &_mul29r, &_mul29i);
    { int _idx = 34; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    double _arr30r = 0, _arr30i = 0;
    { int _idx = 10; _arr30r = (_idx >= 0 && _idx < 16) ? p[_idx] : 0.0; _arr30i = 0; }
    double _arr31r = 0, _arr31i = 0;
    { int _idx = 11; _arr31r = (_idx >= 0 && _idx < 16) ? p[_idx] : 0.0; _arr31i = 0; }
    double _mul32r = 0, _mul32i = 0;
    c_mul(_arr30r, _arr30i, _arr31r, _arr31i, &_mul32r, &_mul32i);
    double _sub33r = 0, _sub33i = 0;
    _sub33r = x1r - x2r; _sub33i = x1i - x2i;
    double _div34r = 0, _div34i = 0;
    c_div(_mul32r, _mul32i, _sub33r, _sub33i, &_div34r, &_div34i);
    { int _idx = 35; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div34r; cIm[_idx] = _div34i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_6_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 52; k++) {
        double _add1r = 0, _add1i = 0;
        _add1r = x1r + x2r; _add1i = x1i + x2i;
        double _mul2r = 0, _mul2i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(_mul2r, _mul2i); _abs3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs3r, _abs3i, k, &_pow4r, &_pow4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _pow4r + _c5r; _add6i = _pow4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_log7r, _log7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_add1r, _add1i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul10r, &_mul10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(_mul10r, _mul10i); _ang11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_ang11r, _ang11i, k, &_pow12r, &_pow12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_pow12r, _pow12i, &_cos13r, &_cos13i);
        double _sub14r = 0, _sub14i = 0;
        _sub14r = x1r - x2r; _sub14i = x1i - x2i;
        double _conj15r = 0, _conj15i = 0;
        _conj15r = _sub14r; _conj15i = -(_sub14i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_cos13r, _cos13i, _conj15r, _conj15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul9r + _mul16r; _add17i = _mul9i + _mul16i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
    }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_7_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 52; k++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(x1r, x1i, k, 0, &_mul1r, &_mul1i);
        double _cos2r = 0, _cos2i = 0;
        c_cos(_mul1r, _mul1i, &_cos2r, &_cos2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 1.0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(x2r, x2i, k, 0, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_c3r, _c3i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _cos2r + _mul6r; _add7i = _cos2i + _mul6i;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _add7r + _log11r; _add12i = _add7i + _log11i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _add12r + _log16r; _add17i = _add12i + _log16i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
    }
    static const double primes[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0};
    for (int _si = 0; _si < 15; _si++) {
        int _si_idx = _si + 1;
        double _elem18r = 0, _elem18i = 0;
        { int _idx = _si; _elem18r = (_idx >= 0 && _idx < 15) ? primes[_idx] : 0.0; _elem18i = 0; }
        double _re19r = 0, _re19i = 0;
        _re19r = x1r; _re19i = 0;
        double _im20r = 0, _im20i = 0;
        _im20r = x2i; _im20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_mul(_im20r, _im20i, _im20r, _im20i, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _re19r + _pow22r; _add23i = _re19i + _pow22i;
        double _smul24r = 0, _smul24i = 0;
        c_mul(_elem18r, _elem18i, _add23r, _add23i, &_smul24r, &_smul24i);
        cRe[_si_idx] += _smul24r; cIm[_si_idx] += _smul24i;
    }
    double _sum25r = 0, _sum25i = 0;
    /* WARNING: unhandled np.sum */
    double _add26r = 0, _add26i = 0;
    _add26r = x1r + x2r; _add26i = x1i + x2i;
    double _div27r = 0, _div27i = 0;
    c_div(_sum25r, _sum25i, _add26r, _add26i, &_div27r, &_div27i);
    cRe[24] += _div27r; cIm[24] += _div27i;
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _re29r = 0, _re29i = 0;
    _re29r = _mul28r; _re29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul30r, &_mul30i);
    double _im31r = 0, _im31i = 0;
    _im31r = _mul30i; _im31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = _re29r + _im31r; _add32i = _re29i + _im31i;
    double _mul33r = 0, _mul33i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul33r, &_mul33i);
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(_mul33r, _mul33i); _abs34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 1.0; _c35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _abs34r + _c35r; _add36i = _abs34i + _c35i;
    double _log37r = 0, _log37i = 0;
    c_log(_add36r, _add36i, &_log37r, &_log37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _add32r + _log37r; _add38i = _add32i + _log37i;
    { double _tr = cRe[49]*_add38r - cIm[49]*_add38i; cIm[49] = cRe[49]*_add38i + cIm[49]*_add38r; cRe[49] = _tr; }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_8_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 25; _si++) {
        int _si_idx = _si + 0;
        double _arange1r = 0, _arange1i = 0;
        _arange1r = (double)(_si + 1); _arange1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 1.0;
        double _c5r = 0, _c5i = 0;
        _c5r = 3.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow6r, &_pow6i);
        c_mul(_pow6r, _pow6i, x2r, x2i, &_pow6r, &_pow6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c4r, _c4i, _pow6r, _pow6i, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow3r + _mul7r; _add8i = _pow3i + _mul7i;
        double _smul9r = 0, _smul9i = 0;
        c_mul(_arange1r, _arange1i, _add8r, _add8i, &_smul9r, &_smul9i);
        cRe[_si_idx] = _smul9r; cIm[_si_idx] = _smul9i;
    }
    double _add10r = 0, _add10i = 0;
    _add10r = x1r + x2r; _add10i = x1i + x2i;
    double _abs11r = 0, _abs11i = 0;
    _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
    { int _idx = 25; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _abs11r; cIm[_idx] = _abs11i; } }
    for (int _si = 0; _si < 25; _si++) {
        int _si_idx = _si + 26;
        double _arange12r = 0, _arange12i = 0;
        _arange12r = (double)(_si + 1); _arange12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow14r, &_pow14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 1.0;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow17r, &_pow17i);
        c_mul(_pow17r, _pow17i, x1r, x1i, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c15r, _c15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
        double _sub19r = 0, _sub19i = 0;
        _sub19r = _pow14r - _mul18r; _sub19i = _pow14i - _mul18i;
        double _smul20r = 0, _smul20i = 0;
        c_mul(_arange12r, _arange12i, _sub19r, _sub19i, &_smul20r, &_smul20i);
        cRe[_si_idx] = _smul20r; cIm[_si_idx] = _smul20i;
    }
    double _sin21r = 0, _sin21i = 0;
    c_sin(x1r, x1i, &_sin21r, &_sin21i);
    double _cf22r = 0, _cf22i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(_cf22r, _cf22i, _cf22r, _cf22i, &_pow24r, &_pow24i);
    double _mul25r = 0, _mul25i = 0;
    c_mul(_sin21r, _sin21i, _pow24r, _pow24i, &_mul25r, &_mul25i);
    { int _idx = 2; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    double _abs26r = 0, _abs26i = 0;
    _abs26r = c_abs(x2r, x2i); _abs26i = 0;
    double _c27r = 0, _c27i = 0;
    _c27r = 1.0; _c27i = 0;
    double _add28r = 0, _add28i = 0;
    _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
    double _log29r = 0, _log29i = 0;
    c_log(_add28r, _add28i, &_log29r, &_log29i);
    double _cf30r = 0, _cf30i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 51) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 3.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(_cf30r, _cf30i, _cf30r, _cf30i, &_pow32r, &_pow32i);
    c_mul(_pow32r, _pow32i, _cf30r, _cf30i, &_pow32r, &_pow32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_log29r, _log29i, _pow32r, _pow32i, &_mul33r, &_mul33i);
    { int _idx = 6; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    double _cf34r = 0, _cf34i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 51) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
    double _cf35r = 0, _cf35i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 51) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
    double _add36r = 0, _add36i = 0;
    _add36r = _cf34r + _cf35r; _add36i = _cf34i + _cf35i;
    { int _idx = 32; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    double _cf37r = 0, _cf37i = 0;
    { int _idx = 32; if (_idx >= 0 && _idx < 51) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
    double _cf38r = 0, _cf38i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 51) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
    double _sub39r = 0, _sub39i = 0;
    _sub39r = _cf37r - _cf38r; _sub39i = _cf37i - _cf38i;
    { int _idx = 36; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub39r; cIm[_idx] = _sub39i; } }
    double _cf40r = 0, _cf40i = 0;
    { int _idx = 32; if (_idx >= 0 && _idx < 51) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
    double _cf41r = 0, _cf41i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 51) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
    double _add42r = 0, _add42i = 0;
    _add42r = _cf40r + _cf41r; _add42i = _cf40i + _cf41i;
    { int _idx = 40; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    double _ang43r = 0, _ang43i = 0;
    _ang43r = c_arg(x1r, x1i); _ang43i = 0;
    double _ang44r = 0, _ang44i = 0;
    _ang44r = c_arg(x2r, x2i); _ang44i = 0;
    double _mul45r = 0, _mul45i = 0;
    c_mul(_ang43r, _ang43i, _ang44r, _ang44i, &_mul45r, &_mul45i);
    { int _idx = 49; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    double _cf46r = 0, _cf46i = 0;
    { int _idx = 40; if (_idx >= 0 && _idx < 51) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
    double _abs47r = 0, _abs47i = 0;
    _abs47r = c_abs(_cf46r, _cf46i); _abs47i = 0;
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _abs47r; cIm[_idx] = _abs47i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_9_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int k = 2; k < 52; k++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _sin3r = 0, _sin3i = 0;
        c_sin(k, 0, &_sin3r, &_sin3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs2r, _abs2i, _sin3r, _sin3i, &_mul4r, &_mul4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _cos6r = 0, _cos6i = 0;
        c_cos(k, 0, &_cos6r, &_cos6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_ang5r, _ang5i, _cos6r, _cos6i, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _mul4r + _mul7r; _add8i = _mul4i + _mul7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 1.0;
        double _add10r = 0, _add10i = 0;
        _add10r = k + _c9r; _add10i = 0 + _c9i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_add8r, _add8i, _abs11r, _abs11i, &_div12r, &_div12i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 51) { cRe[_idx] = _div12r; cIm[_idx] = _div12i; } }
    }
    double _cf13r = 0, _cf13i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 2.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(_cf13r, _cf13i, _cf13r, _cf13i, &_pow15r, &_pow15i);
    double _cf16r = 0, _cf16i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(_cf16r, _cf16i, _cf16r, _cf16i, &_pow18r, &_pow18i);
    double _sub19r = 0, _sub19i = 0;
    _sub19r = _pow15r - _pow18r; _sub19i = _pow15i - _pow18i;
    double _cf20r = 0, _cf20i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 51) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(_cf20r, _cf20i); _abs21i = 0;
    double _c22r = 0, _c22i = 0;
    _c22r = 1.0; _c22i = 0;
    double _add23r = 0, _add23i = 0;
    _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
    double _log24r = 0, _log24i = 0;
    c_log(_add23r, _add23i, &_log24r, &_log24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _sub19r + _log24r; _add25i = _sub19i + _log24i;
    { int _idx = 9; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add25r; cIm[_idx] = _add25i; } }
    double _sum26r = 0, _sum26i = 0;
    _sum26r = 0; _sum26i = 0;
    for (int _si = 0; _si < 19; _si++) { _sum26r += cRe[_si]; _sum26i += cIm[_si]; }
    double _mul27r = 0, _mul27i = 0;
    c_mul(_sum26r, _sum26i, x1r, x1i, &_mul27r, &_mul27i);
    { int _idx = 19; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _prod28r = 0, _prod28i = 0;
    _prod28r = 1; _prod28i = 0;
    for (int _pi = 0; _pi < 29; _pi++) { double _pr = _prod28r*cRe[_pi]-_prod28i*cIm[_pi]; double _pp = _prod28r*cIm[_pi]+_prod28i*cRe[_pi]; _prod28r=_pr; _prod28i=_pp; }
    double _mul29r = 0, _mul29i = 0;
    c_mul(_prod28r, _prod28i, x2r, x2i, &_mul29r, &_mul29i);
    { int _idx = 29; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    double _cf30r = 0, _cf30i = 0;
    { int _idx = 38; if (_idx >= 0 && _idx < 51) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
    double _cf31r = 0, _cf31i = 0;
    { int _idx = 37; if (_idx >= 0 && _idx < 51) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
    double _mul32r = 0, _mul32i = 0;
    c_mul(_cf30r, _cf30i, _cf31r, _cf31i, &_mul32r, &_mul32i);
    double _c33r = 0, _c33i = 0;
    _c33r = 1.0; _c33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul34r, &_mul34i);
    double _add35r = 0, _add35i = 0;
    _add35r = _c33r + _mul34r; _add35i = _c33i + _mul34i;
    double _div36r = 0, _div36i = 0;
    c_div(_mul32r, _mul32i, _add35r, _add35i, &_div36r, &_div36i);
    { int _idx = 39; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _div36r; cIm[_idx] = _div36i; } }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 40;
        double _unk37r = 0, _unk37i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=30), upper=Constant(value=40)) */
        double _cf38r = 0, _cf38i = 0;
        { int _idx = (int)(_unk37r); if (_idx >= 0 && _idx < 51) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
        double _re39r = 0, _re39i = 0;
        _re39r = _cf38r; _re39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _unk41r = 0, _unk41i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=10)) */
        double _cf42r = 0, _cf42i = 0;
        { int _idx = (int)(_unk41r); if (_idx >= 0 && _idx < 51) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
        double _im43r = 0, _im43i = 0;
        _im43r = _cf42i; _im43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c40r, _c40i, _im43r, _im43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _re39r + _mul44r; _add45i = _re39i + _mul44i;
        cRe[_si_idx] = _add45r; cIm[_si_idx] = _add45i;
    }
    double _sum46r = 0, _sum46i = 0;
    _sum46r = 0; _sum46i = 0;
    for (int _si = 0; _si < 50; _si++) { _sum46r += cRe[_si]; _sum46i += cIm[_si]; }
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sum46r; cIm[_idx] = _sum46i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_10_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    double _re2r = 0, _re2i = 0;
    _re2r = _mul1r; _re2i = 0;
    double _im3r = 0, _im3i = 0;
    _im3r = x2i; _im3i = 0;
    double _re4r = 0, _re4i = 0;
    _re4r = x1r; _re4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_im3r, _im3i, _re4r, _re4i, &_mul5r, &_mul5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _re2r + _mul5r; _add6i = _re2i + _mul5i;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add6r; cIm[_idx] = _add6i; } }
    double _mul7r = 0, _mul7i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul7r, &_mul7i);
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(_mul7r, _mul7i); _abs8i = 0;
    double _add9r = 0, _add9i = 0;
    _add9r = x1r + x2r; _add9i = x1i + x2i;
    double _ang10r = 0, _ang10i = 0;
    _ang10r = c_arg(_add9r, _add9i); _ang10i = 0;
    double _cos11r = 0, _cos11i = 0;
    c_cos(_ang10r, _ang10i, &_cos11r, &_cos11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_abs8r, _abs8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    for (int i = 2; i < 51; i++) {
        double _cf13r = 0, _cf13i = 0;
        { int _idx = (i - 2); if (_idx >= 0 && _idx < 51) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
        double _cf14r = 0, _cf14i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 51) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(_cf14r, _cf14i); _abs15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_cf13r, _cf13i, _abs15r, _abs15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = x1r + x2r; _add17i = x1i + x2i;
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(_add17r, _add17i); _ang18i = 0;
        double _sin19r = 0, _sin19i = 0;
        c_sin(_ang18r, _ang18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_mul16r, _mul16i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul20r; cIm[_idx] = _mul20i; } }
    }
    double _mul21r = 0, _mul21i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul21r, &_mul21i);
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(_mul21r, _mul21i); _abs22i = 0;
    double _log23r = 0, _log23i = 0;
    c_log(_abs22r, _abs22i, &_log23r, &_log23i);
    double _cf24r = 0, _cf24i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
    double _add25r = 0, _add25i = 0;
    _add25r = _log23r + _cf24r; _add25i = _log23i + _cf24i;
    double _cf26r = 0, _cf26i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
    double _add27r = 0, _add27i = 0;
    _add27r = _add25r + _cf26r; _add27i = _add25i + _cf26i;
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_11_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    double _add2r = 0, _add2i = 0;
    _add2r = x1r + x2r; _add2i = x1i + x2i;
    double _abs3r = 0, _abs3i = 0;
    _abs3r = c_abs(_add2r, _add2i); _abs3i = 0;
    double _c4r = 0, _c4i = 0;
    _c4r = 1.0; _c4i = 0;
    double _add5r = 0, _add5i = 0;
    _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
    double _log6r = 0, _log6i = 0;
    c_log(_add5r, _add5i, &_log6r, &_log6i);
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _mul1r - _log6r; _sub7i = _mul1i - _log6i;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub7r; cIm[_idx] = _sub7i; } }
    for (int i = 1; i < 50; i++) {
        double _sin8r = 0, _sin8i = 0;
        c_sin(i, 0, &_sin8r, &_sin8i);
        double _pow9r = 0, _pow9i = 0;
        c_powr(x1r, x1i, i, &_pow9r, &_pow9i);
        double _re10r = 0, _re10i = 0;
        _re10r = _pow9r; _re10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_powr(x2r, x2i, i, &_pow11r, &_pow11i);
        double _im12r = 0, _im12i = 0;
        _im12r = _pow11i; _im12i = 0;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _re10r - _im12r; _sub13i = _re10i - _im12i;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin8r, _sin8i, _sub13r, _sub13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(i, 0, &_cos15r, &_cos15i);
        double _pow16r = 0, _pow16i = 0;
        c_powr(x2r, x2i, i, &_pow16r, &_pow16i);
        double _re17r = 0, _re17i = 0;
        _re17r = _pow16r; _re17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(x1r, x1i, i, &_pow18r, &_pow18i);
        double _im19r = 0, _im19i = 0;
        _im19r = _pow18i; _im19i = 0;
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _re17r - _im19r; _sub20i = _re17i - _im19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_cos15r, _cos15i, _sub20r, _sub20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul14r + _mul21r; _add22i = _mul14i + _mul21i;
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
        double _cf23r = 0, _cf23i = 0;
        { int _idx = i; if (_idx >= 0 && _idx < 51) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
        double _cf24r = 0, _cf24i = 0;
        { int _idx = i; if (_idx >= 0 && _idx < 51) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_cf24r, _cf24i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1e-10; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _div28r = 0, _div28i = 0;
        c_div(_cf23r, _cf23i, _add27r, _add27i, &_div28r, &_div28i);
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _div28r; cIm[_idx] = _div28i; } }
    }
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(x1r, x1i); _abs29i = 0;
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(x2r, x2i); _abs30i = 0;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_abs29r, _abs29i, _abs30r, _abs30i, &_mul31r, &_mul31i);
    double _add32r = 0, _add32i = 0;
    _add32r = x1r + x2r; _add32i = x1i + x2i;
    double _ang33r = 0, _ang33i = 0;
    _ang33r = c_arg(_add32r, _add32i); _ang33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_mul31r, _mul31i, _ang33r, _ang33i, &_mul34r, &_mul34i);
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_12_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _im2r = 0, _im2i = 0;
    _im2r = x2i; _im2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(_im2r, _im2i, _im2r, _im2i, &_pow4r, &_pow4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c1r, _c1i, _pow4r, _pow4i, &_mul5r, &_mul5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(x1r, x1i, _mul5r, _mul5i, &_mul6r, &_mul6i);
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul6r; cIm[_idx] = _mul6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _re8r = 0, _re8i = 0;
    _re8r = x1r; _re8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(_re8r, _re8i, _re8r, _re8i, &_pow10r, &_pow10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c7r, _c7i, _pow10r, _pow10i, &_mul11r, &_mul11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(x2r, x2i, _mul11r, _mul11i, &_mul12r, &_mul12i);
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    for (int k = 2; k < 51; k++) {
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs13r, _abs13i, k, &_pow14r, &_pow14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 50.0; _c16i = 0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _c16r - k; _sub17i = _c16i - 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs15r, _abs15i, _sub17r, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _pow14r + _pow18r; _add19i = _pow14i + _pow18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_mul(k, 0, k, 0, &_pow21r, &_pow21i);
        double _div22r = 0, _div22i = 0;
        c_div(_add19r, _add19i, _pow21r, _pow21i, &_div22r, &_div22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(_mul24r, _mul24i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c23r, _c23i, _ang25r, _ang25i, &_mul26r, &_mul26i);
        double _exp27r = 0, _exp27i = 0;
        c_exp2(_mul26r, _mul26i, &_exp27r, &_exp27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_div22r, _div22i, _exp27r, _exp27i, &_mul28r, &_mul28i);
        { int _idx = k; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    double _mul29r = 0, _mul29i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul29r, &_mul29i);
    double _cos30r = 0, _cos30i = 0;
    c_cos(_mul29r, _mul29i, &_cos30r, &_cos30i);
    double _c31r = 0, _c31i = 0;
    _c31r = 0.0; _c31i = 1.0;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c31r, _c31i, x2r, x2i, &_mul32r, &_mul32i);
    double _sub33r = 0, _sub33i = 0;
    _sub33r = x1r - _mul32r; _sub33i = x1i - _mul32i;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_cos30r, _cos30i, _sub33r, _sub33i, &_mul34r, &_mul34i);
    { int _idx = 22; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    double _mul35r = 0, _mul35i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul35r, &_mul35i);
    double _sin36r = 0, _sin36i = 0;
    c_sin(_mul35r, _mul35i, &_sin36r, &_sin36i);
    double _c37r = 0, _c37i = 0;
    _c37r = 0.0; _c37i = 1.0;
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c37r, _c37i, x2r, x2i, &_mul38r, &_mul38i);
    double _sub39r = 0, _sub39i = 0;
    _sub39r = _mul38r - x1r; _sub39i = _mul38i - x1i;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_sin36r, _sin36i, _sub39r, _sub39i, &_mul40r, &_mul40i);
    { int _idx = 34; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    double _add41r = 0, _add41i = 0;
    _add41r = x1r + x2r; _add41i = x1i + x2i;
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(_add41r, _add41i); _abs42i = 0;
    double _log43r = 0, _log43i = 0;
    c_log(_abs42r, _abs42i, &_log43r, &_log43i);
    double _c44r = 0, _c44i = 0;
    _c44r = 3.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(_log43r, _log43i, _log43r, _log43i, &_pow45r, &_pow45i);
    c_mul(_pow45r, _pow45i, _log43r, _log43i, &_pow45r, &_pow45i);
    { int _idx = 49; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _pow45r; cIm[_idx] = _pow45i; } }
    double _conj46r = 0, _conj46i = 0;
    _conj46r = x1r; _conj46i = -(x1i);
    double _conj47r = 0, _conj47i = 0;
    _conj47r = x2r; _conj47i = -(x2i);
    double _mul48r = 0, _mul48i = 0;
    c_mul(_conj46r, _conj46i, _conj47r, _conj47i, &_mul48r, &_mul48i);
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_13_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double fib[] = {1.0, 1.0, 2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0, 89.0, 144.0, 233.0, 377.0, 610.0, 987.0, 1597.0, 2584.0, 4181.0};
    for (int n = 0; n < 19; n++) {
        double _arr1r = 0, _arr1i = 0;
        { int _idx = n; _arr1r = (_idx >= 0 && _idx < 19) ? fib[_idx] : 0.0; _arr1i = 0; }
        double _mul2r = 0, _mul2i = 0;
        c_mul(_arr1r, _arr1i, x1r, x1i, &_mul2r, &_mul2i);
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _cos4r = 0, _cos4i = 0;
        c_cos(_ang3r, _ang3i, &_cos4r, &_cos4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_mul2r, _mul2i, _cos4r, _cos4i, &_mul5r, &_mul5i);
        { int _idx = n; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
        double _arr6r = 0, _arr6i = 0;
        { int _idx = n; _arr6r = (_idx >= 0 && _idx < 19) ? fib[_idx] : 0.0; _arr6i = 0; }
        double _mul7r = 0, _mul7i = 0;
        c_mul(_arr6r, _arr6i, x1r, x1i, &_mul7r, &_mul7i);
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x2r, x2i); _ang8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(_ang8r, _ang8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_mul7r, _mul7i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        { int _idx = (n + 19); if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
        double _arr11r = 0, _arr11i = 0;
        { int _idx = n; _arr11r = (_idx >= 0 && _idx < 19) ? fib[_idx] : 0.0; _arr11i = 0; }
        double _mul12r = 0, _mul12i = 0;
        c_mul(_arr11r, _arr11i, x2r, x2i, &_mul12r, &_mul12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(_ang13r, _ang13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_mul12r, _mul12i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        { int _idx = (n + 38); if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    }
    double _mul16r = 0, _mul16i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul16r, &_mul16i);
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(_mul16r, _mul16i); _abs17i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _abs17r; cIm[_idx] = _abs17i; } }
    double _mul18r = 0, _mul18i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul18r, &_mul18i);
    double _abs19r = 0, _abs19i = 0;
    _abs19r = c_abs(_mul18r, _mul18i); _abs19i = 0;
    double _c20r = 0, _c20i = 0;
    _c20r = 1.0; _c20i = 0;
    double _add21r = 0, _add21i = 0;
    _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
    double _log22r = 0, _log22i = 0;
    c_log(_add21r, _add21i, &_log22r, &_log22i);
    { int _idx = 49; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _log22r; cIm[_idx] = _log22i; } }
    double _re23r = 0, _re23i = 0;
    _re23r = x1r; _re23i = 0;
    double _im24r = 0, _im24i = 0;
    _im24r = x2i; _im24i = 0;
    double _add25r = 0, _add25i = 0;
    _add25r = _re23r + _im24r; _add25i = _re23i + _im24i;
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add25r; cIm[_idx] = _add25i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_14_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double triangleNums[] = {1.0, 3.0, 6.0, 10.0, 15.0, 21.0, 28.0, 36.0, 45.0, 55.0, 66.0, 78.0, 91.0, 105.0, 120.0, 136.0, 153.0, 171.0, 190.0, 210.0, 231.0, 253.0, 276.0, 300.0, 325.0, 351.0, 378.0, 406.0, 435.0, 465.0, 496.0, 528.0, 561.0, 595.0, 630.0, 666.0, 703.0, 741.0, 780.0, 820.0, 861.0, 903.0, 946.0, 990.0, 1035.0, 1081.0, 1128.0, 1176.0, 1225.0, 1275.0};
    double _c1r = 0, _c1i = 0;
    _c1r = 3.0; _c1i = 0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, x2r, x2i, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    for (int k = 1; k < 51; k++) {
        double _arr4r = 0, _arr4i = 0;
        { int _idx = k; _arr4r = (_idx >= 0 && _idx < 50) ? triangleNums[_idx] : 0.0; _arr4i = 0; }
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(x2r, x2i, _log8r, _log8i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = x1r + _mul9r; _add10i = x1i + _mul9i;
        double _pow11r = 0, _pow11i = 0;
        c_powr(_add10r, _add10i, k, &_pow11r, &_pow11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_arr4r, _arr4i, _pow11r, _pow11i, &_mul12r, &_mul12i);
        double _arr13r = 0, _arr13i = 0;
        { int _idx = k; _arr13r = (_idx >= 0 && _idx < 50) ? triangleNums[_idx] : 0.0; _arr13i = 0; }
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(x1r, x1i, _log17r, _log17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = x2r + _mul18r; _add19i = x2i + _mul18i;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_add19r, _add19i, k, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_arr13r, _arr13i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul12r + _mul21r; _add22i = _mul12i + _mul21i;
        { int _idx = k; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    }
    double _abs23r = 0, _abs23i = 0;
    _abs23r = c_abs(x1r, x1i); _abs23i = 0;
    double _re24r = 0, _re24i = 0;
    _re24r = _abs23r; _re24i = 0;
    double _abs25r = 0, _abs25i = 0;
    _abs25r = c_abs(x2r, x2i); _abs25i = 0;
    double _im26r = 0, _im26i = 0;
    _im26r = _abs25i; _im26i = 0;
    double _add27r = 0, _add27i = 0;
    _add27r = _re24r + _im26r; _add27i = _re24i + _im26i;
    { int _idx = 42; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(x2r, x2i); _abs28i = 0;
    double _re29r = 0, _re29i = 0;
    _re29r = _abs28r; _re29i = 0;
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(x1r, x1i); _abs30i = 0;
    double _im31r = 0, _im31i = 0;
    _im31r = _abs30i; _im31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = _re29r + _im31r; _add32i = _re29i + _im31i;
    { int _idx = 20; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    double _mul33r = 0, _mul33i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul33r, &_mul33i);
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(_mul33r, _mul33i); _abs34i = 0;
    double _re35r = 0, _re35i = 0;
    _re35r = _abs34r; _re35i = 0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul36r, &_mul36i);
    double _conj37r = 0, _conj37i = 0;
    _conj37r = _mul36r; _conj37i = -(_mul36i);
    double _im38r = 0, _im38i = 0;
    _im38r = _conj37i; _im38i = 0;
    double _add39r = 0, _add39i = 0;
    _add39r = _re35r + _im38r; _add39i = _re35i + _im38i;
    { int _idx = 31; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    double _c40r = 0, _c40i = 0;
    _c40r = 2.0; _c40i = 0;
    double _sub41r = 0, _sub41i = 0;
    _sub41r = x1r - x2r; _sub41i = x1i - x2i;
    double _re42r = 0, _re42i = 0;
    _re42r = _sub41r; _re42i = 0;
    double _mul43r = 0, _mul43i = 0;
    c_mul(_c40r, _c40i, _re42r, _re42i, &_mul43r, &_mul43i);
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _sub45r = 0, _sub45i = 0;
    _sub45r = x1r - x2r; _sub45i = x1i - x2i;
    double _im46r = 0, _im46i = 0;
    _im46r = _sub45i; _im46i = 0;
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c44r, _c44i, _im46r, _im46i, &_mul47r, &_mul47i);
    double _add48r = 0, _add48i = 0;
    _add48r = _mul43r + _mul47r; _add48i = _mul43i + _mul47i;
    { int _idx = 27; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_15_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double primes[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0, 59.0, 61.0, 67.0, 71.0, 73.0, 79.0, 83.0, 89.0, 97.0, 101.0, 103.0, 107.0, 109.0, 113.0, 127.0, 131.0, 137.0, 139.0, 149.0, 151.0, 157.0, 163.0, 167.0, 173.0, 179.0, 181.0, 191.0, 193.0, 197.0, 199.0, 211.0, 223.0, 227.0, 229.0, 233.0, 239.0, 241.0};
    for (int i = 0; i < 71; i++) {
        double _arr1r = 0, _arr1i = 0;
        { int _idx = i; _arr1r = (_idx >= 0 && _idx < 53) ? primes[_idx] : 0.0; _arr1i = 0; }
        double _mul2r = 0, _mul2i = 0;
        c_mul(_arr1r, _arr1i, x1r, x1i, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 1.0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(x2r, x2i, i, &_pow4r, &_pow4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_c3r, _c3i, _pow4r, _pow4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _mul2r + _mul5r; _add6i = _mul2i + _mul5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _c7r + _abs8r; _add9i = _c7i + _abs8i;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_add9r, _add9i, i, &_pow10r, &_pow10i);
        double _div11r = 0, _div11i = 0;
        c_div(_add6r, _add6i, _pow10r, _pow10i, &_div11r, &_div11i);
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div11r; cIm[_idx] = _div11i; } }
    }
    double _sum12r = 0, _sum12i = 0;
    _sum12r = 0; _sum12i = 0;
    for (int _si = 0; _si < 70; _si++) { _sum12r += cRe[_si]; _sum12i += cIm[_si]; }
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sum12r; cIm[_idx] = _sum12i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_16_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    double _sub6r = 0, _sub6i = 0;
    _sub6r = _pow3r - _pow5r; _sub6i = _pow3i - _pow5i;
    double _re7r = 0, _re7i = 0;
    _re7r = _sub6r; _re7i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _re7r; cIm[_idx] = _re7i; } }
    static const double primes[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0, 59.0, 61.0, 67.0, 71.0, 73.0, 79.0, 83.0, 89.0, 97.0};
    for (int k = 2; k < 25; k++) {
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 51) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (k - 2); _arr9r = (_idx >= 0 && _idx < 25) ? primes[_idx] : 0.0; _arr9i = 0; }
        double _mul10r = 0, _mul10i = 0;
        c_mul(_cf8r, _cf8i, _arr9r, _arr9i, &_mul10r, &_mul10i);
        double _im11r = 0, _im11i = 0;
        _im11r = _mul10i; _im11i = 0;
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_im11r, _im11i, _ang12r, _ang12i, &_mul13r, &_mul13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_mul13r, _mul13i, _abs14r, _abs14i, &_mul15r, &_mul15i);
        { int _idx = k; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    }
    for (int k = 25; k < 50; k++) {
        double _cf16r = 0, _cf16i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 51) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
        double _arr17r = 0, _arr17i = 0;
        { int _idx = (k - 25); _arr17r = (_idx >= 0 && _idx < 25) ? primes[_idx] : 0.0; _arr17i = 0; }
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_mul(_arr17r, _arr17i, _arr17r, _arr17i, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_cf16r, _cf16i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(_mul20r, _mul20i); _abs21i = 0;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_abs21r, _abs21i, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _re24r = 0, _re24i = 0;
        _re24r = x1r; _re24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul23r, _mul23i, _re24r, _re24i, &_mul25r, &_mul25i);
        { int _idx = k; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    double _sum26r = 0, _sum26i = 0;
    _sum26r = 0; _sum26i = 0;
    for (int _si = 0; _si < 51; _si++) { _sum26r += cRe[_si]; _sum26i += cIm[_si]; }
    double _re27r = 0, _re27i = 0;
    _re27r = x2r; _re27i = 0;
    double _sin28r = 0, _sin28i = 0;
    c_sin(_re27r, _re27i, &_sin28r, &_sin28i);
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(x1r, x1i); _abs29i = 0;
    double _c30r = 0, _c30i = 0;
    _c30r = 1.0; _c30i = 0;
    double _add31r = 0, _add31i = 0;
    _add31r = _abs29r + _c30r; _add31i = _abs29i + _c30i;
    double _log32r = 0, _log32i = 0;
    c_log(_add31r, _add31i, &_log32r, &_log32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_sin28r, _sin28i, _log32r, _log32i, &_mul33r, &_mul33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _sum26r + _mul33r; _add34i = _sum26i + _mul33i;
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add34r; cIm[_idx] = _add34i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_17_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 0;
        double _add1r = 0, _add1i = 0;
        _add1r = x1r + x2r; _add1i = x1i + x2i;
        double _arange2r = 0, _arange2i = 0;
        _arange2r = (double)(_si + 1); _arange2i = 0;
        double _smul3r = 0, _smul3i = 0;
        c_mul(_add1r, _add1i, _arange2r, _arange2i, &_smul3r, &_smul3i);
        cRe[_si_idx] = _smul3r; cIm[_si_idx] = _smul3i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 10;
        double _sub4r = 0, _sub4i = 0;
        _sub4r = x1r - x2r; _sub4i = x1i - x2i;
        double _re5r = 0, _re5i = 0;
        _re5r = _sub4r; _re5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 3.0; _c6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_mul(_re5r, _re5i, _re5r, _re5i, &_pow7r, &_pow7i);
        c_mul(_pow7r, _pow7i, _re5r, _re5i, &_pow7r, &_pow7i);
        double _arange8r = 0, _arange8i = 0;
        _arange8r = (double)(_si + 11); _arange8i = 0;
        double _smul9r = 0, _smul9i = 0;
        c_mul(_pow7r, _pow7i, _arange8r, _arange8i, &_smul9r, &_smul9i);
        cRe[_si_idx] = _smul9r; cIm[_si_idx] = _smul9i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 20;
        double _add10r = 0, _add10i = 0;
        _add10r = x1r + x2r; _add10i = x1i + x2i;
        double _im11r = 0, _im11i = 0;
        _im11r = _add10i; _im11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_im11r, _im11i, _im11r, _im11i, &_pow13r, &_pow13i);
        double _arange14r = 0, _arange14i = 0;
        _arange14r = (double)(_si + 21); _arange14i = 0;
        double _smul15r = 0, _smul15i = 0;
        c_mul(_pow13r, _pow13i, _arange14r, _arange14i, &_smul15r, &_smul15i);
        cRe[_si_idx] = _smul15r; cIm[_si_idx] = _smul15i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 30;
        double _sub16r = 0, _sub16i = 0;
        _sub16r = x1r - x2r; _sub16i = x1i - x2i;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_sub16r, _sub16i); _abs17i = 0;
        double _arange18r = 0, _arange18i = 0;
        _arange18r = (double)(_si + 31); _arange18i = 0;
        double _smul19r = 0, _smul19i = 0;
        c_mul(_abs17r, _abs17i, _arange18r, _arange18i, &_smul19r, &_smul19i);
        cRe[_si_idx] = _smul19r; cIm[_si_idx] = _smul19i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 40;
        double _mul20r = 0, _mul20i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(_mul20r, _mul20i); _ang21i = 0;
        double _arange22r = 0, _arange22i = 0;
        _arange22r = (double)(_si + 41); _arange22i = 0;
        double _smul23r = 0, _smul23i = 0;
        c_mul(_ang21r, _ang21i, _arange22r, _arange22i, &_smul23r, &_smul23i);
        cRe[_si_idx] = _smul23r; cIm[_si_idx] = _smul23i;
    }
    double _sin24r = 0, _sin24i = 0;
    c_sin(x1r, x1i, &_sin24r, &_sin24i);
    double _cos25r = 0, _cos25i = 0;
    c_cos(x2r, x2i, &_cos25r, &_cos25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_sin24r, _sin24i, _cos25r, _cos25i, &_mul26r, &_mul26i);
    double _sin27r = 0, _sin27i = 0;
    c_sin(x2r, x2i, &_sin27r, &_sin27i);
    double _cos28r = 0, _cos28i = 0;
    c_cos(x1r, x1i, &_cos28r, &_cos28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_sin27r, _sin27i, _cos28r, _cos28i, &_mul29r, &_mul29i);
    double _add30r = 0, _add30i = 0;
    _add30r = _mul26r + _mul29r; _add30i = _mul26i + _mul29i;
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_18_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 1.0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(_mul1r, _mul1i, _c2r, _c2i, &_mul3r, &_mul3i);
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(_mul3r, _mul3i); _ang4i = 0;
        double arg = _ang4r; /* +_ang4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 1.0;
        double _add6r = 0, _add6i = 0;
        _add6r = x1r + _c5r; _add6i = x1i + _c5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 1.0;
        double _add8r = 0, _add8i = 0;
        _add8r = x2r + _c7r; _add8i = x2i + _c7i;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_add6r, _add6i, _add8r, _add8i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_mul9r, _mul9i); _abs10i = 0;
        double mod = _abs10r; /* +_abs10ii */
        double _c11r = 0, _c11i = 0;
        _c11r = 0.0; _c11i = 0;
        double cyclotomic = _c11r; /* +_c11ii */
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = i + _c12r; _add13i = 0 + _c12i;
        for (int k = 1; k < (int)(_add13r); k++) {
            double _c14r = 0, _c14i = 0;
            _c14r = 0.0; _c14i = 2.0;
            double _unk15r = 0, _unk15i = 0;
            /* WARNING: unhandled node Attribute(value=Name(id='np', ctx=Load()), attr='pi', ctx=Load()) */
            double _mul16r = 0, _mul16i = 0;
            c_mul(_c14r, _c14i, _unk15r, _unk15i, &_mul16r, &_mul16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_mul16r, _mul16i, k, 0, &_mul17r, &_mul17i);
            double _div18r = 0, _div18i = 0;
            c_div(_mul17r, _mul17i, i, 0, &_div18r, &_div18i);
            double _exp19r = 0, _exp19i = 0;
            c_exp2(_div18r, _div18i, &_exp19r, &_exp19i);
            double _sub20r = 0, _sub20i = 0;
            _sub20r = x1r - _exp19r; _sub20i = x1i - _exp19i;
            cyclotomic += _sub20r;
        }
        double _mul21r = 0, _mul21i = 0;
        c_mul(mod, 0, cyclotomic, 0, &_mul21r, &_mul21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul21r, _mul21i, arg, 0, &_mul22r, &_mul22i);
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    }
    double _mul23r = 0, _mul23i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul23r, &_mul23i);
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(_mul23r, _mul23i); _abs24i = 0;
    double _log25r = 0, _log25i = 0;
    c_log(_abs24r, _abs24i, &_log25r, &_log25i);
    double _c26r = 0, _c26i = 0;
    _c26r = 1.0; _c26i = 0;
    double _add27r = 0, _add27i = 0;
    _add27r = _log25r + _c26r; _add27i = _log25i + _c26i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_19_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _im2r = 0, _im2i = 0;
    _im2r = x2i; _im2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = _re1r + _im2r; _add3i = _re1i + _im2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    double _ang4r = 0, _ang4i = 0;
    _ang4r = c_arg(x1r, x1i); _ang4i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _ang4r; cIm[_idx] = _ang4i; } }
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(x2r, x2i); _abs5i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _abs5r; cIm[_idx] = _abs5i; } }
    double _sin6r = 0, _sin6i = 0;
    c_sin(x1r, x1i, &_sin6r, &_sin6i);
    double _cos7r = 0, _cos7i = 0;
    c_cos(x2r, x2i, &_cos7r, &_cos7i);
    double _add8r = 0, _add8i = 0;
    _add8r = _sin6r + _cos7r; _add8i = _sin6i + _cos7i;
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add8r; cIm[_idx] = _add8i; } }
    for (int _si = 0; _si < 6; _si++) {
        int _si_idx = _si + 4;
        double _arange9r = 0, _arange9i = 0;
        _arange9r = (double)(_si + 1); _arange9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 0.2; _c10i = 0;
        double _smul11r = 0, _smul11i = 0;
        c_mul(_arange9r, _arange9i, _c10r, _c10i, &_smul11r, &_smul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _sadd13r = 0, _sadd13i = 0;
        _sadd13r = _smul11r + _c12r; _sadd13i = _smul11i + _c12i;
        cRe[_si_idx] = _sadd13r; cIm[_si_idx] = _sadd13i;
    }
    double _abs14r = 0, _abs14i = 0;
    _abs14r = c_abs(x1r, x1i); _abs14i = 0;
    double _c15r = 0, _c15i = 0;
    _c15r = 1.0; _c15i = 0;
    double _add16r = 0, _add16i = 0;
    _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
    double _log17r = 0, _log17i = 0;
    c_log(_add16r, _add16i, &_log17r, &_log17i);
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(x2r, x2i); _abs18i = 0;
    double _c19r = 0, _c19i = 0;
    _c19r = 1.0; _c19i = 0;
    double _add20r = 0, _add20i = 0;
    _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
    double _log21r = 0, _log21i = 0;
    c_log(_add20r, _add20i, &_log21r, &_log21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _log17r + _log21r; _add22i = _log17i + _log21i;
    { int _idx = 10; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    for (int i = 11; i < 72; i++) {
        double _cf23r = 0, _cf23i = 0;
        { int _idx = (i - 2); if (_idx >= 0 && _idx < 71) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
        double _cf24r = 0, _cf24i = 0;
        { int _idx = (i - 3); if (_idx >= 0 && _idx < 71) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _mul25r = 0, _mul25i = 0;
        c_mul(i, 0, _cf24r, _cf24i, &_mul25r, &_mul25i);
        double _cf26r = 0, _cf26i = 0;
        { int _idx = (i - 4); if (_idx >= 0 && _idx < 71) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_cf26r, _cf26i); _abs27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _mul25r + _abs27r; _add28i = _mul25i + _abs27i;
        double _sin29r = 0, _sin29i = 0;
        c_sin(_add28r, _add28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_cf23r, _cf23i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _cf31r = 0, _cf31i = 0;
        { int _idx = (i - 5); if (_idx >= 0 && _idx < 71) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
        double _add32r = 0, _add32i = 0;
        _add32r = _mul30r + _cf31r; _add32i = _mul30i + _cf31i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_20_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 18; i++) {
        double _add1r = 0, _add1i = 0;
        _add1r = x1r + x2r; _add1i = x1i + x2i;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_add1r, _add1i, i, &_pow2r, &_pow2i);
        double _sub3r = 0, _sub3i = 0;
        _sub3r = x1r - x2r; _sub3i = x1i - x2i;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_sub3r, _sub3i, i, &_pow4r, &_pow4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(i, 0, _pow4r, _pow4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _pow2r + _mul5r; _add6i = _pow2i + _mul5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_c7r, _c7i, i, &_pow8r, &_pow8i);
        double _div9r = 0, _div9i = 0;
        c_div(_add6r, _add6i, _pow8r, _pow8i, &_div9r, &_div9i);
        { int _idx = ((i * 3) - 2); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div9r; cIm[_idx] = _div9i; } }
    }
    double _abs10r = 0, _abs10i = 0;
    _abs10r = c_abs(x1r, x1i); _abs10i = 0;
    double _ang11r = 0, _ang11i = 0;
    _ang11r = c_arg(x1r, x1i); _ang11i = 0;
    double _sin12r = 0, _sin12i = 0;
    c_sin(_ang11r, _ang11i, &_sin12r, &_sin12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_abs10r, _abs10i, _sin12r, _sin12i, &_mul13r, &_mul13i);
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    double _abs14r = 0, _abs14i = 0;
    _abs14r = c_abs(x2r, x2i); _abs14i = 0;
    double _ang15r = 0, _ang15i = 0;
    _ang15r = c_arg(x2r, x2i); _ang15i = 0;
    double _cos16r = 0, _cos16i = 0;
    c_cos(_ang15r, _ang15i, &_cos16r, &_cos16i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(_abs14r, _abs14i, _cos16r, _cos16i, &_mul17r, &_mul17i);
    { int _idx = 7; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul17r; cIm[_idx] = _mul17i; } }
    double _mul18r = 0, _mul18i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul18r, &_mul18i);
    double _abs19r = 0, _abs19i = 0;
    _abs19r = c_abs(_mul18r, _mul18i); _abs19i = 0;
    double _log20r = 0, _log20i = 0;
    c_log(_abs19r, _abs19i, &_log20r, &_log20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = x1r - x2r; _sub21i = x1i - x2i;
    double _ang22r = 0, _ang22i = 0;
    _ang22r = c_arg(_sub21r, _sub21i); _ang22i = 0;
    double _cos23r = 0, _cos23i = 0;
    c_cos(_ang22r, _ang22i, &_cos23r, &_cos23i);
    double _mul24r = 0, _mul24i = 0;
    c_mul(_log20r, _log20i, _cos23r, _cos23i, &_mul24r, &_mul24i);
    { int _idx = 18; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    double _mul25r = 0, _mul25i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul25r, &_mul25i);
    double _abs26r = 0, _abs26i = 0;
    _abs26r = c_abs(_mul25r, _mul25i); _abs26i = 0;
    double _add27r = 0, _add27i = 0;
    _add27r = x1r + x2r; _add27i = x1i + x2i;
    double _ang28r = 0, _ang28i = 0;
    _ang28r = c_arg(_add27r, _add27i); _ang28i = 0;
    double _cos29r = 0, _cos29i = 0;
    c_cos(_ang28r, _ang28i, &_cos29r, &_cos29i);
    double _mul30r = 0, _mul30i = 0;
    c_mul(_abs26r, _abs26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
    { int _idx = 36; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _unk31r = 0, _unk31i = 0;
    /* WARNING: unhandled node List(elts=[Constant(value=20), Constant(value=24), Constant(value=28), Constant(value=32), Constant(value=36), Constant(value=40), Constant(value=44), Constant(value=48), Constant(value=50)], ctx=Load()) */
    double _re32r = 0, _re32i = 0;
    _re32r = x1r; _re32i = 0;
    double _im33r = 0, _im33i = 0;
    _im33r = x2i; _im33i = 0;
    double _add34r = 0, _add34i = 0;
    _add34r = _re32r + _im33r; _add34i = _re32i + _im33i;
    { int _idx = (int)(_unk31r); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add34r; cIm[_idx] = _add34i; } }
    double _unk35r = 0, _unk35i = 0;
    /* WARNING: unhandled node List(elts=[Constant(value=22), Constant(value=26), Constant(value=30), Constant(value=34), Constant(value=38), Constant(value=42), Constant(value=46)], ctx=Load()) */
    double _im36r = 0, _im36i = 0;
    _im36r = x1i; _im36i = 0;
    double _re37r = 0, _re37i = 0;
    _re37r = x2r; _re37i = 0;
    double _add38r = 0, _add38i = 0;
    _add38r = _im36r + _re37r; _add38i = _im36i + _re37i;
    { int _idx = (int)(_unk35r); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    double _abs39r = 0, _abs39i = 0;
    _abs39r = c_abs(x1r, x1i); _abs39i = 0;
    double _c40r = 0, _c40i = 0;
    _c40r = 2.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(_abs39r, _abs39i, _abs39r, _abs39i, &_pow41r, &_pow41i);
    double _c42r = 0, _c42i = 0;
    _c42r = 2.0; _c42i = 0;
    double _ang43r = 0, _ang43i = 0;
    _ang43r = c_arg(x2r, x2i); _ang43i = 0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c42r, _c42i, _ang43r, _ang43i, &_mul44r, &_mul44i);
    double _sin45r = 0, _sin45i = 0;
    c_sin(_mul44r, _mul44i, &_sin45r, &_sin45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(_pow41r, _pow41i, _sin45r, _sin45i, &_mul46r, &_mul46i);
    { int _idx = 49; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    double _abs47r = 0, _abs47i = 0;
    _abs47r = c_abs(x2r, x2i); _abs47i = 0;
    double _c48r = 0, _c48i = 0;
    _c48r = 2.0; _c48i = 0;
    double _pow49r = 0, _pow49i = 0;
    c_mul(_abs47r, _abs47i, _abs47r, _abs47i, &_pow49r, &_pow49i);
    double _c50r = 0, _c50i = 0;
    _c50r = 2.0; _c50i = 0;
    double _ang51r = 0, _ang51i = 0;
    _ang51r = c_arg(x1r, x1i); _ang51i = 0;
    double _mul52r = 0, _mul52i = 0;
    c_mul(_c50r, _c50i, _ang51r, _ang51i, &_mul52r, &_mul52i);
    double _cos53r = 0, _cos53i = 0;
    c_cos(_mul52r, _mul52i, &_cos53r, &_cos53i);
    double _mul54r = 0, _mul54i = 0;
    c_mul(_pow49r, _pow49i, _cos53r, _cos53i, &_mul54r, &_mul54i);
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_21: too complex for auto-transpile, stubbed */
static void poly_21_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_22_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double primes[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0, 59.0, 61.0, 67.0, 71.0, 73.0, 79.0, 83.0, 89.0, 97.0};
    for (int _si = 0; _si < 25; _si++) {
        int _si_idx = _si + 0;
        double _unk1r = 0, _unk1i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=25)) */
        double _arr2r = 0, _arr2i = 0;
        { int _idx = (int)(_unk1r); _arr2r = (_idx >= 0 && _idx < 25) ? primes[_idx] : 0.0; _arr2i = 0; }
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(_re3r, _re3i, _re3r, _re3i, &_pow5r, &_pow5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
        double _im8r = 0, _im8i = 0;
        _im8r = _pow7i; _im8i = 0;
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _pow5r - _im8r; _sub9i = _pow5i - _im8i;
        double _re10r = 0, _re10i = 0;
        _re10r = x2r; _re10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(_re10r, _re10i, _re10r, _re10i, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _sub9r + _pow12r; _add13i = _sub9i + _pow12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow15r, &_pow15i);
        double _im16r = 0, _im16i = 0;
        _im16r = _pow15i; _im16i = 0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _add13r - _im16r; _sub17i = _add13i - _im16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_arr2r, _arr2i, _sub17r, _sub17i, &_mul18r, &_mul18i);
        cRe[_si_idx] = _mul18r; cIm[_si_idx] = _mul18i;
    }
    for (int _si = 0; _si < 25; _si++) {
        int _si_idx = _si + 25;
        double _unk19r = 0, _unk19i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=25)) */
        double _cf20r = 0, _cf20i = 0;
        { int _idx = (int)(_unk19r); if (_idx >= 0 && _idx < 71) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
        double _add21r = 0, _add21i = 0;
        _add21r = x1r + x2r; _add21i = x1i + x2i;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(_add21r, _add21i); _ang22i = 0;
        double _cos23r = 0, _cos23i = 0;
        c_cos(_ang22r, _ang22i, &_cos23r, &_cos23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_abs24r, _abs24i, _abs25r, _abs25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _cos23r + _sin27r; _add28i = _cos23i + _sin27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_cf20r, _cf20i, _add28r, _add28i, &_mul29r, &_mul29i);
        cRe[_si_idx] = _mul29r; cIm[_si_idx] = _mul29i;
    }
    double _sum30r = 0, _sum30i = 0;
    _sum30r = 0; _sum30i = 0;
    for (int _si = 0; _si < 50; _si++) { _sum30r += cRe[_si]; _sum30i += cIm[_si]; }
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sum30r; cIm[_idx] = _sum30i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_23_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul3r, &_mul3i);
    double _add4r = 0, _add4i = 0;
    _add4r = _c2r + _mul3r; _add4i = _c2i + _mul3i;
    double _add5r = 0, _add5i = 0;
    _add5r = x1r + x2r; _add5i = x1i + x2i;
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(_add5r, _add5i); _abs6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 1.0; _c7i = 0;
    double _add8r = 0, _add8i = 0;
    _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
    double _log9r = 0, _log9i = 0;
    c_log(_add8r, _add8i, &_log9r, &_log9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _add4r + _log9r; _add10i = _add4i + _log9i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add10r; cIm[_idx] = _add10i; } }
    double _add11r = 0, _add11i = 0;
    _add11r = x1r + x2r; _add11i = x1i + x2i;
    double _c12r = 0, _c12i = 0;
    _c12r = 1.0; _c12i = 0;
    double _mul13r = 0, _mul13i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul13r, &_mul13i);
    double _sub14r = 0, _sub14i = 0;
    _sub14r = _c12r - _mul13r; _sub14i = _c12i - _mul13i;
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(_sub14r, _sub14i); _abs15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
    double _log18r = 0, _log18i = 0;
    c_log(_add17r, _add17i, &_log18r, &_log18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _add11r + _log18r; _add19i = _add11i + _log18i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    for (int i = 3; i < 72; i++) {
        double _mul20r = 0, _mul20i = 0;
        c_mul(i, 0, x1r, x1i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 51.0; _c21i = 0;
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _c21r - i; _sub22i = _c21i - 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sub22r, _sub22i, x2r, x2i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(x2r, x2i, i, 0, &_mul25r, &_mul25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = x1r - _mul25r; _sub26i = x1i - _mul25i;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_sub26r, _sub26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _abs27r + _c28r; _add29i = _abs27i + _c28i;
        double _log30r = 0, _log30i = 0;
        c_log(_add29r, _add29i, &_log30r, &_log30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add24r + _log30r; _add31i = _add24i + _log30i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    double _cf32r = 0, _cf32i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
    double _cf33r = 0, _cf33i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 71) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
    double _add34r = 0, _add34i = 0;
    _add34r = _cf32r + _cf33r; _add34i = _cf32i + _cf33i;
    double _sin35r = 0, _sin35i = 0;
    c_sin(x1r, x1i, &_sin35r, &_sin35i);
    double _sub36r = 0, _sub36i = 0;
    _sub36r = _add34r - _sin35r; _sub36i = _add34i - _sin35i;
    { int _idx = 10; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    double _cf37r = 0, _cf37i = 0;
    { int _idx = 30; if (_idx >= 0 && _idx < 71) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
    double _cf38r = 0, _cf38i = 0;
    { int _idx = 40; if (_idx >= 0 && _idx < 71) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
    double _add39r = 0, _add39i = 0;
    _add39r = _cf37r + _cf38r; _add39i = _cf37i + _cf38i;
    double _cos40r = 0, _cos40i = 0;
    c_cos(x2r, x2i, &_cos40r, &_cos40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _add39r - _cos40r; _sub41i = _add39i - _cos40i;
    { int _idx = 20; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub41r; cIm[_idx] = _sub41i; } }
    double _cf42r = 0, _cf42i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 71) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
    double _cf43r = 0, _cf43i = 0;
    { int _idx = 40; if (_idx >= 0 && _idx < 71) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
    double _add44r = 0, _add44i = 0;
    _add44r = _cf42r + _cf43r; _add44i = _cf42i + _cf43i;
    double _sin45r = 0, _sin45i = 0;
    c_sin(x1r, x1i, &_sin45r, &_sin45i);
    double _add46r = 0, _add46i = 0;
    _add46r = _add44r + _sin45r; _add46i = _add44i + _sin45i;
    { int _idx = 30; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    double _cf47r = 0, _cf47i = 0;
    { int _idx = 30; if (_idx >= 0 && _idx < 71) { _cf47r = cRe[_idx]; _cf47i = cIm[_idx]; } }
    double _cf48r = 0, _cf48i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 71) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
    double _add49r = 0, _add49i = 0;
    _add49r = _cf47r + _cf48r; _add49i = _cf47i + _cf48i;
    double _cos50r = 0, _cos50i = 0;
    c_cos(x2r, x2i, &_cos50r, &_cos50i);
    double _sub51r = 0, _sub51i = 0;
    _sub51r = _add49r - _cos50r; _sub51i = _add49i - _cos50i;
    { int _idx = 40; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub51r; cIm[_idx] = _sub51i; } }
    double _cf52r = 0, _cf52i = 0;
    { int _idx = 40; if (_idx >= 0 && _idx < 71) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
    double _cf53r = 0, _cf53i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 71) { _cf53r = cRe[_idx]; _cf53i = cIm[_idx]; } }
    double _add54r = 0, _add54i = 0;
    _add54r = _cf52r + _cf53r; _add54i = _cf52i + _cf53i;
    double _sin55r = 0, _sin55i = 0;
    c_sin(x2r, x2i, &_sin55r, &_sin55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _add54r + _sin55r; _add56i = _add54i + _sin55i;
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_24_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _mul2r = 0, _mul2i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul2r; cIm[_idx] = _mul2i; } }
    for (int n = 3; n < 72; n++) {
        double _cf3r = 0, _cf3i = 0;
        { int _idx = (n - 2); if (_idx >= 0 && _idx < 71) { _cf3r = cRe[_idx]; _cf3i = cIm[_idx]; } }
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(_cf3r, _cf3i); _abs4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 1.0;
        double _cf6r = 0, _cf6i = 0;
        { int _idx = (n - 3); if (_idx >= 0 && _idx < 71) { _cf6r = cRe[_idx]; _cf6i = cIm[_idx]; } }
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(_cf6r, _cf6i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c5r, _c5i, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _abs4r + _mul8r; _add9i = _abs4i + _mul8i;
        double _add10r = 0, _add10i = 0;
        _add10r = x1r + x2r; _add10i = x1i + x2i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_c12r, _c12i, n, 0, &_div13r, &_div13i);
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs11r, _abs11i, _div13r, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(n, 0, x2r, x2i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 0.0; _c17i = 1.0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(n, 0, x1r, x1i, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c17r, _c17i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _cos16r + _mul20r; _add21i = _cos16i + _mul20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_pow14r, _pow14i, _add21r, _add21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _add9r + _mul22r; _add23i = _add9i + _mul22i;
        { int _idx = (n - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_25_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    double _re2r = 0, _re2i = 0;
    _re2r = _mul1r; _re2i = 0;
    double _div3r = 0, _div3i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div3r, &_div3i);
    double _im4r = 0, _im4i = 0;
    _im4r = _div3i; _im4i = 0;
    double _add5r = 0, _add5i = 0;
    _add5r = _re2r + _im4r; _add5i = _re2i + _im4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    for (int k = 1; k < 72; k++) {
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_abs6r, _abs6i, k, &_pow7r, &_pow7i);
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x2r, x2i); _ang8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_powr(_ang8r, _ang8i, k, &_pow9r, &_pow9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _pow7r + _pow9r; _add10i = _pow7i + _pow9i;
        double _add11r = 0, _add11i = 0;
        _add11r = x1r + k; _add11i = x1i + 0;
        double _sin12r = 0, _sin12i = 0;
        c_sin(_add11r, _add11i, &_sin12r, &_sin12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _add10r + _sin12r; _add13i = _add10i + _sin12i;
        double _add14r = 0, _add14i = 0;
        _add14r = x2r + k; _add14i = x2i + 0;
        double _cos15r = 0, _cos15i = 0;
        c_cos(_add14r, _add14i, &_cos15r, &_cos15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _add13r + _cos15r; _add16i = _add13i + _cos15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_mul17r, _mul17i); _abs18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_abs18r, _abs18i, k, &_pow19r, &_pow19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _pow19r + _c20r; _add21i = _pow19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _add16r - _log22r; _sub23i = _add16i - _log22i;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub23r; cIm[_idx] = _sub23i; } }
    }
    double _cf24r = 0, _cf24i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
    double _cf25r = 0, _cf25i = 0;
    { int _idx = 34; if (_idx >= 0 && _idx < 71) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
    double _mul26r = 0, _mul26i = 0;
    c_mul(_cf24r, _cf24i, _cf25r, _cf25i, &_mul26r, &_mul26i);
    double _re27r = 0, _re27i = 0;
    _re27r = _mul26r; _re27i = 0;
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _im29r = 0, _im29i = 0;
    _im29r = _mul28i; _im29i = 0;
    double _add30r = 0, _add30i = 0;
    _add30r = _re27r + _im29r; _add30i = _re27i + _im29i;
    { int _idx = 35; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 0.5; _c31i = 0;
    double _cf32r = 0, _cf32i = 0;
    { int _idx = 44; if (_idx >= 0 && _idx < 71) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
    double _conj33r = 0, _conj33i = 0;
    _conj33r = _cf32r; _conj33i = -(_cf32i);
    double _add34r = 0, _add34i = 0;
    _add34r = x1r + _conj33r; _add34i = x1i + _conj33i;
    double _add35r = 0, _add35i = 0;
    _add35r = _add34r + x2r; _add35i = _add34i + x2i;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c31r, _c31i, _add35r, _add35i, &_mul36r, &_mul36i);
    { int _idx = 45; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    double _cf37r = 0, _cf37i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
    double _cf38r = 0, _cf38i = 0;
    { int _idx = 34; if (_idx >= 0 && _idx < 71) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
    double _add39r = 0, _add39i = 0;
    _add39r = _cf37r + _cf38r; _add39i = _cf37i + _cf38i;
    double _cf40r = 0, _cf40i = 0;
    { int _idx = 44; if (_idx >= 0 && _idx < 71) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
    double _add41r = 0, _add41i = 0;
    _add41r = _add39r + _cf40r; _add41i = _add39i + _cf40i;
    double _re42r = 0, _re42i = 0;
    _re42r = x1r; _re42i = 0;
    double _add43r = 0, _add43i = 0;
    _add43r = _add41r + _re42r; _add43i = _add41i + _re42i;
    double _im44r = 0, _im44i = 0;
    _im44r = x2i; _im44i = 0;
    double _add45r = 0, _add45i = 0;
    _add45r = _add43r + _im44r; _add45i = _add43i + _im44i;
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_26_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _mul2r = 0, _mul2i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul2r; cIm[_idx] = _mul2i; } }
    for (int k = 3; k < 72; k++) {
        double _cf3r = 0, _cf3i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf3r = cRe[_idx]; _cf3i = cIm[_idx]; } }
        double _mul4r = 0, _mul4i = 0;
        c_mul(k, 0, _cf3r, _cf3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _cf6r = 0, _cf6i = 0;
        { int _idx = (k - 3); if (_idx >= 0 && _idx < 71) { _cf6r = cRe[_idx]; _cf6i = cIm[_idx]; } }
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, _cf6r, _cf6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _sin5r + _cos8r; _add9i = _sin5i + _cos8i;
        double v = _add9r; /* +_add9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(v, 0); _abs10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(v, 0, _abs10r, _abs10i, &_div11r, &_div11i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div11r; cIm[_idx] = _div11i; } }
    }
    double _sub12r = 0, _sub12i = 0;
    _sub12r = x1r - x2r; _sub12i = x1i - x2i;
    double _abs13r = 0, _abs13i = 0;
    _abs13r = c_abs(_sub12r, _sub12i); _abs13i = 0;
    double _add14r = 0, _add14i = 0;
    _add14r = x1r + x2r; _add14i = x1i + x2i;
    double _ang15r = 0, _ang15i = 0;
    _ang15r = c_arg(_add14r, _add14i); _ang15i = 0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_abs13r, _abs13i, _ang15r, _ang15i, &_mul16r, &_mul16i);
    { int _idx = 14; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul16r; cIm[_idx] = _mul16i; } }
    double _re17r = 0, _re17i = 0;
    _re17r = x2r; _re17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(x1r, x1i, _re17r, _re17i, &_mul18r, &_mul18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 1.0; _c19i = 0;
    double _add20r = 0, _add20i = 0;
    _add20r = _mul18r + _c19r; _add20i = _mul18i + _c19i;
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(_add20r, _add20i); _abs21i = 0;
    double _log22r = 0, _log22i = 0;
    c_log(_abs21r, _abs21i, &_log22r, &_log22i);
    double _im23r = 0, _im23i = 0;
    _im23r = x1i; _im23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(x2r, x2i, _im23r, _im23i, &_mul24r, &_mul24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 1.0; _c25i = 0;
    double _add26r = 0, _add26i = 0;
    _add26r = _mul24r + _c25r; _add26i = _mul24i + _c25i;
    double _abs27r = 0, _abs27i = 0;
    _abs27r = c_abs(_add26r, _add26i); _abs27i = 0;
    double _log28r = 0, _log28i = 0;
    c_log(_abs27r, _abs27i, &_log28r, &_log28i);
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _log22r - _log28r; _sub29i = _log22i - _log28i;
    { int _idx = 29; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(x1r, x1i); _abs30i = 0;
    double _sub31r = 0, _sub31i = 0;
    _sub31r = x1r - x2r; _sub31i = x1i - x2i;
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_sub31r, _sub31i); _abs32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_abs30r, _abs30i, _abs32r, _abs32i, &_mul33r, &_mul33i);
    { int _idx = 49; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    double _sum34r = 0, _sum34i = 0;
    _sum34r = 0; _sum34i = 0;
    for (int _si = 15; _si < 29; _si++) { _sum34r += cRe[_si]; _sum34i += cIm[_si]; }
    double _sum35r = 0, _sum35i = 0;
    _sum35r = 0; _sum35i = 0;
    for (int _si = 30; _si < 44; _si++) { _sum35r += cRe[_si]; _sum35i += cIm[_si]; }
    double _mul36r = 0, _mul36i = 0;
    c_mul(_sum34r, _sum34i, _sum35r, _sum35i, &_mul36r, &_mul36i);
    double _c37r = 0, _c37i = 0;
    _c37r = 2.0; _c37i = 0;
    double _pow38r = 0, _pow38i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow38r, &_pow38i);
    double _add39r = 0, _add39i = 0;
    _add39r = _mul36r + _pow38r; _add39i = _mul36i + _pow38i;
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_27_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 0;
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x2r, x2i); _abs2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(_abs1r, _abs1i, _abs2r, _abs2i, &_mul3r, &_mul3i);
        double _arange4r = 0, _arange4i = 0;
        _arange4r = (double)(_si + 1); _arange4i = 0;
        double _smul5r = 0, _smul5i = 0;
        c_mul(_mul3r, _mul3i, _arange4r, _arange4i, &_smul5r, &_smul5i);
        cRe[_si_idx] = _smul5r; cIm[_si_idx] = _smul5i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 10;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _arange7r = 0, _arange7i = 0;
        _arange7r = (double)(_si + 2); _arange7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_powr(_abs8r, _abs8i, 0, &_pow9r, &_pow9i);
        cRe[_si_idx] = _pow9r; cIm[_si_idx] = _pow9i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 20;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _arange11r = 0, _arange11i = 0;
        _arange11r = (double)(_si + 2); _arange11i = 0;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs12r, _abs12i, 0, &_pow13r, &_pow13i);
        cRe[_si_idx] = _pow13r; cIm[_si_idx] = _pow13i;
    }
    double _mul14r = 0, _mul14i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul14r, &_mul14i);
    double _re15r = 0, _re15i = 0;
    _re15r = _mul14r; _re15i = 0;
    { int _idx = 30; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _re15r; cIm[_idx] = _re15i; } }
    double _mul16r = 0, _mul16i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul16r, &_mul16i);
    double _im17r = 0, _im17i = 0;
    _im17r = _mul16i; _im17i = 0;
    { int _idx = 31; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _im17r; cIm[_idx] = _im17i; } }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 32;
        double _re18r = 0, _re18i = 0;
        _re18r = x1r; _re18i = 0;
        double _arange19r = 0, _arange19i = 0;
        _arange19r = (double)(_si + 1); _arange19i = 0;
        double _smul20r = 0, _smul20i = 0;
        c_mul(_re18r, _re18i, _arange19r, _arange19i, &_smul20r, &_smul20i);
        double _im21r = 0, _im21i = 0;
        _im21r = x2i; _im21i = 0;
        double _arange22r = 0, _arange22i = 0;
        _arange22r = (double)(_si + 1); _arange22i = 0;
        double _smul23r = 0, _smul23i = 0;
        c_mul(_im21r, _im21i, _arange22r, _arange22i, &_smul23r, &_smul23i);
        double _sadd24r = 0, _sadd24i = 0;
        _sadd24r = _smul20r + _smul23r; _sadd24i = _smul20i + _smul23i;
        cRe[_si_idx] = _sadd24r; cIm[_si_idx] = _sadd24i;
    }
    for (int _si = 0; _si < 9; _si++) {
        int _si_idx = _si + 42;
        double _re25r = 0, _re25i = 0;
        _re25r = x2r; _re25i = 0;
        double _arange26r = 0, _arange26i = 0;
        _arange26r = (double)(_si + 1); _arange26i = 0;
        double _smul27r = 0, _smul27i = 0;
        c_mul(_re25r, _re25i, _arange26r, _arange26i, &_smul27r, &_smul27i);
        double _im28r = 0, _im28i = 0;
        _im28r = x1i; _im28i = 0;
        double _arange29r = 0, _arange29i = 0;
        _arange29r = (double)(_si + 1); _arange29i = 0;
        double _smul30r = 0, _smul30i = 0;
        c_mul(_im28r, _im28i, _arange29r, _arange29i, &_smul30r, &_smul30i);
        double _sadd31r = 0, _sadd31i = 0;
        _sadd31r = _smul27r + _smul30r; _sadd31i = _smul27i + _smul30i;
        cRe[_si_idx] = _sadd31r; cIm[_si_idx] = _sadd31i;
    }
    for (int i = 0; i < 50; i++) {
        double _cf32r = 0, _cf32i = 0;
        { int _idx = (i + 1); if (_idx >= 0 && _idx < 71) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
        double _sin33r = 0, _sin33i = 0;
        c_sin(_cf32r, _cf32i, &_sin33r, &_sin33i);
        cRe[i] += _sin33r; cIm[i] += _sin33i;
    }
    for (int i = 51; i < 1; i += -1) {
        double _cf34r = 0, _cf34i = 0;
        { int _idx = (i - 2); if (_idx >= 0 && _idx < 71) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
        double _cos35r = 0, _cos35i = 0;
        c_cos(_cf34r, _cf34i, &_cos35r, &_cos35i);
        cRe[(i - 1)] -= _cos35r; cIm[(i - 1)] -= _cos35i;
    }
    double _ang36r = 0, _ang36i = 0;
    _ang36r = c_arg(x1r, x1i); _ang36i = 0;
    double _ang37r = 0, _ang37i = 0;
    _ang37r = c_arg(x2r, x2i); _ang37i = 0;
    double _add38r = 0, _add38i = 0;
    _add38r = _ang36r + _ang37r; _add38i = _ang36i + _ang37i;
    cRe[50] += _add38r; cIm[50] += _add38i;
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_28_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double primes[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0};
    for (int k = 1; k < 17; k++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _im2r = 0, _im2i = 0;
        _im2r = x2i; _im2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _re1r + _im2r; _add3i = _re1i + _im2i;
        double _arr4r = 0, _arr4i = 0;
        { int _idx = (k - 1); _arr4r = (_idx >= 0 && _idx < 16) ? primes[_idx] : 0.0; _arr4i = 0; }
        double _div5r = 0, _div5i = 0;
        c_div(_add3r, _add3i, _arr4r, _arr4i, &_div5r, &_div5i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div5r; cIm[_idx] = _div5i; } }
        double _im6r = 0, _im6i = 0;
        _im6r = x1i; _im6i = 0;
        double _re7r = 0, _re7i = 0;
        _re7r = x2r; _re7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _im6r + _re7r; _add8i = _im6i + _re7i;
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (k - 1); _arr9r = (_idx >= 0 && _idx < 16) ? primes[_idx] : 0.0; _arr9i = 0; }
        double _mul10r = 0, _mul10i = 0;
        c_mul(_add8r, _add8i, _arr9r, _arr9i, &_mul10r, &_mul10i);
        { int _idx = (71 - k); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    }
    for (int k = 17; k < 36; k++) {
        double _re11r = 0, _re11i = 0;
        _re11r = x1r; _re11i = 0;
        double _im12r = 0, _im12i = 0;
        _im12r = x1i; _im12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _re11r + _im12r; _add13i = _re11i + _im12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(_add13r, _add13i, _add13r, _add13i, &_pow15r, &_pow15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_pow15r, _pow15i, &_sin16r, &_sin16i);
        double _re17r = 0, _re17i = 0;
        _re17r = x2r; _re17i = 0;
        double _im18r = 0, _im18i = 0;
        _im18r = x2i; _im18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _re17r + _im18r; _add19i = _re17i + _im18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _c20r + k; _add21i = _c20i + 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_add19r, _add19i, _add21r, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin16r, _sin16i, _pow22r, _pow22i, &_mul23r, &_mul23i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    }
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(x1r, x1i); _abs24i = 0;
    double _abs25r = 0, _abs25i = 0;
    _abs25r = c_abs(x2r, x2i); _abs25i = 0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_abs24r, _abs24i, _abs25r, _abs25i, &_mul26r, &_mul26i);
    double _c27r = 0, _c27i = 0;
    _c27r = 1.0; _c27i = 0;
    double _add28r = 0, _add28i = 0;
    _add28r = _mul26r + _c27r; _add28i = _mul26i + _c27i;
    double _log29r = 0, _log29i = 0;
    c_log(_add28r, _add28i, &_log29r, &_log29i);
    double _sub30r = 0, _sub30i = 0;
    _sub30r = x2r - x1r; _sub30i = x2i - x1i;
    double _abs31r = 0, _abs31i = 0;
    _abs31r = c_abs(_sub30r, _sub30i); _abs31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = _log29r + _abs31r; _add32i = _log29i + _abs31i;
    { int _idx = 35; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    for (int _si = 0; _si < 15; _si++) {
        int _si_idx = _si + 36;
        double _add33r = 0, _add33i = 0;
        _add33r = x1r + x2r; _add33i = x1i + x2i;
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(_add33r, _add33i); _ang34i = 0;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = x1r - x2r; _sub35i = x1i - x2i;
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(_sub35r, _sub35i); _abs36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _ang34r + _abs36r; _add37i = _ang34i + _abs36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul38r, &_mul38i);
        double _conj39r = 0, _conj39i = 0;
        _conj39r = _mul38r; _conj39i = -(_mul38i);
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(_conj39r, _conj39i); _ang40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _add37r + _ang40r; _add41i = _add37i + _ang40i;
        cRe[_si_idx] = _add41r; cIm[_si_idx] = _add41i;
    }
    double _sum42r = 0, _sum42i = 0;
    _sum42r = 0; _sum42i = 0;
    for (int _si = 0; _si < 50; _si++) { _sum42r += cRe[_si]; _sum42i += cIm[_si]; }
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(_sum42r, _sum42i, _sum42r, _sum42i, &_pow44r, &_pow44i);
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow44r; cIm[_idx] = _pow44i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_29_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 0;
        cRe[_si_idx] = 0; cIm[_si_idx] = 0;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 5;
        cRe[_si_idx] = 0; cIm[_si_idx] = 0;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 10;
        cRe[_si_idx] = 0; cIm[_si_idx] = 0;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 15;
        cRe[_si_idx] = 0; cIm[_si_idx] = 0;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 20;
        cRe[_si_idx] = 0; cIm[_si_idx] = 0;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 30;
        double _arange1r = 0, _arange1i = 0;
        _arange1r = (double)(_si + 1); _arange1i = 0;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _smul3r = 0, _smul3i = 0;
        c_mul(_arange1r, _arange1i, _abs2r, _abs2i, &_smul3r, &_smul3i);
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _smul5r = 0, _smul5i = 0;
        c_mul(_smul3r, _smul3i, _abs4r, _abs4i, &_smul5r, &_smul5i);
        cRe[_si_idx] = _smul5r; cIm[_si_idx] = _smul5i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 40;
        cRe[_si_idx] = 0; cIm[_si_idx] = 0;
    }
    double _add6r = 0, _add6i = 0;
    _add6r = x1r + x2r; _add6i = x1i + x2i;
    double _abs7r = 0, _abs7i = 0;
    _abs7r = c_abs(_add6r, _add6i); _abs7i = 0;
    double _mul8r = 0, _mul8i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul8r, &_mul8i);
    double _ang9r = 0, _ang9i = 0;
    _ang9r = c_arg(_mul8r, _mul8i); _ang9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_abs7r, _abs7i, _ang9r, _ang9i, &_mul10r, &_mul10i);
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_30_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _im2r = 0, _im2i = 0;
        _im2r = x2i; _im2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(_re1r, _re1i, _im2r, _im2i, &_mul3r, &_mul3i);
        double _im4r = 0, _im4i = 0;
        _im4r = x1i; _im4i = 0;
        double _re5r = 0, _re5i = 0;
        _re5r = x2r; _re5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_im4r, _im4i, _re5r, _re5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _mul3r + _mul6r; _add7i = _mul3i + _mul6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_c8r, _c8i, i, 0, &_div9r, &_div9i);
        double _pow10r = 0, _pow10i = 0;
        c_powr(_add7r, _add7i, _div9r, &_pow10r, &_pow10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 50.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(i, 0, _c12r, _c12i, &_div13r, &_div13i);
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs11r, _abs11i, _div13r, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_pow10r, _pow10i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 25.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(i, 0, _c17r, _c17i, &_div18r, &_div18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang16r, _ang16i, _div18r, _div18i, &_mul19r, &_mul19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_mul19r, _mul19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_mul15r, _mul15i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 50.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(i, 0, _c23r, _c23i, &_div24r, &_div24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang22r, _ang22i, _div24r, _div24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul21r, _mul21i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x1r, x1i); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs28r + _c29r; _add30i = _abs28i + _c29i;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul27r + _log31r; _add32i = _mul27i + _log31i;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(x2r, x2i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _abs33r + _c34r; _add35i = _abs33i + _c34i;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add32r + _log36r; _add37i = _add32i + _log36i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_31_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int i = 1; i < 36; i++) {
        double _mul2r = 0, _mul2i = 0;
        c_mul(i, 0, x1r, x1i, &_mul2r, &_mul2i);
        double _cos3r = 0, _cos3i = 0;
        c_cos(_mul2r, _mul2i, &_cos3r, &_cos3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(i, 0, x2r, x2i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _cos3r + _sin5r; _add6i = _cos3i + _sin5i;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_abs7r, _abs7i, _abs8r, _abs8i, &_mul9r, &_mul9i);
        double _pow10r = 0, _pow10i = 0;
        c_powr(_mul9r, _mul9i, i, &_pow10r, &_pow10i);
        double _div11r = 0, _div11i = 0;
        c_div(_add6r, _add6i, _pow10r, _pow10i, &_div11r, &_div11i);
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div11r; cIm[_idx] = _div11i; } }
    }
    for (int i = 36; i < 72; i++) {
        double _pow12r = 0, _pow12i = 0;
        c_powr(x1r, x1i, i, &_pow12r, &_pow12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_pow12r, _pow12i, &_cos13r, &_cos13i);
        double _pow14r = 0, _pow14i = 0;
        c_powr(x2r, x2i, i, &_pow14r, &_pow14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_pow14r, _pow14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _cos13r + _sin15r; _add16i = _cos13i + _sin15i;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs17r, _abs17i, i, &_pow18r, &_pow18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _pow18r + _c19r; _add20i = _pow18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_add16r, _add16i, _log21r, _log21i, &_mul22r, &_mul22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_abs23r, _abs23i, i, &_pow24r, &_pow24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _pow24r + _c25r; _add26i = _pow24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul22r, _mul22i, _log27r, _log27i, &_mul28r, &_mul28i);
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_32_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int k = 1; k < 72; k++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_powr(_abs2r, _abs2i, k, &_pow3r, &_pow3i);
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 71.0; _c5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = _c5r - k; _sub6i = _c5i - 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_abs4r, _abs4i, _sub6r, &_pow7r, &_pow7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow3r + _pow7r; _add8i = _pow3i + _pow7i;
        double r = _add8r; /* +_add8ii */
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_ang9r, _ang9i, k, &_pow10r, &_pow10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x2r, x2i); _ang11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 71.0; _c12i = 0;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _c12r - k; _sub13i = _c12i - 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_ang11r, _ang11i, _sub13r, &_pow14r, &_pow14i);
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _pow10r - _pow14r; _sub15i = _pow10i - _pow14i;
        double theta = _sub15r; /* +_sub15ii */
        double _cos16r = 0, _cos16i = 0;
        c_cos(theta, 0, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(r, 0, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(theta, 0, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(r, 0, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_mul19r, _mul19i, _c20r, _c20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul17r + _mul21r; _add22i = _mul17i + _mul21i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    }
    for (int _si = 0; _si < 68; _si++) {
        int _si_idx = _si + 2;
        double _sub23r = 0, _sub23i = 0;
        _sub23r = x2r - x1r; _sub23i = x2i - x1i;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_sub23r, _sub23i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        cRe[_si_idx] += _log27r; cIm[_si_idx] += _log27i;
    }
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _conj29r = 0, _conj29i = 0;
    _conj29r = _mul28r; _conj29i = -(_mul28i);
    cRe[70] += _conj29r; cIm[70] += _conj29i;
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_33_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node Lambda(args=arguments(args=[arg(arg='z'), arg(arg='n')]), body=BinOp(left=BinOp(left=Name(id='z', ctx=Load()), op=Pow(), right=Name(id='n', ctx=Load())), op=Sub(), right=Constant(value=1))) */
    double f = _unk1r; /* +_unk1ii */
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 0;
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node ListComp(elt=BinOp(left=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='real', ctx=Load()), args=[Call(func=Name(id='f', ctx=Load()), args=[Name(id='t1', ctx=Load()), Name(id='n', ctx=Load())])]), op=Sub(), right=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='imag', ctx=Load()), args=[Call(func=Name(id='f', ctx=Load()), args=[Name(id='t2', ctx=Load()), Name(id='n', ctx=Load())])])), generators=[comprehension(target=Name(id='n', ctx=Store()), iter=Call(func=Name(id='range', ctx=Load()), args=[Constant(value=1), Constant(value=36)]), is_async=0)]) */
        cRe[_si_idx] = _unk2r; cIm[_si_idx] = _unk2i;
    }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 35;
        double _unk3r = 0, _unk3i = 0;
        /* WARNING: unhandled node ListComp(elt=BinOp(left=BinOp(left=BinOp(left=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='log', ctx=Load()), args=[Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='abs', ctx=Load()), args=[Call(func=Name(id='f', ctx=Load()), args=[Name(id='t2', ctx=Load()), Name(id='n', ctx=Load())])])]), op=Add(), right=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='angle', ctx=Load()), args=[Call(func=Name(id='f', ctx=Load()), args=[Name(id='t1', ctx=Load()), Name(id='n', ctx=Load())])])), op=Add(), right=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='sin', ctx=Load()), args=[Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='abs', ctx=Load()), args=[Call(func=Name(id='f', ctx=Load()), args=[Name(id='t1', ctx=Load()), Name(id='n', ctx=Load())])])])), op=Add(), right=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='cos', ctx=Load()), args=[Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='angle', ctx=Load()), args=[Call(func=Name(id='f', ctx=Load()), args=[Name(id='t2', ctx=Load()), Name(id='n', ctx=Load())])])])), generators=[comprehension(target=Name(id='n', ctx=Store()), iter=Call(func=Name(id='range', ctx=Load()), args=[Constant(value=1), Constant(value=36)]), is_async=0)]) */
        cRe[_si_idx] = _unk3r; cIm[_si_idx] = _unk3i;
    }
    double _prod4r = 0, _prod4i = 0;
    _prod4r = 1; _prod4i = 0;
    for (int _pi = 0; _pi < 70; _pi++) { double _pr = _prod4r*cRe[_pi]-_prod4i*cIm[_pi]; double _pp = _prod4r*cIm[_pi]+_prod4i*cRe[_pi]; _prod4r=_pr; _prod4i=_pp; }
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _prod4r; cIm[_idx] = _prod4i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_34_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
        double _mul2r = 0, _mul2i = 0;
        c_mul(i, 0, x1r, x1i, &_mul2r, &_mul2i);
        double _add3r = 0, _add3i = 0;
        _add3r = _mul2r + x2r; _add3i = _mul2i + x2i;
        double _sin4r = 0, _sin4i = 0;
        c_sin(_add3r, _add3i, &_sin4r, &_sin4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_mul1r, _mul1i, _sin4r, _sin4i, &_mul5r, &_mul5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(i, 0, x1r, x1i, &_mul6r, &_mul6i);
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _mul6r - x2r; _sub7i = _mul6i - x2i;
        double _cos8r = 0, _cos8i = 0;
        c_cos(_sub7r, _sub7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _cos8r; _add9i = _mul5i + _cos8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(i, 0, x2r, x2i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = x1r + _mul10r; _add11i = x1i + _mul10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(i, 0, _log15r, _log15i, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(i, 0, x2r, x2i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = x1r + _mul18r; _add19i = x1i + _mul18i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_add19r, _add19i); _abs20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _c17r + _abs20r; _add21i = _c17i + _abs20i;
        double _div22r = 0, _div22i = 0;
        c_div(_mul16r, _mul16i, _add21r, _add21i, &_div22r, &_div22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _add9r + _div22r; _add23i = _add9i + _div22i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    }
    double _c24r = 0, _c24i = 0;
    _c24r = 3.0; _c24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow26r, &_pow26i);
    double _c27r = 0, _c27i = 0;
    _c27r = 2.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow28r, &_pow28i);
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _pow26r - _pow28r; _sub29i = _pow26i - _pow28i;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c24r, _c24i, _sub29r, _sub29i, &_mul30r, &_mul30i);
    { int _idx = 12; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _cf31r = 0, _cf31i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 71) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
    double _mul32r = 0, _mul32i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul32r, &_mul32i);
    double _add33r = 0, _add33i = 0;
    _add33r = x1r + x2r; _add33i = x1i + x2i;
    double _ang34r = 0, _ang34i = 0;
    _ang34r = c_arg(_add33r, _add33i); _ang34i = 0;
    double _sin35r = 0, _sin35i = 0;
    c_sin(_ang34r, _ang34i, &_sin35r, &_sin35i);
    double _mul36r = 0, _mul36i = 0;
    c_mul(_mul32r, _mul32i, _sin35r, _sin35i, &_mul36r, &_mul36i);
    double _add37r = 0, _add37i = 0;
    _add37r = _cf31r + _mul36r; _add37i = _cf31i + _mul36i;
    { int _idx = 13; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    double _c38r = 0, _c38i = 0;
    _c38r = 2.0; _c38i = 0;
    double _cf39r = 0, _cf39i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 71) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c38r, _c38i, _cf39r, _cf39i, &_mul40r, &_mul40i);
    double _mul41r = 0, _mul41i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul41r, &_mul41i);
    double _sub42r = 0, _sub42i = 0;
    _sub42r = x1r - x2r; _sub42i = x1i - x2i;
    double _ang43r = 0, _ang43i = 0;
    _ang43r = c_arg(_sub42r, _sub42i); _ang43i = 0;
    double _cos44r = 0, _cos44i = 0;
    c_cos(_ang43r, _ang43i, &_cos44r, &_cos44i);
    double _mul45r = 0, _mul45i = 0;
    c_mul(_mul41r, _mul41i, _cos44r, _cos44i, &_mul45r, &_mul45i);
    double _sub46r = 0, _sub46i = 0;
    _sub46r = _mul40r - _mul45r; _sub46i = _mul40i - _mul45i;
    { int _idx = 14; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub46r; cIm[_idx] = _sub46i; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 3.0; _c47i = 0;
    double _cf48r = 0, _cf48i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 71) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
    double _mul49r = 0, _mul49i = 0;
    c_mul(_c47r, _c47i, _cf48r, _cf48i, &_mul49r, &_mul49i);
    double _cf50r = 0, _cf50i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 71) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
    double _sub51r = 0, _sub51i = 0;
    _sub51r = _mul49r - _cf50r; _sub51i = _mul49i - _cf50i;
    double _mul52r = 0, _mul52i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul52r, &_mul52i);
    double _c53r = 0, _c53i = 0;
    _c53r = 2.0; _c53i = 0;
    double _sub54r = 0, _sub54i = 0;
    _sub54r = x1r - x2r; _sub54i = x1i - x2i;
    double _ang55r = 0, _ang55i = 0;
    _ang55r = c_arg(_sub54r, _sub54i); _ang55i = 0;
    double _mul56r = 0, _mul56i = 0;
    c_mul(_c53r, _c53i, _ang55r, _ang55i, &_mul56r, &_mul56i);
    double _sin57r = 0, _sin57i = 0;
    c_sin(_mul56r, _mul56i, &_sin57r, &_sin57i);
    double _mul58r = 0, _mul58i = 0;
    c_mul(_mul52r, _mul52i, _sin57r, _sin57i, &_mul58r, &_mul58i);
    double _add59r = 0, _add59i = 0;
    _add59r = _sub51r + _mul58r; _add59i = _sub51i + _mul58i;
    { int _idx = 15; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    double _c60r = 0, _c60i = 0;
    _c60r = 2.0; _c60i = 0;
    double _cf61r = 0, _cf61i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 71) { _cf61r = cRe[_idx]; _cf61i = cIm[_idx]; } }
    double _mul62r = 0, _mul62i = 0;
    c_mul(_c60r, _c60i, _cf61r, _cf61i, &_mul62r, &_mul62i);
    double _c63r = 0, _c63i = 0;
    _c63r = 3.0; _c63i = 0;
    double _cf64r = 0, _cf64i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 71) { _cf64r = cRe[_idx]; _cf64i = cIm[_idx]; } }
    double _mul65r = 0, _mul65i = 0;
    c_mul(_c63r, _c63i, _cf64r, _cf64i, &_mul65r, &_mul65i);
    double _sub66r = 0, _sub66i = 0;
    _sub66r = _mul62r - _mul65r; _sub66i = _mul62i - _mul65i;
    double _cf67r = 0, _cf67i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 71) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
    double _add68r = 0, _add68i = 0;
    _add68r = _sub66r + _cf67r; _add68i = _sub66i + _cf67i;
    double _mul69r = 0, _mul69i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul69r, &_mul69i);
    double _c70r = 0, _c70i = 0;
    _c70r = 2.0; _c70i = 0;
    double _add71r = 0, _add71i = 0;
    _add71r = x1r + x2r; _add71i = x1i + x2i;
    double _ang72r = 0, _ang72i = 0;
    _ang72r = c_arg(_add71r, _add71i); _ang72i = 0;
    double _mul73r = 0, _mul73i = 0;
    c_mul(_c70r, _c70i, _ang72r, _ang72i, &_mul73r, &_mul73i);
    double _cos74r = 0, _cos74i = 0;
    c_cos(_mul73r, _mul73i, &_cos74r, &_cos74i);
    double _mul75r = 0, _mul75i = 0;
    c_mul(_mul69r, _mul69i, _cos74r, _cos74i, &_mul75r, &_mul75i);
    double _sub76r = 0, _sub76i = 0;
    _sub76r = _add68r - _mul75r; _sub76i = _add68i - _mul75i;
    { int _idx = 16; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub76r; cIm[_idx] = _sub76i; } }
    double _c77r = 0, _c77i = 0;
    _c77r = 2.0; _c77i = 0;
    double _cf78r = 0, _cf78i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 71) { _cf78r = cRe[_idx]; _cf78i = cIm[_idx]; } }
    double _mul79r = 0, _mul79i = 0;
    c_mul(_c77r, _c77i, _cf78r, _cf78i, &_mul79r, &_mul79i);
    double _c80r = 0, _c80i = 0;
    _c80r = 3.0; _c80i = 0;
    double _cf81r = 0, _cf81i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 71) { _cf81r = cRe[_idx]; _cf81i = cIm[_idx]; } }
    double _mul82r = 0, _mul82i = 0;
    c_mul(_c80r, _c80i, _cf81r, _cf81i, &_mul82r, &_mul82i);
    double _sub83r = 0, _sub83i = 0;
    _sub83r = _mul79r - _mul82r; _sub83i = _mul79i - _mul82i;
    double _mul84r = 0, _mul84i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul84r, &_mul84i);
    double _c85r = 0, _c85i = 0;
    _c85r = 2.0; _c85i = 0;
    double _mul86r = 0, _mul86i = 0;
    c_mul(_c85r, _c85i, x1r, x1i, &_mul86r, &_mul86i);
    double _sub87r = 0, _sub87i = 0;
    _sub87r = _mul86r - x2r; _sub87i = _mul86i - x2i;
    double _ang88r = 0, _ang88i = 0;
    _ang88r = c_arg(_sub87r, _sub87i); _ang88i = 0;
    double _sin89r = 0, _sin89i = 0;
    c_sin(_ang88r, _ang88i, &_sin89r, &_sin89i);
    double _mul90r = 0, _mul90i = 0;
    c_mul(_mul84r, _mul84i, _sin89r, _sin89i, &_mul90r, &_mul90i);
    double _add91r = 0, _add91i = 0;
    _add91r = _sub83r + _mul90r; _add91i = _sub83i + _mul90i;
    { int _idx = 69; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add91r; cIm[_idx] = _add91i; } }
    double _cf92r = 0, _cf92i = 0;
    { int _idx = 16; if (_idx >= 0 && _idx < 71) { _cf92r = cRe[_idx]; _cf92i = cIm[_idx]; } }
    double _cf93r = 0, _cf93i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 71) { _cf93r = cRe[_idx]; _cf93i = cIm[_idx]; } }
    double _mul94r = 0, _mul94i = 0;
    c_mul(_cf93r, _cf93i, x1r, x1i, &_mul94r, &_mul94i);
    double _mul95r = 0, _mul95i = 0;
    c_mul(_mul94r, _mul94i, x2r, x2i, &_mul95r, &_mul95i);
    double _c96r = 0, _c96i = 0;
    _c96r = 2.0; _c96i = 0;
    double _mul97r = 0, _mul97i = 0;
    c_mul(_c96r, _c96i, x1r, x1i, &_mul97r, &_mul97i);
    double _add98r = 0, _add98i = 0;
    _add98r = _mul97r + x2r; _add98i = _mul97i + x2i;
    double _ang99r = 0, _ang99i = 0;
    _ang99r = c_arg(_add98r, _add98i); _ang99i = 0;
    double _cos100r = 0, _cos100i = 0;
    c_cos(_ang99r, _ang99i, &_cos100r, &_cos100i);
    double _mul101r = 0, _mul101i = 0;
    c_mul(_mul95r, _mul95i, _cos100r, _cos100i, &_mul101r, &_mul101i);
    double _sub102r = 0, _sub102i = 0;
    _sub102r = _cf92r - _mul101r; _sub102i = _cf92i - _mul101i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub102r; cIm[_idx] = _sub102i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_35: too complex for auto-transpile, stubbed */
static void poly_35_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_36_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(x2r, x2i, _c1r, _c1i, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    double _abs4r = 0, _abs4i = 0;
    _abs4r = c_abs(x1r, x1i); _abs4i = 0;
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _pow6r = 0, _pow6i = 0;
    c_mul(_abs4r, _abs4i, _abs4r, _abs4i, &_pow6r, &_pow6i);
    double _ang7r = 0, _ang7i = 0;
    _ang7r = c_arg(x2r, x2i); _ang7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 2.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_mul(_ang7r, _ang7i, _ang7r, _ang7i, &_pow9r, &_pow9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _pow6r + _pow9r; _add10i = _pow6i + _pow9i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add10r; cIm[_idx] = _add10i; } }
    double _add11r = 0, _add11i = 0;
    _add11r = x1r + x2r; _add11i = x1i + x2i;
    double _sin12r = 0, _sin12i = 0;
    c_sin(_add11r, _add11i, &_sin12r, &_sin12i);
    { int _idx = 8; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sin12r; cIm[_idx] = _sin12i; } }
    for (int k = 9; k < 72; k++) {
        double _add13r = 0, _add13i = 0;
        _add13r = x1r + x2r; _add13i = x1i + x2i;
        double _re14r = 0, _re14i = 0;
        _re14r = _add13r; _re14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(k, 0, _re14r, _re14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _conj17r = 0, _conj17i = 0;
        _conj17r = x2r; _conj17i = -(x2i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(x1r, x1i, _conj17r, _conj17i, &_mul18r, &_mul18i);
        double _im19r = 0, _im19i = 0;
        _im19r = _mul18i; _im19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(k, 0, _im19r, _im19i, &_mul20r, &_mul20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_mul20r, _mul20i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _cos16r + _sin21r; _add22i = _cos16i + _sin21i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    }
    double _mul23r = 0, _mul23i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul23r, &_mul23i);
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(_mul23r, _mul23i); _abs24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 1.0; _c25i = 0;
    double _add26r = 0, _add26i = 0;
    _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
    double _log27r = 0, _log27i = 0;
    c_log(_add26r, _add26i, &_log27r, &_log27i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _log27r; cIm[_idx] = _log27i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_37: too complex for auto-transpile, stubbed */
static void poly_37_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_38_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 71; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
        double _add3r = 0, _add3i = 0;
        _add3r = _c1r + _mul2r; _add3i = _c1i + _mul2i;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_add3r, _add3i, k, &_pow4r, &_pow4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
        double _re7r = 0, _re7i = 0;
        _re7r = _mul6r; _re7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(_re7r, _re7i, _re7r, _re7i, &_pow9r, &_pow9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _c5r + _pow9r; _add10i = _c5i + _pow9i;
        double _div11r = 0, _div11i = 0;
        c_div(_pow4r, _pow4i, _add10r, _add10i, &_div11r, &_div11i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div11r; cIm[_idx] = _div11i; } }
    }
    double _abs12r = 0, _abs12i = 0;
    _abs12r = c_abs(x1r, x1i); _abs12i = 0;
    double _abs13r = 0, _abs13i = 0;
    _abs13r = c_abs(x2r, x2i); _abs13i = 0;
    double _add14r = 0, _add14i = 0;
    _add14r = _abs12r + _abs13r; _add14i = _abs12i + _abs13i;
    double _add15r = 0, _add15i = 0;
    _add15r = x1r + x2r; _add15i = x1i + x2i;
    double _ang16r = 0, _ang16i = 0;
    _ang16r = c_arg(_add15r, _add15i); _ang16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = _add14r + _ang16r; _add17i = _add14i + _ang16i;
    double _re18r = 0, _re18i = 0;
    _re18r = x1r; _re18i = 0;
    double _im19r = 0, _im19i = 0;
    _im19r = x2i; _im19i = 0;
    double _add20r = 0, _add20i = 0;
    _add20r = _re18r + _im19r; _add20i = _re18i + _im19i;
    double _sin21r = 0, _sin21i = 0;
    c_sin(_add20r, _add20i, &_sin21r, &_sin21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _add17r + _sin21r; _add22i = _add17i + _sin21i;
    double _re23r = 0, _re23i = 0;
    _re23r = x2r; _re23i = 0;
    double _c24r = 0, _c24i = 0;
    _c24r = 1.0; _c24i = 0;
    double _add25r = 0, _add25i = 0;
    _add25r = _re23r + _c24r; _add25i = _re23i + _c24i;
    double _abs26r = 0, _abs26i = 0;
    _abs26r = c_abs(_add25r, _add25i); _abs26i = 0;
    double _log27r = 0, _log27i = 0;
    c_log(_abs26r, _abs26i, &_log27r, &_log27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _add22r + _log27r; _add28i = _add22i + _log27i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add28r; cIm[_idx] = _add28i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_39_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = x1r; cIm[_idx] = x1i; } }
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = x2r; cIm[_idx] = x2i; } }
    for (int k = 3; k < 72; k++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(k, 0, x1r, x1i, &_mul1r, &_mul1i);
        double _sin2r = 0, _sin2i = 0;
        c_sin(_mul1r, _mul1i, &_sin2r, &_sin2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(k, 0, x2r, x2i, &_mul3r, &_mul3i);
        double _cos4r = 0, _cos4i = 0;
        c_cos(_mul3r, _mul3i, &_cos4r, &_cos4i);
        double _add5r = 0, _add5i = 0;
        _add5r = _sin2r + _cos4r; _add5i = _sin2i + _cos4i;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(k, 0); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _cf10r = 0, _cf10i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf10r = cRe[_idx]; _cf10i = cIm[_idx]; } }
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_cf10r, _cf10i); _abs11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log9r, _log9i, _abs11r, _abs11i, &_mul12r, &_mul12i);
        double _cf13r = 0, _cf13i = 0;
        { int _idx = (k - 3); if (_idx >= 0 && _idx < 71) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_cf13r, _cf13i); _abs14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_mul12r, _mul12i, _abs14r, _abs14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = x1r + x2r; _add16i = x1i + x2i;
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(_add16r, _add16i); _ang17i = 0;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_ang17r, _ang17i); _abs18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul15r, _mul15i, _abs18r, _abs18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add5r + _mul19r; _add20i = _add5i + _mul19i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_40: too complex for auto-transpile, stubbed */
static void poly_40_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_41_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _add2r = 0, _add2i = 0;
    _add2r = x1r + x2r; _add2i = x1i + x2i;
    double _sub3r = 0, _sub3i = 0;
    _sub3r = x1r - x2r; _sub3i = x1i - x2i;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_add2r, _add2i, _sub3r, _sub3i, &_mul4r, &_mul4i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    double _re5r = 0, _re5i = 0;
    _re5r = x1r; _re5i = 0;
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(_re5r, _re5i); _abs6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(_abs6r, _abs6i, _abs6r, _abs6i, &_pow8r, &_pow8i);
    double _im9r = 0, _im9i = 0;
    _im9r = x1i; _im9i = 0;
    double _abs10r = 0, _abs10i = 0;
    _abs10r = c_abs(_im9r, _im9i); _abs10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(_abs10r, _abs10i, _abs10r, _abs10i, &_pow12r, &_pow12i);
    double _add13r = 0, _add13i = 0;
    _add13r = _pow8r + _pow12r; _add13i = _pow8i + _pow12i;
    double _re14r = 0, _re14i = 0;
    _re14r = x2r; _re14i = 0;
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(_re14r, _re14i); _abs15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 2.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(_abs15r, _abs15i, _abs15r, _abs15i, &_pow17r, &_pow17i);
    double _add18r = 0, _add18i = 0;
    _add18r = _add13r + _pow17r; _add18i = _add13i + _pow17i;
    double _im19r = 0, _im19i = 0;
    _im19r = x2i; _im19i = 0;
    double _abs20r = 0, _abs20i = 0;
    _abs20r = c_abs(_im19r, _im19i); _abs20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(_abs20r, _abs20i, _abs20r, _abs20i, &_pow22r, &_pow22i);
    double _add23r = 0, _add23i = 0;
    _add23r = _add18r + _pow22r; _add23i = _add18i + _pow22i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    for (int i = 3; i < 72; i++) {
        double _cf24r = 0, _cf24i = 0;
        { int _idx = (i - 2); if (_idx >= 0 && _idx < 71) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _mul25r = 0, _mul25i = 0;
        c_mul(_cf24r, _cf24i, x1r, x1i, &_mul25r, &_mul25i);
        double _cf26r = 0, _cf26i = 0;
        { int _idx = (i - 3); if (_idx >= 0 && _idx < 71) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
        double _mul27r = 0, _mul27i = 0;
        c_mul(_cf26r, _cf26i, x2r, x2i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul25r + _mul27r; _add28i = _mul25i + _mul27i;
        double _cf29r = 0, _cf29i = 0;
        { int _idx = (i - 4); if (_idx >= 0 && _idx < 71) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
        double _add30r = 0, _add30i = 0;
        _add30r = _add28r + _cf29r; _add30i = _add28i + _cf29i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_42_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 0;
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _ang2r = 0, _ang2i = 0;
        _ang2r = c_arg(x1r, x1i); _ang2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(0, 0, _ang2r, _ang2i, &_mul3r, &_mul3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_mul3r, _mul3i, &_sin4r, &_sin4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_abs1r, _abs1i, _sin4r, _sin4i, &_mul5r, &_mul5i);
        cRe[_si_idx] = _mul5r; cIm[_si_idx] = _mul5i;
    }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 35;
        double _re6r = 0, _re6i = 0;
        _re6r = x2r; _re6i = 0;
        double _im7r = 0, _im7i = 0;
        _im7r = x2i; _im7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(0, 0, _im7r, _im7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_re6r, _re6i, _cos9r, _cos9i, &_mul10r, &_mul10i);
        cRe[_si_idx] = _mul10r; cIm[_si_idx] = _mul10i;
    }
    double _mul11r = 0, _mul11i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul11r, &_mul11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 0.0; _c12i = 1.0;
    double _unk13r = 0, _unk13i = 0;
    /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=70)) */
    double _cf14r = 0, _cf14i = 0;
    { int _idx = (int)(_unk13r); if (_idx >= 0 && _idx < 71) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(_cf14r, _cf14i); _abs15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
    double _log18r = 0, _log18i = 0;
    c_log(_add17r, _add17i, &_log18r, &_log18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c12r, _c12i, _log18r, _log18i, &_mul19r, &_mul19i);
    double _add20r = 0, _add20i = 0;
    _add20r = _mul11r + _mul19r; _add20i = _mul11i + _mul19i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_43_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(x1r, x1i); _abs2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 1.0; _c3i = 0;
    double _add4r = 0, _add4i = 0;
    _add4r = _abs2r + _c3r; _add4i = _abs2i + _c3i;
    double _log5r = 0, _log5i = 0;
    c_log(_add4r, _add4i, &_log5r, &_log5i);
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(x2r, x2i); _abs6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 1.0; _c7i = 0;
    double _add8r = 0, _add8i = 0;
    _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
    double _log9r = 0, _log9i = 0;
    c_log(_add8r, _add8i, &_log9r, &_log9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _log5r + _log9r; _add10i = _log5i + _log9i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add10r; cIm[_idx] = _add10i; } }
    for (int k = 2; k < 72; k++) {
        double _cf11r = 0, _cf11i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, _cf11r, _cf11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(k, 0, x1r, x1i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _sin13r + _cos15r; _add16i = _sin13i + _cos15i;
        double _cf17r = 0, _cf17i = 0;
        { int _idx = (k - 3); if (_idx >= 0 && _idx < 71) { _cf17r = cRe[_idx]; _cf17i = cIm[_idx]; } }
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, _cf17r, _cf17i, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _add16r - _sin19r; _sub20i = _add16i - _sin19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, x2r, x2i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _sub20r + _cos22r; _add23i = _sub20i + _cos22i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
        double _cf24r = 0, _cf24i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _cf25r = 0, _cf25i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_cf25r, _cf25i); _abs26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_cf24r, _cf24i, _abs26r, _abs26i, &_div27r, &_div27i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div27r; cIm[_idx] = _div27i; } }
    }
    double _re28r = 0, _re28i = 0;
    _re28r = x1r; _re28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 3.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_re28r, _re28i, _re28r, _re28i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, _re28r, _re28i, &_pow30r, &_pow30i);
    double _im31r = 0, _im31i = 0;
    _im31r = x2i; _im31i = 0;
    double _c32r = 0, _c32i = 0;
    _c32r = 3.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(_im31r, _im31i, _im31r, _im31i, &_pow33r, &_pow33i);
    c_mul(_pow33r, _pow33i, _im31r, _im31i, &_pow33r, &_pow33i);
    double _sub34r = 0, _sub34i = 0;
    _sub34r = _pow30r - _pow33r; _sub34i = _pow30i - _pow33i;
    { int _idx = 34; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub34r; cIm[_idx] = _sub34i; } }
    double _mul35r = 0, _mul35i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul35r, &_mul35i);
    double _abs36r = 0, _abs36i = 0;
    _abs36r = c_abs(_mul35r, _mul35i); _abs36i = 0;
    double _c37r = 0, _c37i = 0;
    _c37r = 2.0; _c37i = 0;
    double _pow38r = 0, _pow38i = 0;
    c_mul(_abs36r, _abs36i, _abs36r, _abs36i, &_pow38r, &_pow38i);
    double _mul39r = 0, _mul39i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul39r, &_mul39i);
    double _ang40r = 0, _ang40i = 0;
    _ang40r = c_arg(_mul39r, _mul39i); _ang40i = 0;
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _pow38r - _ang40r; _sub41i = _pow38i - _ang40i;
    { int _idx = 52; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub41r; cIm[_idx] = _sub41i; } }
    double _mul42r = 0, _mul42i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul42r, &_mul42i);
    double _re43r = 0, _re43i = 0;
    _re43r = _mul42r; _re43i = 0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul44r, &_mul44i);
    double _im45r = 0, _im45i = 0;
    _im45r = _mul44i; _im45i = 0;
    double _sub46r = 0, _sub46i = 0;
    _sub46r = _re43r - _im45r; _sub46i = _re43i - _im45i;
    double _mul47r = 0, _mul47i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul47r, &_mul47i);
    double _ang48r = 0, _ang48i = 0;
    _ang48r = c_arg(_mul47r, _mul47i); _ang48i = 0;
    double _add49r = 0, _add49i = 0;
    _add49r = _sub46r + _ang48r; _add49i = _sub46i + _ang48i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_44_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _sin1r = 0, _sin1i = 0;
    c_sin(x1r, x1i, &_sin1r, &_sin1i);
    double _cos2r = 0, _cos2i = 0;
    c_cos(x2r, x2i, &_cos2r, &_cos2i);
    double _mul3r = 0, _mul3i = 0;
    c_mul(_sin1r, _sin1i, _cos2r, _cos2i, &_mul3r, &_mul3i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    double _cos4r = 0, _cos4i = 0;
    c_cos(x1r, x1i, &_cos4r, &_cos4i);
    double _sin5r = 0, _sin5i = 0;
    c_sin(x2r, x2i, &_sin5r, &_sin5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _cos4r + _sin5r; _add6i = _cos4i + _sin5i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add6r; cIm[_idx] = _add6i; } }
    double _abs7r = 0, _abs7i = 0;
    _abs7r = c_abs(x1r, x1i); _abs7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 3.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_mul(_abs7r, _abs7i, _abs7r, _abs7i, &_pow9r, &_pow9i);
    c_mul(_pow9r, _pow9i, _abs7r, _abs7i, &_pow9r, &_pow9i);
    double _abs10r = 0, _abs10i = 0;
    _abs10r = c_abs(x2r, x2i); _abs10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 4.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(_abs10r, _abs10i, _abs10r, _abs10i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, _pow12r, _pow12i, &_pow12r, &_pow12i);
    double _sub13r = 0, _sub13i = 0;
    _sub13r = _pow9r - _pow12r; _sub13i = _pow9i - _pow12i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub13r; cIm[_idx] = _sub13i; } }
    double _ang14r = 0, _ang14i = 0;
    _ang14r = c_arg(x1r, x1i); _ang14i = 0;
    double _ang15r = 0, _ang15i = 0;
    _ang15r = c_arg(x2r, x2i); _ang15i = 0;
    double _sub16r = 0, _sub16i = 0;
    _sub16r = _ang14r - _ang15r; _sub16i = _ang14i - _ang15i;
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub16r; cIm[_idx] = _sub16i; } }
    double _mul17r = 0, _mul17i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul17r, &_mul17i);
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(_mul17r, _mul17i); _abs18i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _abs18r; cIm[_idx] = _abs18i; } }
    for (int k = 6; k < 36; k++) {
        double _mul19r = 0, _mul19i = 0;
        c_mul(k, 0, x1r, x1i, &_mul19r, &_mul19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_mul19r, _mul19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, x2r, x2i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _sin20r + _cos22r; _add23i = _sin20i + _cos22i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
        double _c24r = 0, _c24i = 0;
        _c24r = 70.0; _c24i = 0;
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _c24r - k; _sub25i = _c24i - 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_sub25r, _sub25i, x1r, x1i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 70.0; _c28i = 0;
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _c28r - k; _sub29i = _c28i - 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_sub29r, _sub29i, x2r, x2i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _sin27r - _cos31r; _sub32i = _sin27i - _cos31i;
        { int _idx = (k + 34); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub32r; cIm[_idx] = _sub32i; } }
    }
    double _add33r = 0, _add33i = 0;
    _add33r = x1r + x2r; _add33i = x1i + x2i;
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(_add33r, _add33i); _abs34i = 0;
    { int _idx = 35; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _abs34r; cIm[_idx] = _abs34i; } }
    double _mul35r = 0, _mul35i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul35r, &_mul35i);
    double _abs36r = 0, _abs36i = 0;
    _abs36r = c_abs(_mul35r, _mul35i); _abs36i = 0;
    double _c37r = 0, _c37i = 0;
    _c37r = 1.0; _c37i = 0;
    double _add38r = 0, _add38i = 0;
    _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
    double _log39r = 0, _log39i = 0;
    c_log(_add38r, _add38i, &_log39r, &_log39i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _log39r; cIm[_idx] = _log39i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_45_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(k, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _re4r = 0, _re4i = 0;
            _re4r = x2r; _re4i = 0;
            double _add5r = 0, _add5i = 0;
            _add5r = x1r + _re4r; _add5i = x1i + _re4i;
            double _mul6r = 0, _mul6i = 0;
            c_mul(k, 0, _add5r, _add5i, &_mul6r, &_mul6i);
            double _abs7r = 0, _abs7i = 0;
            _abs7r = c_abs(x1r, x1i); _abs7i = 0;
            double _mul8r = 0, _mul8i = 0;
            c_mul(_abs7r, _abs7i, k, 0, &_mul8r, &_mul8i);
            double _sin9r = 0, _sin9i = 0;
            c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(_mul6r, _mul6i, _sin9r, _sin9i, &_mul10r, &_mul10i);
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
        } else {
            double _im11r = 0, _im11i = 0;
            _im11r = x1i; _im11i = 0;
            double _sub12r = 0, _sub12i = 0;
            _sub12r = x2r - _im11r; _sub12i = x2i - _im11i;
            double _mul13r = 0, _mul13i = 0;
            c_mul(k, 0, _sub12r, _sub12i, &_mul13r, &_mul13i);
            double _ang14r = 0, _ang14i = 0;
            _ang14r = c_arg(x2r, x2i); _ang14i = 0;
            double _mul15r = 0, _mul15i = 0;
            c_mul(_ang14r, _ang14i, k, 0, &_mul15r, &_mul15i);
            double _cos16r = 0, _cos16i = 0;
            c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_mul13r, _mul13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul17r; cIm[_idx] = _mul17i; } }
        }
    }
    double _len18r = 0, _len18i = 0;
    /* WARNING: len() not directly translatable */
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _fdiv20r = 0, _fdiv20i = 0;
    c_div(_len18r, _len18i, _c19r, _c19i, &_fdiv20r, &_fdiv20i);
    _fdiv20r = floor(_fdiv20r); _fdiv20i = 0;
    for (int i = 2; i < (int)(_fdiv20r); i++) {
        double _cf21r = 0, _cf21i = 0;
        { int _idx = (i - 2); if (_idx >= 0 && _idx < 71) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.5; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_cf21r, _cf21i, _add24r, _add24i, &_mul25r, &_mul25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x2r, x2i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul25r + _log29r; _add30i = _mul25i + _log29i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
        double _len31r = 0, _len31i = 0;
        /* WARNING: len() not directly translatable */
        double _len32r = 0, _len32i = 0;
        /* WARNING: len() not directly translatable */
        double _cf33r = 0, _cf33i = 0;
        { int _idx = (((int)(_len32r) - i) + 1); if (_idx >= 0 && _idx < 71) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
        double _neg34r = 0, _neg34i = 0;
        _neg34r = -(_cf33r); _neg34i = -(_cf33i);
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(x2r, x2i); _abs35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 0.5; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _abs35r + _c36r; _add37i = _abs35i + _c36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_neg34r, _neg34i, _add37r, _add37i, &_mul38r, &_mul38i);
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(x1r, x1i); _abs39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _abs39r + _c40r; _add41i = _abs39i + _c40i;
        double _log42r = 0, _log42i = 0;
        c_log(_add41r, _add41i, &_log42r, &_log42i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = _mul38r - _log42r; _sub43i = _mul38i - _log42i;
        { int _idx = ((int)(_len31r) - i); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_46: too complex for auto-transpile, stubbed */
static void poly_46_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_47_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _sin2r = 0, _sin2i = 0;
    c_sin(x1r, x1i, &_sin2r, &_sin2i);
    double _cos3r = 0, _cos3i = 0;
    c_cos(x2r, x2i, &_cos3r, &_cos3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_sin2r, _sin2i, _cos3r, _cos3i, &_mul4r, &_mul4i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    for (int _si = 0; _si < 4; _si++) {
        int _si_idx = _si + 2;
        double _add5r = 0, _add5i = 0;
        _add5r = x1r + x2r; _add5i = x1i + x2i;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(_add5r, _add5i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _arange10r = 0, _arange10i = 0;
        _arange10r = (double)(_si + 1); _arange10i = 0;
        double _smul11r = 0, _smul11i = 0;
        c_mul(_log9r, _log9i, _arange10r, _arange10i, &_smul11r, &_smul11i);
        cRe[_si_idx] = _smul11r; cIm[_si_idx] = _smul11i;
    }
    for (int _si = 0; _si < 4; _si++) {
        int _si_idx = _si + 6;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_c12r, _c12i, _abs13r, _abs13i, &_mul14r, &_mul14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul14r, _mul14i, _log18r, _log18i, &_mul19r, &_mul19i);
        double _arange20r = 0, _arange20i = 0;
        _arange20r = (double)(_si + 1); _arange20i = 0;
        double _smul21r = 0, _smul21i = 0;
        c_mul(_mul19r, _mul19i, _arange20r, _arange20i, &_smul21r, &_smul21i);
        cRe[_si_idx] = _smul21r; cIm[_si_idx] = _smul21i;
    }
    for (int _si = 0; _si < 4; _si++) {
        int _si_idx = _si + 10;
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c22r, _c22i, _abs23r, _abs23i, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_mul24r, _mul24i, _log28r, _log28i, &_mul29r, &_mul29i);
        double _arange30r = 0, _arange30i = 0;
        _arange30r = (double)(_si + 1); _arange30i = 0;
        double _smul31r = 0, _smul31i = 0;
        c_mul(_mul29r, _mul29i, _arange30r, _arange30i, &_smul31r, &_smul31i);
        cRe[_si_idx] = _smul31r; cIm[_si_idx] = _smul31i;
    }
    for (int _si = 0; _si < 4; _si++) {
        int _si_idx = _si + 14;
        double _add32r = 0, _add32i = 0;
        _add32r = x1r + x2r; _add32i = x1i + x2i;
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(_add32r, _add32i); _ang33i = 0;
        double _arange34r = 0, _arange34i = 0;
        _arange34r = (double)(_si + 1); _arange34i = 0;
        double _smul35r = 0, _smul35i = 0;
        c_mul(_ang33r, _ang33i, _arange34r, _arange34i, &_smul35r, &_smul35i);
        cRe[_si_idx] = _smul35r; cIm[_si_idx] = _smul35i;
    }
    for (int _si = 0; _si < 8; _si++) {
        int _si_idx = _si + 18;
        double _cos36r = 0, _cos36i = 0;
        c_cos(x1r, x1i, &_cos36r, &_cos36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(x2r, x2i, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_cos36r, _cos36i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _arange39r = 0, _arange39i = 0;
        _arange39r = (double)(_si + 1); _arange39i = 0;
        double _smul40r = 0, _smul40i = 0;
        c_mul(_mul38r, _mul38i, _arange39r, _arange39i, &_smul40r, &_smul40i);
        cRe[_si_idx] = _smul40r; cIm[_si_idx] = _smul40i;
    }
    for (int _si = 0; _si < 8; _si++) {
        int _si_idx = _si + 26;
        double _sin41r = 0, _sin41i = 0;
        c_sin(x1r, x1i, &_sin41r, &_sin41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(x2r, x2i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_sin41r, _sin41i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _arange44r = 0, _arange44i = 0;
        _arange44r = (double)(_si + 1); _arange44i = 0;
        double _smul45r = 0, _smul45i = 0;
        c_mul(_mul43r, _mul43i, _arange44r, _arange44i, &_smul45r, &_smul45i);
        cRe[_si_idx] = _smul45r; cIm[_si_idx] = _smul45i;
    }
    for (int _si = 0; _si < 16; _si++) {
        int _si_idx = _si + 34;
        double _add46r = 0, _add46i = 0;
        _add46r = x1r + x2r; _add46i = x1i + x2i;
        double _arange47r = 0, _arange47i = 0;
        _arange47r = (double)(_si + 1); _arange47i = 0;
        double _sdiv48r = 0, _sdiv48i = 0;
        c_div(_add46r, _add46i, _arange47r, _arange47i, &_sdiv48r, &_sdiv48i);
        cRe[_si_idx] = _sdiv48r; cIm[_si_idx] = _sdiv48i;
    }
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = 0; cIm[_idx] = 0; } }
    for (int _si = 0; _si < 21; _si++) {
        int _si_idx = _si + 51;
        double _sub49r = 0, _sub49i = 0;
        _sub49r = x1r - x2r; _sub49i = x1i - x2i;
        double _arange50r = 0, _arange50i = 0;
        _arange50r = (double)(_si + 20); _arange50i = 0;
        double _sdiv51r = 0, _sdiv51i = 0;
        c_div(_sub49r, _sub49i, _arange50r, _arange50i, &_sdiv51r, &_sdiv51i);
        cRe[_si_idx] = _sdiv51r; cIm[_si_idx] = _sdiv51i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_48_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = x1r + x2r; _add2i = x1i + x2i;
        double _div3r = 0, _div3i = 0;
        c_div(i, 0, _add2r, _add2i, &_div3r, &_div3i);
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs1r, _abs1i, _div3r, &_pow4r, &_pow4i);
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow4r; cIm[_idx] = _pow4i; } }
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 0;
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 10.0; _c6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_ang5r, _ang5i, _c6r, _c6i, &_mul7r, &_mul7i);
        cRe[_si_idx] += _mul7r; cIm[_si_idx] += _mul7i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 10;
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x1r, x1i); _ang8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 10.0; _c9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_ang8r, _ang8i, _c9r, _c9i, &_mul10r, &_mul10i);
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 20;
        double _re11r = 0, _re11i = 0;
        _re11r = x1r; _re11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_re11r, _re11i, _re11r, _re11i, &_pow13r, &_pow13i);
        cRe[_si_idx] += _pow13r; cIm[_si_idx] += _pow13i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 30;
        double _im14r = 0, _im14i = 0;
        _im14r = x2i; _im14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(_im14r, _im14i, _im14r, _im14i, &_pow16r, &_pow16i);
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 40;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_abs17r, _abs17i, _log21r, _log21i, &_mul22r, &_mul22i);
        cRe[_si_idx] += _mul22r; cIm[_si_idx] += _mul22i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 50;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_abs23r, _abs23i, _log27r, _log27i, &_mul28r, &_mul28i);
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 60;
        double _add29r = 0, _add29i = 0;
        _add29r = x1r + x2r; _add29i = x1i + x2i;
        double _sin30r = 0, _sin30i = 0;
        c_sin(_add29r, _add29i, &_sin30r, &_sin30i);
        cRe[_si_idx] += _sin30r; cIm[_si_idx] += _sin30i;
    }
    double _prod31r = 0, _prod31i = 0;
    _prod31r = 1; _prod31i = 0;
    for (int _pi = 0; _pi < 70; _pi++) { double _pr = _prod31r*cRe[_pi]-_prod31i*cIm[_pi]; double _pp = _prod31r*cIm[_pi]+_prod31i*cRe[_pi]; _prod31r=_pr; _prod31i=_pp; }
    double _c32r = 0, _c32i = 0;
    _c32r = 70.0; _c32i = 0;
    double _div33r = 0, _div33i = 0;
    c_div(_prod31r, _prod31i, _c32r, _c32i, &_div33r, &_div33i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div33r; cIm[_idx] = _div33i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_49_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_mul(i, 0, i, 0, &_pow2r, &_pow2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 3.0; _c3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 1.0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_c3r, _c3i, _c4r, _c4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _pow2r + _mul5r; _add6i = _pow2i + _mul5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _add6r + _c7r; _add8i = _add6i + _c7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_add8r, _add8i, _pow10r, _pow10i, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(i, 0, i, 0, &_pow13r, &_pow13i);
        c_mul(_pow13r, _pow13i, i, 0, &_pow13r, &_pow13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(i, 0, i, 0, &_pow15r, &_pow15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = _pow13r - _pow15r; _sub16i = _pow13i - _pow15i;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _sub16r + _c17r; _add18i = _sub16i + _c17i;
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_add18r, _add18i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul11r + _mul21r; _add22i = _mul11i + _mul21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(x1r, x1i, _c23r, _c23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul24r + x2r; _add25i = _mul24i + x2i;
        double _sin26r = 0, _sin26i = 0;
        c_sin(_add25r, _add25i, &_sin26r, &_sin26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add22r + _sin26r; _add27i = _add22i + _sin26i;
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(x1r, x1i, _c28r, _c28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul29r - x2r; _sub30i = _mul29i - x2i;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(_sub30r, _sub30i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _abs31r + _c32r; _add33i = _abs31i + _c32i;
        double _log34r = 0, _log34i = 0;
        c_log(_add33r, _add33i, &_log34r, &_log34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add27r + _log34r; _add35i = _add27i + _log34i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
        double _c36r = 0, _c36i = 0;
        _c36r = 2.0; _c36i = 0;
        double _mod37r = 0, _mod37i = 0;
        _mod37r = fmod(i, _c36r); _mod37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 0;
        if (_mod37r == _c38r) {
            double _c39r = 0, _c39i = 0;
            _c39r = 0.0; _c39i = 1.0;
            double _mul40r = 0, _mul40i = 0;
            c_mul(_c39r, _c39i, x2r, x2i, &_mul40r, &_mul40i);
            double _add41r = 0, _add41i = 0;
            _add41r = x1r + _mul40r; _add41i = x1i + _mul40i;
            double _c42r = 0, _c42i = 0;
            _c42r = 2.0; _c42i = 0;
            double _pow43r = 0, _pow43i = 0;
            c_mul(_add41r, _add41i, _add41r, _add41i, &_pow43r, &_pow43i);
            cRe[(i - 1)] += _pow43r; cIm[(i - 1)] += _pow43i;
        } else {
            double _c44r = 0, _c44i = 0;
            _c44r = 3.0; _c44i = 0;
            double _mod45r = 0, _mod45i = 0;
            _mod45r = fmod(i, _c44r); _mod45i = 0;
            double _c46r = 0, _c46i = 0;
            _c46r = 0.0; _c46i = 0;
            if (_mod45r == _c46r) {
                double _c47r = 0, _c47i = 0;
                _c47r = 0.0; _c47i = 1.0;
                double _mul48r = 0, _mul48i = 0;
                c_mul(_c47r, _c47i, x2r, x2i, &_mul48r, &_mul48i);
                double _add49r = 0, _add49i = 0;
                _add49r = x1r + _mul48r; _add49i = x1i + _mul48i;
                double _abs50r = 0, _abs50i = 0;
                _abs50r = c_abs(_add49r, _add49i); _abs50i = 0;
                double _c51r = 0, _c51i = 0;
                _c51r = 3.0; _c51i = 0;
                double _pow52r = 0, _pow52i = 0;
                c_mul(_abs50r, _abs50i, _abs50r, _abs50i, &_pow52r, &_pow52i);
                c_mul(_pow52r, _pow52i, _abs50r, _abs50i, &_pow52r, &_pow52i);
                cRe[(i - 1)] += _pow52r; cIm[(i - 1)] += _pow52i;
            } else {
                double _c53r = 0, _c53i = 0;
                _c53r = 0.0; _c53i = 1.0;
                double _mul54r = 0, _mul54i = 0;
                c_mul(_c53r, _c53i, x2r, x2i, &_mul54r, &_mul54i);
                double _add55r = 0, _add55i = 0;
                _add55r = x1r + _mul54r; _add55i = x1i + _mul54i;
                double _re56r = 0, _re56i = 0;
                _re56r = _add55r; _re56i = 0;
                double _c57r = 0, _c57i = 0;
                _c57r = 4.0; _c57i = 0;
                double _pow58r = 0, _pow58i = 0;
                c_mul(_re56r, _re56i, _re56r, _re56i, &_pow58r, &_pow58i);
                c_mul(_pow58r, _pow58i, _pow58r, _pow58i, &_pow58r, &_pow58i);
                cRe[(i - 1)] += _pow58r; cIm[(i - 1)] += _pow58i;
            }
        }
    }
    double _c59r = 0, _c59i = 0;
    _c59r = 10000.0; _c59i = 0;
    { double _tr = cRe[0]*_c59r - cIm[0]*_c59i; cIm[0] = cRe[0]*_c59i + cIm[0]*_c59r; cRe[0] = _tr; }
    double _c60r = 0, _c60i = 0;
    _c60r = 1000.0; _c60i = 0;
    { double _tr = cRe[1]*_c60r - cIm[1]*_c60i; cIm[1] = cRe[1]*_c60i + cIm[1]*_c60r; cRe[1] = _tr; }
    double _c61r = 0, _c61i = 0;
    _c61r = 100.0; _c61i = 0;
    { double _tr = cRe[2]*_c61r - cIm[2]*_c61i; cIm[2] = cRe[2]*_c61i + cIm[2]*_c61r; cRe[2] = _tr; }
    double _c62r = 0, _c62i = 0;
    _c62r = 10.0; _c62i = 0;
    { double _tr = cRe[3]*_c62r - cIm[3]*_c62i; cIm[3] = cRe[3]*_c62i + cIm[3]*_c62r; cRe[3] = _tr; }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_50_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _sin2r = 0, _sin2i = 0;
    c_sin(_add1r, _add1i, &_sin2r, &_sin2i);
    double _sub3r = 0, _sub3i = 0;
    _sub3r = x1r - x2r; _sub3i = x1i - x2i;
    double _cos4r = 0, _cos4i = 0;
    c_cos(_sub3r, _sub3i, &_cos4r, &_cos4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _sin2r + _cos4r; _add5i = _sin2i + _cos4i;
    double complex_val = _add5r; /* +_add5ii */
    for (int k = 1; k < 72; k++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _mod7r = 0, _mod7i = 0;
        _mod7r = fmod(k, _c6r); _mod7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        if (_mod7r == _c8r) {
            double _abs9r = 0, _abs9i = 0;
            _abs9r = c_abs(k, 0); _abs9i = 0;
            double _div10r = 0, _div10i = 0;
            c_div(complex_val, 0, _abs9r, _abs9i, &_div10r, &_div10i);
            double _abs11r = 0, _abs11i = 0;
            _abs11r = c_abs(x1r, x1i); _abs11i = 0;
            double _sub12r = 0, _sub12i = 0;
            _sub12r = _div10r - _abs11r; _sub12i = _div10i - _abs11i;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub12r; cIm[_idx] = _sub12i; } }
        } else {
            double _abs13r = 0, _abs13i = 0;
            _abs13r = c_abs(k, 0); _abs13i = 0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(complex_val, 0, _abs13r, _abs13i, &_mul14r, &_mul14i);
            double _abs15r = 0, _abs15i = 0;
            _abs15r = c_abs(k, 0); _abs15i = 0;
            double _c16r = 0, _c16i = 0;
            _c16r = 1.0; _c16i = 0;
            double _add17r = 0, _add17i = 0;
            _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
            double _log18r = 0, _log18i = 0;
            c_log(_add17r, _add17i, &_log18r, &_log18i);
            double _add19r = 0, _add19i = 0;
            _add19r = _mul14r + _log18r; _add19i = _mul14i + _log18i;
            double _im20r = 0, _im20i = 0;
            _im20r = x2i; _im20i = 0;
            double _add21r = 0, _add21i = 0;
            _add21r = _add19r + _im20r; _add21i = _add19i + _im20i;
            double _re22r = 0, _re22i = 0;
            _re22r = x1r; _re22i = 0;
            double _sub23r = 0, _sub23i = 0;
            _sub23r = _add21r - _re22r; _sub23i = _add21i - _re22i;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub23r; cIm[_idx] = _sub23i; } }
        }
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _mod25r = 0, _mod25i = 0;
        _mod25r = fmod(k, _c24r); _mod25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        if (_mod25r == _c26r) {
            double _c27r = 0, _c27i = 0;
            _c27r = 3.0; _c27i = 0;
            double _cf28r = 0, _cf28i = 0;
            { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf28r = cRe[_idx]; _cf28i = cIm[_idx]; } }
            double _mul29r = 0, _mul29i = 0;
            c_mul(_c27r, _c27i, _cf28r, _cf28i, &_mul29r, &_mul29i);
            cRe[(k - 1)] += _mul29r; cIm[(k - 1)] += _mul29i;
        }
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _mod31r = 0, _mod31i = 0;
        _mod31r = fmod(k, _c30r); _mod31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 0;
        if (_mod31r == _c32r) {
            double _c33r = 0, _c33i = 0;
            _c33r = 5.0; _c33i = 0;
            double _cf34r = 0, _cf34i = 0;
            { int _idx = (k - 3); if (_idx >= 0 && _idx < 71) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
            double _mul35r = 0, _mul35i = 0;
            c_mul(_c33r, _c33i, _cf34r, _cf34i, &_mul35r, &_mul35i);
            cRe[(k - 1)] += _mul35r; cIm[(k - 1)] += _mul35i;
        }
        double _c36r = 0, _c36i = 0;
        _c36r = 7.0; _c36i = 0;
        double _mod37r = 0, _mod37i = 0;
        _mod37r = fmod(k, _c36r); _mod37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 0;
        if (_mod37r == _c38r) {
            double _c39r = 0, _c39i = 0;
            _c39r = 7.0; _c39i = 0;
            double _cf40r = 0, _cf40i = 0;
            { int _idx = (k - 4); if (_idx >= 0 && _idx < 71) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
            double _mul41r = 0, _mul41i = 0;
            c_mul(_c39r, _c39i, _cf40r, _cf40i, &_mul41r, &_mul41i);
            cRe[(k - 1)] += _mul41r; cIm[(k - 1)] += _mul41i;
        }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_51_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 71; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(k, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _add4r = 0, _add4i = 0;
            _add4r = x1r + x2r; _add4i = x1i + x2i;
            double _pow5r = 0, _pow5i = 0;
            c_powr(_add4r, _add4i, k, &_pow5r, &_pow5i);
            double _mul6r = 0, _mul6i = 0;
            c_mul(k, 0, _pow5r, _pow5i, &_mul6r, &_mul6i);
            double _sin7r = 0, _sin7i = 0;
            c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
            double _sub8r = 0, _sub8i = 0;
            _sub8r = x1r - x2r; _sub8i = x1i - x2i;
            double _pow9r = 0, _pow9i = 0;
            c_powr(_sub8r, _sub8i, k, &_pow9r, &_pow9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, _pow9r, _pow9i, &_mul10r, &_mul10i);
            double _cos11r = 0, _cos11i = 0;
            c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
            double _add12r = 0, _add12i = 0;
            _add12r = _sin7r + _cos11r; _add12i = _sin7i + _cos11i;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
        } else {
            double _re13r = 0, _re13i = 0;
            _re13r = x1r; _re13i = 0;
            double _pow14r = 0, _pow14i = 0;
            c_powr(_re13r, _re13i, k, &_pow14r, &_pow14i);
            double _im15r = 0, _im15i = 0;
            _im15r = x2i; _im15i = 0;
            double _pow16r = 0, _pow16i = 0;
            c_powr(_im15r, _im15i, k, &_pow16r, &_pow16i);
            double _add17r = 0, _add17i = 0;
            _add17r = _pow14r + _pow16r; _add17i = _pow14i + _pow16i;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
        }
    }
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(x1r, x1i); _abs18i = 0;
    double _c19r = 0, _c19i = 0;
    _c19r = 3.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(_abs18r, _abs18i, _abs18r, _abs18i, &_pow20r, &_pow20i);
    c_mul(_pow20r, _pow20i, _abs18r, _abs18i, &_pow20r, &_pow20i);
    double _ang21r = 0, _ang21i = 0;
    _ang21r = c_arg(x2r, x2i); _ang21i = 0;
    double _add22r = 0, _add22i = 0;
    _add22r = _pow20r + _ang21r; _add22i = _pow20i + _ang21i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_52_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 7.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_powr(x1r, x1i, 7.0, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 7.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_powr(x2r, x2i, 7.0, &_pow4r, &_pow4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _pow2r + _pow4r; _add5i = _pow2i + _pow4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    for (int k = 2; k < 36; k++) {
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, _log9r, _log9i, &_mul10r, &_mul10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul10r + _log14r; _add15i = _mul10i + _log14i;
        double _sin16r = 0, _sin16i = 0;
        c_sin(_add15r, _add15i, &_sin16r, &_sin16i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sin16r; cIm[_idx] = _sin16i; } }
    }
    for (int k = 36; k < 71; k++) {
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, _log20r, _log20i, &_mul21r, &_mul21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul21r - _log25r; _sub26i = _mul21i - _log25i;
        double _cos27r = 0, _cos27i = 0;
        c_cos(_sub26r, _sub26i, &_cos27r, &_cos27i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _cos27r; cIm[_idx] = _cos27i; } }
    }
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _add29r = 0, _add29i = 0;
    _add29r = x1r + x2r; _add29i = x1i + x2i;
    double _c30r = 0, _c30i = 0;
    _c30r = 2.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(_add29r, _add29i, _add29r, _add29i, &_pow31r, &_pow31i);
    double _sub32r = 0, _sub32i = 0;
    _sub32r = _mul28r - _pow31r; _sub32i = _mul28i - _pow31i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub32r; cIm[_idx] = _sub32i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_53_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 1.0;
    double _add3r = 0, _add3i = 0;
    _add3r = x1r + x2r; _add3i = x1i + x2i;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c2r, _c2i, _add3r, _add3i, &_mul4r, &_mul4i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    double _re5r = 0, _re5i = 0;
    _re5r = x1r; _re5i = 0;
    double _im6r = 0, _im6i = 0;
    _im6r = x2i; _im6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = _re5r + _im6r; _add7i = _re5i + _im6i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    double _cf8r = 0, _cf8i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
    double _sin9r = 0, _sin9i = 0;
    c_sin(_cf8r, _cf8i, &_sin9r, &_sin9i);
    double _cf10r = 0, _cf10i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { _cf10r = cRe[_idx]; _cf10i = cIm[_idx]; } }
    double _cos11r = 0, _cos11i = 0;
    c_cos(_cf10r, _cf10i, &_cos11r, &_cos11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_sin9r, _sin9i, _cos11r, _cos11i, &_mul12r, &_mul12i);
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    double _sub13r = 0, _sub13i = 0;
    _sub13r = x1r - x2r; _sub13i = x1i - x2i;
    double _abs14r = 0, _abs14i = 0;
    _abs14r = c_abs(_sub13r, _sub13i); _abs14i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _abs14r; cIm[_idx] = _abs14i; } }
    for (int k = 6; k < 37; k++) {
        double _cf15r = 0, _cf15i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf15r = cRe[_idx]; _cf15i = cIm[_idx]; } }
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_cf15r, _cf15i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_mul(_abs16r, _abs16i, _abs16r, _abs16i, &_pow18r, &_pow18i);
        double _cf19r = 0, _cf19i = 0;
        { int _idx = (k - 3); if (_idx >= 0 && _idx < 71) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_cf19r, _cf19i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _pow18r - _log23r; _sub24i = _pow18i - _log23i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub24r; cIm[_idx] = _sub24i; } }
    }
    for (int k = 36; k < 72; k++) {
        double _cf25r = 0, _cf25i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _cf26r = 0, _cf26i = 0;
        { int _idx = (k - 4); if (_idx >= 0 && _idx < 71) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
        double _mul27r = 0, _mul27i = 0;
        c_mul(_cf25r, _cf25i, _cf26r, _cf26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(_mul27r, _mul27i); _ang28i = 0;
        double _cf29r = 0, _cf29i = 0;
        { int _idx = (k - 3); if (_idx >= 0 && _idx < 71) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
        double _cf30r = 0, _cf30i = 0;
        { int _idx = (k - 6); if (_idx >= 0 && _idx < 71) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
        double _mul31r = 0, _mul31i = 0;
        c_mul(_cf29r, _cf29i, _cf30r, _cf30i, &_mul31r, &_mul31i);
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(_mul31r, _mul31i); _abs32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang28r, _ang28i, _abs32r, _abs32i, &_mul33r, &_mul33i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    double _re34r = 0, _re34i = 0;
    _re34r = x1r; _re34i = 0;
    double _im35r = 0, _im35i = 0;
    _im35r = x2i; _im35i = 0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_re34r, _re34i, _im35r, _im35i, &_mul36r, &_mul36i);
    double _re37r = 0, _re37i = 0;
    _re37r = x2r; _re37i = 0;
    double _im38r = 0, _im38i = 0;
    _im38r = x1i; _im38i = 0;
    double _mul39r = 0, _mul39i = 0;
    c_mul(_re37r, _re37i, _im38r, _im38i, &_mul39r, &_mul39i);
    double _sub40r = 0, _sub40i = 0;
    _sub40r = _mul36r - _mul39r; _sub40i = _mul36i - _mul39i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub40r; cIm[_idx] = _sub40i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_54_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(i, 0, x2r, x2i, &_mul1r, &_mul1i);
        double _c2r = 0, _c2i = 0;
        _c2r = 15.0; _c2i = 0;
        double _div3r = 0, _div3i = 0;
        c_div(_mul1r, _mul1i, _c2r, _c2i, &_div3r, &_div3i);
        double _cos4r = 0, _cos4i = 0;
        c_cos(_div3r, _div3i, &_cos4r, &_cos4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(x1r, x1i, _cos4r, _cos4i, &_mul5r, &_mul5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(i, 0, x1r, x1i, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 15.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(x2r, x2i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul5r + _mul10r; _add11i = _mul5i + _mul10i;
        double z = _add11r; /* +_add11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(z, 0); _ang12i = 0;
        double phi = _ang12r; /* +_ang12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(z, 0); _abs13i = 0;
        double r = _abs13r; /* +_abs13ii */
        double _c14r = 0, _c14i = 0;
        _c14r = 0.0; _c14i = 1.0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_c14r, _c14i, phi, 0, &_mul15r, &_mul15i);
        double _exp16r = 0, _exp16i = 0;
        c_exp2(_mul15r, _mul15i, &_exp16r, &_exp16i);
        double _pow17r = 0, _pow17i = 0;
        c_powr(_exp16r, _exp16i, i, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(r, 0, _pow17r, _pow17i, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _neg20r = 0, _neg20i = 0;
        _neg20r = -(_c19r); _neg20i = -(_c19i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = i + _c21r; _add22i = 0 + _c21i;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_neg20r, _neg20i, _add22r, &_pow23r, &_pow23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_mul(i, 0, i, 0, &_pow25r, &_pow25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_pow23r, _pow23i, _pow25r, _pow25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul18r + _mul26r; _add27i = _mul18i + _mul26i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
    }
    for (int _si = 0; _si < 30; _si++) {
        int _si_idx = _si + 0;
        double _unk28r = 0, _unk28i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=30)) */
        double _cf29r = 0, _cf29i = 0;
        { int _idx = (int)(_unk28r); if (_idx >= 0 && _idx < 71) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x2r, x2i); _abs31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_abs30r, _abs30i, _abs31r, _abs31i, &_mul32r, &_mul32i);
        double _arange33r = 0, _arange33i = 0;
        _arange33r = (double)(_si + 1); _arange33i = 0;
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(x1r, x1i); _abs34i = 0;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(x2r, x2i); _abs35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_abs34r, _abs34i, _abs35r, _abs35i, &_mul36r, &_mul36i);
        double _pow37r = 0, _pow37i = 0;
        c_powr(_mul36r, _mul36i, 0, &_pow37r, &_pow37i);
        double _smul38r = 0, _smul38i = 0;
        c_mul(_cf29r, _cf29i, _pow37r, _pow37i, &_smul38r, &_smul38i);
        cRe[_si_idx] = _smul38r; cIm[_si_idx] = _smul38i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_55_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _im3r = 0, _im3i = 0;
    _im3r = x2i; _im3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_re2r, _re2i, _im3r, _im3i, &_mul4r, &_mul4i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    double _re5r = 0, _re5i = 0;
    _re5r = x2r; _re5i = 0;
    double _im6r = 0, _im6i = 0;
    _im6r = x1i; _im6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_re5r, _re5i, _im6r, _im6i, &_mul7r, &_mul7i);
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    for (int _si = 0; _si < 7; _si++) {
        int _si_idx = _si + 3;
        double _np8r = 0, _np8i = 0;
        /* WARNING: unhandled np.linspace */
        cRe[_si_idx] = _np8r; cIm[_si_idx] = _np8i;
    }
    for (int _si = 0; _si < 20; _si++) {
        int _si_idx = _si + 10;
        double _unk9r = 0, _unk9i = 0;
        /* WARNING: unhandled node ListComp(elt=BinOp(left=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='cos', ctx=Load()), args=[Subscript(value=Name(id='cf', ctx=Load()), slice=BinOp(left=Name(id='i', ctx=Load()), op=Sub(), right=Constant(value=1)), ctx=Load())]), op=Add(), right=BinOp(left=BinOp(left=BinOp(left=Name(id='t1', ctx=Load()), op=Add(), right=Name(id='t2', ctx=Load())), op=Pow(), right=Name(id='i', ctx=Load())), op=Div(), right=BinOp(left=Name(id='i', ctx=Load()), op=Add(), right=Constant(value=1)))), generators=[comprehension(target=Name(id='i', ctx=Store()), iter=Call(func=Name(id='range', ctx=Load()), args=[Constant(value=11), Constant(value=31)]), is_async=0)]) */
        cRe[_si_idx] = _unk9r; cIm[_si_idx] = _unk9i;
    }
    for (int _si = 0; _si < 20; _si++) {
        int _si_idx = _si + 30;
        double _unk10r = 0, _unk10i = 0;
        /* WARNING: unhandled node ListComp(elt=BinOp(left=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='sin', ctx=Load()), args=[Subscript(value=Name(id='cf', ctx=Load()), slice=BinOp(left=Name(id='i', ctx=Load()), op=Sub(), right=Constant(value=1)), ctx=Load())]), op=Add(), right=BinOp(left=BinOp(left=BinOp(left=Name(id='t1', ctx=Load()), op=Sub(), right=Name(id='t2', ctx=Load())), op=Pow(), right=Name(id='i', ctx=Load())), op=Div(), right=BinOp(left=Name(id='i', ctx=Load()), op=Add(), right=Constant(value=1)))), generators=[comprehension(target=Name(id='i', ctx=Store()), iter=Call(func=Name(id='range', ctx=Load()), args=[Constant(value=31), Constant(value=51)]), is_async=0)]) */
        cRe[_si_idx] = _unk10r; cIm[_si_idx] = _unk10i;
    }
    for (int _si = 0; _si < 20; _si++) {
        int _si_idx = _si + 50;
        double _unk11r = 0, _unk11i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=20)) */
        double _cf12r = 0, _cf12i = 0;
        { int _idx = (int)(_unk11r); if (_idx >= 0 && _idx < 71) { _cf12r = cRe[_idx]; _cf12i = cIm[_idx]; } }
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_cf12r, _cf12i); _abs13i = 0;
        double _unk14r = 0, _unk14i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=20), upper=Constant(value=40)) */
        double _cf15r = 0, _cf15i = 0;
        { int _idx = (int)(_unk14r); if (_idx >= 0 && _idx < 71) { _cf15r = cRe[_idx]; _cf15i = cIm[_idx]; } }
        double _add16r = 0, _add16i = 0;
        _add16r = _cf15r + x1r; _add16i = _cf15i + x1i;
        double _add17r = 0, _add17i = 0;
        _add17r = _add16r + x2r; _add17i = _add16i + x2i;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_add17r, _add17i); _abs18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs13r + _abs18r; _add19i = _abs13i + _abs18i;
        cRe[_si_idx] = _add19r; cIm[_si_idx] = _add19i;
    }
    double _prod20r = 0, _prod20i = 0;
    _prod20r = 1; _prod20i = 0;
    for (int _pi = 0; _pi < 70; _pi++) { double _pr = _prod20r*cRe[_pi]-_prod20i*cIm[_pi]; double _pp = _prod20r*cIm[_pi]+_prod20i*cRe[_pi]; _prod20r=_pr; _prod20i=_pp; }
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _prod20r; cIm[_idx] = _prod20i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_56_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 1.0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_c1r, _c1i, x2r, x2i, &_mul2r, &_mul2i);
        double _add3r = 0, _add3i = 0;
        _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_add3r, _add3i, k, &_pow4r, &_pow4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 1.0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_c5r, _c5i, x1r, x1i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = x2r + _mul6r; _add7i = x2i + _mul6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 71.0; _c8i = 0;
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _c8r - k; _sub9i = _c8i - 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_add7r, _add7i, _sub9r, &_pow10r, &_pow10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _pow4r + _pow10r; _add11i = _pow4i + _pow10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_add11r, _add11i, _c12r, _c12i, &_div13r, &_div13i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div13r; cIm[_idx] = _div13i; } }
    }
    for (int _si = 0; _si < 65; _si++) {
        int _si_idx = _si + 3;
        double _unk14r = 0, _unk14i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=3), upper=Constant(value=68)) */
        double _cf15r = 0, _cf15i = 0;
        { int _idx = (int)(_unk14r); if (_idx >= 0 && _idx < 71) { _cf15r = cRe[_idx]; _cf15i = cIm[_idx]; } }
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 0.0; _c17i = 1.0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c17r, _c17i, x2r, x2i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = x1r + _mul18r; _add19i = x1i + _mul18i;
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(_add19r, _add19i); _ang20i = 0;
        double _sin21r = 0, _sin21i = 0;
        c_sin(_ang20r, _ang20i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _c16r + _sin21r; _add22i = _c16i + _sin21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_cf15r, _cf15i, _add22r, _add22i, &_mul23r, &_mul23i);
        cRe[_si_idx] = _mul23r; cIm[_si_idx] = _mul23i;
    }
    for (int _si = 0; _si < 3; _si++) {
        int _si_idx = _si + 0;
        double _unk24r = 0, _unk24i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=3)) */
        double _cf25r = 0, _cf25i = 0;
        { int _idx = (int)(_unk24r); if (_idx >= 0 && _idx < 71) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, x2r, x2i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = x1r + _mul28r; _add29i = x1i + _mul28i;
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(_add29r, _add29i); _ang30i = 0;
        double _cos31r = 0, _cos31i = 0;
        c_cos(_ang30r, _ang30i, &_cos31r, &_cos31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _c26r + _cos31r; _add32i = _c26i + _cos31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_cf25r, _cf25i, _add32r, _add32i, &_mul33r, &_mul33i);
        cRe[_si_idx] = _mul33r; cIm[_si_idx] = _mul33i;
    }
    for (int _si = 0; _si < 3; _si++) {
        int _si_idx = _si + 68;
        double _unk34r = 0, _unk34i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=68), upper=Constant(value=71)) */
        double _cf35r = 0, _cf35i = 0;
        { int _idx = (int)(_unk34r); if (_idx >= 0 && _idx < 71) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, x2r, x2i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = x1r + _mul37r; _add38i = x1i + _mul37i;
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(_add38r, _add38i); _abs39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_cf35r, _cf35i, _abs39r, _abs39i, &_mul40r, &_mul40i);
        cRe[_si_idx] = _mul40r; cIm[_si_idx] = _mul40i;
    }
    double _cf41r = 0, _cf41i = 0;
    { int _idx = 34; if (_idx >= 0 && _idx < 71) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
    double _c42r = 0, _c42i = 0;
    _c42r = 0.0; _c42i = 1.0;
    double _mul43r = 0, _mul43i = 0;
    c_mul(_c42r, _c42i, x2r, x2i, &_mul43r, &_mul43i);
    double _add44r = 0, _add44i = 0;
    _add44r = x1r + _mul43r; _add44i = x1i + _mul43i;
    double _im45r = 0, _im45i = 0;
    _im45r = _add44i; _im45i = 0;
    double _abs46r = 0, _abs46i = 0;
    _abs46r = c_abs(_im45r, _im45i); _abs46i = 0;
    double _c47r = 0, _c47i = 0;
    _c47r = 1.0; _c47i = 0;
    double _add48r = 0, _add48i = 0;
    _add48r = _abs46r + _c47r; _add48i = _abs46i + _c47i;
    double _log49r = 0, _log49i = 0;
    c_log(_add48r, _add48i, &_log49r, &_log49i);
    double _mul50r = 0, _mul50i = 0;
    c_mul(_cf41r, _cf41i, _log49r, _log49i, &_mul50r, &_mul50i);
    { int _idx = 34; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_57_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 36; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_mul(k, 0, k, 0, &_pow2r, &_pow2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(x1r, x1i, _pow2r, _pow2i, &_mul3r, &_mul3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 70.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - k; _sub5i = _c4i - 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(x2r, x2i, _sub5r, _sub5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _mul3r + _mul6r; _add7i = _mul3i + _mul6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _neg10r = 0, _neg10i = 0;
        _neg10r = -(_c9r); _neg10i = -(_c9i);
        double _pow11r = 0, _pow11i = 0;
        c_powr(_neg10r, _neg10i, k, &_pow11r, &_pow11i);
        double _sub12r = 0, _sub12i = 0;
        _sub12r = _c8r - _pow11r; _sub12i = _c8i - _pow11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_add7r, _add7i, _sub12r, _sub12i, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
    }
    for (int k = 36; k < 71; k++) {
        double _conj16r = 0, _conj16i = 0;
        _conj16r = x2r; _conj16i = -(x2i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(x1r, x1i, _conj16r, _conj16i, &_mul17r, &_mul17i);
        double _pow18r = 0, _pow18i = 0;
        c_powr(_mul17r, _mul17i, k, &_pow18r, &_pow18i);
        double _sub19r = 0, _sub19i = 0;
        _sub19r = x1r - x2r; _sub19i = x1i - x2i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_sub19r, _sub19i); _abs20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_pow18r, _pow18i, _abs20r, _abs20i, &_mul21r, &_mul21i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul21r; cIm[_idx] = _mul21i; } }
    }
    double _re22r = 0, _re22i = 0;
    _re22r = x2r; _re22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(x1r, x1i, _re22r, _re22i, &_mul23r, &_mul23i);
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(_mul23r, _mul23i); _abs24i = 0;
    double _sub25r = 0, _sub25i = 0;
    _sub25r = x2r - x1r; _sub25i = x2i - x1i;
    double _abs26r = 0, _abs26i = 0;
    _abs26r = c_abs(_sub25r, _sub25i); _abs26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_abs24r, _abs24i, _abs26r, _abs26i, &_mul27r, &_mul27i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_58: too complex for auto-transpile, stubbed */
static void poly_58_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_59_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = i + _c1r; _add2i = 0 + _c1i;
        double _div3r = 0, _div3i = 0;
        c_div(x1r, x1i, _add2r, _add2i, &_div3r, &_div3i);
        double _pow4r = 0, _pow4i = 0;
        c_powr(_div3r, _div3i, i, &_pow4r, &_pow4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = i + _c5r; _add6i = 0 + _c5i;
        double _div7r = 0, _div7i = 0;
        c_div(x2r, x2i, _add6r, _add6i, &_div7r, &_div7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 2.0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_div7r, _div7i, _c8r, _c8i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _pow4r + _mul9r; _add10i = _pow4i + _mul9i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add10r; cIm[_idx] = _add10i; } }
    }
    double _unk11r = 0, _unk11i = 0;
    /* WARNING: unhandled node List(elts=[Constant(value=1), Constant(value=3), Constant(value=5), Constant(value=7), Constant(value=9), Constant(value=11), Constant(value=13), Constant(value=15), Constant(value=17), Constant(value=19)], ctx=Load()) */
    double _c12r = 0, _c12i = 0;
    _c12r = 2.0; _c12i = 0;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c12r, _c12i, x2r, x2i, &_mul13r, &_mul13i);
    double _add14r = 0, _add14i = 0;
    _add14r = x1r + _mul13r; _add14i = x1i + _mul13i;
    { double _tr = cRe[(int)(_unk11r)]*_add14r - cIm[(int)(_unk11r)]*_add14i; cIm[(int)(_unk11r)] = cRe[(int)(_unk11r)]*_add14i + cIm[(int)(_unk11r)]*_add14r; cRe[(int)(_unk11r)] = _tr; }
    double _unk15r = 0, _unk15i = 0;
    /* WARNING: unhandled node List(elts=[Constant(value=2), Constant(value=5), Constant(value=8), Constant(value=11), Constant(value=14), Constant(value=17), Constant(value=20), Constant(value=23), Constant(value=26), Constant(value=29)], ctx=Load()) */
    double _c16r = 0, _c16i = 0;
    _c16r = 2.0; _c16i = 0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_c16r, _c16i, x2r, x2i, &_mul17r, &_mul17i);
    double _sub18r = 0, _sub18i = 0;
    _sub18r = x1r - _mul17r; _sub18i = x1i - _mul17i;
    { double _tr = cRe[(int)(_unk15r)]*_sub18r - cIm[(int)(_unk15r)]*_sub18i; cIm[(int)(_unk15r)] = cRe[(int)(_unk15r)]*_sub18i + cIm[(int)(_unk15r)]*_sub18r; cRe[(int)(_unk15r)] = _tr; }
    for (int _si = 0; _si < 32; _si++) {
        int _si_idx = _si + 4;
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c19r, _c19i, x1r, x1i, &_mul20r, &_mul20i);
        cRe[_si_idx] += _mul20r; cIm[_si_idx] += _mul20i;
    }
    for (int _si = 0; _si < 31; _si++) {
        int _si_idx = _si + 36;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c21r, _c21i, x2r, x2i, &_mul22r, &_mul22i);
    }
    for (int _si = 0; _si < 4; _si++) {
        int _si_idx = _si + 67;
        double _unk23r = 0, _unk23i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=67), upper=Constant(value=71)) */
        double _cf24r = 0, _cf24i = 0;
        { int _idx = (int)(_unk23r); if (_idx >= 0 && _idx < 71) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _log25r = 0, _log25i = 0;
        c_log(_cf24r, _cf24i, &_log25r, &_log25i);
        double _re26r = 0, _re26i = 0;
        _re26r = _log25r; _re26i = 0;
        double _sum27r = 0, _sum27i = 0;
        /* WARNING: unhandled np.sum */
        double _add28r = 0, _add28i = 0;
        _add28r = _re26r + _sum27r; _add28i = _re26i + _sum27i;
        cRe[_si_idx] = _add28r; cIm[_si_idx] = _add28i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_60_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _pow1r = 0, _pow1i = 0;
        c_powr(x1r, x1i, k, &_pow1r, &_pow1i);
        double _conj2r = 0, _conj2i = 0;
        _conj2r = x2r; _conj2i = -(x2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _sub4r = 0, _sub4i = 0;
        _sub4r = k - _c3r; _sub4i = 0 - _c3i;
        double _pow5r = 0, _pow5i = 0;
        c_powr(_conj2r, _conj2i, _sub4r, &_pow5r, &_pow5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _mod7r = 0, _mod7i = 0;
        _mod7r = fmod(k, _c6r); _mod7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _mod7r + _c8r; _add9i = _mod7i + _c8i;
        double _div10r = 0, _div10i = 0;
        c_div(_pow5r, _pow5i, _add9r, _add9i, &_div10r, &_div10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _pow1r + _div10r; _add11i = _pow1i + _div10i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add11r; cIm[_idx] = _add11i; } }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 0;
        double _unk12r = 0, _unk12i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=5)) */
        double _cf13r = 0, _cf13i = 0;
        { int _idx = (int)(_unk12r); if (_idx >= 0 && _idx < 71) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_cf13r, _cf13i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(_abs14r, _abs14i, _abs14r, _abs14i, &_pow16r, &_pow16i);
        cRe[_si_idx] = _pow16r; cIm[_si_idx] = _pow16i;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 5;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        { double _tr = cRe[_si_idx]*_log20r - cIm[_si_idx]*_log20i; cIm[_si_idx] = cRe[_si_idx]*_log20i + cIm[_si_idx]*_log20r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 10;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = x1r - x2r; _sub21i = x1i - x2i;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_sub21r, _sub21i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 3.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(_abs22r, _abs22i, _abs22r, _abs22i, &_pow24r, &_pow24i);
        c_mul(_pow24r, _pow24i, _abs22r, _abs22i, &_pow24r, &_pow24i);
        { double _tr = cRe[_si_idx]*_pow24r - cIm[_si_idx]*_pow24i; cIm[_si_idx] = cRe[_si_idx]*_pow24i + cIm[_si_idx]*_pow24r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 15;
        double _add25r = 0, _add25i = 0;
        _add25r = x1r + x2r; _add25i = x1i + x2i;
        double _sin26r = 0, _sin26i = 0;
        c_sin(_add25r, _add25i, &_sin26r, &_sin26i);
        { double _tr = cRe[_si_idx]*_sin26r - cIm[_si_idx]*_sin26i; cIm[_si_idx] = cRe[_si_idx]*_sin26i + cIm[_si_idx]*_sin26r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 20;
        double _sub27r = 0, _sub27i = 0;
        _sub27r = x1r - x2r; _sub27i = x1i - x2i;
        double _cos28r = 0, _cos28i = 0;
        c_cos(_sub27r, _sub27i, &_cos28r, &_cos28i);
        { double _tr = cRe[_si_idx]*_cos28r - cIm[_si_idx]*_cos28i; cIm[_si_idx] = cRe[_si_idx]*_cos28i + cIm[_si_idx]*_cos28r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 25;
        double _add29r = 0, _add29i = 0;
        _add29r = x1r + x2r; _add29i = x1i + x2i;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(_add29r, _add29i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(_abs30r, _abs30i, _abs30r, _abs30i, &_pow32r, &_pow32i);
        { double _tr = cRe[_si_idx]*_pow32r - cIm[_si_idx]*_pow32i; cIm[_si_idx] = cRe[_si_idx]*_pow32i + cIm[_si_idx]*_pow32r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 30;
        double _sub33r = 0, _sub33i = 0;
        _sub33r = x1r - x2r; _sub33i = x1i - x2i;
        double _sin34r = 0, _sin34i = 0;
        c_sin(_sub33r, _sub33i, &_sin34r, &_sin34i);
        { double _tr = cRe[_si_idx]*_sin34r - cIm[_si_idx]*_sin34i; cIm[_si_idx] = cRe[_si_idx]*_sin34i + cIm[_si_idx]*_sin34r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 35;
        double _add35r = 0, _add35i = 0;
        _add35r = x1r + x2r; _add35i = x1i + x2i;
        double _cos36r = 0, _cos36i = 0;
        c_cos(_add35r, _add35i, &_cos36r, &_cos36i);
        { double _tr = cRe[_si_idx]*_cos36r - cIm[_si_idx]*_cos36i; cIm[_si_idx] = cRe[_si_idx]*_cos36i + cIm[_si_idx]*_cos36r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 40;
        double _sub37r = 0, _sub37i = 0;
        _sub37r = x1r - x2r; _sub37i = x1i - x2i;
        double _abs38r = 0, _abs38i = 0;
        _abs38r = c_abs(_sub37r, _sub37i); _abs38i = 0;
        { double _tr = cRe[_si_idx]*_abs38r - cIm[_si_idx]*_abs38i; cIm[_si_idx] = cRe[_si_idx]*_abs38i + cIm[_si_idx]*_abs38r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 45;
        double _add39r = 0, _add39i = 0;
        _add39r = x1r + x2r; _add39i = x1i + x2i;
        double _sin40r = 0, _sin40i = 0;
        c_sin(_add39r, _add39i, &_sin40r, &_sin40i);
        { double _tr = cRe[_si_idx]*_sin40r - cIm[_si_idx]*_sin40i; cIm[_si_idx] = cRe[_si_idx]*_sin40i + cIm[_si_idx]*_sin40r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 50;
        double _sub41r = 0, _sub41i = 0;
        _sub41r = x1r - x2r; _sub41i = x1i - x2i;
        double _cos42r = 0, _cos42i = 0;
        c_cos(_sub41r, _sub41i, &_cos42r, &_cos42i);
        { double _tr = cRe[_si_idx]*_cos42r - cIm[_si_idx]*_cos42i; cIm[_si_idx] = cRe[_si_idx]*_cos42i + cIm[_si_idx]*_cos42r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 55;
        double _add43r = 0, _add43i = 0;
        _add43r = x1r + x2r; _add43i = x1i + x2i;
        double _abs44r = 0, _abs44i = 0;
        _abs44r = c_abs(_add43r, _add43i); _abs44i = 0;
        { double _tr = cRe[_si_idx]*_abs44r - cIm[_si_idx]*_abs44i; cIm[_si_idx] = cRe[_si_idx]*_abs44i + cIm[_si_idx]*_abs44r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 60;
        double _sub45r = 0, _sub45i = 0;
        _sub45r = x1r - x2r; _sub45i = x1i - x2i;
        double _sin46r = 0, _sin46i = 0;
        c_sin(_sub45r, _sub45i, &_sin46r, &_sin46i);
        { double _tr = cRe[_si_idx]*_sin46r - cIm[_si_idx]*_sin46i; cIm[_si_idx] = cRe[_si_idx]*_sin46i + cIm[_si_idx]*_sin46r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 65;
        double _add47r = 0, _add47i = 0;
        _add47r = x1r + x2r; _add47i = x1i + x2i;
        double _cos48r = 0, _cos48i = 0;
        c_cos(_add47r, _add47i, &_cos48r, &_cos48i);
        { double _tr = cRe[_si_idx]*_cos48r - cIm[_si_idx]*_cos48i; cIm[_si_idx] = cRe[_si_idx]*_cos48i + cIm[_si_idx]*_cos48r; cRe[_si_idx] = _tr; }
    }
    double _sub49r = 0, _sub49i = 0;
    _sub49r = x1r - x2r; _sub49i = x1i - x2i;
    double _abs50r = 0, _abs50i = 0;
    _abs50r = c_abs(_sub49r, _sub49i); _abs50i = 0;
    { double _tr = cRe[70]*_abs50r - cIm[70]*_abs50i; cIm[70] = cRe[70]*_abs50i + cIm[70]*_abs50r; cRe[70] = _tr; }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_61_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 0;
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _arange2r = 0, _arange2i = 0;
        _arange2r = (double)(_si + 1); _arange2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 3.0; _c3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 3.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(0, 0, 0, 0, &_pow5r, &_pow5i);
        c_mul(_pow5r, _pow5i, 0, 0, &_pow5r, &_pow5i);
        double _smul6r = 0, _smul6i = 0;
        c_mul(_re1r, _re1i, _pow5r, _pow5i, &_smul6r, &_smul6i);
        double _im7r = 0, _im7i = 0;
        _im7r = x2i; _im7i = 0;
        double _sin8r = 0, _sin8i = 0;
        c_sin(0, 0, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_im7r, _im7i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _sadd10r = 0, _sadd10i = 0;
        _sadd10r = _smul6r + _mul9r; _sadd10i = _smul6i + _mul9i;
        cRe[_si_idx] = _sadd10r; cIm[_si_idx] = _sadd10i;
    }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 35;
        double _im11r = 0, _im11i = 0;
        _im11r = x1i; _im11i = 0;
        double _arange12r = 0, _arange12i = 0;
        _arange12r = (double)(_si + 70); _arange12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(0, 0, 0, 0, &_pow15r, &_pow15i);
        double _smul16r = 0, _smul16i = 0;
        c_mul(_im11r, _im11i, _pow15r, _pow15i, &_smul16r, &_smul16i);
        double _re17r = 0, _re17i = 0;
        _re17r = x2r; _re17i = 0;
        double _cos18r = 0, _cos18i = 0;
        c_cos(0, 0, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_re17r, _re17i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _sadd20r = 0, _sadd20i = 0;
        _sadd20r = _smul16r + _mul19r; _sadd20i = _smul16i + _mul19i;
        cRe[_si_idx] = _sadd20r; cIm[_si_idx] = _sadd20i;
    }
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(x1r, x1i); _abs21i = 0;
    double _ang22r = 0, _ang22i = 0;
    _ang22r = c_arg(x2r, x2i); _ang22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_abs21r, _abs21i, _ang22r, _ang22i, &_mul23r, &_mul23i);
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(x2r, x2i); _abs24i = 0;
    double _ang25r = 0, _ang25i = 0;
    _ang25r = c_arg(x1r, x1i); _ang25i = 0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_abs24r, _abs24i, _ang25r, _ang25i, &_mul26r, &_mul26i);
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _mul23r - _mul26r; _sub27i = _mul23i - _mul26i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub27r; cIm[_idx] = _sub27i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_62_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 5.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_powr(x1r, x1i, 5.0, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 5.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_powr(x2r, x2i, 5.0, &_pow4r, &_pow4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _pow2r + _pow4r; _add5i = _pow2i + _pow4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    for (int i = 2; i < 72; i++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _mod7r = 0, _mod7i = 0;
        _mod7r = fmod(i, _c6r); _mod7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        if (_mod7r == _c8r) {
            double _cf9r = 0, _cf9i = 0;
            { int _idx = (i - 2); if (_idx >= 0 && _idx < 71) { _cf9r = cRe[_idx]; _cf9i = cIm[_idx]; } }
            double _c10r = 0, _c10i = 0;
            _c10r = 2.0; _c10i = 0;
            double _pow11r = 0, _pow11i = 0;
            c_mul(_cf9r, _cf9i, _cf9r, _cf9i, &_pow11r, &_pow11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(i, 0, _pow11r, _pow11i, &_mul12r, &_mul12i);
            { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
        } else {
            double _cf13r = 0, _cf13i = 0;
            { int _idx = (i - 2); if (_idx >= 0 && _idx < 71) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
            double _c14r = 0, _c14i = 0;
            _c14r = 2.0; _c14i = 0;
            double _pow15r = 0, _pow15i = 0;
            c_mul(_cf13r, _cf13i, _cf13r, _cf13i, &_pow15r, &_pow15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(i, 0, _pow15r, _pow15i, &_mul16r, &_mul16i);
            double _c17r = 0, _c17i = 0;
            _c17r = 1.0; _c17i = 0;
            double _c18r = 0, _c18i = 0;
            _c18r = 0.1; _c18i = 0;
            double _mul19r = 0, _mul19i = 0;
            c_mul(_c18r, _c18i, x2r, x2i, &_mul19r, &_mul19i);
            double _add20r = 0, _add20i = 0;
            _add20r = _c17r + _mul19r; _add20i = _c17i + _mul19i;
            double _mul21r = 0, _mul21i = 0;
            c_mul(_mul16r, _mul16i, _add20r, _add20i, &_mul21r, &_mul21i);
            { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul21r; cIm[_idx] = _mul21i; } }
        }
    }
    double _c22r = 0, _c22i = 0;
    _c22r = 2.0; _c22i = 0;
    double _cf23r = 0, _cf23i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c22r, _c22i, _cf23r, _cf23i, &_mul24r, &_mul24i);
    cRe[0] += _mul24r; cIm[0] += _mul24i;
    double _c25r = 0, _c25i = 0;
    _c25r = 3.0; _c25i = 0;
    double _cf26r = 0, _cf26i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c25r, _c25i, _cf26r, _cf26i, &_mul27r, &_mul27i);
    cRe[1] -= _mul27r; cIm[1] -= _mul27i;
    for (int i = 3; i < 70; i++) {
        double _cf28r = 0, _cf28i = 0;
        { int _idx = (i + 1); if (_idx >= 0 && _idx < 71) { _cf28r = cRe[_idx]; _cf28i = cIm[_idx]; } }
        double _cf29r = 0, _cf29i = 0;
        { int _idx = (i + 2); if (_idx >= 0 && _idx < 71) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _cf28r - _cf29r; _sub30i = _cf28i - _cf29i;
        cRe[i] += _sub30r; cIm[i] += _sub30i;
    }
    double _cf31r = 0, _cf31i = 0;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
    cRe[69] += _cf31r; cIm[69] += _cf31i;
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(x1r, x1i); _abs32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_abs32r, _abs32i, _abs32r, _abs32i, &_pow34r, &_pow34i);
    double _abs35r = 0, _abs35i = 0;
    _abs35r = c_abs(x2r, x2i); _abs35i = 0;
    double _c36r = 0, _c36i = 0;
    _c36r = 2.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(_abs35r, _abs35i, _abs35r, _abs35i, &_pow37r, &_pow37i);
    double _sub38r = 0, _sub38i = 0;
    _sub38r = _pow34r - _pow37r; _sub38i = _pow34i - _pow37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _im40r = 0, _im40i = 0;
    _im40r = x1i; _im40i = 0;
    double _mul41r = 0, _mul41i = 0;
    c_mul(_c39r, _c39i, _im40r, _im40i, &_mul41r, &_mul41i);
    double _im42r = 0, _im42i = 0;
    _im42r = x2i; _im42i = 0;
    double _mul43r = 0, _mul43i = 0;
    c_mul(_mul41r, _mul41i, _im42r, _im42i, &_mul43r, &_mul43i);
    double _add44r = 0, _add44i = 0;
    _add44r = _sub38r + _mul43r; _add44i = _sub38i + _mul43i;
    double _ang45r = 0, _ang45i = 0;
    _ang45r = c_arg(x2r, x2i); _ang45i = 0;
    double _sub46r = 0, _sub46i = 0;
    _sub46r = _add44r - _ang45r; _sub46i = _add44i - _ang45i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub46r; cIm[_idx] = _sub46i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_63_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 72; j++) {
        double _pow1r = 0, _pow1i = 0;
        c_powr(x1r, x1i, j, &_pow1r, &_pow1i);
        double _sin2r = 0, _sin2i = 0;
        c_sin(_pow1r, _pow1i, &_sin2r, &_sin2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 71.0; _c3i = 0;
        double _sub4r = 0, _sub4i = 0;
        _sub4r = _c3r - j; _sub4i = _c3i - 0;
        double _pow5r = 0, _pow5i = 0;
        c_powr(x2r, x2i, _sub4r, &_pow5r, &_pow5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_pow5r, _pow5i, &_cos6r, &_cos6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_sin2r, _sin2i, _cos6r, _cos6i, &_mul7r, &_mul7i);
        double _pow8r = 0, _pow8i = 0;
        c_powr(x2r, x2i, j, &_pow8r, &_pow8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(x1r, x1i, _pow8r, _pow8i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_mul9r, _mul9i); _abs10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_mul7r, _mul7i, _abs10r, _abs10i, &_mul11r, &_mul11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _mul12r + _c13r; _add14i = _mul12i + _c13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(_add14r, _add14i); _abs15i = 0;
        double _log16r = 0, _log16i = 0;
        c_log(_abs15r, _abs15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul11r, _mul11i, _log16r, _log16i, &_mul17r, &_mul17i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul17r; cIm[_idx] = _mul17i; } }
    }
    for (int _si = 0; _si < 30; _si++) {
        int _si_idx = _si + 0;
        double _unk18r = 0, _unk18i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=30), upper=Constant(value=60)) */
        double _cf19r = 0, _cf19i = 0;
        { int _idx = (int)(_unk18r); if (_idx >= 0 && _idx < 71) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
        cRe[_si_idx] += _cf19r; cIm[_si_idx] += _cf19i;
    }
    for (int _si = 0; _si < 40; _si++) {
        int _si_idx = _si + 32;
        double _unk20r = 0, _unk20i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=40)) */
        double _cf21r = 0, _cf21i = 0;
        { int _idx = (int)(_unk20r); if (_idx >= 0 && _idx < 71) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
    }
    for (int _si = 0; _si < 50; _si++) {
        int _si_idx = _si + 10;
        double _re22r = 0, _re22i = 0;
        _re22r = x1r; _re22i = 0;
        double _im23r = 0, _im23i = 0;
        _im23r = x2i; _im23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_re22r, _re22i, _im23r, _im23i, &_mul24r, &_mul24i);
        double _unk25r = 0, _unk25i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=50)) */
        double _cf26r = 0, _cf26i = 0;
        { int _idx = (int)(_unk25r); if (_idx >= 0 && _idx < 71) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul24r, _mul24i, _cf26r, _cf26i, &_mul27r, &_mul27i);
        cRe[_si_idx] += _mul27r; cIm[_si_idx] += _mul27i;
    }
    for (int _si = 0; _si < 40; _si++) {
        int _si_idx = _si + 30;
        double _im28r = 0, _im28i = 0;
        _im28r = x1i; _im28i = 0;
        double _re29r = 0, _re29i = 0;
        _re29r = x2r; _re29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_im28r, _im28i, _re29r, _re29i, &_mul30r, &_mul30i);
        double _unk31r = 0, _unk31i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=1), upper=Constant(value=41)) */
        double _cf32r = 0, _cf32i = 0;
        { int _idx = (int)(_unk31r); if (_idx >= 0 && _idx < 71) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
        double _mul33r = 0, _mul33i = 0;
        c_mul(_mul30r, _mul30i, _cf32r, _cf32i, &_mul33r, &_mul33i);
    }
    for (int _si = 0; _si < 20; _si++) {
        int _si_idx = _si + 20;
        double _pow34r = 0, _pow34i = 0;
        c_powr(x1r, x1i, x2r, &_pow34r, &_pow34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(_pow34r, _pow34i); _ang35i = 0;
        double _unk36r = 0, _unk36i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=30), upper=Constant(value=50)) */
        double _cf37r = 0, _cf37i = 0;
        { int _idx = (int)(_unk36r); if (_idx >= 0 && _idx < 71) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang35r, _ang35i, _cf37r, _cf37i, &_mul38r, &_mul38i);
        cRe[_si_idx] += _mul38r; cIm[_si_idx] += _mul38i;
    }
    for (int _si = 0; _si < 32; _si++) {
        int _si_idx = _si + 40;
        double _pow39r = 0, _pow39i = 0;
        c_powr(x2r, x2i, x1r, &_pow39r, &_pow39i);
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(_pow39r, _pow39i); _ang40i = 0;
        double _unk41r = 0, _unk41i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=32)) */
        double _cf42r = 0, _cf42i = 0;
        { int _idx = (int)(_unk41r); if (_idx >= 0 && _idx < 71) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang40r, _ang40i, _cf42r, _cf42i, &_mul43r, &_mul43i);
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_64_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_abs1r, _abs1i, k, &_pow2r, &_pow2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x2r, x2i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 71.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - k; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_abs3r, _abs3i, _sub5r, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x1r, x1i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_ang8r, _ang8i, k, 0, &_mul9r, &_mul9i);
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x2r, x2i); _ang10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 71.0; _c11i = 0;
        double _sub12r = 0, _sub12i = 0;
        _sub12r = _c11r - k; _sub12i = _c11i - 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_ang10r, _ang10i, _sub12r, _sub12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul9r + _mul13r; _add14i = _mul9i + _mul13i;
        double _cos15r = 0, _cos15i = 0;
        c_cos(_add14r, _add14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_add7r, _add7i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul16r; cIm[_idx] = _mul16i; } }
    }
    for (int _si = 0; _si < 70; _si++) {
        int _si_idx = _si + 1;
        double _c17r = 0, _c17i = 0;
        _c17r = 0.0; _c17i = 1.0;
        { double _tr = cRe[_si_idx]*_c17r - cIm[_si_idx]*_c17i; cIm[_si_idx] = cRe[_si_idx]*_c17i + cIm[_si_idx]*_c17r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 69; _si++) {
        int _si_idx = _si + 2;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _neg19r = 0, _neg19i = 0;
        _neg19r = -(_c18r); _neg19i = -(_c18i);
        { double _tr = cRe[_si_idx]*_neg19r - cIm[_si_idx]*_neg19i; cIm[_si_idx] = cRe[_si_idx]*_neg19i + cIm[_si_idx]*_neg19r; cRe[_si_idx] = _tr; }
    }
    double _c20r = 0, _c20i = 0;
    _c20r = 100.0; _c20i = 0;
    { double _tr = cRe[0]*_c20r - cIm[0]*_c20i; cIm[0] = cRe[0]*_c20i + cIm[0]*_c20r; cRe[0] = _tr; }
    double _c21r = 0, _c21i = 0;
    _c21r = 100.0; _c21i = 0;
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_65_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _add1r = 0, _add1i = 0;
        _add1r = x1r + x2r; _add1i = x1i + x2i;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(_c2r, _c2i, k, 0, &_mul3r, &_mul3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _mul3r - _c4r; _sub5i = _mul3i - _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_add1r, _add1i, _sub5r, &_pow6r, &_pow6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, x1r, x1i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(k, 0, x2r, x2i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_sin8r, _sin8i, _cos10r, _cos10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _pow6r + _mul11r; _add12i = _pow6i + _mul11i;
        double _pow13r = 0, _pow13i = 0;
        c_powr(k, 0, x2r, &_pow13r, &_pow13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_pow13r, _pow13i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _pow18r = 0, _pow18i = 0;
        c_powr(x1r, x1i, x2r, &_pow18r, &_pow18i);
        double _re19r = 0, _re19i = 0;
        _re19r = _pow18r; _re19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log17r, _log17i, _re19r, _re19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add12r + _mul20r; _add21i = _add12i + _mul20i;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c22r, _c22i, k, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _mul23r + _c24r; _add25i = _mul23i + _c24i;
        double _pow26r = 0, _pow26i = 0;
        c_powr(x1r, x1i, _add25r, &_pow26r, &_pow26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, k, 0, &_mul28r, &_mul28i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(x2r, x2i, _mul28r, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _pow26r + _pow29r; _add30i = _pow26i + _pow29i;
        double _im31r = 0, _im31i = 0;
        _im31r = _add30i; _im31i = 0;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(_im31r, _im31i); _abs32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _add21r + _abs32r; _add33i = _add21i + _abs32i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
        double _cf34r = 0, _cf34i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
        double _conj35r = 0, _conj35i = 0;
        _conj35r = _cf34r; _conj35i = -(_cf34i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _neg37r = 0, _neg37i = 0;
        _neg37r = -(_c36r); _neg37i = -(_c36i);
        double _pow38r = 0, _pow38i = 0;
        c_powr(_neg37r, _neg37i, k, &_pow38r, &_pow38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_conj35r, _conj35i, _pow38r, _pow38i, &_mul39r, &_mul39i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
        double _c40r = 0, _c40i = 0;
        _c40r = 2.0; _c40i = 0;
        double _mod41r = 0, _mod41i = 0;
        _mod41r = fmod(k, _c40r); _mod41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 0;
        if (_mod41r == _c42r) {
            double _add43r = 0, _add43i = 0;
            _add43r = k + x1r; _add43i = 0 + x1i;
        }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_66_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 25; _si++) {
        int _si_idx = _si + 0;
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x2r, x2i); _abs2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs2r + _c3r; _add4i = _abs2i + _c3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_re1r, _re1i, _log5r, _log5i, &_mul6r, &_mul6i);
        double _arange7r = 0, _arange7i = 0;
        _arange7r = (double)(_si + 1); _arange7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_mul(0, 0, 0, 0, &_pow10r, &_pow10i);
        double _smul11r = 0, _smul11i = 0;
        c_mul(_mul6r, _mul6i, _pow10r, _pow10i, &_smul11r, &_smul11i);
        cRe[_si_idx] = _smul11r; cIm[_si_idx] = _smul11i;
    }
    for (int _si = 0; _si < 25; _si++) {
        int _si_idx = _si + 25;
        double _im12r = 0, _im12i = 0;
        _im12r = x2i; _im12i = 0;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_im12r, _im12i, _log16r, _log16i, &_mul17r, &_mul17i);
        double _arange18r = 0, _arange18i = 0;
        _arange18r = (double)(_si + 1); _arange18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_mul(0, 0, 0, 0, &_pow21r, &_pow21i);
        c_mul(_pow21r, _pow21i, 0, 0, &_pow21r, &_pow21i);
        double _smul22r = 0, _smul22i = 0;
        c_mul(_mul17r, _mul17i, _pow21r, _pow21i, &_smul22r, &_smul22i);
        cRe[_si_idx] = _smul22r; cIm[_si_idx] = _smul22i;
    }
    for (int _si = 0; _si < 20; _si++) {
        int _si_idx = _si + 50;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x1r, x1i); _abs23i = 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_abs23r, _abs23i, _abs24r, _abs24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = x1r + x2r; _add26i = x1i + x2i;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_add26r, _add26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _abs27r + _c28r; _add29i = _abs27i + _c28i;
        double _log30r = 0, _log30i = 0;
        c_log(_add29r, _add29i, &_log30r, &_log30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_mul25r, _mul25i, _log30r, _log30i, &_mul31r, &_mul31i);
        double _arange32r = 0, _arange32i = 0;
        _arange32r = (double)(_si + 1); _arange32i = 0;
        double _smul33r = 0, _smul33i = 0;
        c_mul(_mul31r, _mul31i, _arange32r, _arange32i, &_smul33r, &_smul33i);
        cRe[_si_idx] = _smul33r; cIm[_si_idx] = _smul33i;
    }
    double _sum34r = 0, _sum34i = 0;
    _sum34r = 0; _sum34i = 0;
    for (int _si = 0; _si < 70; _si++) { _sum34r += cRe[_si]; _sum34i += cIm[_si]; }
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _ang36r = 0, _ang36i = 0;
    _ang36r = c_arg(_add35r, _add35i); _ang36i = 0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_sum34r, _sum34i, _ang36r, _ang36i, &_mul37r, &_mul37i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_67_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 7.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_powr(x1r, x1i, 7.0, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 1.0;
    double _c4r = 0, _c4i = 0;
    _c4r = 8.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_powr(x2r, x2i, 8.0, &_pow5r, &_pow5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c3r, _c3i, _pow5r, _pow5i, &_mul6r, &_mul6i);
    double _add7r = 0, _add7i = 0;
    _add7r = _pow2r + _mul6r; _add7i = _pow2i + _mul6i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 2.0; _c8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 6.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_powr(x1r, x1i, 6.0, &_pow10r, &_pow10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c8r, _c8i, _pow10r, _pow10i, &_mul11r, &_mul11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 0.0; _c12i = 1.0;
    double _c13r = 0, _c13i = 0;
    _c13r = 7.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_powr(x2r, x2i, 7.0, &_pow14r, &_pow14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c12r, _c12i, _pow14r, _pow14i, &_mul15r, &_mul15i);
    double _sub16r = 0, _sub16i = 0;
    _sub16r = _mul11r - _mul15r; _sub16i = _mul11i - _mul15i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub16r; cIm[_idx] = _sub16i; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 5.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_powr(x1r, x1i, 5.0, &_pow19r, &_pow19i);
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c17r, _c17i, _pow19r, _pow19i, &_mul20r, &_mul20i);
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 3.0;
    double _c22r = 0, _c22i = 0;
    _c22r = 6.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_powr(x2r, x2i, 6.0, &_pow23r, &_pow23i);
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c21r, _c21i, _pow23r, _pow23i, &_mul24r, &_mul24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _mul20r + _mul24r; _add25i = _mul20i + _mul24i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add25r; cIm[_idx] = _add25i; } }
    double _c26r = 0, _c26i = 0;
    _c26r = 5.0; _c26i = 0;
    double _c27r = 0, _c27i = 0;
    _c27r = 4.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow28r, &_pow28i);
    c_mul(_pow28r, _pow28i, _pow28r, _pow28i, &_pow28r, &_pow28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_c26r, _c26i, _pow28r, _pow28i, &_mul29r, &_mul29i);
    double _c30r = 0, _c30i = 0;
    _c30r = 0.0; _c30i = 5.0;
    double _c31r = 0, _c31i = 0;
    _c31r = 5.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_powr(x2r, x2i, 5.0, &_pow32r, &_pow32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c30r, _c30i, _pow32r, _pow32i, &_mul33r, &_mul33i);
    double _sub34r = 0, _sub34i = 0;
    _sub34r = _mul29r - _mul33r; _sub34i = _mul29i - _mul33i;
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub34r; cIm[_idx] = _sub34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 7.0; _c35i = 0;
    double _c36r = 0, _c36i = 0;
    _c36r = 3.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow37r, &_pow37i);
    c_mul(_pow37r, _pow37i, x1r, x1i, &_pow37r, &_pow37i);
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c35r, _c35i, _pow37r, _pow37i, &_mul38r, &_mul38i);
    double _c39r = 0, _c39i = 0;
    _c39r = 0.0; _c39i = 7.0;
    double _c40r = 0, _c40i = 0;
    _c40r = 4.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow41r, &_pow41i);
    c_mul(_pow41r, _pow41i, _pow41r, _pow41i, &_pow41r, &_pow41i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c39r, _c39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _mul38r + _mul42r; _add43i = _mul38i + _mul42i;
    { int _idx = 4; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 11.0; _c44i = 0;
    double _c45r = 0, _c45i = 0;
    _c45r = 2.0; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow46r, &_pow46i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c44r, _c44i, _pow46r, _pow46i, &_mul47r, &_mul47i);
    double _c48r = 0, _c48i = 0;
    _c48r = 0.0; _c48i = 11.0;
    double _c49r = 0, _c49i = 0;
    _c49r = 3.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow50r, &_pow50i);
    c_mul(_pow50r, _pow50i, x2r, x2i, &_pow50r, &_pow50i);
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c48r, _c48i, _pow50r, _pow50i, &_mul51r, &_mul51i);
    double _sub52r = 0, _sub52i = 0;
    _sub52r = _mul47r - _mul51r; _sub52i = _mul47i - _mul51i;
    { int _idx = 5; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub52r; cIm[_idx] = _sub52i; } }
    double _c53r = 0, _c53i = 0;
    _c53r = 13.0; _c53i = 0;
    double _mul54r = 0, _mul54i = 0;
    c_mul(_c53r, _c53i, x1r, x1i, &_mul54r, &_mul54i);
    double _c55r = 0, _c55i = 0;
    _c55r = 0.0; _c55i = 13.0;
    double _mul56r = 0, _mul56i = 0;
    c_mul(_c55r, _c55i, x2r, x2i, &_mul56r, &_mul56i);
    double _add57r = 0, _add57i = 0;
    _add57r = _mul54r + _mul56r; _add57i = _mul54i + _mul56i;
    { int _idx = 6; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add57r; cIm[_idx] = _add57i; } }
    double _mul58r = 0, _mul58i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul58r, &_mul58i);
    double _abs59r = 0, _abs59i = 0;
    _abs59r = c_abs(_mul58r, _mul58i); _abs59i = 0;
    double _c60r = 0, _c60i = 0;
    _c60r = 2.0; _c60i = 0;
    double _pow61r = 0, _pow61i = 0;
    c_mul(_abs59r, _abs59i, _abs59r, _abs59i, &_pow61r, &_pow61i);
    double _mul62r = 0, _mul62i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul62r, &_mul62i);
    double _ang63r = 0, _ang63i = 0;
    _ang63r = c_arg(_mul62r, _mul62i); _ang63i = 0;
    double _mul64r = 0, _mul64i = 0;
    c_mul(_pow61r, _pow61i, _ang63r, _ang63i, &_mul64r, &_mul64i);
    double _re65r = 0, _re65i = 0;
    _re65r = x1r; _re65i = 0;
    double _sin66r = 0, _sin66i = 0;
    c_sin(_re65r, _re65i, &_sin66r, &_sin66i);
    double _add67r = 0, _add67i = 0;
    _add67r = _mul64r + _sin66r; _add67i = _mul64i + _sin66i;
    double _im68r = 0, _im68i = 0;
    _im68r = x2i; _im68i = 0;
    double _cos69r = 0, _cos69i = 0;
    c_cos(_im68r, _im68i, &_cos69r, &_cos69i);
    double _sub70r = 0, _sub70i = 0;
    _sub70r = _add67r - _cos69r; _sub70i = _add67i - _cos69i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub70r; cIm[_idx] = _sub70i; } }
    for (int k = 8; k < 71; k++) {
        double _c71r = 0, _c71i = 0;
        _c71r = 0.0; _c71i = 1.0;
        double _mul72r = 0, _mul72i = 0;
        c_mul(_c71r, _c71i, x2r, x2i, &_mul72r, &_mul72i);
        double _add73r = 0, _add73i = 0;
        _add73r = x1r + _mul72r; _add73i = x1i + _mul72i;
        double _c74r = 0, _c74i = 0;
        _c74r = 70.0; _c74i = 0;
        double _sub75r = 0, _sub75i = 0;
        _sub75r = _c74r - k; _sub75i = _c74i - 0;
        double _pow76r = 0, _pow76i = 0;
        c_powr(_add73r, _add73i, _sub75r, &_pow76r, &_pow76i);
        double _re77r = 0, _re77i = 0;
        _re77r = _pow76r; _re77i = 0;
        double _c78r = 0, _c78i = 0;
        _c78r = 0.0; _c78i = 1.0;
        double _mul79r = 0, _mul79i = 0;
        c_mul(_c78r, _c78i, x2r, x2i, &_mul79r, &_mul79i);
        double _sub80r = 0, _sub80i = 0;
        _sub80r = x1r - _mul79r; _sub80i = x1i - _mul79i;
        double _c81r = 0, _c81i = 0;
        _c81r = 1.0; _c81i = 0;
        double _sub82r = 0, _sub82i = 0;
        _sub82r = k - _c81r; _sub82i = 0 - _c81i;
        double _pow83r = 0, _pow83i = 0;
        c_powr(_sub80r, _sub80i, _sub82r, &_pow83r, &_pow83i);
        double _im84r = 0, _im84i = 0;
        _im84r = _pow83i; _im84i = 0;
        double _sub85r = 0, _sub85i = 0;
        _sub85r = _re77r - _im84r; _sub85i = _re77i - _im84i;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub85r; cIm[_idx] = _sub85i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_68_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 0;
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _arange2r = 0, _arange2i = 0;
        _arange2r = (double)(_si + 1); _arange2i = 0;
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs3r, _abs3i, 0, &_pow4r, &_pow4i);
        cRe[_si_idx] = _pow4r; cIm[_si_idx] = _pow4i;
    }
    for (int i = 6; i < 71; i++) {
        double _mul5r = 0, _mul5i = 0;
        c_mul(i, 0, x1r, x1i, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c6r, _c6i, i, 0, &_mul7r, &_mul7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_mul7r, _mul7i, x2r, x2i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _mul8r; _add9i = _mul5i + _mul8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = i + _c10r; _add11i = 0 + _c10i;
        double _div12r = 0, _div12i = 0;
        c_div(_add9r, _add9i, _add11r, _add11i, &_div12r, &_div12i);
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div12r; cIm[_idx] = _div12i; } }
    }
    double _abs13r = 0, _abs13i = 0;
    _abs13r = c_abs(x1r, x1i); _abs13i = 0;
    double _abs14r = 0, _abs14i = 0;
    _abs14r = c_abs(x2r, x2i); _abs14i = 0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_abs13r, _abs13i, _abs14r, _abs14i, &_mul15r, &_mul15i);
    double _ang16r = 0, _ang16i = 0;
    _ang16r = c_arg(x1r, x1i); _ang16i = 0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_mul15r, _mul15i, _ang16r, _ang16i, &_mul17r, &_mul17i);
    double _ang18r = 0, _ang18i = 0;
    _ang18r = c_arg(x2r, x2i); _ang18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_mul17r, _mul17i, _ang18r, _ang18i, &_mul19r, &_mul19i);
    double _add20r = 0, _add20i = 0;
    _add20r = x1r + x2r; _add20i = x1i + x2i;
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(_add20r, _add20i); _abs21i = 0;
    double _sin22r = 0, _sin22i = 0;
    c_sin(_abs21r, _abs21i, &_sin22r, &_sin22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_mul19r, _mul19i, _sin22r, _sin22i, &_mul23r, &_mul23i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 20;
        double _add24r = 0, _add24i = 0;
        _add24r = x1r + x2r; _add24i = x1i + x2i;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_add24r, _add24i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _unk30r = 0, _unk30i = 0;
        /* WARNING: unhandled node Attribute(value=Name(id='np', ctx=Load()), attr='pi', ctx=Load()) */
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c29r, _c29i, _unk30r, _unk30i, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 10.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_mul31r, _mul31i, _c32r, _c32i, &_div33r, &_div33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_div33r, _div33i, 0, 0, &_mul34r, &_mul34i);
        double _exp35r = 0, _exp35i = 0;
        c_exp2(_mul34r, _mul34i, &_exp35r, &_exp35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_log28r, _log28i, _exp35r, _exp35i, &_mul36r, &_mul36i);
        cRe[_si_idx] += _mul36r; cIm[_si_idx] += _mul36i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 50;
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _unk38r = 0, _unk38i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=10)) */
        double _cf39r = 0, _cf39i = 0;
        { int _idx = (int)(_unk38r); if (_idx >= 0 && _idx < 71) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
        double _arange40r = 0, _arange40i = 0;
        _arange40r = (double)(_si + 11); _arange40i = 0;
        double _sdiv41r = 0, _sdiv41i = 0;
        c_div(_cf39r, _cf39i, _arange40r, _arange40i, &_sdiv41r, &_sdiv41i);
        double _smul42r = 0, _smul42i = 0;
        c_mul(_c37r, _c37i, _sdiv41r, _sdiv41i, &_smul42r, &_smul42i);
        cRe[_si_idx] += _smul42r; cIm[_si_idx] += _smul42i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 60;
        double _unk43r = 0, _unk43i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=10)) */
        double _cf44r = 0, _cf44i = 0;
        { int _idx = (int)(_unk43r); if (_idx >= 0 && _idx < 71) { _cf44r = cRe[_idx]; _cf44i = cIm[_idx]; } }
        double _sin45r = 0, _sin45i = 0;
        c_sin(_cf44r, _cf44i, &_sin45r, &_sin45i);
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 30;
        double _add46r = 0, _add46i = 0;
        _add46r = x1r + x2r; _add46i = x1i + x2i;
        double _cos47r = 0, _cos47i = 0;
        c_cos(_add46r, _add46i, &_cos47r, &_cos47i);
        double _arange48r = 0, _arange48i = 0;
        _arange48r = (double)(_si + 1); _arange48i = 0;
        double _add49r = 0, _add49i = 0;
        _add49r = x1r + x2r; _add49i = x1i + x2i;
        double _cos50r = 0, _cos50i = 0;
        c_cos(_add49r, _add49i, &_cos50r, &_cos50i);
        double _pow51r = 0, _pow51i = 0;
        c_powr(_cos50r, _cos50i, 0, &_pow51r, &_pow51i);
        cRe[_si_idx] += _pow51r; cIm[_si_idx] += _pow51i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_69_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 71.0; _c1i = 0;
        double _sub2r = 0, _sub2i = 0;
        _sub2r = _c1r - i; _sub2i = _c1i - 0;
        double j = _sub2r; /* +_sub2ii */
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _im4r = 0, _im4i = 0;
        _im4r = x1i; _im4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_im4r, _im4i, j, 0, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _re3r + _mul5r; _add6i = _re3i + _mul5i;
        double _add7r = 0, _add7i = 0;
        _add7r = x2r + i; _add7i = x2i + 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_add7r, _add7i); _abs8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_add6r, _add6i, _abs8r, _abs8i, &_div9r, &_div9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(x2r, x2i, i, 0, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = x1r + _mul10r; _add11i = x1i + _mul10i;
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(_add11r, _add11i); _ang12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(_ang12r, _ang12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_div9r, _div9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul15r, &_mul15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_mul15r, _mul15i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _unk21r = 0, _unk21i = 0;
        /* WARNING: unhandled node Attribute(value=Name(id='np', ctx=Load()), attr='pi', ctx=Load()) */
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c20r, _c20i, _unk21r, _unk21i, &_mul22r, &_mul22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul22r, _mul22i, i, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 71.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log19r, _log19i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul14r + _mul27r; _add28i = _mul14i + _mul27i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add28r; cIm[_idx] = _add28i; } }
    }
    double _unk29r = 0, _unk29i = 0;
    /* WARNING: unhandled node Compare(left=Name(id='cf', ctx=Load()), ops=[Eq()], comparators=[Constant(value=0)]) */
    double _re30r = 0, _re30i = 0;
    _re30r = x1r; _re30i = 0;
    double _c31r = 0, _c31i = 0;
    _c31r = 2.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(_re30r, _re30i, _re30r, _re30i, &_pow32r, &_pow32i);
    double _im33r = 0, _im33i = 0;
    _im33r = x1i; _im33i = 0;
    double _im34r = 0, _im34i = 0;
    _im34r = x2i; _im34i = 0;
    double _mul35r = 0, _mul35i = 0;
    c_mul(_im33r, _im33i, _im34r, _im34i, &_mul35r, &_mul35i);
    double _sub36r = 0, _sub36i = 0;
    _sub36r = _pow32r - _mul35r; _sub36i = _pow32i - _mul35i;
    { int _idx = (int)(_unk29r); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_70_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _re2r = 0, _re2i = 0;
        _re2r = x2r; _re2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(_re1r, _re1i, _re2r, _re2i, &_mul3r, &_mul3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(i, 0, i, 0, &_pow5r, &_pow5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_mul3r, _mul3i, _pow5r, _pow5i, &_mul6r, &_mul6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 1.0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_abs7r, _abs7i, _c8r, _c8i, &_mul9r, &_mul9i);
        double _exp10r = 0, _exp10i = 0;
        c_exp2(_mul9r, _mul9i, &_exp10r, &_exp10i);
        double _div11r = 0, _div11i = 0;
        c_div(_mul6r, _mul6i, _exp10r, _exp10i, &_div11r, &_div11i);
        double _im12r = 0, _im12i = 0;
        _im12r = x1i; _im12i = 0;
        double _im13r = 0, _im13i = 0;
        _im13r = x2i; _im13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_im12r, _im12i, _im13r, _im13i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(i, 0, i, 0, &_pow16r, &_pow16i);
        c_mul(_pow16r, _pow16i, i, 0, &_pow16r, &_pow16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul14r, _mul14i, _pow16r, _pow16i, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.0; _c19i = 1.0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_abs18r, _abs18i, _c19r, _c19i, &_mul20r, &_mul20i);
        double _exp21r = 0, _exp21i = 0;
        c_exp2(_mul20r, _mul20i, &_exp21r, &_exp21i);
        double _div22r = 0, _div22i = 0;
        c_div(_mul17r, _mul17i, _exp21r, _exp21i, &_div22r, &_div22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _div11r + _div22r; _add23i = _div11i + _div22i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    }
    for (int _si = 0; _si < 70; _si++) {
        int _si_idx = _si + 1;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _neg25r = 0, _neg25i = 0;
        _neg25r = -(_c24r); _neg25i = -(_c24i);
        { double _tr = cRe[_si_idx]*_neg25r - cIm[_si_idx]*_neg25i; cIm[_si_idx] = cRe[_si_idx]*_neg25i + cIm[_si_idx]*_neg25r; cRe[_si_idx] = _tr; }
    }
    static const double p[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0, 64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0, 71.0};
    double _unk26r = 0, _unk26i = 0;
    /* WARNING: unhandled node Compare(left=BinOp(left=Name(id='p', ctx=Load()), op=Pow(), right=Constant(value=2)), ops=[LtE()], comparators=[Constant(value=71)]) */
    double _c27r = 0, _c27i = 0;
    _c27r = 0.0; _c27i = 1.0;
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(x1r, x1i); _abs28i = 0;
    double _mul29r = 0, _mul29i = 0;
    c_mul(_c27r, _c27i, _abs28r, _abs28i, &_mul29r, &_mul29i);
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(x2r, x2i); _abs30i = 0;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_mul29r, _mul29i, _abs30r, _abs30i, &_mul31r, &_mul31i);
    cRe[(int)(_unk26r)] += _mul31r; cIm[(int)(_unk26r)] += _mul31i;
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_71_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_re1r, _re1i, i, &_pow2r, &_pow2i);
        double _im3r = 0, _im3i = 0;
        _im3r = x2i; _im3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_im3r, _im3i, i, &_pow4r, &_pow4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_pow2r, _pow2i, _pow4r, _pow4i, &_mul5r, &_mul5i);
        double _im6r = 0, _im6i = 0;
        _im6r = x1i; _im6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_im6r, _im6i, i, &_pow7r, &_pow7i);
        double _re8r = 0, _re8i = 0;
        _re8r = x2r; _re8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_powr(_re8r, _re8i, i, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_pow7r, _pow7i, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul5r + _mul10r; _add11i = _mul5i + _mul10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(i, 0, i, 0, &_pow13r, &_pow13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _pow13r + _c14r; _add15i = _pow13i + _c14i;
        double _div16r = 0, _div16i = 0;
        c_div(_add11r, _add11i, _add15r, _add15i, &_div16r, &_div16i);
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div16r; cIm[_idx] = _div16i; } }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1000.0; _c17i = 0;
        { double _tr = cRe[_si_idx]*_c17r - cIm[_si_idx]*_c17i; cIm[_si_idx] = cRe[_si_idx]*_c17i + cIm[_si_idx]*_c17r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 7; _si++) {
        int _si_idx = _si + 8;
        double _c18r = 0, _c18i = 0;
        _c18r = 500.0; _c18i = 0;
        double _neg19r = 0, _neg19i = 0;
        _neg19r = -(_c18r); _neg19i = -(_c18i);
        { double _tr = cRe[_si_idx]*_neg19r - cIm[_si_idx]*_neg19i; cIm[_si_idx] = cRe[_si_idx]*_neg19i + cIm[_si_idx]*_neg19r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 8; _si++) {
        int _si_idx = _si + 17;
        double _c20r = 0, _c20i = 0;
        _c20r = 250.0; _c20i = 0;
        { double _tr = cRe[_si_idx]*_c20r - cIm[_si_idx]*_c20i; cIm[_si_idx] = cRe[_si_idx]*_c20i + cIm[_si_idx]*_c20r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 6; _si++) {
        int _si_idx = _si + 29;
        double _c21r = 0, _c21i = 0;
        _c21r = 125.0; _c21i = 0;
        double _neg22r = 0, _neg22i = 0;
        _neg22r = -(_c21r); _neg22i = -(_c21i);
        { double _tr = cRe[_si_idx]*_neg22r - cIm[_si_idx]*_neg22i; cIm[_si_idx] = cRe[_si_idx]*_neg22i + cIm[_si_idx]*_neg22r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 9; _si++) {
        int _si_idx = _si + 36;
        double _c23r = 0, _c23i = 0;
        _c23r = 60.0; _c23i = 0;
        { double _tr = cRe[_si_idx]*_c23r - cIm[_si_idx]*_c23i; cIm[_si_idx] = cRe[_si_idx]*_c23i + cIm[_si_idx]*_c23r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 49;
        double _c24r = 0, _c24i = 0;
        _c24r = 30.0; _c24i = 0;
        double _neg25r = 0, _neg25i = 0;
        _neg25r = -(_c24r); _neg25i = -(_c24i);
        { double _tr = cRe[_si_idx]*_neg25r - cIm[_si_idx]*_neg25i; cIm[_si_idx] = cRe[_si_idx]*_neg25i + cIm[_si_idx]*_neg25r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 9; _si++) {
        int _si_idx = _si + 55;
        double _c26r = 0, _c26i = 0;
        _c26r = 15.0; _c26i = 0;
        { double _tr = cRe[_si_idx]*_c26r - cIm[_si_idx]*_c26i; cIm[_si_idx] = cRe[_si_idx]*_c26i + cIm[_si_idx]*_c26r; cRe[_si_idx] = _tr; }
    }
    for (int _si = 0; _si < 2; _si++) {
        int _si_idx = _si + 69;
        double _c27r = 0, _c27i = 0;
        _c27r = 5.0; _c27i = 0;
        double _neg28r = 0, _neg28i = 0;
        _neg28r = -(_c27r); _neg28i = -(_c27i);
        { double _tr = cRe[_si_idx]*_neg28r - cIm[_si_idx]*_neg28i; cIm[_si_idx] = cRe[_si_idx]*_neg28i + cIm[_si_idx]*_neg28r; cRe[_si_idx] = _tr; }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_72: too complex for auto-transpile, stubbed */
static void poly_72_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_73_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 10.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 30.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_powr(_c1r, _c1i, 30.0, &_pow3r, &_pow3i);
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + x2r; _add4i = x1i + x2i;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_pow3r, _pow3i, _add4r, _add4i, &_mul5r, &_mul5i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 10.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 28.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_powr(_c6r, _c6i, 28.0, &_pow8r, &_pow8i);
    double _sub9r = 0, _sub9i = 0;
    _sub9r = x1r - x2r; _sub9i = x1i - x2i;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_pow8r, _pow8i, _sub9r, _sub9i, &_mul10r, &_mul10i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 10.0; _c11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 26.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_powr(_c11r, _c11i, 26.0, &_pow13r, &_pow13i);
    double _add14r = 0, _add14i = 0;
    _add14r = x1r + x2r; _add14i = x1i + x2i;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_pow13r, _pow13i, _add14r, _add14i, &_mul15r, &_mul15i);
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    for (int k = 4; k < 22; k++) {
        double _c16r = 0, _c16i = 0;
        _c16r = 10.0; _c16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 30.0; _c17i = 0;
        double _sub18r = 0, _sub18i = 0;
        _sub18r = _c17r - k; _sub18i = _c17i - 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_c16r, _c16i, _sub18r, &_pow19r, &_pow19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(x1r, x1i, &_cos20r, &_cos20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(x2r, x2i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _cos20r + _sin21r; _add22i = _cos20i + _sin21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_pow19r, _pow19i, _add22r, _add22i, &_mul23r, &_mul23i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    }
    for (int k = 22; k < 32; k++) {
        double _c24r = 0, _c24i = 0;
        _c24r = 10.0; _c24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 21.0; _c25i = 0;
        double _sub26r = 0, _sub26i = 0;
        _sub26r = k - _c25r; _sub26i = 0 - _c25i;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_c24r, _c24i, _sub26r, &_pow27r, &_pow27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(x1r, x1i, &_cos28r, &_cos28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(x2r, x2i, &_sin29r, &_sin29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _cos28r - _sin29r; _sub30i = _cos28i - _sin29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_pow27r, _pow27i, _sub30r, _sub30i, &_mul31r, &_mul31i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    for (int k = 32; k < 42; k++) {
        double _c32r = 0, _c32i = 0;
        _c32r = 10.0; _c32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 42.0; _c33i = 0;
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _c33r - k; _sub34i = _c33i - 0;
        double _pow35r = 0, _pow35i = 0;
        c_powr(_c32r, _c32i, _sub34r, &_pow35r, &_pow35i);
        double _add36r = 0, _add36i = 0;
        _add36r = x1r + x2r; _add36i = x1i + x2i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_pow35r, _pow35i, _add36r, _add36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = x1r + x2r; _add38i = x1i + x2i;
        double _cos39r = 0, _cos39i = 0;
        c_cos(_add38r, _add38i, &_cos39r, &_cos39i);
        double _sub40r = 0, _sub40i = 0;
        _sub40r = x1r - x2r; _sub40i = x1i - x2i;
        double _sin41r = 0, _sin41i = 0;
        c_sin(_sub40r, _sub40i, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos39r + _sin41r; _add42i = _cos39i + _sin41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_mul37r, _mul37i, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    double _c44r = 0, _c44i = 0;
    _c44r = 10.0; _c44i = 0;
    double _c45r = 0, _c45i = 0;
    _c45r = 21.0; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_powr(_c44r, _c44i, 21.0, &_pow46r, &_pow46i);
    double _sub47r = 0, _sub47i = 0;
    _sub47r = x1r - x2r; _sub47i = x1i - x2i;
    double _mul48r = 0, _mul48i = 0;
    c_mul(_pow46r, _pow46i, _sub47r, _sub47i, &_mul48r, &_mul48i);
    { int _idx = 41; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    for (int k = 43; k < 54; k++) {
        double _c49r = 0, _c49i = 0;
        _c49r = 10.0; _c49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 53.0; _c50i = 0;
        double _sub51r = 0, _sub51i = 0;
        _sub51r = _c50r - k; _sub51i = _c50i - 0;
        double _pow52r = 0, _pow52i = 0;
        c_powr(_c49r, _c49i, _sub51r, &_pow52r, &_pow52i);
        double _add53r = 0, _add53i = 0;
        _add53r = x1r + x2r; _add53i = x1i + x2i;
        double _abs54r = 0, _abs54i = 0;
        _abs54r = c_abs(_add53r, _add53i); _abs54i = 0;
        double _sub55r = 0, _sub55i = 0;
        _sub55r = x1r - x2r; _sub55i = x1i - x2i;
        double _ang56r = 0, _ang56i = 0;
        _ang56r = c_arg(_sub55r, _sub55i); _ang56i = 0;
        double _add57r = 0, _add57i = 0;
        _add57r = _abs54r + _ang56r; _add57i = _abs54i + _ang56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(_pow52r, _pow52i, _add57r, _add57i, &_mul58r, &_mul58i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int k = 54; k < 65; k++) {
        double _c59r = 0, _c59i = 0;
        _c59r = 10.0; _c59i = 0;
        double _c60r = 0, _c60i = 0;
        _c60r = 64.0; _c60i = 0;
        double _sub61r = 0, _sub61i = 0;
        _sub61r = _c60r - k; _sub61i = _c60i - 0;
        double _pow62r = 0, _pow62i = 0;
        c_powr(_c59r, _c59i, _sub61r, &_pow62r, &_pow62i);
        double _sub63r = 0, _sub63i = 0;
        _sub63r = x1r - x2r; _sub63i = x1i - x2i;
        double _abs64r = 0, _abs64i = 0;
        _abs64r = c_abs(_sub63r, _sub63i); _abs64i = 0;
        double _add65r = 0, _add65i = 0;
        _add65r = x1r + x2r; _add65i = x1i + x2i;
        double _ang66r = 0, _ang66i = 0;
        _ang66r = c_arg(_add65r, _add65i); _ang66i = 0;
        double _add67r = 0, _add67i = 0;
        _add67r = _abs64r + _ang66r; _add67i = _abs64i + _ang66i;
        double _mul68r = 0, _mul68i = 0;
        c_mul(_pow62r, _pow62i, _add67r, _add67i, &_mul68r, &_mul68i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul68r; cIm[_idx] = _mul68i; } }
    }
    for (int k = 65; k < 72; k++) {
        double _c69r = 0, _c69i = 0;
        _c69r = 10.0; _c69i = 0;
        double _c70r = 0, _c70i = 0;
        _c70r = 71.0; _c70i = 0;
        double _sub71r = 0, _sub71i = 0;
        _sub71r = _c70r - k; _sub71i = _c70i - 0;
        double _pow72r = 0, _pow72i = 0;
        c_powr(_c69r, _c69i, _sub71r, &_pow72r, &_pow72i);
        double _sin73r = 0, _sin73i = 0;
        c_sin(x1r, x1i, &_sin73r, &_sin73i);
        double _cos74r = 0, _cos74i = 0;
        c_cos(x2r, x2i, &_cos74r, &_cos74i);
        double _add75r = 0, _add75i = 0;
        _add75r = _sin73r + _cos74r; _add75i = _sin73i + _cos74i;
        double _mul76r = 0, _mul76i = 0;
        c_mul(_pow72r, _pow72i, _add75r, _add75i, &_mul76r, &_mul76i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul76r; cIm[_idx] = _mul76i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_74: too complex for auto-transpile, stubbed */
static void poly_74_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_75_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double powers[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0, 64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0};
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, x1r, x1i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 110.0; _c5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _mul4r + _mul8r; _add9i = _mul4i + _mul8i;
    double _c10r = 0, _c10i = 0;
    _c10r = 120.0; _c10i = 0;
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c10r, _c10i, x2r, x2i, &_mul11r, &_mul11i);
    double _add12r = 0, _add12i = 0;
    _add12r = _add9r + _mul11r; _add12i = _add9i + _mul11i;
    double _c13r = 0, _c13i = 0;
    _c13r = 130.0; _c13i = 0;
    double _sub14r = 0, _sub14i = 0;
    _sub14r = _add12r - _c13r; _sub14i = _add12i - _c13i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub14r; cIm[_idx] = _sub14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 200.0; _c15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 3.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow17r, &_pow17i);
    c_mul(_pow17r, _pow17i, x2r, x2i, &_pow17r, &_pow17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c15r, _c15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 210.0; _c19i = 0;
    double _c20r = 0, _c20i = 0;
    _c20r = 2.0; _c20i = 0;
    double _pow21r = 0, _pow21i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow21r, &_pow21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c19r, _c19i, _pow21r, _pow21i, &_mul22r, &_mul22i);
    double _sub23r = 0, _sub23i = 0;
    _sub23r = _mul18r - _mul22r; _sub23i = _mul18i - _mul22i;
    double _c24r = 0, _c24i = 0;
    _c24r = 220.0; _c24i = 0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c24r, _c24i, x2r, x2i, &_mul25r, &_mul25i);
    double _add26r = 0, _add26i = 0;
    _add26r = _sub23r + _mul25r; _add26i = _sub23i + _mul25i;
    double _c27r = 0, _c27i = 0;
    _c27r = 230.0; _c27i = 0;
    double _sub28r = 0, _sub28i = 0;
    _sub28r = _add26r - _c27r; _sub28i = _add26i - _c27i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub28r; cIm[_idx] = _sub28i; } }
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(x1r, x1i); _abs29i = 0;
    double _c30r = 0, _c30i = 0;
    _c30r = 4.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(_abs29r, _abs29i, _abs29r, _abs29i, &_pow31r, &_pow31i);
    c_mul(_pow31r, _pow31i, _pow31r, _pow31i, &_pow31r, &_pow31i);
    { int _idx = 4; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow31r; cIm[_idx] = _pow31i; } }
    double _ang32r = 0, _ang32i = 0;
    _ang32r = c_arg(x2r, x2i); _ang32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 6.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_powr(_ang32r, _ang32i, 6.0, &_pow34r, &_pow34i);
    { int _idx = 9; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow34r; cIm[_idx] = _pow34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 0.0; _c35i = 1.0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c35r, _c35i, x2r, x2i, &_mul36r, &_mul36i);
    double _add37r = 0, _add37i = 0;
    _add37r = x1r + _mul36r; _add37i = x1i + _mul36i;
    double _abs38r = 0, _abs38i = 0;
    _abs38r = c_abs(_add37r, _add37i); _abs38i = 0;
    double _log39r = 0, _log39i = 0;
    c_log(_abs38r, _abs38i, &_log39r, &_log39i);
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = _log39r + _c40r; _add41i = _log39i + _c40i;
    { int _idx = 14; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    double _unk42r = 0, _unk42i = 0;
    /* WARNING: unhandled node List(elts=[Constant(value=19), Constant(value=39)], ctx=Load()) */
    double _c43r = 0, _c43i = 0;
    _c43r = 0.0; _c43i = 1.0;
    double _cf44r = 0, _cf44i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 71) { _cf44r = cRe[_idx]; _cf44i = cIm[_idx]; } }
    double _mul45r = 0, _mul45i = 0;
    c_mul(_c43r, _c43i, _cf44r, _cf44i, &_mul45r, &_mul45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(_mul45r, _mul45i, x1r, x1i, &_mul46r, &_mul46i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_mul46r, _mul46i, x2r, x2i, &_mul47r, &_mul47i);
    double _re48r = 0, _re48i = 0;
    _re48r = _mul47r; _re48i = 0;
    { int _idx = (int)(_unk42r); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _re48r; cIm[_idx] = _re48i; } }
    double _unk49r = 0, _unk49i = 0;
    /* WARNING: unhandled node List(elts=[Constant(value=29), Constant(value=59)], ctx=Load()) */
    double _cf50r = 0, _cf50i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
    double _cf51r = 0, _cf51i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
    double _conj52r = 0, _conj52i = 0;
    _conj52r = _cf51r; _conj52i = -(_cf51i);
    double _mul53r = 0, _mul53i = 0;
    c_mul(_cf50r, _cf50i, _conj52r, _conj52i, &_mul53r, &_mul53i);
    double _im54r = 0, _im54i = 0;
    _im54r = _mul53i; _im54i = 0;
    { int _idx = (int)(_unk49r); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _im54r; cIm[_idx] = _im54i; } }
    double _cf55r = 0, _cf55i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
    double _sin56r = 0, _sin56i = 0;
    c_sin(_cf55r, _cf55i, &_sin56r, &_sin56i);
    double _cf57r = 0, _cf57i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
    double _cos58r = 0, _cos58i = 0;
    c_cos(_cf57r, _cf57i, &_cos58r, &_cos58i);
    double _add59r = 0, _add59i = 0;
    _add59r = _sin56r + _cos58r; _add59i = _sin56i + _cos58i;
    { int _idx = 34; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    double _cf60r = 0, _cf60i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 71) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
    double _abs61r = 0, _abs61i = 0;
    _abs61r = c_abs(_cf60r, _cf60i); _abs61i = 0;
    double _c62r = 0, _c62i = 0;
    _c62r = 2.0; _c62i = 0;
    double _pow63r = 0, _pow63i = 0;
    c_mul(_abs61r, _abs61i, _abs61r, _abs61i, &_pow63r, &_pow63i);
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow63r; cIm[_idx] = _pow63i; } }
    double _prod64r = 0, _prod64i = 0;
    /* WARNING: unhandled np.prod */
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _prod64r; cIm[_idx] = _prod64i; } }
    double _sum65r = 0, _sum65i = 0;
    /* WARNING: unhandled np.sum */
    { int _idx = 8; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sum65r; cIm[_idx] = _sum65i; } }
    for (int _si = 0; _si < 56; _si++) {
        int _si_idx = _si + 15;
        double _unk66r = 0, _unk66i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=15), upper=Constant(value=71)) */
        double _arr67r = 0, _arr67i = 0;
        { int _idx = (int)(_unk66r); _arr67r = (_idx >= 0 && _idx < 71) ? powers[_idx] : 0.0; _arr67i = 0; }
        double _sub68r = 0, _sub68i = 0;
        _sub68r = x1r - x2r; _sub68i = x1i - x2i;
        double _abs69r = 0, _abs69i = 0;
        _abs69r = c_abs(_sub68r, _sub68i); _abs69i = 0;
        double _mul70r = 0, _mul70i = 0;
        c_mul(_arr67r, _arr67i, _abs69r, _abs69i, &_mul70r, &_mul70i);
        cRe[_si_idx] = _mul70r; cIm[_si_idx] = _mul70i;
    }
    double _prod71r = 0, _prod71i = 0;
    _prod71r = 1; _prod71i = 0;
    for (int _pi = 0; _pi < 4; _pi++) { double _pr = _prod71r*cRe[_pi]-_prod71i*cIm[_pi]; double _pp = _prod71r*cIm[_pi]+_prod71i*cRe[_pi]; _prod71r=_pr; _prod71i=_pp; }
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _prod71r; cIm[_idx] = _prod71i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_76_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 7.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_powr(x1r, x1i, 7.0, &_pow2r, &_pow2i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow2r; cIm[_idx] = _pow2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 7.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_powr(x2r, x2i, 7.0, &_pow4r, &_pow4i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow4r; cIm[_idx] = _pow4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 6.0; _c5i = 0;
    double _pow6r = 0, _pow6i = 0;
    c_powr(x1r, x1i, 6.0, &_pow6r, &_pow6i);
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow6r; cIm[_idx] = _pow6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 6.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_powr(x2r, x2i, 6.0, &_pow8r, &_pow8i);
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow8r; cIm[_idx] = _pow8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 5.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_powr(x1r, x1i, 5.0, &_pow10r, &_pow10i);
    { int _idx = 4; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow10r; cIm[_idx] = _pow10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 5.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_powr(x2r, x2i, 5.0, &_pow12r, &_pow12i);
    { int _idx = 5; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow12r; cIm[_idx] = _pow12i; } }
    double _c13r = 0, _c13i = 0;
    _c13r = 4.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow14r, &_pow14i);
    c_mul(_pow14r, _pow14i, _pow14r, _pow14i, &_pow14r, &_pow14i);
    { int _idx = 6; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow14r; cIm[_idx] = _pow14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 4.0; _c15i = 0;
    double _pow16r = 0, _pow16i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow16r, &_pow16i);
    c_mul(_pow16r, _pow16i, _pow16r, _pow16i, &_pow16r, &_pow16i);
    { int _idx = 7; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow16r; cIm[_idx] = _pow16i; } }
    for (int k = 9; k < 23; k++) {
        double _add17r = 0, _add17i = 0;
        _add17r = x1r + x2r; _add17i = x1i + x2i;
        double _c18r = 0, _c18i = 0;
        _c18r = 8.0; _c18i = 0;
        double _sub19r = 0, _sub19i = 0;
        _sub19r = k - _c18r; _sub19i = 0 - _c18i;
        double _div20r = 0, _div20i = 0;
        c_div(_add17r, _add17i, _sub19r, _sub19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _pow22r = 0, _pow22i = 0;
        c_powr(_sin21r, _sin21i, k, &_pow22r, &_pow22i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow22r; cIm[_idx] = _pow22i; } }
    }
    for (int k = 23; k < 37; k++) {
        double _sub23r = 0, _sub23i = 0;
        _sub23r = x1r - x2r; _sub23i = x1i - x2i;
        double _c24r = 0, _c24i = 0;
        _c24r = 22.0; _c24i = 0;
        double _sub25r = 0, _sub25i = 0;
        _sub25r = k - _c24r; _sub25i = 0 - _c24i;
        double _div26r = 0, _div26i = 0;
        c_div(_sub23r, _sub23i, _sub25r, _sub25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _pow28r = 0, _pow28i = 0;
        c_powr(_cos27r, _cos27i, k, &_pow28r, &_pow28i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow28r; cIm[_idx] = _pow28i; } }
    }
    for (int k = 37; k < 51; k++) {
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, x2r, x2i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = x1r + _mul30r; _add31i = x1i + _mul30i;
        double _c32r = 0, _c32i = 0;
        _c32r = 36.0; _c32i = 0;
        double _sub33r = 0, _sub33i = 0;
        _sub33r = k - _c32r; _sub33i = 0 - _c32i;
        double _div34r = 0, _div34i = 0;
        c_div(_add31r, _add31i, _sub33r, _sub33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _pow36r = 0, _pow36i = 0;
        c_powr(_cos35r, _cos35i, k, &_pow36r, &_pow36i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow36r; cIm[_idx] = _pow36i; } }
    }
    for (int k = 51; k < 65; k++) {
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, x2r, x2i, &_mul38r, &_mul38i);
        double _sub39r = 0, _sub39i = 0;
        _sub39r = x1r - _mul38r; _sub39i = x1i - _mul38i;
        double _c40r = 0, _c40i = 0;
        _c40r = 50.0; _c40i = 0;
        double _sub41r = 0, _sub41i = 0;
        _sub41r = k - _c40r; _sub41i = 0 - _c40i;
        double _div42r = 0, _div42i = 0;
        c_div(_sub39r, _sub39i, _sub41r, _sub41i, &_div42r, &_div42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_div42r, _div42i, &_sin43r, &_sin43i);
        double _pow44r = 0, _pow44i = 0;
        c_powr(_sin43r, _sin43i, k, &_pow44r, &_pow44i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow44r; cIm[_idx] = _pow44i; } }
    }
    for (int k = 65; k < 72; k++) {
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, x2r, x2i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = x1r + _mul46r; _add47i = x1i + _mul46i;
        double _c48r = 0, _c48i = 0;
        _c48r = 64.0; _c48i = 0;
        double _sub49r = 0, _sub49i = 0;
        _sub49r = k - _c48r; _sub49i = 0 - _c48i;
        double _pow50r = 0, _pow50i = 0;
        c_powr(_add47r, _add47i, _sub49r, &_pow50r, &_pow50i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow50r; cIm[_idx] = _pow50i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_77_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(k, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _abs4r = 0, _abs4i = 0;
            _abs4r = c_abs(x1r, x1i); _abs4i = 0;
            double _c5r = 0, _c5i = 0;
            _c5r = 1.0; _c5i = 0;
            double _add6r = 0, _add6i = 0;
            _add6r = _abs4r + _c5r; _add6i = _abs4i + _c5i;
            double _log7r = 0, _log7i = 0;
            c_log(_add6r, _add6i, &_log7r, &_log7i);
            double _pow8r = 0, _pow8i = 0;
            c_powr(_log7r, _log7i, k, &_pow8r, &_pow8i);
            double _abs9r = 0, _abs9i = 0;
            _abs9r = c_abs(x2r, x2i); _abs9i = 0;
            double _c10r = 0, _c10i = 0;
            _c10r = 1.0; _c10i = 0;
            double _add11r = 0, _add11i = 0;
            _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
            double _log12r = 0, _log12i = 0;
            c_log(_add11r, _add11i, &_log12r, &_log12i);
            double _c13r = 0, _c13i = 0;
            _c13r = 71.0; _c13i = 0;
            double _sub14r = 0, _sub14i = 0;
            _sub14r = _c13r - k; _sub14i = _c13i - 0;
            double _pow15r = 0, _pow15i = 0;
            c_powr(_log12r, _log12i, _sub14r, &_pow15r, &_pow15i);
            double _add16r = 0, _add16i = 0;
            _add16r = _pow8r + _pow15r; _add16i = _pow8i + _pow15i;
            double _mul17r = 0, _mul17i = 0;
            c_mul(k, 0, x1r, x1i, &_mul17r, &_mul17i);
            double _c18r = 0, _c18i = 0;
            _c18r = 71.0; _c18i = 0;
            double _sub19r = 0, _sub19i = 0;
            _sub19r = _c18r - k; _sub19i = _c18i - 0;
            double _mul20r = 0, _mul20i = 0;
            c_mul(_sub19r, _sub19i, x2r, x2i, &_mul20r, &_mul20i);
            double _add21r = 0, _add21i = 0;
            _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
            double _sin22r = 0, _sin22i = 0;
            c_sin(_add21r, _add21i, &_sin22r, &_sin22i);
            double _mul23r = 0, _mul23i = 0;
            c_mul(_add16r, _add16i, _sin22r, _sin22i, &_mul23r, &_mul23i);
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
        } else {
            double _abs24r = 0, _abs24i = 0;
            _abs24r = c_abs(x1r, x1i); _abs24i = 0;
            double _c25r = 0, _c25i = 0;
            _c25r = 1.0; _c25i = 0;
            double _add26r = 0, _add26i = 0;
            _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
            double _log27r = 0, _log27i = 0;
            c_log(_add26r, _add26i, &_log27r, &_log27i);
            double _pow28r = 0, _pow28i = 0;
            c_powr(_log27r, _log27i, k, &_pow28r, &_pow28i);
            double _abs29r = 0, _abs29i = 0;
            _abs29r = c_abs(x2r, x2i); _abs29i = 0;
            double _c30r = 0, _c30i = 0;
            _c30r = 1.0; _c30i = 0;
            double _add31r = 0, _add31i = 0;
            _add31r = _abs29r + _c30r; _add31i = _abs29i + _c30i;
            double _log32r = 0, _log32i = 0;
            c_log(_add31r, _add31i, &_log32r, &_log32i);
            double _c33r = 0, _c33i = 0;
            _c33r = 71.0; _c33i = 0;
            double _sub34r = 0, _sub34i = 0;
            _sub34r = _c33r - k; _sub34i = _c33i - 0;
            double _pow35r = 0, _pow35i = 0;
            c_powr(_log32r, _log32i, _sub34r, &_pow35r, &_pow35i);
            double _sub36r = 0, _sub36i = 0;
            _sub36r = _pow28r - _pow35r; _sub36i = _pow28i - _pow35i;
            double _mul37r = 0, _mul37i = 0;
            c_mul(k, 0, x1r, x1i, &_mul37r, &_mul37i);
            double _c38r = 0, _c38i = 0;
            _c38r = 71.0; _c38i = 0;
            double _sub39r = 0, _sub39i = 0;
            _sub39r = _c38r - k; _sub39i = _c38i - 0;
            double _mul40r = 0, _mul40i = 0;
            c_mul(_sub39r, _sub39i, x2r, x2i, &_mul40r, &_mul40i);
            double _sub41r = 0, _sub41i = 0;
            _sub41r = _mul37r - _mul40r; _sub41i = _mul37i - _mul40i;
            double _cos42r = 0, _cos42i = 0;
            c_cos(_sub41r, _sub41i, &_cos42r, &_cos42i);
            double _mul43r = 0, _mul43i = 0;
            c_mul(_sub36r, _sub36i, _cos42r, _cos42i, &_mul43r, &_mul43i);
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
        }
    }
    double _abs44r = 0, _abs44i = 0;
    _abs44r = c_abs(x1r, x1i); _abs44i = 0;
    double _abs45r = 0, _abs45i = 0;
    _abs45r = c_abs(x2r, x2i); _abs45i = 0;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_abs44r, _abs44i, _abs45r, _abs45i, &_mul46r, &_mul46i);
    double r = _mul46r; /* +_mul46ii */
    for (int k = 50; k < 72; k++) {
        double _c47r = 0, _c47i = 0;
        _c47r = 50.0; _c47i = 0;
        double _sub48r = 0, _sub48i = 0;
        _sub48r = k - _c47r; _sub48i = 0 - _c47i;
        double _pow49r = 0, _pow49i = 0;
        c_powr(r, 0, _sub48r, &_pow49r, &_pow49i);
        { double _tr = cRe[(k - 1)]*_pow49r - cIm[(k - 1)]*_pow49i; cIm[(k - 1)] = cRe[(k - 1)]*_pow49i + cIm[(k - 1)]*_pow49r; cRe[(k - 1)] = _tr; }
    }
    for (int k = 15; k < 36; k++) {
        double _c50r = 0, _c50i = 0;
        _c50r = 2.0; _c50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 71.0; _c51i = 0;
        double _sub52r = 0, _sub52i = 0;
        _sub52r = _c51r - k; _sub52i = _c51i - 0;
        double _pow53r = 0, _pow53i = 0;
        c_powr(r, 0, _sub52r, &_pow53r, &_pow53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c50r, _c50i, _pow53r, _pow53i, &_mul54r, &_mul54i);
        { double _tr = cRe[(k - 1)]*_mul54r - cIm[(k - 1)]*_mul54i; cIm[(k - 1)] = cRe[(k - 1)]*_mul54i + cIm[(k - 1)]*_mul54r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_78_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 0;
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _arange2r = 0, _arange2i = 0;
        _arange2r = (double)(_si + 1); _arange2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _sdiv4r = 0, _sdiv4i = 0;
        c_div(_arange2r, _arange2i, _c3r, _c3i, &_sdiv4r, &_sdiv4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 5.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(0, 0, _c6r, _c6i, &_div7r, &_div7i);
        double _pow8r = 0, _pow8i = 0;
        c_powr(_abs5r, _abs5i, _div7r, &_pow8r, &_pow8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _c9r + _abs10r; _add11i = _c9i + _abs10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _smul13r = 0, _smul13i = 0;
        c_mul(_pow8r, _pow8i, _log12r, _log12i, &_smul13r, &_smul13i);
        cRe[_si_idx] = _smul13r; cIm[_si_idx] = _smul13i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 10;
        double _re14r = 0, _re14i = 0;
        _re14r = x1r; _re14i = 0;
        double _arange15r = 0, _arange15i = 0;
        _arange15r = (double)(_si + 1); _arange15i = 0;
        double _re16r = 0, _re16i = 0;
        _re16r = x1r; _re16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_re16r, _re16i, 0, &_pow17r, &_pow17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _smul19r = 0, _smul19i = 0;
        c_mul(_pow17r, _pow17i, _ang18r, _ang18i, &_smul19r, &_smul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _neg21r = 0, _neg21i = 0;
        _neg21r = -(_c20r); _neg21i = -(_c20i);
        double _arange22r = 0, _arange22i = 0;
        _arange22r = (double)(_si + 1); _arange22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _neg24r = 0, _neg24i = 0;
        _neg24r = -(_c23r); _neg24i = -(_c23i);
        double _pow25r = 0, _pow25i = 0;
        c_powr(_neg24r, _neg24i, 0, &_pow25r, &_pow25i);
        double _smul26r = 0, _smul26i = 0;
        c_mul(_smul19r, _smul19i, _pow25r, _pow25i, &_smul26r, &_smul26i);
        cRe[_si_idx] = _smul26r; cIm[_si_idx] = _smul26i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 20;
        double _im27r = 0, _im27i = 0;
        _im27r = x1i; _im27i = 0;
        double _arange28r = 0, _arange28i = 0;
        _arange28r = (double)(_si + 1); _arange28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _sdiv30r = 0, _sdiv30i = 0;
        c_div(_arange28r, _arange28i, _c29r, _c29i, &_sdiv30r, &_sdiv30i);
        double _im31r = 0, _im31i = 0;
        _im31r = x1i; _im31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 3.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(0, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _pow34r = 0, _pow34i = 0;
        c_powr(_im31r, _im31i, _div33r, &_pow34r, &_pow34i);
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(x2r, x2i); _abs35i = 0;
        double _arange36r = 0, _arange36i = 0;
        _arange36r = (double)(_si + 1); _arange36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 4.0; _c37i = 0;
        double _sdiv38r = 0, _sdiv38i = 0;
        c_div(_arange36r, _arange36i, _c37r, _c37i, &_sdiv38r, &_sdiv38i);
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(x2r, x2i); _abs39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 4.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(0, 0, _c40r, _c40i, &_div41r, &_div41i);
        double _pow42r = 0, _pow42i = 0;
        c_powr(_abs39r, _abs39i, _div41r, &_pow42r, &_pow42i);
        double _smul43r = 0, _smul43i = 0;
        c_mul(_pow34r, _pow34i, _pow42r, _pow42i, &_smul43r, &_smul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _neg45r = 0, _neg45i = 0;
        _neg45r = -(_c44r); _neg45i = -(_c44i);
        double _arange46r = 0, _arange46i = 0;
        _arange46r = (double)(_si + 1); _arange46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _neg48r = 0, _neg48i = 0;
        _neg48r = -(_c47r); _neg48i = -(_c47i);
        double _pow49r = 0, _pow49i = 0;
        c_powr(_neg48r, _neg48i, 0, &_pow49r, &_pow49i);
        double _smul50r = 0, _smul50i = 0;
        c_mul(_smul43r, _smul43i, _pow49r, _pow49i, &_smul50r, &_smul50i);
        cRe[_si_idx] = _smul50r; cIm[_si_idx] = _smul50i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 30;
        double _mul51r = 0, _mul51i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul51r, &_mul51i);
        double _abs52r = 0, _abs52i = 0;
        _abs52r = c_abs(_mul51r, _mul51i); _abs52i = 0;
        double _arange53r = 0, _arange53i = 0;
        _arange53r = (double)(_si + 1); _arange53i = 0;
        double _c54r = 0, _c54i = 0;
        _c54r = 2.0; _c54i = 0;
        double _sdiv55r = 0, _sdiv55i = 0;
        c_div(_arange53r, _arange53i, _c54r, _c54i, &_sdiv55r, &_sdiv55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul56r, &_mul56i);
        double _abs57r = 0, _abs57i = 0;
        _abs57r = c_abs(_mul56r, _mul56i); _abs57i = 0;
        double _c58r = 0, _c58i = 0;
        _c58r = 2.0; _c58i = 0;
        double _div59r = 0, _div59i = 0;
        c_div(0, 0, _c58r, _c58i, &_div59r, &_div59i);
        double _pow60r = 0, _pow60i = 0;
        c_powr(_abs57r, _abs57i, _div59r, &_pow60r, &_pow60i);
        double _arange61r = 0, _arange61i = 0;
        _arange61r = (double)(_si + 1); _arange61i = 0;
        double _smul62r = 0, _smul62i = 0;
        c_mul(_pow60r, _pow60i, _arange61r, _arange61i, &_smul62r, &_smul62i);
        cRe[_si_idx] = _smul62r; cIm[_si_idx] = _smul62i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 40;
        double _add63r = 0, _add63i = 0;
        _add63r = x1r + x2r; _add63i = x1i + x2i;
        double _c64r = 0, _c64i = 0;
        _c64r = 2.0; _c64i = 0;
        double _div65r = 0, _div65i = 0;
        c_div(0, 0, _c64r, _c64i, &_div65r, &_div65i);
        double _pow66r = 0, _pow66i = 0;
        c_powr(_add63r, _add63i, _div65r, &_pow66r, &_pow66i);
        double _re67r = 0, _re67i = 0;
        _re67r = _pow66r; _re67i = 0;
        double _mul68r = 0, _mul68i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul68r, &_mul68i);
        double _ang69r = 0, _ang69i = 0;
        _ang69r = c_arg(_mul68r, _mul68i); _ang69i = 0;
        double _cos70r = 0, _cos70i = 0;
        c_cos(_ang69r, _ang69i, &_cos70r, &_cos70i);
        double _mul71r = 0, _mul71i = 0;
        c_mul(_re67r, _re67i, _cos70r, _cos70i, &_mul71r, &_mul71i);
        double _c72r = 0, _c72i = 0;
        _c72r = 1.0; _c72i = 0;
        double _neg73r = 0, _neg73i = 0;
        _neg73r = -(_c72r); _neg73i = -(_c72i);
        double _arange74r = 0, _arange74i = 0;
        _arange74r = (double)(_si + 1); _arange74i = 0;
        double _c75r = 0, _c75i = 0;
        _c75r = 1.0; _c75i = 0;
        double _neg76r = 0, _neg76i = 0;
        _neg76r = -(_c75r); _neg76i = -(_c75i);
        double _pow77r = 0, _pow77i = 0;
        c_powr(_neg76r, _neg76i, 0, &_pow77r, &_pow77i);
        double _smul78r = 0, _smul78i = 0;
        c_mul(_mul71r, _mul71i, _pow77r, _pow77i, &_smul78r, &_smul78i);
        cRe[_si_idx] = _smul78r; cIm[_si_idx] = _smul78i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 50;
        double _add79r = 0, _add79i = 0;
        _add79r = x1r + x2r; _add79i = x1i + x2i;
        double _c80r = 0, _c80i = 0;
        _c80r = 3.0; _c80i = 0;
        double _div81r = 0, _div81i = 0;
        c_div(0, 0, _c80r, _c80i, &_div81r, &_div81i);
        double _pow82r = 0, _pow82i = 0;
        c_powr(_add79r, _add79i, _div81r, &_pow82r, &_pow82i);
        double _im83r = 0, _im83i = 0;
        _im83r = _pow82i; _im83i = 0;
        double _sub84r = 0, _sub84i = 0;
        _sub84r = x1r - x2r; _sub84i = x1i - x2i;
        double _ang85r = 0, _ang85i = 0;
        _ang85r = c_arg(_sub84r, _sub84i); _ang85i = 0;
        double _sin86r = 0, _sin86i = 0;
        c_sin(_ang85r, _ang85i, &_sin86r, &_sin86i);
        double _mul87r = 0, _mul87i = 0;
        c_mul(_im83r, _im83i, _sin86r, _sin86i, &_mul87r, &_mul87i);
        double _c88r = 0, _c88i = 0;
        _c88r = 1.0; _c88i = 0;
        double _neg89r = 0, _neg89i = 0;
        _neg89r = -(_c88r); _neg89i = -(_c88i);
        double _arange90r = 0, _arange90i = 0;
        _arange90r = (double)(_si + 1); _arange90i = 0;
        double _c91r = 0, _c91i = 0;
        _c91r = 1.0; _c91i = 0;
        double _neg92r = 0, _neg92i = 0;
        _neg92r = -(_c91r); _neg92i = -(_c91i);
        double _pow93r = 0, _pow93i = 0;
        c_powr(_neg92r, _neg92i, 0, &_pow93r, &_pow93i);
        double _smul94r = 0, _smul94i = 0;
        c_mul(_mul87r, _mul87i, _pow93r, _pow93i, &_smul94r, &_smul94i);
        cRe[_si_idx] = _smul94r; cIm[_si_idx] = _smul94i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 60;
        double _pow95r = 0, _pow95i = 0;
        c_powr(x1r, x1i, 0, &_pow95r, &_pow95i);
        double _re96r = 0, _re96i = 0;
        _re96r = _pow95r; _re96i = 0;
        double _pow97r = 0, _pow97i = 0;
        c_powr(x2r, x2i, 0, &_pow97r, &_pow97i);
        double _abs98r = 0, _abs98i = 0;
        _abs98r = c_abs(_pow97r, _pow97i); _abs98i = 0;
        double _mul99r = 0, _mul99i = 0;
        c_mul(_re96r, _re96i, _abs98r, _abs98i, &_mul99r, &_mul99i);
        double _c100r = 0, _c100i = 0;
        _c100r = 1.0; _c100i = 0;
        double _add101r = 0, _add101i = 0;
        _add101r = x1r + x2r; _add101i = x1i + x2i;
        double _abs102r = 0, _abs102i = 0;
        _abs102r = c_abs(_add101r, _add101i); _abs102i = 0;
        double _add103r = 0, _add103i = 0;
        _add103r = _c100r + _abs102r; _add103i = _c100i + _abs102i;
        double _log104r = 0, _log104i = 0;
        c_log(_add103r, _add103i, &_log104r, &_log104i);
        double _mul105r = 0, _mul105i = 0;
        c_mul(_mul99r, _mul99i, _log104r, _log104i, &_mul105r, &_mul105i);
        cRe[_si_idx] = _mul105r; cIm[_si_idx] = _mul105i;
    }
    double _sub106r = 0, _sub106i = 0;
    _sub106r = x1r - x2r; _sub106i = x1i - x2i;
    double _abs107r = 0, _abs107i = 0;
    _abs107r = c_abs(_sub106r, _sub106i); _abs107i = 0;
    double _c108r = 0, _c108i = 0;
    _c108r = 1.0; _c108i = 0;
    double _abs109r = 0, _abs109i = 0;
    _abs109r = c_abs(x1r, x1i); _abs109i = 0;
    double _add110r = 0, _add110i = 0;
    _add110r = _c108r + _abs109r; _add110i = _c108i + _abs109i;
    double _log111r = 0, _log111i = 0;
    c_log(_add110r, _add110i, &_log111r, &_log111i);
    double _mul112r = 0, _mul112i = 0;
    c_mul(_abs107r, _abs107i, _log111r, _log111i, &_mul112r, &_mul112i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul112r; cIm[_idx] = _mul112i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_79_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 0;
        double _arange1r = 0, _arange1i = 0;
        _arange1r = (double)(_si + 1); _arange1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = x1r + x2r; _add2i = x1i + x2i;
        double _smul3r = 0, _smul3i = 0;
        c_mul(_arange1r, _arange1i, _add2r, _add2i, &_smul3r, &_smul3i);
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _arange5r = 0, _arange5i = 0;
        _arange5r = (double)(_si + 1); _arange5i = 0;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_abs6r, _abs6i, 0, &_pow7r, &_pow7i);
        double _smul8r = 0, _smul8i = 0;
        c_mul(_smul3r, _smul3i, _pow7r, _pow7i, &_smul8r, &_smul8i);
        cRe[_si_idx] = _smul8r; cIm[_si_idx] = _smul8i;
    }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 35;
        double _arange9r = 0, _arange9i = 0;
        _arange9r = (double)(_si + 35); _arange9i = 0;
        double _sub10r = 0, _sub10i = 0;
        _sub10r = x1r - x2r; _sub10i = x1i - x2i;
        double _smul11r = 0, _smul11i = 0;
        c_mul(_arange9r, _arange9i, _sub10r, _sub10i, &_smul11r, &_smul11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _arange13r = 0, _arange13i = 0;
        _arange13r = (double)(_si + 35); _arange13i = 0;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs14r, _abs14i, 0, &_pow15r, &_pow15i);
        double _smul16r = 0, _smul16i = 0;
        c_mul(_smul11r, _smul11i, _pow15r, _pow15i, &_smul16r, &_smul16i);
        cRe[_si_idx] = _smul16r; cIm[_si_idx] = _smul16i;
    }
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x1r, x1i); _abs17i = 0;
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(x2r, x2i); _abs18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_abs17r, _abs17i, _abs18r, _abs18i, &_mul19r, &_mul19i);
    double _conj20r = 0, _conj20i = 0;
    _conj20r = x2r; _conj20i = -(x2i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(x1r, x1i, _conj20r, _conj20i, &_mul21r, &_mul21i);
    double _im22r = 0, _im22i = 0;
    _im22r = _mul21i; _im22i = 0;
    double _add23r = 0, _add23i = 0;
    _add23r = _mul19r + _im22r; _add23i = _mul19i + _im22i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_80_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 70.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_powr(x1r, x1i, 70.0, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 70.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_powr(x2r, x2i, 70.0, &_pow4r, &_pow4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _pow2r + _pow4r; _add5i = _pow2i + _pow4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    for (int _si = 0; _si < 69; _si++) {
        int _si_idx = _si + 1;
        double _mul6r = 0, _mul6i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_mul6r, _mul6i, 0, 0, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_mul(_sin8r, _sin8i, _sin8r, _sin8i, &_pow10r, &_pow10i);
        cRe[_si_idx] = _pow10r; cIm[_si_idx] = _pow10i;
    }
    for (int _si = 0; _si < 15; _si++) {
        int _si_idx = _si + 13;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(_log14r, _log14i, _log14r, _log14i, &_pow16r, &_pow16i);
        double _unk17r = 0, _unk17i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=13), upper=Constant(value=28)) */
        double _cf18r = 0, _cf18i = 0;
        { int _idx = (int)(_unk17r); if (_idx >= 0 && _idx < 71) { _cf18r = cRe[_idx]; _cf18i = cIm[_idx]; } }
        double _mul19r = 0, _mul19i = 0;
        c_mul(_pow16r, _pow16i, _cf18r, _cf18i, &_mul19r, &_mul19i);
        cRe[_si_idx] = _mul19r; cIm[_si_idx] = _mul19i;
    }
    for (int _si = 0; _si < 16; _si++) {
        int _si_idx = _si + 30;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        { double _tr = cRe[_si_idx]*_log23r - cIm[_si_idx]*_log23i; cIm[_si_idx] = cRe[_si_idx]*_log23i + cIm[_si_idx]*_log23r; cRe[_si_idx] = _tr; }
    }
    for (int i = 2; i < 5; i++) {
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(i, 0, _abs24r, _abs24i, &_mul25r, &_mul25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x2r, x2i); _abs26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul25r, _mul25i, _abs26r, _abs26i, &_mul27r, &_mul27i);
        cRe[(i * 15)] += _mul27r; cIm[(i * 15)] += _mul27i;
    }
    double _re28r = 0, _re28i = 0;
    _re28r = x1r; _re28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 3.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_re28r, _re28i, _re28r, _re28i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, _re28r, _re28i, &_pow30r, &_pow30i);
    double _im31r = 0, _im31i = 0;
    _im31r = x2i; _im31i = 0;
    double _c32r = 0, _c32i = 0;
    _c32r = 2.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(_im31r, _im31i, _im31r, _im31i, &_pow33r, &_pow33i);
    double _sub34r = 0, _sub34i = 0;
    _sub34r = _pow30r - _pow33r; _sub34i = _pow30i - _pow33i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub34r; cIm[_idx] = _sub34i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_81_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 20.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double _mul3r = 0, _mul3i = 0;
        c_mul(x1r, x1i, k, 0, &_mul3r, &_mul3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_mul3r, _mul3i, &_sin4r, &_sin4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_add2r, _add2i, _sin4r, _sin4i, &_mul5r, &_mul5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(x2r, x2i, k, 0, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_mul5r, _mul5i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_abs9r, _abs9i, k, &_pow10r, &_pow10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul8r + _pow10r; _add11i = _mul8i + _pow10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs12r, _abs12i, k, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add11r + _pow13r; _add14i = _add11i + _pow13i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add14r; cIm[_idx] = _add14i; } }
    }
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(x1r, x1i); _abs15i = 0;
    double _abs16r = 0, _abs16i = 0;
    _abs16r = c_abs(x2r, x2i); _abs16i = 0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_abs15r, _abs15i, _abs16r, _abs16i, &_mul17r, &_mul17i);
    cRe[(int)(0)] += _mul17r; cIm[(int)(0)] += _mul17i;
    double _c18r = 0, _c18i = 0;
    _c18r = 1.0; _c18i = 0;
    double _neg19r = 0, _neg19i = 0;
    _neg19r = -(_c18r); _neg19i = -(_c18i);
    double _pow20r = 0, _pow20i = 0;
    c_powr(_neg19r, _neg19i, 0, &_pow20r, &_pow20i);
    double _add21r = 0, _add21i = 0;
    _add21r = x1r + x2r; _add21i = x1i + x2i;
    double _ang22r = 0, _ang22i = 0;
    _ang22r = c_arg(_add21r, _add21i); _ang22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_pow20r, _pow20i, _ang22r, _ang22i, &_mul23r, &_mul23i);
    cRe[(int)(0)] += _mul23r; cIm[(int)(0)] += _mul23i;
    double _c24r = 0, _c24i = 0;
    _c24r = 1.0; _c24i = 0;
    double _neg25r = 0, _neg25i = 0;
    _neg25r = -(_c24r); _neg25i = -(_c24i);
    double _pow26r = 0, _pow26i = 0;
    c_powr(_neg25r, _neg25i, 0, &_pow26r, &_pow26i);
    double _add27r = 0, _add27i = 0;
    _add27r = x1r + x2r; _add27i = x1i + x2i;
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(_add27r, _add27i); _abs28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 1.0; _c29i = 0;
    double _add30r = 0, _add30i = 0;
    _add30r = _abs28r + _c29r; _add30i = _abs28i + _c29i;
    double _log31r = 0, _log31i = 0;
    c_log(_add30r, _add30i, &_log31r, &_log31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_pow26r, _pow26i, _log31r, _log31i, &_mul32r, &_mul32i);
    cRe[(int)(0)] += _mul32r; cIm[(int)(0)] += _mul32i;
    double _add33r = 0, _add33i = 0;
    _add33r = x1r + x2r; _add33i = x1i + x2i;
    double _re34r = 0, _re34i = 0;
    _re34r = _add33r; _re34i = 0;
    { double _tr = cRe[(int)(0)]*_re34r - cIm[(int)(0)]*_re34i; cIm[(int)(0)] = cRe[(int)(0)]*_re34i + cIm[(int)(0)]*_re34r; cRe[(int)(0)] = _tr; }
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _im36r = 0, _im36i = 0;
    _im36r = _add35i; _im36i = 0;
    { double _tr = cRe[(int)(0)]*_im36r - cIm[(int)(0)]*_im36i; cIm[(int)(0)] = cRe[(int)(0)]*_im36i + cIm[(int)(0)]*_im36r; cRe[(int)(0)] = _tr; }
    double _add37r = 0, _add37i = 0;
    _add37r = x1r + x2r; _add37i = x1i + x2i;
    double _conj38r = 0, _conj38i = 0;
    _conj38r = _add37r; _conj38i = -(_add37i);
    { double _tr = cRe[(int)(0)]*_conj38r - cIm[(int)(0)]*_conj38i; cIm[(int)(0)] = cRe[(int)(0)]*_conj38i + cIm[(int)(0)]*_conj38r; cRe[(int)(0)] = _tr; }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_82_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = _c1r + _mul2r; _add3i = _c1i + _mul2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(x1r, x1i); _abs5i = 0;
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(x2r, x2i); _abs6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_abs5r, _abs5i, _abs6r, _abs6i, &_mul7r, &_mul7i);
    double _add8r = 0, _add8i = 0;
    _add8r = _c4r + _mul7r; _add8i = _c4i + _mul7i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add8r; cIm[_idx] = _add8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 3.0; _c9i = 0;
    double _add10r = 0, _add10i = 0;
    _add10r = x1r + x2r; _add10i = x1i + x2i;
    double _abs11r = 0, _abs11i = 0;
    _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
    double _add12r = 0, _add12i = 0;
    _add12r = _c9r + _abs11r; _add12i = _c9i + _abs11i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
    for (int i = 4; i < 37; i++) {
        double _cf13r = 0, _cf13i = 0;
        { int _idx = (i - 2); if (_idx >= 0 && _idx < 71) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
        double _mul14r = 0, _mul14i = 0;
        c_mul(i, 0, x2r, x2i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_cf13r, _cf13i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = i + _mul16r; _add17i = 0 + _mul16i;
        double _cf18r = 0, _cf18i = 0;
        { int _idx = (i - 3); if (_idx >= 0 && _idx < 71) { _cf18r = cRe[_idx]; _cf18i = cIm[_idx]; } }
        double _mul19r = 0, _mul19i = 0;
        c_mul(i, 0, x1r, x1i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_cf18r, _cf18i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _add17r + _mul21r; _add22i = _add17i + _mul21i;
        double _cf23r = 0, _cf23i = 0;
        { int _idx = (i - 4); if (_idx >= 0 && _idx < 71) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
        double _mul24r = 0, _mul24i = 0;
        c_mul(i, 0, x1r, x1i, &_mul24r, &_mul24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul24r, _mul24i, x2r, x2i, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _mul25r + _c26r; _add27i = _mul25i + _c26i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_add27r, _add27i); _abs28i = 0;
        double _log29r = 0, _log29i = 0;
        c_log(_abs28r, _abs28i, &_log29r, &_log29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_cf23r, _cf23i, _log29r, _log29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add22r + _mul30r; _add31i = _add22i + _mul30i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    for (int i = 37; i < 71; i++) {
        double _c32r = 0, _c32i = 0;
        _c32r = 70.0; _c32i = 0;
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _c32r - i; _sub33i = _c32i - 0;
        double _call34r = 0, _call34i = 0;
        /* WARNING: unhandled call Name(id='min', ctx=Load()) */
        double _cf35r = 0, _cf35i = 0;
        { int _idx = (70 - (int)(_call34r)); if (_idx >= 0 && _idx < 71) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
        double _c36r = 0, _c36i = 0;
        _c36r = 70.0; _c36i = 0;
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _c36r - i; _sub37i = _c36i - 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_sub37r, _sub37i, x1r, x1i, &_mul38r, &_mul38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_mul38r, _mul38i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_cf35r, _cf35i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _sub33r + _mul40r; _add41i = _sub33i + _mul40i;
        double _call42r = 0, _call42i = 0;
        /* WARNING: unhandled call Name(id='min', ctx=Load()) */
        double _cf43r = 0, _cf43i = 0;
        { int _idx = (69 - (int)(_call42r)); if (_idx >= 0 && _idx < 71) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
        double _c44r = 0, _c44i = 0;
        _c44r = 70.0; _c44i = 0;
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _c44r - i; _sub45i = _c44i - 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_sub45r, _sub45i, x2r, x2i, &_mul46r, &_mul46i);
        double _cos47r = 0, _cos47i = 0;
        c_cos(_mul46r, _mul46i, &_cos47r, &_cos47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_cf43r, _cf43i, _cos47r, _cos47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _add41r + _mul48r; _add49i = _add41i + _mul48i;
        double _call50r = 0, _call50i = 0;
        /* WARNING: unhandled call Name(id='min', ctx=Load()) */
        double _cf51r = 0, _cf51i = 0;
        { int _idx = (68 - (int)(_call50r)); if (_idx >= 0 && _idx < 71) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
        double _c52r = 0, _c52i = 0;
        _c52r = 70.0; _c52i = 0;
        double _sub53r = 0, _sub53i = 0;
        _sub53r = _c52r - i; _sub53i = _c52i - 0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_sub53r, _sub53i, x1r, x1i, &_mul54r, &_mul54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_mul54r, _mul54i, x2r, x2i, &_mul55r, &_mul55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 1.0; _c56i = 0;
        double _add57r = 0, _add57i = 0;
        _add57r = _mul55r + _c56r; _add57i = _mul55i + _c56i;
        double _abs58r = 0, _abs58i = 0;
        _abs58r = c_abs(_add57r, _add57i); _abs58i = 0;
        double _log59r = 0, _log59i = 0;
        c_log(_abs58r, _abs58i, &_log59r, &_log59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_cf51r, _cf51i, _log59r, _log59i, &_mul60r, &_mul60i);
        double _add61r = 0, _add61i = 0;
        _add61r = _add49r + _mul60r; _add61i = _add49i + _mul60i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add61r; cIm[_idx] = _add61i; } }
    }
    double _sum62r = 0, _sum62i = 0;
    _sum62r = 0; _sum62i = 0;
    for (int _si = 0; _si < 70; _si++) { _sum62r += cRe[_si]; _sum62i += cIm[_si]; }
    double _sub63r = 0, _sub63i = 0;
    _sub63r = x1r - x2r; _sub63i = x1i - x2i;
    double _ang64r = 0, _ang64i = 0;
    _ang64r = c_arg(_sub63r, _sub63i); _ang64i = 0;
    double _re65r = 0, _re65i = 0;
    _re65r = _ang64r; _re65i = 0;
    double _add66r = 0, _add66i = 0;
    _add66r = _sum62r + _re65r; _add66i = _sum62i + _re65i;
    double _add67r = 0, _add67i = 0;
    _add67r = x1r + x2r; _add67i = x1i + x2i;
    double _ang68r = 0, _ang68i = 0;
    _ang68r = c_arg(_add67r, _add67i); _ang68i = 0;
    double _im69r = 0, _im69i = 0;
    _im69r = _ang68i; _im69i = 0;
    double _add70r = 0, _add70i = 0;
    _add70r = _add66r + _im69r; _add70i = _add66i + _im69i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add70r; cIm[_idx] = _add70i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_83_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 1; i < 72; i++) {
        double _add1r = 0, _add1i = 0;
        _add1r = x1r + x2r; _add1i = x1i + x2i;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _sub3r = 0, _sub3i = 0;
        _sub3r = i - _c2r; _sub3i = 0 - _c2i;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_add1r, _add1i, _sub3r, &_pow4r, &_pow4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(i, 0, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_pow4r, _pow4i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _sub7r = 0, _sub7i = 0;
        _sub7r = x1r - x2r; _sub7i = x1i - x2i;
        double _c8r = 0, _c8i = 0;
        _c8r = 70.0; _c8i = 0;
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _c8r - i; _sub9i = _c8i - 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _sub9r + _c10r; _add11i = _sub9i + _c10i;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_sub7r, _sub7i, _add11r, &_pow12r, &_pow12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(i, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_pow12r, _pow12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul6r + _mul14r; _add15i = _mul6i + _mul14i;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add15r; cIm[_idx] = _add15i; } }
    }
    double _cf16r = 0, _cf16i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
    double _re17r = 0, _re17i = 0;
    _re17r = _cf16r; _re17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 0.0; _c18i = 1.0;
    double _cf19r = 0, _cf19i = 0;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
    double _im20r = 0, _im20i = 0;
    _im20r = _cf19i; _im20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c18r, _c18i, _im20r, _im20i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _re17r + _mul21r; _add22i = _re17i + _mul21i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    double _cf23r = 0, _cf23i = 0;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
    double _re24r = 0, _re24i = 0;
    _re24r = _cf23r; _re24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 0.0; _c25i = 1.0;
    double _cf26r = 0, _cf26i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
    double _im27r = 0, _im27i = 0;
    _im27r = _cf26i; _im27i = 0;
    double _mul28r = 0, _mul28i = 0;
    c_mul(_c25r, _c25i, _im27r, _im27i, &_mul28r, &_mul28i);
    double _add29r = 0, _add29i = 0;
    _add29r = _re24r + _mul28r; _add29i = _re24i + _mul28i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    double _mul30r = 0, _mul30i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul30r, &_mul30i);
    double _abs31r = 0, _abs31i = 0;
    _abs31r = c_abs(_mul30r, _mul30i); _abs31i = 0;
    double _log32r = 0, _log32i = 0;
    c_log(_abs31r, _abs31i, &_log32r, &_log32i);
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_log32r, _log32i, _log32r, _log32i, &_pow34r, &_pow34i);
    cRe[35] += _pow34r; cIm[35] += _pow34i;
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_84_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 1000.0;
    double _add3r = 0, _add3i = 0;
    _add3r = _re1r + _c2r; _add3i = _re1i + _c2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 1.0; _c4i = 0;
    double _add5r = 0, _add5i = 0;
    _add5r = x1r + x2r; _add5i = x1i + x2i;
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(_add5r, _add5i); _abs6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = _c4r + _abs6r; _add7i = _c4i + _abs6i;
    double _log8r = 0, _log8i = 0;
    c_log(_add7r, _add7i, &_log8r, &_log8i);
    double _c9r = 0, _c9i = 0;
    _c9r = 1000.0; _c9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_log8r, _log8i, _c9r, _c9i, &_mul10r, &_mul10i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    for (int k = 3; k < 36; k++) {
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _neg12r = 0, _neg12i = 0;
        _neg12r = -(_c11r); _neg12i = -(_c11i);
        double _pow13r = 0, _pow13i = 0;
        c_powr(_neg12r, _neg12i, k, &_pow13r, &_pow13i);
        double _pow14r = 0, _pow14i = 0;
        c_powr(x1r, x1i, k, &_pow14r, &_pow14i);
        double _re15r = 0, _re15i = 0;
        _re15r = _pow14r; _re15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_powr(x2r, x2i, k, &_pow16r, &_pow16i);
        double _im17r = 0, _im17i = 0;
        _im17r = _pow16i; _im17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _re15r + _im17r; _add18i = _re15i + _im17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_pow13r, _pow13i, _add18r, _add18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1000.0; _c20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_mul19r, _mul19i, _c20r, _c20i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(k, 0, k, 0, &_pow23r, &_pow23i);
        double _div24r = 0, _div24i = 0;
        c_div(_mul21r, _mul21i, _pow23r, _pow23i, &_div24r, &_div24i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div24r; cIm[_idx] = _div24i; } }
    }
    for (int k = 36; k < 71; k++) {
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _neg26r = 0, _neg26i = 0;
        _neg26r = -(_c25r); _neg26i = -(_c25i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = k + _c27r; _add28i = 0 + _c27i;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_neg26r, _neg26i, _add28r, &_pow29r, &_pow29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 70.0; _c31i = 0;
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _c31r - k; _sub32i = _c31i - 0;
        double _pow33r = 0, _pow33i = 0;
        c_powr(_abs30r, _abs30i, _sub32r, &_pow33r, &_pow33i);
        double _add34r = 0, _add34i = 0;
        _add34r = x1r + x2r; _add34i = x1i + x2i;
        double _sin35r = 0, _sin35i = 0;
        c_sin(_add34r, _add34i, &_sin35r, &_sin35i);
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(_sin35r, _sin35i); _abs36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _pow33r + _abs36r; _add37i = _pow33i + _abs36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_pow29r, _pow29i, _add37r, _add37i, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 2.0; _c39i = 0;
        double _pow40r = 0, _pow40i = 0;
        c_mul(k, 0, k, 0, &_pow40r, &_pow40i);
        double _div41r = 0, _div41i = 0;
        c_div(_mul38r, _mul38i, _pow40r, _pow40i, &_div41r, &_div41i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div41r; cIm[_idx] = _div41i; } }
    }
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(x1r, x1i); _abs42i = 0;
    double _ang43r = 0, _ang43i = 0;
    _ang43r = c_arg(x2r, x2i); _ang43i = 0;
    double _cos44r = 0, _cos44i = 0;
    c_cos(_ang43r, _ang43i, &_cos44r, &_cos44i);
    double _c45r = 0, _c45i = 0;
    _c45r = 1000.0; _c45i = 0;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_cos44r, _cos44i, _c45r, _c45i, &_mul46r, &_mul46i);
    double _add47r = 0, _add47i = 0;
    _add47r = _abs42r + _mul46r; _add47i = _abs42i + _mul46i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_85_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _pow1r = 0, _pow1i = 0;
        c_powr(x1r, x1i, k, &_pow1r, &_pow1i);
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _sub3r = 0, _sub3i = 0;
        _sub3r = k - _c2r; _sub3i = 0 - _c2i;
        double _pow4r = 0, _pow4i = 0;
        c_powr(x2r, x2i, _sub3r, &_pow4r, &_pow4i);
        double _add5r = 0, _add5i = 0;
        _add5r = _pow1r + _pow4r; _add5i = _pow1i + _pow4i;
        double _re6r = 0, _re6i = 0;
        _re6r = _add5r; _re6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = x1r + x2r; _add7i = x1i + x2i;
        double _c8r = 0, _c8i = 0;
        _c8r = 70.0; _c8i = 0;
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _c8r - k; _sub9i = _c8i - 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_add7r, _add7i, _sub9r, &_pow10r, &_pow10i);
        double _im11r = 0, _im11i = 0;
        _im11r = _pow10i; _im11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _re6r + _im11r; _add12i = _re6i + _im11i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _mod14r = 0, _mod14i = 0;
        _mod14r = fmod(k, _c13r); _mod14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 0;
        if (_mod14r == _c15r) {
            double _add16r = 0, _add16i = 0;
            _add16r = x1r + x2r; _add16i = x1i + x2i;
            double _ang17r = 0, _ang17i = 0;
            _ang17r = c_arg(_add16r, _add16i); _ang17i = 0;
            double _mul18r = 0, _mul18i = 0;
            c_mul(_ang17r, _ang17i, k, 0, &_mul18r, &_mul18i);
            double _sin19r = 0, _sin19i = 0;
            c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
            cRe[(k - 1)] += _sin19r; cIm[(k - 1)] += _sin19i;
        } else {
            double _add20r = 0, _add20i = 0;
            _add20r = x1r + x2r; _add20i = x1i + x2i;
            double _ang21r = 0, _ang21i = 0;
            _ang21r = c_arg(_add20r, _add20i); _ang21i = 0;
            double _pow22r = 0, _pow22i = 0;
            c_powr(_ang21r, _ang21i, k, &_pow22r, &_pow22i);
            double _cos23r = 0, _cos23i = 0;
            c_cos(_pow22r, _pow22i, &_cos23r, &_cos23i);
            cRe[(k - 1)] += _cos23r; cIm[(k - 1)] += _cos23i;
        }
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _mod25r = 0, _mod25i = 0;
        _mod25r = fmod(k, _c24r); _mod25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        if (_mod25r == _c26r) {
            double _sub27r = 0, _sub27i = 0;
            _sub27r = x1r - x2r; _sub27i = x1i - x2i;
            double _abs28r = 0, _abs28i = 0;
            _abs28r = c_abs(_sub27r, _sub27i); _abs28i = 0;
            double _c29r = 0, _c29i = 0;
            _c29r = 10.0; _c29i = 0;
            double _div30r = 0, _div30i = 0;
            c_div(k, 0, _c29r, _c29i, &_div30r, &_div30i);
            double _pow31r = 0, _pow31i = 0;
            c_powr(_abs28r, _abs28i, _div30r, &_pow31r, &_pow31i);
            { double _tr = cRe[(k - 1)]*_pow31r - cIm[(k - 1)]*_pow31i; cIm[(k - 1)] = cRe[(k - 1)]*_pow31i + cIm[(k - 1)]*_pow31r; cRe[(k - 1)] = _tr; }
        }
        double _c32r = 0, _c32i = 0;
        _c32r = 4.0; _c32i = 0;
        double _mod33r = 0, _mod33i = 0;
        _mod33r = fmod(k, _c32r); _mod33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 0;
        if (_mod33r == _c34r) {
            double _abs35r = 0, _abs35i = 0;
            _abs35r = c_abs(x1r, x1i); _abs35i = 0;
            double _c36r = 0, _c36i = 0;
            _c36r = 1.0; _c36i = 0;
            double _add37r = 0, _add37i = 0;
            _add37r = _abs35r + _c36r; _add37i = _abs35i + _c36i;
            double _log38r = 0, _log38i = 0;
            c_log(_add37r, _add37i, &_log38r, &_log38i);
            double _pow39r = 0, _pow39i = 0;
            c_powr(_log38r, _log38i, k, &_pow39r, &_pow39i);
            cRe[(k - 1)] += _pow39r; cIm[(k - 1)] += _pow39i;
        }
        double _c40r = 0, _c40i = 0;
        _c40r = 5.0; _c40i = 0;
        double _mod41r = 0, _mod41i = 0;
        _mod41r = fmod(k, _c40r); _mod41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 0;
        if (_mod41r == _c42r) {
            double _abs43r = 0, _abs43i = 0;
            _abs43r = c_abs(x2r, x2i); _abs43i = 0;
            double _c44r = 0, _c44i = 0;
            _c44r = 1.0; _c44i = 0;
            double _add45r = 0, _add45i = 0;
            _add45r = _abs43r + _c44r; _add45i = _abs43i + _c44i;
            double _log46r = 0, _log46i = 0;
            c_log(_add45r, _add45i, &_log46r, &_log46i);
            double _c47r = 0, _c47i = 0;
            _c47r = 71.0; _c47i = 0;
            double _sub48r = 0, _sub48i = 0;
            _sub48r = _c47r - k; _sub48i = _c47i - 0;
            double _pow49r = 0, _pow49i = 0;
            c_powr(_log46r, _log46i, _sub48r, &_pow49r, &_pow49i);
            cRe[(k - 1)] -= _pow49r; cIm[(k - 1)] -= _pow49i;
        }
    }
    double _mul50r = 0, _mul50i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul50r, &_mul50i);
    { double _tr = cRe[35]*_mul50r - cIm[35]*_mul50i; cIm[35] = cRe[35]*_mul50i + cIm[35]*_mul50r; cRe[35] = _tr; }
    double _conj51r = 0, _conj51i = 0;
    _conj51r = x1r; _conj51i = -(x1i);
    double _conj52r = 0, _conj52i = 0;
    _conj52r = x2r; _conj52i = -(x2i);
    double _mul53r = 0, _mul53i = 0;
    c_mul(_conj51r, _conj51i, _conj52r, _conj52i, &_mul53r, &_mul53i);
    { double _tr = cRe[65]*_mul53r - cIm[65]*_mul53i; cIm[65] = cRe[65]*_mul53i + cIm[65]*_mul53r; cRe[65] = _tr; }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_86_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(k, 0, x1r, x1i, &_mul1r, &_mul1i);
        double _cos2r = 0, _cos2i = 0;
        c_cos(_mul1r, _mul1i, &_cos2r, &_cos2i);
        double _pow3r = 0, _pow3i = 0;
        c_powr(_cos2r, _cos2i, k, &_pow3r, &_pow3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 1.0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, x2r, x2i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _pow7r = 0, _pow7i = 0;
        c_powr(_sin6r, _sin6i, k, &_pow7r, &_pow7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c4r, _c4i, _pow7r, _pow7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow3r + _mul8r; _add9i = _pow3i + _mul8i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
    }
    for (int _si = 0; _si < 70; _si++) {
        int _si_idx = _si + 1;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _neg11r = 0, _neg11i = 0;
        _neg11r = -(_c10r); _neg11i = -(_c10i);
    }
    for (int _si = 0; _si < 69; _si++) {
        int _si_idx = _si + 2;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _neg13r = 0, _neg13i = 0;
        _neg13r = -(_c12r); _neg13i = -(_c12i);
    }
    for (int r = 5; r < 66; r += 5) {
        double _mul14r = 0, _mul14i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul14r, &_mul14i);
        double _pow15r = 0, _pow15i = 0;
        c_powr(_mul14r, _mul14i, r, &_pow15r, &_pow15i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow15r; cIm[_idx] = _pow15i; } }
    }
    double _abs16r = 0, _abs16i = 0;
    _abs16r = c_abs(x1r, x1i); _abs16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(_abs16r, _abs16i, _abs16r, _abs16i, &_pow18r, &_pow18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _re20r = 0, _re20i = 0;
    _re20r = x1r; _re20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c19r, _c19i, _re20r, _re20i, &_mul21r, &_mul21i);
    double _im22r = 0, _im22i = 0;
    _im22r = x2i; _im22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_mul21r, _mul21i, _im22r, _im22i, &_mul23r, &_mul23i);
    double _add24r = 0, _add24i = 0;
    _add24r = _pow18r + _mul23r; _add24i = _pow18i + _mul23i;
    double _c25r = 0, _c25i = 0;
    _c25r = 3.0; _c25i = 0;
    double _abs26r = 0, _abs26i = 0;
    _abs26r = c_abs(x2r, x2i); _abs26i = 0;
    double _c27r = 0, _c27i = 0;
    _c27r = 2.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(_abs26r, _abs26i, _abs26r, _abs26i, &_pow28r, &_pow28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_c25r, _c25i, _pow28r, _pow28i, &_mul29r, &_mul29i);
    double _add30r = 0, _add30i = 0;
    _add30r = _add24r + _mul29r; _add30i = _add24i + _mul29i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_87_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 36; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 1.0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_c1r, _c1i, x2r, x2i, &_mul2r, &_mul2i);
        double _add3r = 0, _add3i = 0;
        _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_add3r, _add3i, k, &_pow4r, &_pow4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, x2r, x2i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = x1r + _mul5r; _add6i = x1i + _mul5i;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_add6r, _add6i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul11r, &_mul11i);
        double _re12r = 0, _re12i = 0;
        _re12r = _mul11r; _re12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log10r, _log10i, _re12r, _re12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _pow4r + _mul13r; _add14i = _pow4i + _mul13i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add14r; cIm[_idx] = _add14i; } }
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 1.0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c15r, _c15i, x2r, x2i, &_mul16r, &_mul16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = x1r - _mul16r; _sub17i = x1i - _mul16i;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_sub17r, _sub17i, k, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(k, 0, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(k, 0, x1r, x1i, &_mul20r, &_mul20i);
        double _sub21r = 0, _sub21i = 0;
        _sub21r = x2r - _mul20r; _sub21i = x2i - _mul20i;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_sub21r, _sub21i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul26r, &_mul26i);
        double _im27r = 0, _im27i = 0;
        _im27r = _mul26i; _im27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log25r, _log25i, _im27r, _im27i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul19r - _mul28r; _sub29i = _mul19i - _mul28i;
        { int _idx = (70 - k); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    }
    double _c30r = 0, _c30i = 0;
    _c30r = 100.0; _c30i = 0;
    double _abs31r = 0, _abs31i = 0;
    _abs31r = c_abs(x1r, x1i); _abs31i = 0;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c30r, _c30i, _abs31r, _abs31i, &_mul32r, &_mul32i);
    double _abs33r = 0, _abs33i = 0;
    _abs33r = c_abs(x2r, x2i); _abs33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_mul32r, _mul32i, _abs33r, _abs33i, &_mul34r, &_mul34i);
    { int _idx = 35; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 200.0; _c35i = 0;
    double _ang36r = 0, _ang36i = 0;
    _ang36r = c_arg(x1r, x1i); _ang36i = 0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c35r, _c35i, _ang36r, _ang36i, &_mul37r, &_mul37i);
    double _ang38r = 0, _ang38i = 0;
    _ang38r = c_arg(x2r, x2i); _ang38i = 0;
    double _mul39r = 0, _mul39i = 0;
    c_mul(_mul37r, _mul37i, _ang38r, _ang38i, &_mul39r, &_mul39i);
    { int _idx = 36; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 37;
        double _unk40r = 0, _unk40i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=34)) */
        double _cf41r = 0, _cf41i = 0;
        { int _idx = (int)(_unk40r); if (_idx >= 0 && _idx < 71) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
        double _unk42r = 0, _unk42i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=37), upper=Constant(value=72)) */
        double _cf43r = 0, _cf43i = 0;
        { int _idx = (int)(_unk42r); if (_idx >= 0 && _idx < 71) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
        double _sub44r = 0, _sub44i = 0;
        _sub44r = _cf41r - _cf43r; _sub44i = _cf41i - _cf43i;
        cRe[_si_idx] = _sub44r; cIm[_si_idx] = _sub44i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_88_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _abs1r = 0, _abs1i = 0;
    _abs1r = c_abs(x1r, x1i); _abs1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = _abs1r + _c2r; _add3i = _abs1i + _c2i;
    double _log4r = 0, _log4i = 0;
    c_log(_add3r, _add3i, &_log4r, &_log4i);
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(x2r, x2i); _abs5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
    double _log8r = 0, _log8i = 0;
    c_log(_add7r, _add7i, &_log8r, &_log8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_log4r, _log4i, _log8r, _log8i, &_mul9r, &_mul9i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
    for (int k = 2; k < 37; k++) {
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, x1r, x1i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, x2r, x2i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin11r + _cos13r; _add14i = _sin11i + _cos13i;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(k, 0, k, 0, &_pow16r, &_pow16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _add14r - _pow16r; _sub17i = _add14i - _pow16i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub17r; cIm[_idx] = _sub17i; } }
    }
    for (int k = 37; k < 71; k++) {
        double _c18r = 0, _c18i = 0;
        _c18r = 71.0; _c18i = 0;
        double _sub19r = 0, _sub19i = 0;
        _sub19r = _c18r - k; _sub19i = _c18i - 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sub19r, _sub19i, x1r, x1i, &_mul20r, &_mul20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_mul20r, _mul20i, &_sin21r, &_sin21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 71.0; _c22i = 0;
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _c22r - k; _sub23i = _c22i - 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sub23r, _sub23i, x2r, x2i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _sin21r - _cos25r; _sub26i = _sin21i - _cos25i;
        double _c27r = 0, _c27i = 0;
        _c27r = 71.0; _c27i = 0;
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _c27r - k; _sub28i = _c27i - 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_mul(_sub28r, _sub28i, _sub28r, _sub28i, &_pow30r, &_pow30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _sub26r + _pow30r; _add31i = _sub26i + _pow30i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    double _mul32r = 0, _mul32i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul32r, &_mul32i);
    double _re33r = 0, _re33i = 0;
    _re33r = _mul32r; _re33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul34r, &_mul34i);
    double _im35r = 0, _im35i = 0;
    _im35r = _mul34i; _im35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _re33r + _im35r; _add36i = _re33i + _im35i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_89_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 1.0;
    double _sub3r = 0, _sub3i = 0;
    _sub3r = x2r - x1r; _sub3i = x2i - x1i;
    double _abs4r = 0, _abs4i = 0;
    _abs4r = c_abs(_sub3r, _sub3i); _abs4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _abs4r, _abs4i, &_mul5r, &_mul5i);
    double _sub6r = 0, _sub6i = 0;
    _sub6r = _mul1r - _mul5r; _sub6i = _mul1i - _mul5i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub6r; cIm[_idx] = _sub6i; } }
    for (int k = 2; k < 7; k++) {
        double _cf7r = 0, _cf7i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf7r = cRe[_idx]; _cf7i = cIm[_idx]; } }
        double _mul8r = 0, _mul8i = 0;
        c_mul(k, 0, x1r, x1i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _cf7r + _sin9r; _add10i = _cf7i + _sin9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(k, 0, x2r, x2i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _add10r + _cos12r; _add13i = _add10i + _cos12i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add13r; cIm[_idx] = _add13i; } }
    }
    for (int k = 7; k < 12; k++) {
        double _cf14r = 0, _cf14i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
        double _sub15r = 0, _sub15i = 0;
        _sub15r = x1r - k; _sub15i = x1i - 0;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_sub15r, _sub15i); _abs16i = 0;
        double _log17r = 0, _log17i = 0;
        c_log(_abs16r, _abs16i, &_log17r, &_log17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _cf14r + _log17r; _add18i = _cf14i + _log17i;
        double _sub19r = 0, _sub19i = 0;
        _sub19r = x2r - k; _sub19i = x2i - 0;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_sub19r, _sub19i); _abs20i = 0;
        double _log21r = 0, _log21i = 0;
        c_log(_abs20r, _abs20i, &_log21r, &_log21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _add18r - _log21r; _sub22i = _add18i - _log21i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub22r; cIm[_idx] = _sub22i; } }
    }
    for (int k = 12; k < 17; k++) {
        double _cf23r = 0, _cf23i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
        double _mul24r = 0, _mul24i = 0;
        c_mul(k, 0, x1r, x1i, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_mul24r, _mul24i, &_sin25r, &_sin25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _cf23r - _sin25r; _sub26i = _cf23i - _sin25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(k, 0, x2r, x2i, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _sub26r - _cos28r; _sub29i = _sub26i - _cos28i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    }
    for (int k = 17; k < 22; k++) {
        double _cf30r = 0, _cf30i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
        double _sub31r = 0, _sub31i = 0;
        _sub31r = x1r - k; _sub31i = x1i - 0;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(_sub31r, _sub31i); _abs32i = 0;
        double _log33r = 0, _log33i = 0;
        c_log(_abs32r, _abs32i, &_log33r, &_log33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _cf30r - _log33r; _sub34i = _cf30i - _log33i;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = x2r - k; _sub35i = x2i - 0;
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(_sub35r, _sub35i); _abs36i = 0;
        double _log37r = 0, _log37i = 0;
        c_log(_abs36r, _abs36i, &_log37r, &_log37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sub34r + _log37r; _add38i = _sub34i + _log37i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    }
    for (int k = 22; k < 27; k++) {
        double _cf39r = 0, _cf39i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
        double _mul40r = 0, _mul40i = 0;
        c_mul(k, 0, x1r, x1i, &_mul40r, &_mul40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_mul40r, _mul40i, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cf39r + _sin41r; _add42i = _cf39i + _sin41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(k, 0, x2r, x2i, &_mul43r, &_mul43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_mul43r, _mul43i, &_cos44r, &_cos44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _add42r + _cos44r; _add45i = _add42i + _cos44i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    }
    for (int k = 27; k < 32; k++) {
        double _cf46r = 0, _cf46i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
        double _sub47r = 0, _sub47i = 0;
        _sub47r = x1r - k; _sub47i = x1i - 0;
        double _abs48r = 0, _abs48i = 0;
        _abs48r = c_abs(_sub47r, _sub47i); _abs48i = 0;
        double _log49r = 0, _log49i = 0;
        c_log(_abs48r, _abs48i, &_log49r, &_log49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _cf46r + _log49r; _add50i = _cf46i + _log49i;
        double _sub51r = 0, _sub51i = 0;
        _sub51r = x2r - k; _sub51i = x2i - 0;
        double _abs52r = 0, _abs52i = 0;
        _abs52r = c_abs(_sub51r, _sub51i); _abs52i = 0;
        double _log53r = 0, _log53i = 0;
        c_log(_abs52r, _abs52i, &_log53r, &_log53i);
        double _sub54r = 0, _sub54i = 0;
        _sub54r = _add50r - _log53r; _sub54i = _add50i - _log53i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub54r; cIm[_idx] = _sub54i; } }
    }
    for (int k = 32; k < 37; k++) {
        double _cf55r = 0, _cf55i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
        double _mul56r = 0, _mul56i = 0;
        c_mul(k, 0, x1r, x1i, &_mul56r, &_mul56i);
        double _sin57r = 0, _sin57i = 0;
        c_sin(_mul56r, _mul56i, &_sin57r, &_sin57i);
        double _sub58r = 0, _sub58i = 0;
        _sub58r = _cf55r - _sin57r; _sub58i = _cf55i - _sin57i;
        double _mul59r = 0, _mul59i = 0;
        c_mul(k, 0, x2r, x2i, &_mul59r, &_mul59i);
        double _cos60r = 0, _cos60i = 0;
        c_cos(_mul59r, _mul59i, &_cos60r, &_cos60i);
        double _sub61r = 0, _sub61i = 0;
        _sub61r = _sub58r - _cos60r; _sub61i = _sub58i - _cos60i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub61r; cIm[_idx] = _sub61i; } }
    }
    for (int k = 37; k < 42; k++) {
        double _cf62r = 0, _cf62i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf62r = cRe[_idx]; _cf62i = cIm[_idx]; } }
        double _sub63r = 0, _sub63i = 0;
        _sub63r = x1r - k; _sub63i = x1i - 0;
        double _abs64r = 0, _abs64i = 0;
        _abs64r = c_abs(_sub63r, _sub63i); _abs64i = 0;
        double _log65r = 0, _log65i = 0;
        c_log(_abs64r, _abs64i, &_log65r, &_log65i);
        double _sub66r = 0, _sub66i = 0;
        _sub66r = _cf62r - _log65r; _sub66i = _cf62i - _log65i;
        double _sub67r = 0, _sub67i = 0;
        _sub67r = x2r - k; _sub67i = x2i - 0;
        double _abs68r = 0, _abs68i = 0;
        _abs68r = c_abs(_sub67r, _sub67i); _abs68i = 0;
        double _log69r = 0, _log69i = 0;
        c_log(_abs68r, _abs68i, &_log69r, &_log69i);
        double _add70r = 0, _add70i = 0;
        _add70r = _sub66r + _log69r; _add70i = _sub66i + _log69i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add70r; cIm[_idx] = _add70i; } }
    }
    for (int k = 42; k < 47; k++) {
        double _cf71r = 0, _cf71i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf71r = cRe[_idx]; _cf71i = cIm[_idx]; } }
        double _mul72r = 0, _mul72i = 0;
        c_mul(k, 0, x1r, x1i, &_mul72r, &_mul72i);
        double _sin73r = 0, _sin73i = 0;
        c_sin(_mul72r, _mul72i, &_sin73r, &_sin73i);
        double _add74r = 0, _add74i = 0;
        _add74r = _cf71r + _sin73r; _add74i = _cf71i + _sin73i;
        double _mul75r = 0, _mul75i = 0;
        c_mul(k, 0, x2r, x2i, &_mul75r, &_mul75i);
        double _cos76r = 0, _cos76i = 0;
        c_cos(_mul75r, _mul75i, &_cos76r, &_cos76i);
        double _add77r = 0, _add77i = 0;
        _add77r = _add74r + _cos76r; _add77i = _add74i + _cos76i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add77r; cIm[_idx] = _add77i; } }
    }
    for (int k = 47; k < 52; k++) {
        double _cf78r = 0, _cf78i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf78r = cRe[_idx]; _cf78i = cIm[_idx]; } }
        double _sub79r = 0, _sub79i = 0;
        _sub79r = x1r - k; _sub79i = x1i - 0;
        double _abs80r = 0, _abs80i = 0;
        _abs80r = c_abs(_sub79r, _sub79i); _abs80i = 0;
        double _log81r = 0, _log81i = 0;
        c_log(_abs80r, _abs80i, &_log81r, &_log81i);
        double _add82r = 0, _add82i = 0;
        _add82r = _cf78r + _log81r; _add82i = _cf78i + _log81i;
        double _sub83r = 0, _sub83i = 0;
        _sub83r = x2r - k; _sub83i = x2i - 0;
        double _abs84r = 0, _abs84i = 0;
        _abs84r = c_abs(_sub83r, _sub83i); _abs84i = 0;
        double _log85r = 0, _log85i = 0;
        c_log(_abs84r, _abs84i, &_log85r, &_log85i);
        double _sub86r = 0, _sub86i = 0;
        _sub86r = _add82r - _log85r; _sub86i = _add82i - _log85i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub86r; cIm[_idx] = _sub86i; } }
    }
    for (int k = 52; k < 57; k++) {
        double _cf87r = 0, _cf87i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf87r = cRe[_idx]; _cf87i = cIm[_idx]; } }
        double _mul88r = 0, _mul88i = 0;
        c_mul(k, 0, x1r, x1i, &_mul88r, &_mul88i);
        double _sin89r = 0, _sin89i = 0;
        c_sin(_mul88r, _mul88i, &_sin89r, &_sin89i);
        double _sub90r = 0, _sub90i = 0;
        _sub90r = _cf87r - _sin89r; _sub90i = _cf87i - _sin89i;
        double _mul91r = 0, _mul91i = 0;
        c_mul(k, 0, x2r, x2i, &_mul91r, &_mul91i);
        double _cos92r = 0, _cos92i = 0;
        c_cos(_mul91r, _mul91i, &_cos92r, &_cos92i);
        double _sub93r = 0, _sub93i = 0;
        _sub93r = _sub90r - _cos92r; _sub93i = _sub90i - _cos92i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub93r; cIm[_idx] = _sub93i; } }
    }
    for (int k = 57; k < 62; k++) {
        double _cf94r = 0, _cf94i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf94r = cRe[_idx]; _cf94i = cIm[_idx]; } }
        double _sub95r = 0, _sub95i = 0;
        _sub95r = x1r - k; _sub95i = x1i - 0;
        double _abs96r = 0, _abs96i = 0;
        _abs96r = c_abs(_sub95r, _sub95i); _abs96i = 0;
        double _log97r = 0, _log97i = 0;
        c_log(_abs96r, _abs96i, &_log97r, &_log97i);
        double _sub98r = 0, _sub98i = 0;
        _sub98r = _cf94r - _log97r; _sub98i = _cf94i - _log97i;
        double _sub99r = 0, _sub99i = 0;
        _sub99r = x2r - k; _sub99i = x2i - 0;
        double _abs100r = 0, _abs100i = 0;
        _abs100r = c_abs(_sub99r, _sub99i); _abs100i = 0;
        double _log101r = 0, _log101i = 0;
        c_log(_abs100r, _abs100i, &_log101r, &_log101i);
        double _add102r = 0, _add102i = 0;
        _add102r = _sub98r + _log101r; _add102i = _sub98i + _log101i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add102r; cIm[_idx] = _add102i; } }
    }
    for (int k = 62; k < 67; k++) {
        double _cf103r = 0, _cf103i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf103r = cRe[_idx]; _cf103i = cIm[_idx]; } }
        double _mul104r = 0, _mul104i = 0;
        c_mul(k, 0, x1r, x1i, &_mul104r, &_mul104i);
        double _sin105r = 0, _sin105i = 0;
        c_sin(_mul104r, _mul104i, &_sin105r, &_sin105i);
        double _add106r = 0, _add106i = 0;
        _add106r = _cf103r + _sin105r; _add106i = _cf103i + _sin105i;
        double _mul107r = 0, _mul107i = 0;
        c_mul(k, 0, x2r, x2i, &_mul107r, &_mul107i);
        double _cos108r = 0, _cos108i = 0;
        c_cos(_mul107r, _mul107i, &_cos108r, &_cos108i);
        double _add109r = 0, _add109i = 0;
        _add109r = _add106r + _cos108r; _add109i = _add106i + _cos108i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add109r; cIm[_idx] = _add109i; } }
    }
    for (int k = 67; k < 72; k++) {
        double _cf110r = 0, _cf110i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf110r = cRe[_idx]; _cf110i = cIm[_idx]; } }
        double _sub111r = 0, _sub111i = 0;
        _sub111r = x1r - k; _sub111i = x1i - 0;
        double _abs112r = 0, _abs112i = 0;
        _abs112r = c_abs(_sub111r, _sub111i); _abs112i = 0;
        double _log113r = 0, _log113i = 0;
        c_log(_abs112r, _abs112i, &_log113r, &_log113i);
        double _add114r = 0, _add114i = 0;
        _add114r = _cf110r + _log113r; _add114i = _cf110i + _log113i;
        double _sub115r = 0, _sub115i = 0;
        _sub115r = x2r - k; _sub115i = x2i - 0;
        double _abs116r = 0, _abs116i = 0;
        _abs116r = c_abs(_sub115r, _sub115i); _abs116i = 0;
        double _log117r = 0, _log117i = 0;
        c_log(_abs116r, _abs116i, &_log117r, &_log117i);
        double _sub118r = 0, _sub118i = 0;
        _sub118r = _add114r - _log117r; _sub118i = _add114i - _log117i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub118r; cIm[_idx] = _sub118i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_90_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 3.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow2r, &_pow2i);
    c_mul(_pow2r, _pow2i, x1r, x1i, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow4r, &_pow4i);
    double _sub5r = 0, _sub5i = 0;
    _sub5r = _pow2r - _pow4r; _sub5i = _pow2i - _pow4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub5r; cIm[_idx] = _sub5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 3.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_mul9r, _mul9i, x2r, x2i, &_mul10r, &_mul10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 3.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, x2r, x2i, &_pow12r, &_pow12i);
    double _sub13r = 0, _sub13i = 0;
    _sub13r = _mul10r - _pow12r; _sub13i = _mul10i - _pow12i;
    double _re14r = 0, _re14i = 0;
    _re14r = _sub13r; _re14i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _re14r; cIm[_idx] = _re14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 3.0; _c15i = 0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c15r, _c15i, x1r, x1i, &_mul16r, &_mul16i);
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_mul16r, _mul16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 3.0; _c20i = 0;
    double _pow21r = 0, _pow21i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow21r, &_pow21i);
    c_mul(_pow21r, _pow21i, x1r, x1i, &_pow21r, &_pow21i);
    double _sub22r = 0, _sub22i = 0;
    _sub22r = _mul19r - _pow21r; _sub22i = _mul19i - _pow21i;
    double _im23r = 0, _im23i = 0;
    _im23r = _sub22i; _im23i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _im23r; cIm[_idx] = _im23i; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 4.0; _c24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c24r, _c24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _c28r = 0, _c28i = 0;
    _c28r = 6.0; _c28i = 0;
    double _mul29r = 0, _mul29i = 0;
    c_mul(_c28r, _c28i, x1r, x1i, &_mul29r, &_mul29i);
    double _mul30r = 0, _mul30i = 0;
    c_mul(_mul29r, _mul29i, x2r, x2i, &_mul30r, &_mul30i);
    double _sub31r = 0, _sub31i = 0;
    _sub31r = _mul27r - _mul30r; _sub31i = _mul27i - _mul30i;
    double _c32r = 0, _c32i = 0;
    _c32r = 4.0; _c32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow34r, &_pow34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c32r, _c32i, _pow34r, _pow34i, &_mul35r, &_mul35i);
    double _add36r = 0, _add36i = 0;
    _add36r = _sub31r + _mul35r; _add36i = _sub31i + _mul35i;
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    for (int k = 5; k < 72; k++) {
        double _mul37r = 0, _mul37i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul37r, &_mul37i);
        double _abs38r = 0, _abs38i = 0;
        _abs38r = c_abs(_mul37r, _mul37i); _abs38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(k, 0, x1r, x1i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul39r + x2r; _add40i = _mul39i + x2i;
        double _sin41r = 0, _sin41i = 0;
        c_sin(_add40r, _add40i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_abs38r, _abs38i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = x1r + x2r; _add43i = x1i + x2i;
        double _conj44r = 0, _conj44i = 0;
        _conj44r = _add43r; _conj44i = -(_add43i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(k, 0, _conj44r, _conj44i, &_mul45r, &_mul45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(_mul45r, _mul45i, &_cos46r, &_cos46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _mul42r + _cos46r; _add47i = _mul42i + _cos46i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_91_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 71; k++) {
        double _pow1r = 0, _pow1i = 0;
        c_powr(x1r, x1i, k, &_pow1r, &_pow1i);
        double _pow2r = 0, _pow2i = 0;
        c_powr(x2r, x2i, k, &_pow2r, &_pow2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(_pow1r, _pow1i, _pow2r, _pow2i, &_mul3r, &_mul3i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    }
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + x2r; _add4i = x1i + x2i;
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(_add4r, _add4i); _abs5i = 0;
    double _log6r = 0, _log6i = 0;
    c_log(_abs5r, _abs5i, &_log6r, &_log6i);
    double _c7r = 0, _c7i = 0;
    _c7r = 1.0; _c7i = 0;
    double _add8r = 0, _add8i = 0;
    _add8r = _log6r + _c7r; _add8i = _log6i + _c7i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add8r; cIm[_idx] = _add8i; } }
    double _mul9r = 0, _mul9i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul9r, &_mul9i);
    double _abs10r = 0, _abs10i = 0;
    _abs10r = c_abs(_mul9r, _mul9i); _abs10i = 0;
    double _log11r = 0, _log11i = 0;
    c_log(_abs10r, _abs10i, &_log11r, &_log11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 1.0; _c12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _log11r + _c12r; _add13i = _log11i + _c12i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add13r; cIm[_idx] = _add13i; } }
    double _abs14r = 0, _abs14i = 0;
    _abs14r = c_abs(x1r, x1i); _abs14i = 0;
    double _c15r = 0, _c15i = 0;
    _c15r = 2.0; _c15i = 0;
    double _pow16r = 0, _pow16i = 0;
    c_mul(_abs14r, _abs14i, _abs14r, _abs14i, &_pow16r, &_pow16i);
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x2r, x2i); _abs17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(_abs17r, _abs17i, _abs17r, _abs17i, &_pow19r, &_pow19i);
    double _add20r = 0, _add20i = 0;
    _add20r = _pow16r + _pow19r; _add20i = _pow16i + _pow19i;
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(x1r, x1i); _abs21i = 0;
    double _c22r = 0, _c22i = 0;
    _c22r = 3.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(_abs21r, _abs21i, _abs21r, _abs21i, &_pow23r, &_pow23i);
    c_mul(_pow23r, _pow23i, _abs21r, _abs21i, &_pow23r, &_pow23i);
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(x2r, x2i); _abs24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 3.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(_abs24r, _abs24i, _abs24r, _abs24i, &_pow26r, &_pow26i);
    c_mul(_pow26r, _pow26i, _abs24r, _abs24i, &_pow26r, &_pow26i);
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _pow23r - _pow26r; _sub27i = _pow23i - _pow26i;
    { int _idx = 5; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub27r; cIm[_idx] = _sub27i; } }
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(x1r, x1i); _abs28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 4.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_abs28r, _abs28i, _abs28r, _abs28i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, _pow30r, _pow30i, &_pow30r, &_pow30i);
    double _abs31r = 0, _abs31i = 0;
    _abs31r = c_abs(x2r, x2i); _abs31i = 0;
    double _c32r = 0, _c32i = 0;
    _c32r = 4.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(_abs31r, _abs31i, _abs31r, _abs31i, &_pow33r, &_pow33i);
    c_mul(_pow33r, _pow33i, _pow33r, _pow33i, &_pow33r, &_pow33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _pow30r + _pow33r; _add34i = _pow30i + _pow33i;
    { int _idx = 7; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add34r; cIm[_idx] = _add34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 5.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_powr(x1r, x1i, 5.0, &_pow36r, &_pow36i);
    double _c37r = 0, _c37i = 0;
    _c37r = 5.0; _c37i = 0;
    double _pow38r = 0, _pow38i = 0;
    c_powr(x2r, x2i, 5.0, &_pow38r, &_pow38i);
    double _sub39r = 0, _sub39i = 0;
    _sub39r = _pow36r - _pow38r; _sub39i = _pow36i - _pow38i;
    { int _idx = 9; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub39r; cIm[_idx] = _sub39i; } }
    double _abs40r = 0, _abs40i = 0;
    _abs40r = c_abs(x1r, x1i); _abs40i = 0;
    double _ang41r = 0, _ang41i = 0;
    _ang41r = c_arg(x2r, x2i); _ang41i = 0;
    double _sin42r = 0, _sin42i = 0;
    c_sin(_ang41r, _ang41i, &_sin42r, &_sin42i);
    double _mul43r = 0, _mul43i = 0;
    c_mul(_abs40r, _abs40i, _sin42r, _sin42i, &_mul43r, &_mul43i);
    { int _idx = 19; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    double _abs44r = 0, _abs44i = 0;
    _abs44r = c_abs(x2r, x2i); _abs44i = 0;
    double _re45r = 0, _re45i = 0;
    _re45r = x1r; _re45i = 0;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_abs44r, _abs44i, _re45r, _re45i, &_mul46r, &_mul46i);
    { int _idx = 29; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    double _abs47r = 0, _abs47i = 0;
    _abs47r = c_abs(x1r, x1i); _abs47i = 0;
    double _im48r = 0, _im48i = 0;
    _im48r = x2i; _im48i = 0;
    double _mul49r = 0, _mul49i = 0;
    c_mul(_abs47r, _abs47i, _im48r, _im48i, &_mul49r, &_mul49i);
    { int _idx = 39; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    double _abs50r = 0, _abs50i = 0;
    _abs50r = c_abs(x2r, x2i); _abs50i = 0;
    double _ang51r = 0, _ang51i = 0;
    _ang51r = c_arg(x1r, x1i); _ang51i = 0;
    double _mul52r = 0, _mul52i = 0;
    c_mul(_abs50r, _abs50i, _ang51r, _ang51i, &_mul52r, &_mul52i);
    { int _idx = 49; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    double _mul53r = 0, _mul53i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul53r, &_mul53i);
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(_mul53r, _mul53i); _abs54i = 0;
    double _ang55r = 0, _ang55i = 0;
    _ang55r = c_arg(x2r, x2i); _ang55i = 0;
    double _cos56r = 0, _cos56i = 0;
    c_cos(_ang55r, _ang55i, &_cos56r, &_cos56i);
    double _mul57r = 0, _mul57i = 0;
    c_mul(_abs54r, _abs54i, _cos56r, _cos56i, &_mul57r, &_mul57i);
    { int _idx = 59; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    double _mul58r = 0, _mul58i = 0;
    c_mul(x2r, x2i, x1r, x1i, &_mul58r, &_mul58i);
    double _abs59r = 0, _abs59i = 0;
    _abs59r = c_abs(_mul58r, _mul58i); _abs59i = 0;
    double _re60r = 0, _re60i = 0;
    _re60r = x1r; _re60i = 0;
    double _mul61r = 0, _mul61i = 0;
    c_mul(_abs59r, _abs59i, _re60r, _re60i, &_mul61r, &_mul61i);
    { int _idx = 69; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul61r; cIm[_idx] = _mul61i; } }
    double _sum62r = 0, _sum62i = 0;
    _sum62r = 0; _sum62i = 0;
    for (int _si = 0; _si < 70; _si++) { _sum62r += cRe[_si]; _sum62i += cIm[_si]; }
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sum62r; cIm[_idx] = _sum62i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_92_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _pow1r = 0, _pow1i = 0;
        c_powr(x1r, x1i, k, &_pow1r, &_pow1i);
        double _c2r = 0, _c2i = 0;
        _c2r = 71.0; _c2i = 0;
        double _sub3r = 0, _sub3i = 0;
        _sub3r = _c2r - k; _sub3i = _c2i - 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(x2r, x2i, _sub3r, &_pow4r, &_pow4i);
        double _add5r = 0, _add5i = 0;
        _add5r = _pow1r + _pow4r; _add5i = _pow1i + _pow4i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    }
    for (int _si = 0; _si < 11; _si++) {
        int _si_idx = _si + 14;
        double _c6r = 0, _c6i = 0;
        _c6r = 0.0; _c6i = 1.0;
        double _unk7r = 0, _unk7i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=14), upper=Constant(value=25)) */
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (int)(_unk7r); if (_idx >= 0 && _idx < 71) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_c6r, _c6i, _cf8r, _cf8i, &_mul9r, &_mul9i);
        cRe[_si_idx] = _mul9r; cIm[_si_idx] = _mul9i;
    }
    for (int _si = 0; _si < 16; _si++) {
        int _si_idx = _si + 29;
        double _unk10r = 0, _unk10i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=29), upper=Constant(value=45)) */
        double _cf11r = 0, _cf11i = 0;
        { int _idx = (int)(_unk10r); if (_idx >= 0 && _idx < 71) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _conj12r = 0, _conj12i = 0;
        _conj12r = _cf11r; _conj12i = -(_cf11i);
        cRe[_si_idx] = _conj12r; cIm[_si_idx] = _conj12i;
    }
    for (int _si = 0; _si < 25; _si++) {
        int _si_idx = _si + 45;
        double _unk13r = 0, _unk13i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=45), upper=Constant(value=70)) */
        double _cf14r = 0, _cf14i = 0;
        { int _idx = (int)(_unk13r); if (_idx >= 0 && _idx < 71) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
        double _neg15r = 0, _neg15i = 0;
        _neg15r = -(_cf14r); _neg15i = -(_cf14i);
        cRe[_si_idx] = _neg15r; cIm[_si_idx] = _neg15i;
    }
    for (int _si = 0; _si < 17; _si++) {
        int _si_idx = _si + 54;
        double _unk16r = 0, _unk16i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=54), upper=Constant(value=71)) */
        double _cf17r = 0, _cf17i = 0;
        { int _idx = (int)(_unk16r); if (_idx >= 0 && _idx < 71) { _cf17r = cRe[_idx]; _cf17i = cIm[_idx]; } }
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.0; _c19i = 2.0;
        double _add20r = 0, _add20i = 0;
        _add20r = _c18r + _c19r; _add20i = _c18i + _c19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_cf17r, _cf17i, _add20r, _add20i, &_mul21r, &_mul21i);
        cRe[_si_idx] = _mul21r; cIm[_si_idx] = _mul21i;
    }
    double _re22r = 0, _re22i = 0;
    _re22r = x1r; _re22i = 0;
    double _c23r = 0, _c23i = 0;
    _c23r = 3.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(_re22r, _re22i, _re22r, _re22i, &_pow24r, &_pow24i);
    c_mul(_pow24r, _pow24i, _re22r, _re22i, &_pow24r, &_pow24i);
    double _im25r = 0, _im25i = 0;
    _im25r = x2i; _im25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 3.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(_im25r, _im25i, _im25r, _im25i, &_pow27r, &_pow27i);
    c_mul(_pow27r, _pow27i, _im25r, _im25i, &_pow27r, &_pow27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _pow24r + _pow27r; _add28i = _pow24i + _pow27i;
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(x1r, x1i); _abs29i = 0;
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(x2r, x2i); _abs30i = 0;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_abs29r, _abs29i, _abs30r, _abs30i, &_mul31r, &_mul31i);
    double _c32r = 0, _c32i = 0;
    _c32r = 1.0; _c32i = 0;
    double _add33r = 0, _add33i = 0;
    _add33r = _mul31r + _c32r; _add33i = _mul31i + _c32i;
    double _log34r = 0, _log34i = 0;
    c_log(_add33r, _add33i, &_log34r, &_log34i);
    double _sub35r = 0, _sub35i = 0;
    _sub35r = _add28r - _log34r; _sub35i = _add28i - _log34i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub35r; cIm[_idx] = _sub35i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_93_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 36; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_c1r, _c1i, k, 0, &_mul2r, &_mul2i);
        double _pow3r = 0, _pow3i = 0;
        c_powr(x1r, x1i, _mul2r, &_pow3r, &_pow3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(k, 0, _pow3r, _pow3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 35.0; _c6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _c6r - k; _sub7i = _c6i - 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c5r, _c5i, _sub7r, _sub7i, &_mul8r, &_mul8i);
        double _pow9r = 0, _pow9i = 0;
        c_powr(x2r, x2i, _mul8r, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_mul4r, _mul4i, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double _unk11r = 0, _unk11i = 0;
        /* WARNING: unhandled node Attribute(value=Name(id='np', ctx=Load()), attr='pi', ctx=Load()) */
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, _unk11r, _unk11i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 180.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _div16r = 0, _div16i = 0;
        c_div(_mul10r, _mul10i, _sin15r, _sin15i, &_div16r, &_div16i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div16r; cIm[_idx] = _div16i; } }
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c17r, _c17i, k, 0, &_mul18r, &_mul18i);
        double _pow19r = 0, _pow19i = 0;
        c_powr(x2r, x2i, _mul18r, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(k, 0, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 35.0; _c22i = 0;
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _c22r - k; _sub23i = _c22i - 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c21r, _c21i, _sub23r, _sub23i, &_mul24r, &_mul24i);
        double _pow25r = 0, _pow25i = 0;
        c_powr(x1r, x1i, _mul24r, &_pow25r, &_pow25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul20r, _mul20i, _pow25r, _pow25i, &_mul26r, &_mul26i);
        double _unk27r = 0, _unk27i = 0;
        /* WARNING: unhandled node Attribute(value=Name(id='np', ctx=Load()), attr='pi', ctx=Load()) */
        double _mul28r = 0, _mul28i = 0;
        c_mul(k, 0, _unk27r, _unk27i, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 180.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _div32r = 0, _div32i = 0;
        c_div(_mul26r, _mul26i, _cos31r, _cos31i, &_div32r, &_div32i);
        { int _idx = (71 - k); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div32r; cIm[_idx] = _div32i; } }
    }
    double _c33r = 0, _c33i = 0;
    _c33r = 100.0; _c33i = 0;
    double _re34r = 0, _re34i = 0;
    _re34r = x1r; _re34i = 0;
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c33r, _c33i, _re34r, _re34i, &_mul35r, &_mul35i);
    double _im36r = 0, _im36i = 0;
    _im36r = x2i; _im36i = 0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_mul35r, _mul35i, _im36r, _im36i, &_mul37r, &_mul37i);
    double _c38r = 0, _c38i = 0;
    _c38r = 100.0; _c38i = 0;
    double _im39r = 0, _im39i = 0;
    _im39r = x1i; _im39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c38r, _c38i, _im39r, _im39i, &_mul40r, &_mul40i);
    double _re41r = 0, _re41i = 0;
    _re41r = x2r; _re41i = 0;
    double _mul42r = 0, _mul42i = 0;
    c_mul(_mul40r, _mul40i, _re41r, _re41i, &_mul42r, &_mul42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _mul37r + _mul42r; _add43i = _mul37i + _mul42i;
    { int _idx = 35; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    double _add44r = 0, _add44i = 0;
    _add44r = x1r + x2r; _add44i = x1i + x2i;
    double _abs45r = 0, _abs45i = 0;
    _abs45r = c_abs(_add44r, _add44i); _abs45i = 0;
    double _sub46r = 0, _sub46i = 0;
    _sub46r = x1r - x2r; _sub46i = x1i - x2i;
    double _ang47r = 0, _ang47i = 0;
    _ang47r = c_arg(_sub46r, _sub46i); _ang47i = 0;
    double _mul48r = 0, _mul48i = 0;
    c_mul(_abs45r, _abs45i, _ang47r, _ang47i, &_mul48r, &_mul48i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_94: too complex for auto-transpile, stubbed */
static void poly_94_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_95_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(_re1r, _re1i, _re1r, _re1i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, _re1r, _re1i, &_pow3r, &_pow3i);
    double _im4r = 0, _im4i = 0;
    _im4r = x1i; _im4i = 0;
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _pow6r = 0, _pow6i = 0;
    c_mul(_im4r, _im4i, _im4r, _im4i, &_pow6r, &_pow6i);
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _pow3r - _pow6r; _sub7i = _pow3i - _pow6i;
    double _c8r = 0, _c8i = 0;
    _c8r = 2.0; _c8i = 0;
    double _im9r = 0, _im9i = 0;
    _im9r = x1i; _im9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c8r, _c8i, _im9r, _im9i, &_mul10r, &_mul10i);
    double _re11r = 0, _re11i = 0;
    _re11r = x1r; _re11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_mul10r, _mul10i, _re11r, _re11i, &_mul12r, &_mul12i);
    double _add13r = 0, _add13i = 0;
    _add13r = _sub7r + _mul12r; _add13i = _sub7i + _mul12i;
    double _re14r = 0, _re14i = 0;
    _re14r = x2r; _re14i = 0;
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _add13r - _re14r; _sub15i = _add13i - _re14i;
    double _im16r = 0, _im16i = 0;
    _im16r = x2i; _im16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(_im16r, _im16i, _im16r, _im16i, &_pow18r, &_pow18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _sub15r + _pow18r; _add19i = _sub15i + _pow18i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    double _im20r = 0, _im20i = 0;
    _im20r = x1i; _im20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(_im20r, _im20i, _im20r, _im20i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, _im20r, _im20i, &_pow22r, &_pow22i);
    double _c23r = 0, _c23i = 0;
    _c23r = 5.0; _c23i = 0;
    double _re24r = 0, _re24i = 0;
    _re24r = x1r; _re24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(_re24r, _re24i, _re24r, _re24i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c23r, _c23i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _sub28r = 0, _sub28i = 0;
    _sub28r = _pow22r - _mul27r; _sub28i = _pow22i - _mul27i;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _re30r = 0, _re30i = 0;
    _re30r = x1r; _re30i = 0;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_c29r, _c29i, _re30r, _re30i, &_mul31r, &_mul31i);
    double _im32r = 0, _im32i = 0;
    _im32r = x1i; _im32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_mul31r, _mul31i, _im32r, _im32i, &_mul33r, &_mul33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _sub28r + _mul33r; _add34i = _sub28i + _mul33i;
    double _c35r = 0, _c35i = 0;
    _c35r = 5.0; _c35i = 0;
    double _re36r = 0, _re36i = 0;
    _re36r = x2r; _re36i = 0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c35r, _c35i, _re36r, _re36i, &_mul37r, &_mul37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _add34r + _mul37r; _add38i = _add34i + _mul37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _im40r = 0, _im40i = 0;
    _im40r = x2i; _im40i = 0;
    double _c41r = 0, _c41i = 0;
    _c41r = 2.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_mul(_im40r, _im40i, _im40r, _im40i, &_pow42r, &_pow42i);
    double _mul43r = 0, _mul43i = 0;
    c_mul(_c39r, _c39i, _pow42r, _pow42i, &_mul43r, &_mul43i);
    double _sub44r = 0, _sub44i = 0;
    _sub44r = _add38r - _mul43r; _sub44i = _add38i - _mul43i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub44r; cIm[_idx] = _sub44i; } }
    for (int k = 3; k < 72; k++) {
        double _mul45r = 0, _mul45i = 0;
        c_mul(k, 0, x1r, x1i, &_mul45r, &_mul45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_mul45r, _mul45i, &_sin46r, &_sin46i);
        double _abs47r = 0, _abs47i = 0;
        _abs47r = c_abs(_sin46r, _sin46i); _abs47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(k, 0, x2r, x2i, &_mul48r, &_mul48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_mul48r, _mul48i, &_cos49r, &_cos49i);
        double _abs50r = 0, _abs50i = 0;
        _abs50r = c_abs(_cos49r, _cos49i); _abs50i = 0;
        double _add51r = 0, _add51i = 0;
        _add51r = _abs47r + _abs50r; _add51i = _abs47i + _abs50i;
        double _pow52r = 0, _pow52i = 0;
        c_powr(x1r, x1i, k, &_pow52r, &_pow52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 1.0; _c53i = 0;
        double _sub54r = 0, _sub54i = 0;
        _sub54r = k - _c53r; _sub54i = 0 - _c53i;
        double _pow55r = 0, _pow55i = 0;
        c_powr(x2r, x2i, _sub54r, &_pow55r, &_pow55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _pow52r + _pow55r; _add56i = _pow52i + _pow55i;
        double _abs57r = 0, _abs57i = 0;
        _abs57r = c_abs(_add56r, _add56i); _abs57i = 0;
        double _sub58r = 0, _sub58i = 0;
        _sub58r = _add51r - _abs57r; _sub58i = _add51i - _abs57i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub58r; cIm[_idx] = _sub58i; } }
    }
    for (int _si = 0; _si < 11; _si++) {
        int _si_idx = _si + 29;
        double _unk59r = 0, _unk59i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=29), upper=Constant(value=40)) */
        double _cf60r = 0, _cf60i = 0;
        { int _idx = (int)(_unk59r); if (_idx >= 0 && _idx < 71) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
        double _abs61r = 0, _abs61i = 0;
        _abs61r = c_abs(_cf60r, _cf60i); _abs61i = 0;
        double _sub62r = 0, _sub62i = 0;
        _sub62r = x1r - x2r; _sub62i = x1i - x2i;
        double _abs63r = 0, _abs63i = 0;
        _abs63r = c_abs(_sub62r, _sub62i); _abs63i = 0;
        double _c64r = 0, _c64i = 0;
        _c64r = 2.0; _c64i = 0;
        double _pow65r = 0, _pow65i = 0;
        c_mul(_abs63r, _abs63i, _abs63r, _abs63i, &_pow65r, &_pow65i);
        double _c66r = 0, _c66i = 0;
        _c66r = 1.0; _c66i = 0;
        double _add67r = 0, _add67i = 0;
        _add67r = _pow65r + _c66r; _add67i = _pow65i + _c66i;
        double _div68r = 0, _div68i = 0;
        c_div(_abs61r, _abs61i, _add67r, _add67i, &_div68r, &_div68i);
        cRe[_si_idx] = _div68r; cIm[_si_idx] = _div68i;
    }
    for (int _si = 0; _si < 11; _si++) {
        int _si_idx = _si + 49;
        double _unk69r = 0, _unk69i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=49), upper=Constant(value=60)) */
        double _cf70r = 0, _cf70i = 0;
        { int _idx = (int)(_unk69r); if (_idx >= 0 && _idx < 71) { _cf70r = cRe[_idx]; _cf70i = cIm[_idx]; } }
        double _abs71r = 0, _abs71i = 0;
        _abs71r = c_abs(_cf70r, _cf70i); _abs71i = 0;
        double _neg72r = 0, _neg72i = 0;
        _neg72r = -(_abs71r); _neg72i = -(_abs71i);
        double _add73r = 0, _add73i = 0;
        _add73r = x1r + x2r; _add73i = x1i + x2i;
        double _abs74r = 0, _abs74i = 0;
        _abs74r = c_abs(_add73r, _add73i); _abs74i = 0;
        double _c75r = 0, _c75i = 0;
        _c75r = 2.0; _c75i = 0;
        double _pow76r = 0, _pow76i = 0;
        c_mul(_abs74r, _abs74i, _abs74r, _abs74i, &_pow76r, &_pow76i);
        double _c77r = 0, _c77i = 0;
        _c77r = 1.0; _c77i = 0;
        double _add78r = 0, _add78i = 0;
        _add78r = _pow76r + _c77r; _add78i = _pow76i + _c77i;
        double _div79r = 0, _div79i = 0;
        c_div(_neg72r, _neg72i, _add78r, _add78i, &_div79r, &_div79i);
        cRe[_si_idx] = _div79r; cIm[_si_idx] = _div79i;
    }
    for (int _si = 0; _si < 7; _si++) {
        int _si_idx = _si + 64;
        double _unk80r = 0, _unk80i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Constant(value=7)) */
        double _cf81r = 0, _cf81i = 0;
        { int _idx = (int)(_unk80r); if (_idx >= 0 && _idx < 71) { _cf81r = cRe[_idx]; _cf81i = cIm[_idx]; } }
        double _abs82r = 0, _abs82i = 0;
        _abs82r = c_abs(x1r, x1i); _abs82i = 0;
        double _c83r = 0, _c83i = 0;
        _c83r = 2.0; _c83i = 0;
        double _pow84r = 0, _pow84i = 0;
        c_mul(_abs82r, _abs82i, _abs82r, _abs82i, &_pow84r, &_pow84i);
        double _abs85r = 0, _abs85i = 0;
        _abs85r = c_abs(x2r, x2i); _abs85i = 0;
        double _c86r = 0, _c86i = 0;
        _c86r = 2.0; _c86i = 0;
        double _pow87r = 0, _pow87i = 0;
        c_mul(_abs85r, _abs85i, _abs85r, _abs85i, &_pow87r, &_pow87i);
        double _add88r = 0, _add88i = 0;
        _add88r = _pow84r + _pow87r; _add88i = _pow84i + _pow87i;
        double _mul89r = 0, _mul89i = 0;
        c_mul(_cf81r, _cf81i, _add88r, _add88i, &_mul89r, &_mul89i);
        cRe[_si_idx] = _mul89r; cIm[_si_idx] = _mul89i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_96_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 5.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_powr(x1r, x1i, 5.0, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 4.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, _pow4r, _pow4i, &_pow4r, &_pow4i);
    double _sub5r = 0, _sub5i = 0;
    _sub5r = _pow2r - _pow4r; _sub5i = _pow2i - _pow4i;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
    double _add8r = 0, _add8i = 0;
    _add8r = _sub5r + _pow7r; _add8i = _sub5i + _pow7i;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow10r, &_pow10i);
    double _sub11r = 0, _sub11i = 0;
    _sub11r = _add8r - _pow10r; _sub11i = _add8i - _pow10i;
    double _abs12r = 0, _abs12i = 0;
    _abs12r = c_abs(x1r, x1i); _abs12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _sub11r + _abs12r; _add13i = _sub11i + _abs12i;
    double _abs14r = 0, _abs14i = 0;
    _abs14r = c_abs(x2r, x2i); _abs14i = 0;
    double _add15r = 0, _add15i = 0;
    _add15r = _add13r + _abs14r; _add15i = _add13i + _abs14i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add15r; cIm[_idx] = _add15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 6.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_powr(x2r, x2i, 6.0, &_pow17r, &_pow17i);
    double _c18r = 0, _c18i = 0;
    _c18r = 4.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow19r, &_pow19i);
    c_mul(_pow19r, _pow19i, _pow19r, _pow19i, &_pow19r, &_pow19i);
    double _sub20r = 0, _sub20i = 0;
    _sub20r = _pow17r - _pow19r; _sub20i = _pow17i - _pow19i;
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, x2r, x2i, &_pow22r, &_pow22i);
    double _add23r = 0, _add23i = 0;
    _add23r = _sub20r + _pow22r; _add23i = _sub20i + _pow22i;
    double _c24r = 0, _c24i = 0;
    _c24r = 2.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow25r, &_pow25i);
    double _sub26r = 0, _sub26i = 0;
    _sub26r = _add23r - _pow25r; _sub26i = _add23i - _pow25i;
    double _ang27r = 0, _ang27i = 0;
    _ang27r = c_arg(x1r, x1i); _ang27i = 0;
    double _add28r = 0, _add28i = 0;
    _add28r = _sub26r + _ang27r; _add28i = _sub26i + _ang27i;
    double _sin29r = 0, _sin29i = 0;
    c_sin(x2r, x2i, &_sin29r, &_sin29i);
    double _add30r = 0, _add30i = 0;
    _add30r = _add28r + _sin29r; _add30i = _add28i + _sin29i;
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 7.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_powr(x1r, x1i, 7.0, &_pow32r, &_pow32i);
    double _c33r = 0, _c33i = 0;
    _c33r = 5.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_powr(x2r, x2i, 5.0, &_pow34r, &_pow34i);
    double _add35r = 0, _add35i = 0;
    _add35r = _pow32r + _pow34r; _add35i = _pow32i + _pow34i;
    double _c36r = 0, _c36i = 0;
    _c36r = 3.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow37r, &_pow37i);
    c_mul(_pow37r, _pow37i, x1r, x1i, &_pow37r, &_pow37i);
    double _sub38r = 0, _sub38i = 0;
    _sub38r = _add35r - _pow37r; _sub38i = _add35i - _pow37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow40r, &_pow40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _sub38r - _pow40r; _sub41i = _sub38i - _pow40i;
    double _cos42r = 0, _cos42i = 0;
    c_cos(x1r, x1i, &_cos42r, &_cos42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _sub41r + _cos42r; _add43i = _sub41i + _cos42i;
    double _sin44r = 0, _sin44i = 0;
    c_sin(x2r, x2i, &_sin44r, &_sin44i);
    double _sub45r = 0, _sub45i = 0;
    _sub45r = _add43r - _sin44r; _sub45i = _add43i - _sin44i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    for (int k = 2; k < 51; k++) {
        double _cf46r = 0, _cf46i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
        double _mul47r = 0, _mul47i = 0;
        c_mul(k, 0, _cf46r, _cf46i, &_mul47r, &_mul47i);
        double _cf48r = 0, _cf48i = 0;
        { int _idx = 0; if (_idx >= 0 && _idx < 71) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
        double _abs49r = 0, _abs49i = 0;
        _abs49r = c_abs(_cf48r, _cf48i); _abs49i = 0;
        double _div50r = 0, _div50i = 0;
        c_div(_abs49r, _abs49i, k, 0, &_div50r, &_div50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul47r + _div50r; _add51i = _mul47i + _div50i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    for (int r = 52; r < 71; r++) {
        double _cf52r = 0, _cf52i = 0;
        { int _idx = (r - 2); if (_idx >= 0 && _idx < 71) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
        double _mul53r = 0, _mul53i = 0;
        c_mul(r, 0, _cf52r, _cf52i, &_mul53r, &_mul53i);
        double _cf54r = 0, _cf54i = 0;
        { int _idx = 50; if (_idx >= 0 && _idx < 71) { _cf54r = cRe[_idx]; _cf54i = cIm[_idx]; } }
        double _abs55r = 0, _abs55i = 0;
        _abs55r = c_abs(_cf54r, _cf54i); _abs55i = 0;
        double _div56r = 0, _div56i = 0;
        c_div(_abs55r, _abs55i, r, 0, &_div56r, &_div56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _mul53r + _div56r; _add57i = _mul53i + _div56i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add57r; cIm[_idx] = _add57i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_97_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 72; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 1.0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_c1r, _c1i, x2r, x2i, &_mul2r, &_mul2i);
        double _add3r = 0, _add3i = 0;
        _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _neg5r = 0, _neg5i = 0;
        _neg5r = -(_abs4r); _neg5i = -(_abs4i);
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = k + _c6r; _add7i = 0 + _c6i;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_add7r, _add7i); _abs8i = 0;
        double _log9r = 0, _log9i = 0;
        c_log(_abs8r, _abs8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_neg5r, _neg5i, _log9r, _log9i, &_mul10r, &_mul10i);
        double _pow11r = 0, _pow11i = 0;
        c_powr(k, 0, _mul10r, &_pow11r, &_pow11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_add3r, _add3i, _pow11r, _pow11i, &_mul12r, &_mul12i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    }
    for (int k = 1; k < 11; k++) {
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 1.0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_c13r, _c13i, x2r, x2i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = x1r + _mul14r; _add15i = x1i + _mul14i;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_mul(k, 0, k, 0, &_pow17r, &_pow17i);
        c_mul(_pow17r, _pow17i, k, 0, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, x2r, x2i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = x1r + _mul18r; _add19i = x1i + _mul18i;
        double _im20r = 0, _im20i = 0;
        _im20r = _add19i; _im20i = 0;
        double _cos21r = 0, _cos21i = 0;
        c_cos(_im20r, _im20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_pow17r, _pow17i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(k, 0, x2r, x2i, &_mul23r, &_mul23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = x1r - _mul23r; _sub24i = x1i - _mul23i;
        double _re25r = 0, _re25i = 0;
        _re25r = _sub24r; _re25i = 0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(_re25r, _re25i, &_sin26r, &_sin26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _mul22r - _sin26r; _sub27i = _mul22i - _sin26i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_sub27r, _sub27i); _abs28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_add15r, _add15i, _abs28r, _abs28i, &_mul29r, &_mul29i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    }
    for (int k = 61; k < 72; k++) {
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, x2r, x2i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = x1r + _mul31r; _add32i = x1i + _mul31i;
        double _cf33r = 0, _cf33i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
        double _mul34r = 0, _mul34i = 0;
        c_mul(k, 0, _cf33r, _cf33i, &_mul34r, &_mul34i);
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(_mul34r, _mul34i); _abs35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_add32r, _add32i, _abs35r, _abs35i, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 3.0; _c37i = 0;
        double _pow38r = 0, _pow38i = 0;
        c_mul(k, 0, k, 0, &_pow38r, &_pow38i);
        c_mul(_pow38r, _pow38i, k, 0, &_pow38r, &_pow38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(k, 0, x2r, x2i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = x1r + _mul39r; _add40i = x1i + _mul39i;
        double _im41r = 0, _im41i = 0;
        _im41r = _add40i; _im41i = 0;
        double _cos42r = 0, _cos42i = 0;
        c_cos(_im41r, _im41i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_pow38r, _pow38i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _abs44r = 0, _abs44i = 0;
        _abs44r = c_abs(_mul43r, _mul43i); _abs44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(_mul36r, _mul36i, _abs44r, _abs44i, &_div45r, &_div45i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div45r; cIm[_idx] = _div45i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_98_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 36; k++) {
        double _add1r = 0, _add1i = 0;
        _add1r = x1r + x2r; _add1i = x1i + x2i;
        double _mul2r = 0, _mul2i = 0;
        c_mul(x2r, x2i, k, 0, &_mul2r, &_mul2i);
        double _sin3r = 0, _sin3i = 0;
        c_sin(_mul2r, _mul2i, &_sin3r, &_sin3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(_add1r, _add1i, _sin3r, _sin3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(k, 0, k, 0, &_pow6r, &_pow6i);
        double _div7r = 0, _div7i = 0;
        c_div(_mul4r, _mul4i, _pow6r, _pow6i, &_div7r, &_div7i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div7r; cIm[_idx] = _div7i; } }
    }
    for (int k = 36; k < 71; k++) {
        double _sub8r = 0, _sub8i = 0;
        _sub8r = x1r - x2r; _sub8i = x1i - x2i;
        double _c9r = 0, _c9i = 0;
        _c9r = 71.0; _c9i = 0;
        double _sub10r = 0, _sub10i = 0;
        _sub10r = _c9r - k; _sub10i = _c9i - 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(x1r, x1i, _sub10r, _sub10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sub8r, _sub8i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 71.0; _c14i = 0;
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _c14r - k; _sub15i = _c14i - 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_mul(_sub15r, _sub15i, _sub15r, _sub15i, &_pow17r, &_pow17i);
        double _div18r = 0, _div18i = 0;
        c_div(_mul13r, _mul13i, _pow17r, _pow17i, &_div18r, &_div18i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div18r; cIm[_idx] = _div18i; } }
    }
    double _re19r = 0, _re19i = 0;
    _re19r = x1r; _re19i = 0;
    double _im20r = 0, _im20i = 0;
    _im20r = x2i; _im20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_re19r, _re19i, _im20r, _im20i, &_mul21r, &_mul21i);
    double _re22r = 0, _re22i = 0;
    _re22r = x2r; _re22i = 0;
    double _im23r = 0, _im23i = 0;
    _im23r = x1i; _im23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_re22r, _re22i, _im23r, _im23i, &_mul24r, &_mul24i);
    double _sub25r = 0, _sub25i = 0;
    _sub25r = _mul21r - _mul24r; _sub25i = _mul21i - _mul24i;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub25r; cIm[_idx] = _sub25i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_99_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = _add1r + _c2r; _add3i = _add1i + _c2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    for (int _si = 0; _si < 9; _si++) {
        int _si_idx = _si + 1;
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(_re4r, _re4i, _re4r, _re4i, &_pow6r, &_pow6i);
        double _im7r = 0, _im7i = 0;
        _im7r = x2i; _im7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(_im7r, _im7i, _im7r, _im7i, &_pow9r, &_pow9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _pow6r + _pow9r; _add10i = _pow6i + _pow9i;
        cRe[_si_idx] = _add10r; cIm[_si_idx] = _add10i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 10;
        double _re11r = 0, _re11i = 0;
        _re11r = x2r; _re11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_re11r, _re11i, _re11r, _re11i, &_pow13r, &_pow13i);
        double _im14r = 0, _im14i = 0;
        _im14r = x1i; _im14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(_im14r, _im14i, _im14r, _im14i, &_pow16r, &_pow16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _pow13r + _pow16r; _add17i = _pow13i + _pow16i;
        cRe[_si_idx] = _add17r; cIm[_si_idx] = _add17i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 20;
        double _mul18r = 0, _mul18i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(_mul18r, _mul18i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_mul(_abs19r, _abs19i, _abs19r, _abs19i, &_pow21r, &_pow21i);
        cRe[_si_idx] = _pow21r; cIm[_si_idx] = _pow21i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 30;
        double _add22r = 0, _add22i = 0;
        _add22r = x1r + x2r; _add22i = x1i + x2i;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_add22r, _add22i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_mul(_abs23r, _abs23i, _abs23r, _abs23i, &_pow25r, &_pow25i);
        cRe[_si_idx] = _pow25r; cIm[_si_idx] = _pow25i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 40;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x2r, x2i); _abs27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_abs26r, _abs26i, _abs27r, _abs27i, &_mul28r, &_mul28i);
        cRe[_si_idx] = _mul28r; cIm[_si_idx] = _mul28i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 50;
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _ang29r + _ang30r; _add31i = _ang29i + _ang30i;
        cRe[_si_idx] = _add31r; cIm[_si_idx] = _add31i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 60;
        double _add32r = 0, _add32i = 0;
        _add32r = x1r + x2r; _add32i = x1i + x2i;
        double _sin33r = 0, _sin33i = 0;
        c_sin(_add32r, _add32i, &_sin33r, &_sin33i);
        cRe[_si_idx] = _sin33r; cIm[_si_idx] = _sin33i;
    }
    double _sub34r = 0, _sub34i = 0;
    _sub34r = x1r - x2r; _sub34i = x1i - x2i;
    double _cos35r = 0, _cos35i = 0;
    c_cos(_sub34r, _sub34i, &_cos35r, &_cos35i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _cos35r; cIm[_idx] = _cos35i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_100: too complex for auto-transpile, stubbed */
static void poly_100_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

