/* AUTO-GENERATED from poly800.py — do not edit manually */
/* 108 coefficient functions */

static void poly_701_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 0; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _sqrt11r = 0, _sqrt11i = 0;
        c_powr(_add10r, _add10i, 0.5, &_sqrt11r, &_sqrt11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr8r, _attr8i, _sqrt11r, _sqrt11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _mul12r; _add13i = _mul7i + _mul12i;
        double r_part = _add13r; /* +_add13ii */
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 4.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_attr14r, _attr14i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_attr20r, _attr20i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul19r + _mul25r; _add26i = _mul19i + _mul25i;
        double i_part = _add26r; /* +_add26ii */
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x1r, x1i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 3.0; _c28i = 0;
        double _mod29r = 0, _mod29i = 0;
        _mod29r = fmod(j, _c28r); _mod29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _mod29r + _c30r; _add31i = _mod29i + _c30i;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_abs27r, _abs27i, _add31r, &_pow32r, &_pow32i);
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(x2r, x2i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 2.0; _c34i = 0;
        double _mod35r = 0, _mod35i = 0;
        _mod35r = fmod(j, _c34r); _mod35i = 0;
        double _sub36r = 0, _sub36i = 0;
        _sub36r = degree - _mod35r; _sub36i = 0 - _mod35i;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _sub36r + _c37r; _add38i = _sub36i + _c37i;
        double _pow39r = 0, _pow39i = 0;
        c_powr(_abs33r, _abs33i, _add38r, &_pow39r, &_pow39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _pow32r + _pow39r; _add40i = _pow32i + _pow39i;
        double magnitude = _add40r; /* +_add40ii */
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x1r, x1i); _ang41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang41r, _ang41i, j, 0, &_mul42r, &_mul42i);
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x2r, x2i); _ang43i = 0;
        double _sub44r = 0, _sub44i = 0;
        _sub44r = degree - j; _sub44i = 0 - 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_ang43r, _ang43i, _sub44r, _sub44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul42r + _mul45r; _add46i = _mul42i + _mul45i;
        double angle = _add46r; /* +_add46ii */
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c47r, _c47i, i_part, 0, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = r_part + _mul48r; _add49i = 0 + _mul48i;
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c50r, _c50i, angle, 0, &_mul51r, &_mul51i);
        double _exp52r = 0, _exp52i = 0;
        c_exp2(_mul51r, _mul51i, &_exp52r, &_exp52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_add49r, _add49i, _exp52r, _exp52i, &_mul53r, &_mul53i);
        double _abs54r = 0, _abs54i = 0;
        _abs54r = c_abs(magnitude, 0); _abs54i = 0;
        double _c55r = 0, _c55i = 0;
        _c55r = 1.0; _c55i = 0;
        double _add56r = 0, _add56i = 0;
        _add56r = _abs54r + _c55r; _add56i = _abs54i + _c55i;
        double _log57r = 0, _log57i = 0;
        c_log(_add56r, _add56i, &_log57r, &_log57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _mul53r + _log57r; _add58i = _mul53i + _log57i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_702_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    for (int j = 1; j < 10; j++) {
        double _arr5r = 0, _arr5i = 0;
        { int _idx = (j - 1); _arr5r = (_idx >= 0 && _idx < 9) ? rec[_idx] : 0.0; _arr5i = 0; }
        double _arr6r = 0, _arr6i = 0;
        { int _idx = (j - 1); _arr6r = (_idx >= 0 && _idx < 9) ? imc[_idx] : 0.0; _arr6i = 0; }
        double _add7r = 0, _add7i = 0;
        _add7r = _arr5r + _arr6r; _add7i = _arr5i + _arr6i;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_add7r, _add7i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 4.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log11r, _log11i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 6.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul16r + _cos20r; _add21i = _mul16i + _cos20i;
        double magnitude = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, M_PI, 0, &_mul24r, &_mul24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul24r, _mul24i, j, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 9.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang22r, _ang22i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 4.0; _c31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, M_PI, 0, &_mul32r, &_mul32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_mul32r, _mul32i, j, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 9.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang30r, _ang30i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul29r + _mul37r; _add38i = _mul29i + _mul37i;
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
        c_mul(magnitude, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_703_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x2r, x2i); _abs2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs1r + _abs2r; _add3i = _abs1i + _abs2i;
        double _add4r = 0, _add4i = 0;
        _add4r = _add3r + j; _add4i = _add3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_mul(j, 0, j, 0, &_pow7r, &_pow7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _pow7r + _sin10r; _add11i = _pow7i + _sin10i;
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _cos13r = 0, _cos13i = 0;
        c_cos(_attr12r, _attr12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add11r + _cos13r; _add14i = _add11i + _cos13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log5r, _log5i, _add14r, _add14i, &_mul15r, &_mul15i);
        double mag = _mul15r; /* +_mul15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(j, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _cos20r = 0, _cos20i = 0;
        c_cos(j, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
        double angle = _add22r; /* +_add22ii */
        double _cos23r = 0, _cos23i = 0;
        c_cos(angle, 0, &_cos23r, &_cos23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(angle, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c24r, _c24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _cos23r + _mul26r; _add27i = _cos23i + _mul26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag, 0, _add27r, _add27i, &_mul28r, &_mul28i);
        double _conj29r = 0, _conj29i = 0;
        _conj29r = x1r; _conj29i = -(x1i);
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2r; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_conj29r, _conj29i, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_mul31r, _mul31i, j, 0, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul28r + _mul32r; _add33i = _mul28i + _mul32i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_704_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 0; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        double r = _add4r; /* +_add4ii */
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_abs5r, _abs5i, j, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mul6r + _c7r; _add8i = _mul6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _sub11r = 0, _sub11i = 0;
        _sub11r = degree - j; _sub11i = 0 - 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _sub11r + _c12r; _add13i = _sub11i + _c12i;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs10r, _abs10i, _add13r, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log9r, _log9i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_mul(_sin18r, _sin18i, _sin18r, _sin18i, &_pow20r, &_pow20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul15r + _pow20r; _add21i = _mul15i + _pow20i;
        double mag = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(j, 0, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _cos26r = 0, _cos26i = 0;
        c_cos(j, 0, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul24r + _mul27r; _add28i = _mul24i + _mul27i;
        double ang = _add28r; /* +_add28ii */
        double _cos29r = 0, _cos29i = 0;
        c_cos(ang, 0, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag, 0, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x1r; _attr31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 3.0; _c32i = 0;
        double _mod33r = 0, _mod33i = 0;
        _mod33r = fmod(j, _c32r); _mod33i = 0;
        double _pow34r = 0, _pow34i = 0;
        c_powr(_attr31r, _attr31i, _mod33r, &_pow34r, &_pow34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul30r + _pow34r; _add35i = _mul30i + _pow34i;
        double real_part = _add35r; /* +_add35ii */
        double _sin36r = 0, _sin36i = 0;
        c_sin(ang, 0, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(mag, 0, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x2i; _attr38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 2.0; _c39i = 0;
        double _mod40r = 0, _mod40i = 0;
        _mod40r = fmod(j, _c39r); _mod40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _mod40r + _c41r; _add42i = _mod40i + _c41i;
        double _pow43r = 0, _pow43i = 0;
        c_powr(_attr38r, _attr38i, _add42r, &_pow43r, &_pow43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul37r + _pow43r; _add44i = _mul37i + _pow43i;
        double imag_part = _add44r; /* +_add44ii */
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, ang, 0, &_mul46r, &_mul46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(mag, 0, _exp47r, _exp47i, &_mul48r, &_mul48i);
        double _conj49r = 0, _conj49i = 0;
        _conj49r = x1r; _conj49i = -(x1i);
        double _pow50r = 0, _pow50i = 0;
        c_powr(_conj49r, _conj49i, j, &_pow50r, &_pow50i);
        double _re51r = 0, _re51i = 0;
        _re51r = x2r; _re51i = 0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(j, 0, _re51r, _re51i, &_mul52r, &_mul52i);
        double _cos53r = 0, _cos53i = 0;
        c_cos(_mul52r, _mul52i, &_cos53r, &_cos53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_pow50r, _pow50i, _cos53r, _cos53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _mul48r + _mul54r; _add55i = _mul48i + _mul54i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_705_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x2r, x2i); _abs2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs1r + _abs2r; _add3i = _abs1i + _abs2i;
        double _add4r = 0, _add4i = 0;
        _add4r = _add3r + j; _add4i = _add3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1r; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2i; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin9r, _sin9i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _c6r + _mul13r; _add14i = _c6i + _mul13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log5r, _log5i, _add14r, _add14i, &_mul15r, &_mul15i);
        double mag_variation = _mul15r; /* +_mul15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _sqrt17r = 0, _sqrt17i = 0;
        c_powr(j, 0, 0.5, &_sqrt17r, &_sqrt17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sqrt17r, _sqrt17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_ang19r, _ang19i, _add21r, _add21i, &_div22r, &_div22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _mul18r - _div22r; _sub23i = _mul18i - _div22i;
        double angle_variation = _sub23r; /* +_sub23ii */
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, angle_variation, 0, &_mul25r, &_mul25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag_variation, 0, _exp26r, _exp26i, &_mul27r, &_mul27i);
        double _conj28r = 0, _conj28i = 0;
        _conj28r = x1r; _conj28i = -(x1i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(j, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_conj28r, _conj28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul27r + _mul30r; _add31i = _mul27i + _mul30i;
        double _conj32r = 0, _conj32i = 0;
        _conj32r = x2r; _conj32i = -(x2i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(j, 0, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_conj32r, _conj32i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _add31r - _mul34r; _sub35i = _add31i - _mul34i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _sub35r; cIm[_idx] = _sub35i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_706_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
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
            double _attr5r = 0, _attr5i = 0;
            _attr5r = x1r; _attr5i = 0;
            double _add6r = 0, _add6i = 0;
            _add6r = _attr5r + k; _add6i = _attr5i + 0;
            double _abs7r = 0, _abs7i = 0;
            _abs7r = c_abs(_add6r, _add6i); _abs7i = 0;
            double _log8r = 0, _log8i = 0;
            c_log(_abs7r, _abs7i, &_log8r, &_log8i);
            double _attr9r = 0, _attr9i = 0;
            _attr9r = x2r; _attr9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_log8r, _log8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            mag += _mul12r;
            double _attr13r = 0, _attr13i = 0;
            _attr13r = x1i; _attr13i = 0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(k, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
            double _cos15r = 0, _cos15i = 0;
            c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
            double _ang16r = 0, _ang16i = 0;
            _ang16r = c_arg(x2r, x2i); _ang16i = 0;
            double _pow17r = 0, _pow17i = 0;
            c_powr(_ang16r, _ang16i, k, &_pow17r, &_pow17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(_cos15r, _cos15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
            angle += _mul18r;
        }
        double _cos19r = 0, _cos19i = 0;
        c_cos(angle, 0, &_cos19r, &_cos19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _sin21r = 0, _sin21i = 0;
        c_sin(angle, 0, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c20r, _c20i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _cos19r + _mul22r; _add23i = _cos19i + _mul22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(mag, 0, _add23r, _add23i, &_mul24r, &_mul24i);
        double _conj25r = 0, _conj25i = 0;
        _conj25r = x1r; _conj25i = -(x1i);
        double _pow26r = 0, _pow26i = 0;
        c_powr(_conj25r, _conj25i, j, &_pow26r, &_pow26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul24r + _pow26r; _add27i = _mul24i + _pow26i;
        double _conj28r = 0, _conj28i = 0;
        _conj28r = x2r; _conj28i = -(x2i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(_conj28r, _conj28i, j, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add27r + _pow29r; _add30i = _add27i + _pow29i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_707_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    for (int j = 1; j < 10; j++) {
        double _c5r = 0, _c5i = 0;
        _c5r = 5.0; _c5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_c5r, _c5i, M_PI, 0, &_mul6r, &_mul6i);
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 9) ? imc[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(_mul6r, _mul6i, _arr7r, _arr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _exp10r = 0, _exp10i = 0;
        c_exp2(_sin9r, _sin9i, &_exp10r, &_exp10i);
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 9) ? rec[_idx] : 0.0; _arr11i = 0; }
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_arr11r, _arr11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, M_PI, 0, &_mul17r, &_mul17i);
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 9) ? rec[_idx] : 0.0; _arr18i = 0; }
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul17r, _mul17i, _arr18r, _arr18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log15r, _log15i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _exp10r + _mul21r; _add22i = _exp10i + _mul21i;
        double mag = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, M_PI, 0, &_mul25r, &_mul25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul25r, _mul25i, j, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 9.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang23r, _ang23i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 4.0; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, M_PI, 0, &_mul33r, &_mul33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_mul33r, _mul33i, j, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 9.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang31r, _ang31i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul30r + _mul38r; _add39i = _mul30i + _mul38i;
        double ang = _add39r; /* +_add39ii */
        double _cos40r = 0, _cos40i = 0;
        c_cos(ang, 0, &_cos40r, &_cos40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _sin42r = 0, _sin42i = 0;
        c_sin(ang, 0, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c41r, _c41i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _cos40r + _mul43r; _add44i = _cos40i + _mul43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag, 0, _add44r, _add44i, &_mul45r, &_mul45i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_708_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_mul(j, 0, j, 0, &_pow2r, &_pow2i);
        double k = _pow2r; /* +_pow2ii */
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x2r, x2i); _ang4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, _ang4r, _ang4i, &_mul5r, &_mul5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_mul5r, _mul5i, &_cos6r, &_cos6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _cos6r, _cos6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(k, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _sin10r; _add11i = _mul7i + _sin10i;
        double r = _add11r; /* +_add11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = k + _abs13r; _add14i = 0 + _abs13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_attr12r, _attr12i, _log15r, _log15i, &_mul16r, &_mul16i);
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x1r; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, _attr17r, _attr17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul16r + _cos19r; _add20i = _mul16i + _cos19i;
        double im = _add20r; /* +_add20ii */
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_mul(r, 0, r, 0, &_pow22r, &_pow22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(im, 0, im, 0, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _pow22r + _pow24r; _add25i = _pow22i + _pow24i;
        double _sqrt26r = 0, _sqrt26i = 0;
        c_powr(_add25r, _add25i, 0.5, &_sqrt26r, &_sqrt26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _c27r + j; _add28i = _c27i + 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sqrt26r, _sqrt26i, _add28r, _add28i, &_mul29r, &_mul29i);
        double mag = _mul29r; /* +_mul29ii */
        double _at230r = 0, _at230i = 0;
        _at230r = atan2(im, r); _at230i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = x1r + x2r; _add31i = x1i + x2i;
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(_add31r, _add31i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, _ang32r, _ang32i, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _at230r + _sin34r; _add35i = _at230i + _sin34i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_709_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 0; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _abs4r; _add5i = _abs3i + _abs4i;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _add5r + _c6r; _add7i = _add5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _pow14r = 0, _pow14i = 0;
        c_powr(_add10r, _add10i, _sin13r, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log8r, _log8i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _cos18r; _add19i = _mul15i + _cos18i;
        double mag_part = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _sin21r = 0, _sin21i = 0;
        c_sin(j, 0, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang20r, _ang20i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _cos24r = 0, _cos24i = 0;
        c_cos(j, 0, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul22r + _mul25r; _add26i = _mul22i + _mul25i;
        double angle_part = _add26r; /* +_add26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, angle_part, 0, &_mul28r, &_mul28i);
        double _exp29r = 0, _exp29i = 0;
        c_exp2(_mul28r, _mul28i, &_exp29r, &_exp29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag_part, 0, _exp29r, _exp29i, &_mul30r, &_mul30i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    double _c31r = 0, _c31i = 0;
    _c31r = 1.0; _c31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = degree + _c31r; _add32i = 0 + _c31i;
    for (int k = 0; k < (int)(_add32r); k++) {
        double _cf33r = 0, _cf33i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 0.5; _c35i = 0;
        double _attr36r = 0, _attr36i = 0;
        _attr36r = x1i; _attr36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(k, 0, _attr36r, _attr36i, &_mul37r, &_mul37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_mul37r, _mul37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c35r, _c35i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _c34r + _mul39r; _add40i = _c34i + _mul39i;
        double _c41r = 0, _c41i = 0;
        _c41r = 0.3; _c41i = 0;
        double _attr42r = 0, _attr42i = 0;
        _attr42r = x2i; _attr42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(k, 0, _attr42r, _attr42i, &_mul43r, &_mul43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_mul43r, _mul43i, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c41r, _c41i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _sub46r = 0, _sub46i = 0;
        _sub46r = _add40r - _mul45r; _sub46i = _add40i - _mul45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_cf33r, _cf33i, _sub46r, _sub46i, &_mul47r, &_mul47i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_710_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double sum_part = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        for (int k = 1; k < (int)(_add5r); k++) {
            double _attr6r = 0, _attr6i = 0;
            _attr6r = x1r; _attr6i = 0;
            double _mul7r = 0, _mul7i = 0;
            c_mul(k, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
            double _cos8r = 0, _cos8i = 0;
            c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
            double _attr9r = 0, _attr9i = 0;
            _attr9r = x2i; _attr9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_cos8r, _cos8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            sum_part += _mul12r;
        }
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _abs14r; _add15i = _abs13i + _abs14i;
        double _add16r = 0, _add16i = 0;
        _add16r = _add15r + sum_part; _add16i = _add15i + 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _add16r + _c17r; _add18i = _add16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double mag = _log19r; /* +_log19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 0.5; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_ang20r, _ang20i, 0.5, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_pow22r, _pow22i, j, 0, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 0.3; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_ang24r, _ang24i, 0.3, &_pow26r, &_pow26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = degree - j; _sub27i = 0 - 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _sub27r + _c28r; _add29i = _sub27i + _c28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow26r, _pow26i, _add29r, _add29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul23r + _mul30r; _add31i = _mul23i + _mul30i;
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
        c_mul(mag, 0, _add36r, _add36i, &_mul37r, &_mul37i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_711_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 8;
    for (int _i = 0; _i < 8; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 9; k++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_attr1r, _attr1i, k, &_pow2r, &_pow2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x2r, x2i); _abs3i = 0;
        double _log4r = 0, _log4i = 0;
        c_log(_abs3r, _abs3i, &_log4r, &_log4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(_log4r, _log4i, _log4r, _log4i, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double r_part = _add7r; /* +_add7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(k, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _cos10r + _sin13r; _add14i = _cos10i + _sin13i;
        double i_part = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang15r, _ang15i, k, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(k, 0, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sin19r, _sin19i, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _div18r + _div22r; _add23i = _div18i + _div22i;
        double angle = _add23r; /* +_add23ii */
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, i_part, 0, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = r_part + _mul25r; _add26i = 0 + _mul25i;
        double _cos27r = 0, _cos27i = 0;
        c_cos(angle, 0, &_cos27r, &_cos27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(angle, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c28r, _c28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _cos27r + _mul30r; _add31i = _cos27i + _mul30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_add26r, _add26i, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 8) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 8; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_712_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 3.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 4.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin10r, _sin10i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double angle_component = _mul15r; /* +_mul15ii */
        double _arr16r = 0, _arr16i = 0;
        { int _idx = (j - 1); _arr16r = (_idx >= 0 && _idx < 9) ? rec[_idx] : 0.0; _arr16i = 0; }
        double _arr17r = 0, _arr17i = 0;
        { int _idx = (j - 1); _arr17r = (_idx >= 0 && _idx < 9) ? imc[_idx] : 0.0; _arr17i = 0; }
        double _mul18r = 0, _mul18i = 0;
        c_mul(_arr16r, _arr16i, _arr17r, _arr17i, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(_mul18r, _mul18i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_attr23r, _attr23i, j, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _log22r + _pow24r; _add25i = _log22i + _pow24i;
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x2i; _attr26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = degree + _c27r; _add28i = 0 + _c27i;
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _add28r - j; _sub29i = _add28i - 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(_attr26r, _attr26i, _sub29r, &_pow30r, &_pow30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _add25r - _pow30r; _sub31i = _add25i - _pow30i;
        double magnitude_component = _sub31r; /* +_sub31ii */
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x1r, x1i); _ang33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang33r, _ang33i, j, 0, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = angle_component + _mul34r; _add35i = 0 + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c32r, _c32i, _add35r, _add35i, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(magnitude_component, 0, _exp37r, _exp37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_713_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 8;
    for (int _i = 0; _i < 8; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 9; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _c1r, _c1i, &_mul2r, &_mul2i);
        double k = _mul2r; /* +_mul2ii */
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_attr3r, _attr3i, j, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log8r, _log8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _pow4r + _mul12r; _add13i = _pow4i + _mul12i;
        double r = _add13r; /* +_add13ii */
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x2i; _attr14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_attr14r, _attr14i, k, &_pow15r, &_pow15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _ang20r, _ang20i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log19r, _log19i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _pow15r - _mul23r; _sub24i = _pow15i - _mul23i;
        double i_part = _sub24r; /* +_sub24ii */
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x2r, x2i); _abs26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_abs25r, _abs25i, _abs26r, _abs26i, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _mul27r + _c28r; _add29i = _mul27i + _c28i;
        double _log30r = 0, _log30i = 0;
        c_log(_add29r, _add29i, &_log30r, &_log30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log30r, _log30i, j, 0, &_mul31r, &_mul31i);
        double mag = _mul31r; /* +_mul31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang33r, _ang33i, j, 0, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _ang32r + _mul34r; _add35i = _ang32i + _mul34i;
        double _sin36r = 0, _sin36i = 0;
        c_sin(j, 0, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add35r + _sin36r; _add37i = _add35i + _sin36i;
        double _cos38r = 0, _cos38i = 0;
        c_cos(k, 0, &_cos38r, &_cos38i);
        double _sub39r = 0, _sub39i = 0;
        _sub39r = _add37r - _cos38r; _sub39i = _add37i - _cos38i;
        double ang = _sub39r; /* +_sub39ii */
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, i_part, 0, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = r + _mul41r; _add42i = 0 + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_add42r, _add42i, mag, 0, &_mul43r, &_mul43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(ang, 0, &_cos44r, &_cos44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _sin46r = 0, _sin46i = 0;
        c_sin(ang, 0, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c45r, _c45i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _cos44r + _mul47r; _add48i = _cos44i + _mul47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_mul43r, _mul43i, _add48r, _add48i, &_mul49r, &_mul49i);
        double _conj50r = 0, _conj50i = 0;
        _conj50r = x1r; _conj50i = -(x1i);
        double _pow51r = 0, _pow51i = 0;
        c_powr(_conj50r, _conj50i, j, &_pow51r, &_pow51i);
        double _conj52r = 0, _conj52i = 0;
        _conj52r = x2r; _conj52i = -(x2i);
        double _pow53r = 0, _pow53i = 0;
        c_powr(_conj52r, _conj52i, k, &_pow53r, &_pow53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_pow51r, _pow51i, _pow53r, _pow53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _mul49r + _mul54r; _add55i = _mul49i + _mul54i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 8) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    }
    for (int _i = 0; _i < 8; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_714_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(j, 0, j, 0, &_pow3r, &_pow3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(_attr1r, _attr1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2r; _attr5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _div8r = 0, _div8i = 0;
        c_div(_attr5r, _attr5i, _add7r, _add7i, &_div8r, &_div8i);
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _mul4r - _div8r; _sub9i = _mul4i - _div8i;
        double r = _sub9r; /* +_sub9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2i; _attr11i = 0;
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr11r, _attr11i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _attr10r + _mul13r; _add14i = _attr10i + _mul13i;
        double im = _add14r; /* +_add14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _abs16r; _add17i = _abs15i + _abs16i;
        double _add18r = 0, _add18i = 0;
        _add18r = _add17r + j; _add18i = _add17i + 0;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_sin24r, _sin24i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _c20r + _abs25r; _add26i = _c20i + _abs25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log19r, _log19i, _add26r, _add26i, &_mul27r, &_mul27i);
        double mag = _mul27r; /* +_mul27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _cos29r = 0, _cos29i = 0;
        c_cos(j, 0, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 4.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang31r, _ang31i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul30r + _mul36r; _add37i = _mul30i + _mul36i;
        double ang = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, ang, 0, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(mag, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
        double _conj42r = 0, _conj42i = 0;
        _conj42r = x1r; _conj42i = -(x1i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(j, 0, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_conj42r, _conj42i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul41r + _mul44r; _add45i = _mul41i + _mul44i;
        double _conj46r = 0, _conj46i = 0;
        _conj46r = x2r; _conj46i = -(x2i);
        double _cos47r = 0, _cos47i = 0;
        c_cos(j, 0, &_cos47r, &_cos47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_conj46r, _conj46i, _cos47r, _cos47i, &_mul48r, &_mul48i);
        double _sub49r = 0, _sub49i = 0;
        _sub49r = _add45r - _mul48r; _sub49i = _add45i - _mul48i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _sub49r; cIm[_idx] = _sub49i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_715_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _attr2r = 0, _attr2i = 0;
        _attr2r = x2r; _attr2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(j, 0, _attr2r, _attr2i, &_mul3r, &_mul3i);
        double _add4r = 0, _add4i = 0;
        _add4r = _attr1r + _mul3r; _add4i = _attr1i + _mul3i;
        double rj = _add4r; /* +_add4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1i; _attr5i = 0;
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sub8r = 0, _sub8i = 0;
        _sub8r = _attr5r - _mul7r; _sub8i = _attr5i - _mul7i;
        double ij = _sub8r; /* +_sub8ii */
        double _add9r = 0, _add9i = 0;
        _add9r = rj + ij; _add9i = 0 + 0;
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
        _c16r = 4.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _c14r + _sin18r; _add19i = _c14i + _sin18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log13r, _log13i, _add19r, _add19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.5; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(j, 0, 1.5, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul20r, _mul20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul28r + _mul33r; _add34i = _mul28i + _mul33i;
        double ang = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, ang, 0, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(mag, 0, _exp37r, _exp37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_716_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x1r, x1i); _ang4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _ang4r, _ang4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _sin6r + _cos9r; _add10i = _sin6i + _cos9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log3r, _log3i, _add10r, _add10i, &_mul11r, &_mul11i);
        double mag = _mul11r; /* +_mul11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _sqrt13r = 0, _sqrt13i = 0;
        c_powr(j, 0, 0.5, &_sqrt13r, &_sqrt13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _sqrt13r, _sqrt13i, &_mul14r, &_mul14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = j + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_ang15r, _ang15i, _add17r, _add17i, &_div18r, &_div18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul14r + _div18r; _add19i = _mul14i + _div18i;
        double angle = _add19r; /* +_add19ii */
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c20r, _c20i, angle, 0, &_mul21r, &_mul21i);
        double _exp22r = 0, _exp22i = 0;
        c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(mag, 0, _exp22r, _exp22i, &_mul23r, &_mul23i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_717_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_abs1r, _abs1i, j, &_pow2r, &_pow2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x2r, x2i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _div5r = 0, _div5i = 0;
        c_div(j, 0, _c4r, _c4i, &_div5r, &_div5i);
        double _pow6r = 0, _pow6i = 0;
        c_powr(_abs3r, _abs3i, _div5r, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1r; _attr9i = 0;
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2i; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr9r, _attr9i, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _log8r + _mul11r; _add12i = _log8i + _mul11i;
        double mag = _add12r; /* +_add12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _cos14r = 0, _cos14i = 0;
        c_cos(j, 0, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang13r, _ang13i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(j, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _mul18r; _add19i = _mul15i + _mul18i;
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x2i; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_sin22r, _sin22i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add19r + _mul26r; _add27i = _add19i + _mul26i;
        double angle = _add27r; /* +_add27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c28r, _c28i, angle, 0, &_mul29r, &_mul29i);
        double _exp30r = 0, _exp30i = 0;
        c_exp2(_mul29r, _mul29i, &_exp30r, &_exp30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag, 0, _exp30r, _exp30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = x1r + x2r; _add32i = x1i + x2i;
        double _conj33r = 0, _conj33i = 0;
        _conj33r = _add32r; _conj33i = -(_add32i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_conj33r, _conj33i, _add35r, _add35i, &_div36r, &_div36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul31r + _div36r; _add37i = _mul31i + _div36i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_718_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double k = _add2r; /* +_add2ii */
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _cos4r = 0, _cos4i = 0;
        c_cos(j, 0, &_cos4r, &_cos4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr3r, _attr3i, _cos4r, _cos4i, &_mul5r, &_mul5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double _sin7r = 0, _sin7i = 0;
        c_sin(j, 0, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr6r, _attr6i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _mul5r - _mul8r; _sub9i = _mul5i - _mul8i;
        double r = _sub9r; /* +_sub9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2r; _attr10i = 0;
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr10r, _attr10i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _cos14r = 0, _cos14i = 0;
        c_cos(j, 0, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr13r, _attr13i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _mul15r; _add16i = _mul12i + _mul15i;
        double im = _add16r; /* +_add16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _abs18r; _add19i = _abs17i + _abs18i;
        double _add20r = 0, _add20i = 0;
        _add20r = _add19r + j; _add20i = _add19i + 0;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(k, 0, &_sin23r, &_sin23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _c22r + _sin23r; _add24i = _c22i + _sin23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log21r, _log21i, _add24r, _add24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(k, 0, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul25r + _cos26r; _add27i = _mul25i + _cos26i;
        double magnitude = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(k, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _cos32r = 0, _cos32i = 0;
        c_cos(k, 0, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul30r + _mul33r; _add34i = _mul30i + _mul33i;
        double angle = _add34r; /* +_add34ii */
        double _cos35r = 0, _cos35i = 0;
        c_cos(angle, 0, &_cos35r, &_cos35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _sin37r = 0, _sin37i = 0;
        c_sin(angle, 0, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c36r, _c36i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _cos35r + _mul38r; _add39i = _cos35i + _mul38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(magnitude, 0, _add39r, _add39i, &_mul40r, &_mul40i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_719_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 0; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        double k = _add4r; /* +_add4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_attr5r, _attr5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2i; _attr10i = 0;
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr10r, _attr10i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul9r + _mul14r; _add15i = _mul9i + _mul14i;
        double r = _add15r; /* +_add15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _abs17r; _add18i = _abs16i + _abs17i;
        double _add19r = 0, _add19i = 0;
        _add19r = _add18r + j; _add19i = _add18i + 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _add19r + _c20r; _add21i = _add19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = j + _c23r; _add24i = 0 + _c23i;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(_add24r, _add24i, _add24r, _add24i, &_pow26r, &_pow26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log22r, _log22i, _pow26r, _pow26i, &_mul27r, &_mul27i);
        double mag = _mul27r; /* +_mul27ii */
        double _sin28r = 0, _sin28i = 0;
        c_sin(r, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(k, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 4.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _sin28r + _cos32r; _add33i = _sin28i + _cos32i;
        double angle = _add33r; /* +_add33ii */
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
        c_mul(mag, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_720_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int k = 0; k < (int)(_add2r); k++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs3r, _abs3i, k, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = degree - k; _sub6i = 0 - 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_abs5r, _abs5i, _sub6r, &_pow7r, &_pow7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_pow4r, _pow4i, _pow7r, _pow7i, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _abs10r; _add11i = _abs9i + _abs10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _add11r + _c12r; _add13i = _add11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul8r + _log14r; _add15i = _mul8i + _log14i;
        double mag_part = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(k, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _cos20r = 0, _cos20i = 0;
        c_cos(k, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
        double angle_part = _add22r; /* +_add22ii */
        double _mul23r = 0, _mul23i = 0;
        c_mul(k, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(k, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 4.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _sin26r + _cos30r; _add31i = _sin26i + _cos30i;
        double variation = _add31r; /* +_add31ii */
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag_part, 0, variation, 0, &_mul32r, &_mul32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(angle_part, 0, &_cos33r, &_cos33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _sin35r = 0, _sin35i = 0;
        c_sin(angle_part, 0, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c34r, _c34i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _cos33r + _mul36r; _add37i = _cos33i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_mul32r, _mul32i, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_721_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _attr3r, _attr3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _sin5r + _cos8r; _add9i = _sin5i + _cos8i;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_ang10r, _ang10i, j, 0, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _add9r + _mul11r; _add12i = _add9i + _mul11i;
        double angle = _add12r; /* +_add12ii */
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_abs16r, _abs16i, j, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log15r, _log15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = degree + _c20r; _add21i = 0 + _c20i;
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _add21r - j; _sub22i = _add21i - 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs19r, _abs19i, _sub22r, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul18r + _pow23r; _add24i = _mul18i + _pow23i;
        double mag = _add24r; /* +_add24ii */
        double _cos25r = 0, _cos25i = 0;
        c_cos(angle, 0, &_cos25r, &_cos25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(angle, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c26r, _c26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos25r + _mul28r; _add29i = _cos25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    double _c31r = 0, _c31i = 0;
    _c31r = 2.0; _c31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = degree + _c31r; _add32i = 0 + _c31i;
    for (int k = 1; k < (int)(_add32r); k++) {
        double _cf33r = 0, _cf33i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
        double _conj34r = 0, _conj34i = 0;
        _conj34r = x1r; _conj34i = -(x1i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_cf33r, _cf33i, _conj34r, _conj34i, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _c36r + k; _add37i = _c36i + 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul35r, _mul35i, _add37r, _add37i, &_div38r, &_div38i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div38r; cIm[_idx] = _div38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_722_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_abs1r, _abs1i, j, &_pow2r, &_pow2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x2r, x2i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 9.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_abs3r, _abs3i, _sub5r, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _add7r + _c8r; _add9i = _add7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double mag_part = _log10r; /* +_log10ii */
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 9.0; _c14i = 0;
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _c14r - j; _sub15i = _c14i - 0;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_sub15r, _sub15i, _ang16r, _ang16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _sin13r + _cos18r; _add19i = _sin13i + _cos18i;
        double angle_part = _add19r; /* +_add19ii */
        double _cos20r = 0, _cos20i = 0;
        c_cos(angle_part, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(mag_part, 0, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x1r; _attr22i = 0;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_abs23r, _abs23i, j, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_attr22r, _attr22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul21r + _mul25r; _add26i = _mul21i + _mul25i;
        double real_part = _add26r; /* +_add26ii */
        double _sin27r = 0, _sin27i = 0;
        c_sin(angle_part, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_part, 0, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(_abs30r, _abs30i, j, &_pow31r, &_pow31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_attr29r, _attr29i, _pow31r, _pow31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul28r + _mul32r; _add33i = _mul28i + _mul32i;
        double imag_part = _add33r; /* +_add33ii */
        double _cplx34r = 0, _cplx34i = 0;
        _cplx34r = real_part; _cplx34i = imag_part;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _cplx34r; cIm[_idx] = _cplx34i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_723_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x2r, x2i); _ang4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _ang4r, _ang4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _cos10r; _add11i = _mul7i + _cos10i;
        double mag_part = _add11r; /* +_add11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_ang12r, _ang12i, j, &_pow13r, &_pow13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x2r, x2i); _ang14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _mod16r = 0, _mod16i = 0;
        _mod16r = fmod(j, _c15r); _mod16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_ang14r, _ang14i, _mod16r, &_pow17r, &_pow17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = _pow13r - _pow17r; _sub18i = _pow13i - _pow17i;
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x1r; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _attr19r, _attr19i, &_mul20r, &_mul20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_mul20r, _mul20i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _sub18r + _sin21r; _add22i = _sub18i + _sin21i;
        double angle_part = _add22r; /* +_add22ii */
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, angle_part, 0, &_mul24r, &_mul24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag_part, 0, _exp25r, _exp25i, &_mul26r, &_mul26i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_724_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs3r, _abs3i, j, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_pow4r, _pow4i, _log8r, _log8i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _sub11r = 0, _sub11i = 0;
        _sub11r = degree - j; _sub11i = 0 - 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _sub11r + _c12r; _add13i = _sub11i + _c12i;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs10r, _abs10i, _add13r, &_pow14r, &_pow14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1r; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_pow14r, _pow14i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul9r + _mul18r; _add19i = _mul9i + _mul18i;
        double mag_part = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2r; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x1r; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul24r + _mul29r; _add30i = _mul24i + _mul29i;
        double angle_part = _add30r; /* +_add30ii */
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, angle_part, 0, &_mul32r, &_mul32i);
        double _exp33r = 0, _exp33i = 0;
        c_exp2(_mul32r, _mul32i, &_exp33r, &_exp33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(mag_part, 0, _exp33r, _exp33i, &_mul34r, &_mul34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul35r, &_mul35i);
        double _conj36r = 0, _conj36i = 0;
        _conj36r = _mul35r; _conj36i = -(_mul35i);
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = j + _c37r; _add38i = 0 + _c37i;
        double _div39r = 0, _div39i = 0;
        c_div(_conj36r, _conj36i, _add38r, _add38i, &_div39r, &_div39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul34r + _div39r; _add40i = _mul34i + _div39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_725_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 5.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mod2r + _c3r; _add4i = _mod2i + _c3i;
        double k = _add4r; /* +_add4ii */
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs5r + j; _add6i = _abs5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _c8r + _sin11r; _add12i = _c8i + _sin11i;
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(k, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _add12r + _cos15r; _add16i = _add12i + _cos15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log7r, _log7i, _add16r, _add16i, &_mul17r, &_mul17i);
        double r = _mul17r; /* +_mul17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(j, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(k, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang21r, _ang21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
        double angle = _add24r; /* +_add24ii */
        double _cos25r = 0, _cos25i = 0;
        c_cos(angle, 0, &_cos25r, &_cos25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(angle, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c26r, _c26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos25r + _mul28r; _add29i = _cos25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(r, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_726_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 10; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(k, 0, k, 0, &_pow3r, &_pow3i);
        double _add4r = 0, _add4i = 0;
        _add4r = _abs1r + _pow3r; _add4i = _abs1i + _pow3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(k, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul9r + _cos12r; _add13i = _mul9i + _cos12i;
        double mag = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang14r, _ang14i, _log18r, _log18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul19r + _sin22r; _add23i = _mul19i + _sin22i;
        double angle = _add23r; /* +_add23ii */
        double _cos24r = 0, _cos24i = 0;
        c_cos(angle, 0, &_cos24r, &_cos24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(angle, 0, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_c25r, _c25i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _cos24r + _mul27r; _add28i = _cos24i + _mul27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(mag, 0, _add28r, _add28i, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _neg31r = 0, _neg31i = 0;
        _neg31r = -(_c30r); _neg31i = -(_c30i);
        double _pow32r = 0, _pow32i = 0;
        c_powr(_neg31r, _neg31i, k, &_pow32r, &_pow32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_mul29r, _mul29i, _pow32r, _pow32i, &_mul33r, &_mul33i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_727_old_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 9.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + k; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(k, 0, k, 0, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log11r, _log11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(k, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(k, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_727_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 9.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + k; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(k, 0, k, 0, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log11r, _log11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(k, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(k, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_727_v1_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 9.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    for (int k = 0; k < (int)(n); k++) {
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _abs6r; _add7i = _abs5i + _abs6i;
        double _add8r = 0, _add8i = 0;
        _add8r = _add7r + k; _add8i = _add7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_mul(k, 0, k, 0, &_pow11r, &_pow11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log9r, _log9i, _pow11r, _pow11i, &_mul12r, &_mul12i);
        double mag = _mul12r; /* +_mul12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(k, 0, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang13r, _ang13i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _cos17r = 0, _cos17i = 0;
        c_cos(k, 0, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _mul18r; _add19i = _mul15i + _mul18i;
        double angle = _add19r; /* +_add19ii */
        double _cos20r = 0, _cos20i = 0;
        c_cos(angle, 0, &_cos20r, &_cos20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(angle, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c21r, _c21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _cos20r + _mul23r; _add24i = _cos20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag, 0, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_727a_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 9.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + k; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(k, 0, k, 0, &_pow13r, &_pow13i);
        c_mul(_pow13r, _pow13i, k, 0, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log11r, _log11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(k, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(k, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_727b_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 13.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[13];
    for (int _li = 0; _li < 13; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 12.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[13];
    for (int _li = 0; _li < 13; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 12.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + k; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(k, 0, k, 0, &_pow13r, &_pow13i);
        c_mul(_pow13r, _pow13i, k, 0, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log11r, _log11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(k, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(k, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_727c_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 29.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[29];
    for (int _li = 0; _li < 29; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 28.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[29];
    for (int _li = 0; _li < 29; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 28.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + k; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(k, 0, k, 0, &_pow13r, &_pow13i);
        c_mul(_pow13r, _pow13i, k, 0, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log11r, _log11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(k, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(k, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_727d_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 29.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[29];
    for (int _li = 0; _li < 29; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 28.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[29];
    for (int _li = 0; _li < 29; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 28.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + k; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(k, 0, k, 0, &_pow13r, &_pow13i);
        c_mul(_pow13r, _pow13i, k, 0, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log11r, _log11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(k, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(k, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_727_alt_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 9.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + k; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(k, 0, k, 0, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log11r, _log11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(k, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(k, 0, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _sin17r + _cos18r; _add19i = _sin17i + _cos18i;
        double angle = _add19r; /* +_add19ii */
        double _cos20r = 0, _cos20i = 0;
        c_cos(angle, 0, &_cos20r, &_cos20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(angle, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c21r, _c21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _cos20r + _mul23r; _add24i = _cos20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag, 0, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_728_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_sin6r, _sin6i); _abs7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log3r, _log3i, _abs7r, _abs7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_attr9r, _attr9i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs10r + j; _add11i = _abs10i + 0;
        double _sqrt12r = 0, _sqrt12i = 0;
        c_powr(_add11r, _add11i, 0.5, &_sqrt12r, &_sqrt12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul8r + _sqrt12r; _add13i = _mul8i + _sqrt12i;
        double mag = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang14r, _ang14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2r; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _attr19r, _attr19i, &_mul20r, &_mul20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_mul20r, _mul20i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _sin21r; _add22i = _mul18i + _sin21i;
        double ang = _add22r; /* +_add22ii */
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, ang, 0, &_mul24r, &_mul24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag, 0, _exp25r, _exp25i, &_mul26r, &_mul26i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_729_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 0; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_attr3r, _attr3i, _pow5r, _pow5i, &_mul6r, &_mul6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2r; _attr7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        double _sqrt10r = 0, _sqrt10i = 0;
        c_powr(_add9r, _add9i, 0.5, &_sqrt10r, &_sqrt10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr7r, _attr7i, _sqrt10r, _sqrt10i, &_mul11r, &_mul11i);
        double _sub12r = 0, _sub12i = 0;
        _sub12r = _mul6r - _mul11r; _sub12i = _mul6i - _mul11i;
        double r_part = _sub12r; /* +_sub12ii */
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x2i; _attr14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _attr13r + _attr14r; _add15i = _attr13i + _attr14i;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = j + _c16r; _add17i = 0 + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_add15r, _add15i, _log18r, _log18i, &_mul19r, &_mul19i);
        double im_part = _mul19r; /* +_mul19ii */
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _mod22r = 0, _mod22i = 0;
        _mod22r = fmod(j, _c21r); _mod22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _mod22r + _c23r; _add24i = _mod22i + _c23i;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_abs20r, _abs20i, _add24r, &_pow25r, &_pow25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x2r, x2i); _abs26i = 0;
        double _sub27r = 0, _sub27i = 0;
        _sub27r = degree - j; _sub27i = 0 - 0;
        double _pow28r = 0, _pow28i = 0;
        c_powr(_abs26r, _abs26i, _sub27r, &_pow28r, &_pow28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _pow25r + _pow28r; _add29i = _pow25i + _pow28i;
        double magnitude = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(j, 0, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang30r, _ang30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _cos34r = 0, _cos34i = 0;
        c_cos(j, 0, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang33r, _ang33i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul32r + _mul35r; _add36i = _mul32i + _mul35i;
        double angle = _add36r; /* +_add36ii */
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, im_part, 0, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = r_part + _mul38r; _add39i = 0 + _mul38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_add39r, _add39i, magnitude, 0, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_mul40r, _mul40i, _exp43r, _exp43i, &_mul44r, &_mul44i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_730_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs1r + _c2r; _add3i = _abs1i + _c2i;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1r; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _pow10r = 0, _pow10i = 0;
        c_powr(_add6r, _add6i, _sin9r, &_pow10r, &_pow10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log4r, _log4i, _pow10r, _pow10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2r; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul11r + _cos14r; _add15i = _mul11i + _cos14i;
        double mag_part = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(j, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _cos20r = 0, _cos20i = 0;
        c_cos(j, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
        double angle_part = _add22r; /* +_add22ii */
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, angle_part, 0, &_mul24r, &_mul24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag_part, 0, _exp25r, _exp25i, &_mul26r, &_mul26i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_731_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_abs1r, _abs1i, j, 0, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mul2r + _c3r; _add4i = _mul2i + _c3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr6r, _attr6i, j, 0, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _div12r = 0, _div12i = 0;
        c_div(_attr9r, _attr9i, _add11r, _add11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin8r + _cos13r; _add14i = _sin8i + _cos13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(_add14r, _add14i); _abs15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log5r, _log5i, _abs15r, _abs15i, &_mul16r, &_mul16i);
        double mag_part = _mul16r; /* +_mul16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _sqrt18r = 0, _sqrt18i = 0;
        c_powr(j, 0, 0.5, &_sqrt18r, &_sqrt18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang17r, _ang17i, _sqrt18r, _sqrt18i, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_ang20r, _ang20i, _add22r, _add22i, &_div23r, &_div23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul19r + _div23r; _add24i = _mul19i + _div23i;
        double angle_part = _add24r; /* +_add24ii */
        double _cos25r = 0, _cos25i = 0;
        c_cos(angle_part, 0, &_cos25r, &_cos25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(angle_part, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c26r, _c26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos25r + _mul28r; _add29i = _cos25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag_part, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_732_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 9; k++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr1r, _attr1i, _log4r, _log4i, &_mul5r, &_mul5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = k + _c7r; _add8i = 0 + _c7i;
        double _sqrt9r = 0, _sqrt9i = 0;
        c_powr(_add8r, _add8i, 0.5, &_sqrt9r, &_sqrt9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr6r, _attr6i, _sqrt9r, _sqrt9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul5r + _mul10r; _add11i = _mul5i + _mul10i;
        double r_part = _add11r; /* +_add11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1i; _attr12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(k, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr12r, _attr12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x2i; _attr15i = 0;
        double _cos16r = 0, _cos16i = 0;
        c_cos(k, 0, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_attr15r, _attr15i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = _mul14r - _mul17r; _sub18i = _mul14i - _mul17i;
        double i_part = _sub18r; /* +_sub18ii */
        double _sin19r = 0, _sin19i = 0;
        c_sin(r_part, 0, &_sin19r, &_sin19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(i_part, 0, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _sin19r + _cos20r; _add21i = _sin19i + _cos20i;
        double angle = _add21r; /* +_add21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _abs23r; _add24i = _abs22i + _abs23i;
        double _add25r = 0, _add25i = 0;
        _add25r = _add24r + k; _add25i = _add24i + 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _add25r + _c26r; _add27i = _add25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = k + _c29r; _add30i = 0 + _c29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log28r, _log28i, _add30r, _add30i, &_mul31r, &_mul31i);
        double magnitude = _mul31r; /* +_mul31ii */
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
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x2r; _conj39i = -(x2i);
        double _pow40r = 0, _pow40i = 0;
        c_powr(_conj39r, _conj39i, k, &_pow40r, &_pow40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj38r, _conj38i, _pow40r, _pow40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul37r + _mul41r; _add42i = _mul37i + _mul41i;
        { int _idx = k; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_733_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 10; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0.0;
        double _add3r = 0, _add3i = 0;
        _add3r = _c1r + _c2r; _add3i = _c1i + _c2i;
        double tmp = _add3r; /* +_add3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        for (int j = 1; j < (int)(_add5r); j++) {
            double _attr6r = 0, _attr6i = 0;
            _attr6r = x1r; _attr6i = 0;
            double _pow7r = 0, _pow7i = 0;
            c_powr(_attr6r, _attr6i, j, &_pow7r, &_pow7i);
            double _c8r = 0, _c8i = 0;
            _c8r = 1.0; _c8i = 0;
            double _add9r = 0, _add9i = 0;
            _add9r = j + _c8r; _add9i = 0 + _c8i;
            double _div10r = 0, _div10i = 0;
            c_div(_pow7r, _pow7i, _add9r, _add9i, &_div10r, &_div10i);
            double _c11r = 0, _c11i = 0;
            _c11r = 0.0; _c11i = 1.0;
            double _attr12r = 0, _attr12i = 0;
            _attr12r = x2r; _attr12i = 0;
            double _mul13r = 0, _mul13i = 0;
            c_mul(j, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
            double _sin14r = 0, _sin14i = 0;
            c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
            double _mul15r = 0, _mul15i = 0;
            c_mul(_c11r, _c11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
            double _exp16r = 0, _exp16i = 0;
            c_exp2(_mul15r, _mul15i, &_exp16r, &_exp16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_div10r, _div10i, _exp16r, _exp16i, &_mul17r, &_mul17i);
            tmp += _mul17r;
        }
        double _c18r = 0, _c18i = 0;
        _c18r = 3.0; _c18i = 0;
        double _mod19r = 0, _mod19i = 0;
        _mod19r = fmod(k, _c18r); _mod19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _mod19r + _c20r; _add21i = _mod19i + _c20i;
        for (int r = 1; r < (int)(_add21r); r++) {
            double _attr22r = 0, _attr22i = 0;
            _attr22r = x2i; _attr22i = 0;
            double _pow23r = 0, _pow23i = 0;
            c_powr(_attr22r, _attr22i, r, &_pow23r, &_pow23i);
            double _c24r = 0, _c24i = 0;
            _c24r = 2.0; _c24i = 0;
            double _add25r = 0, _add25i = 0;
            _add25r = r + _c24r; _add25i = 0 + _c24i;
            double _div26r = 0, _div26i = 0;
            c_div(_pow23r, _pow23i, _add25r, _add25i, &_div26r, &_div26i);
            double _c27r = 0, _c27i = 0;
            _c27r = 0.0; _c27i = 1.0;
            double _attr28r = 0, _attr28i = 0;
            _attr28r = x1i; _attr28i = 0;
            double _mul29r = 0, _mul29i = 0;
            c_mul(r, 0, _attr28r, _attr28i, &_mul29r, &_mul29i);
            double _cos30r = 0, _cos30i = 0;
            c_cos(_mul29r, _mul29i, &_cos30r, &_cos30i);
            double _mul31r = 0, _mul31i = 0;
            c_mul(_c27r, _c27i, _cos30r, _cos30i, &_mul31r, &_mul31i);
            double _exp32r = 0, _exp32i = 0;
            c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
            double _mul33r = 0, _mul33i = 0;
            c_mul(_div26r, _div26i, _exp32r, _exp32i, &_mul33r, &_mul33i);
            tmp += _mul33r;
        }
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = tmp; cIm[_idx] = 0; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_734_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(j, 0, j, 0, &_pow3r, &_pow3i);
        double _add4r = 0, _add4i = 0;
        _add4r = _abs1r + _pow3r; _add4i = _abs1i + _pow3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_mul11r, _mul11i, _attr12r, _attr12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul9r + _cos14r; _add15i = _mul9i + _cos14i;
        double mag = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x2r; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _attr17r, _attr17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang16r, _ang16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(j, 0, &_sin21r, &_sin21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_sin21r, _sin21i, _log25r, _log25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul20r + _mul26r; _add27i = _mul20i + _mul26i;
        double angle = _add27r; /* +_add27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c28r, _c28i, angle, 0, &_mul29r, &_mul29i);
        double _exp30r = 0, _exp30i = 0;
        c_exp2(_mul29r, _mul29i, &_exp30r, &_exp30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag, 0, _exp30r, _exp30i, &_mul31r, &_mul31i);
        double _conj32r = 0, _conj32i = 0;
        _conj32r = x1r; _conj32i = -(x1i);
        double _pow33r = 0, _pow33i = 0;
        c_powr(x2r, x2i, j, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_conj32r, _conj32i, _pow33r, _pow33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_735_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_attr1r, _attr1i, j, &_pow2r, &_pow2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x2r, x2i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log6r, _log6i, j, 0, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow2r + _mul7r; _add8i = _pow2i + _mul7i;
        double real_part = _add8r; /* +_add8ii */
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr9r, _attr9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x2r; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul13r + _cos16r; _add17i = _mul13i + _cos16i;
        double imag_part = _add17r; /* +_add17ii */
        double _add18r = 0, _add18i = 0;
        _add18r = x1r + x2r; _add18i = x1i + x2i;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(_add18r, _add18i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs19r + j; _add20i = _abs19i + 0;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(j, 0, j, 0, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _c22r + _pow24r; _add25i = _c22i + _pow24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log21r, _log21i, _add25r, _add25i, &_mul26r, &_mul26i);
        double magnitude = _mul26r; /* +_mul26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _cos28r = 0, _cos28i = 0;
        c_cos(j, 0, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang27r, _ang27i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _ang30r, _ang30i, &_mul31r, &_mul31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_mul31r, _mul31i, &_sin32r, &_sin32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul29r + _sin32r; _add33i = _mul29i + _sin32i;
        double _attr34r = 0, _attr34i = 0;
        _attr34r = x1i; _attr34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, _attr34r, _attr34i, &_mul35r, &_mul35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_mul35r, _mul35i, &_cos36r, &_cos36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _add33r - _cos36r; _sub37i = _add33i - _cos36i;
        double angle = _sub37r; /* +_sub37ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_736_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x2r, x2i); _ang4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _ang4r, _ang4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _cos10r; _add11i = _mul7i + _cos10i;
        double mag_part = _add11r; /* +_add11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _cos13r = 0, _cos13i = 0;
        c_cos(j, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul14r + _sin17r; _add18i = _mul14i + _sin17i;
        double ang_part = _add18r; /* +_add18ii */
        double _c19r = 0, _c19i = 0;
        _c19r = 0.0; _c19i = 1.0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c19r, _c19i, ang_part, 0, &_mul20r, &_mul20i);
        double _exp21r = 0, _exp21i = 0;
        c_exp2(_mul20r, _mul20i, &_exp21r, &_exp21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(mag_part, 0, _exp21r, _exp21i, &_mul22r, &_mul22i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_737_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[9];
    for (int _li = 0; _li < 9; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[9];
    for (int _li = 0; _li < 9; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    for (int k = 1; k < 10; k++) {
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _abs6r; _add7i = _abs5i + _abs6i;
        double _add8r = 0, _add8i = 0;
        _add8r = _add7r + k; _add8i = _add7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 3.0; _c11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_c11r, _c11i, M_PI, 0, &_mul12r, &_mul12i);
        double _arr13r = 0, _arr13i = 0;
        { int _idx = (k - 1); _arr13r = (_idx >= 0 && _idx < 9) ? rec[_idx] : 0.0; _arr13i = 0; }
        double _mul14r = 0, _mul14i = 0;
        c_mul(_mul12r, _mul12i, _arr13r, _arr13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c10r + _sin15r; _add16i = _c10i + _sin15i;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c17r, _c17i, M_PI, 0, &_mul18r, &_mul18i);
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (k - 1); _arr19r = (_idx >= 0 && _idx < 9) ? imc[_idx] : 0.0; _arr19i = 0; }
        double _mul20r = 0, _mul20i = 0;
        c_mul(_mul18r, _mul18i, _arr19r, _arr19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _add16r + _cos21r; _add22i = _add16i + _cos21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log9r, _log9i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 5.0; _c25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c25r, _c25i, M_PI, 0, &_mul26r, &_mul26i);
        double _arr27r = 0, _arr27i = 0;
        { int _idx = (k - 1); _arr27r = (_idx >= 0 && _idx < 9) ? imc[_idx] : 0.0; _arr27i = 0; }
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul26r, _mul26i, _arr27r, _arr27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang24r, _ang24i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 4.0; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, M_PI, 0, &_mul33r, &_mul33i);
        double _arr34r = 0, _arr34i = 0;
        { int _idx = (k - 1); _arr34r = (_idx >= 0 && _idx < 9) ? rec[_idx] : 0.0; _arr34i = 0; }
        double _mul35r = 0, _mul35i = 0;
        c_mul(_mul33r, _mul33i, _arr34r, _arr34i, &_mul35r, &_mul35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_mul35r, _mul35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang31r, _ang31i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _mul30r - _mul37r; _sub38i = _mul30i - _mul37i;
        double ang = _sub38r; /* +_sub38ii */
        double _cos39r = 0, _cos39i = 0;
        c_cos(ang, 0, &_cos39r, &_cos39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _sin41r = 0, _sin41i = 0;
        c_sin(ang, 0, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c40r, _c40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cos39r + _mul42r; _add43i = _cos39i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_738_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2i; _attr2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_attr1r, _attr1i, _attr2r, _attr2i, &_mul3r, &_mul3i);
    double r = _mul3r; /* +_mul3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2r; _attr5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_attr4r, _attr4i, _attr5r, _attr5i, &_mul6r, &_mul6i);
    double i = _mul6r; /* +_mul6ii */
    double _c7r = 0, _c7i = 0;
    _c7r = 0.0; _c7i = 1.0;
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c7r, _c7i, r, 0, &_mul8r, &_mul8i);
    double _exp9r = 0, _exp9i = 0;
    c_exp2(_mul8r, _mul8i, &_exp9r, &_exp9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(i, 0, _exp9r, _exp9i, &_mul10r, &_mul10i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 0.0; _c11i = 100.0;
    double _sin12r = 0, _sin12i = 0;
    c_sin(r, 0, &_sin12r, &_sin12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c11r, _c11i, _sin12r, _sin12i, &_mul13r, &_mul13i);
    double _cos14r = 0, _cos14i = 0;
    c_cos(i, 0, &_cos14r, &_cos14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_mul13r, _mul13i, _cos14r, _cos14i, &_mul15r, &_mul15i);
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 1.0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_c16r, _c16i, r, 0, &_mul17r, &_mul17i);
    double _c18r = 0, _c18i = 0;
    _c18r = 0.0; _c18i = 1.0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(r, 0, _c18r, _c18i, &_mul19r, &_mul19i);
    double _add20r = 0, _add20i = 0;
    _add20r = _mul17r + _mul19r; _add20i = _mul17i + _mul19i;
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    double _mul21r = 0, _mul21i = 0;
    c_mul(r, 0, i, 0, &_mul21r, &_mul21i);
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(_mul21r, _mul21i); _abs22i = 0;
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _at224r = 0, _at224i = 0;
    _at224r = atan2(r, i); _at224i = 0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c23r, _c23i, _at224r, _at224i, &_mul25r, &_mul25i);
    double _exp26r = 0, _exp26i = 0;
    c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_abs22r, _abs22i, _exp26r, _exp26i, &_mul27r, &_mul27i);
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 100.0; _c28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 1.0; _c29i = 0;
    double _neg30r = 0, _neg30i = 0;
    _neg30r = -(r); _neg30i = -(0);
    double _exp31r = 0, _exp31i = 0;
    c_exp2(_neg30r, _neg30i, &_exp31r, &_exp31i);
    double _add32r = 0, _add32i = 0;
    _add32r = _c29r + _exp31r; _add32i = _c29i + _exp31i;
    double _div33r = 0, _div33i = 0;
    c_div(_c28r, _c28i, _add32r, _add32i, &_div33r, &_div33i);
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _div33r; cIm[_idx] = _div33i; } }
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(i, 0); _abs34i = 0;
    double _mul35r = 0, _mul35i = 0;
    c_mul(r, 0, _abs34r, _abs34i, &_mul35r, &_mul35i);
    double _abs36r = 0, _abs36i = 0;
    _abs36r = c_abs(r, 0); _abs36i = 0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(i, 0, _abs36r, _abs36i, &_mul37r, &_mul37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _mul35r + _mul37r; _add38i = _mul35i + _mul37i;
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    double _mul39r = 0, _mul39i = 0;
    c_mul(r, 0, i, 0, &_mul39r, &_mul39i);
    double _abs40r = 0, _abs40i = 0;
    _abs40r = c_abs(_mul39r, _mul39i); _abs40i = 0;
    double _c41r = 0, _c41i = 0;
    _c41r = 1.0; _c41i = 0;
    double _c42r = 0, _c42i = 0;
    _c42r = 3.0; _c42i = 0;
    double _div43r = 0, _div43i = 0;
    c_div(_c41r, _c41i, _c42r, _c42i, &_div43r, &_div43i);
    double _pow44r = 0, _pow44i = 0;
    c_powr(_abs40r, _abs40i, _div43r, &_pow44r, &_pow44i);
    double _sqrt45r = 0, _sqrt45i = 0;
    c_powr(_pow44r, _pow44i, 0.5, &_sqrt45r, &_sqrt45i);
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 1.0;
    double _at247r = 0, _at247i = 0;
    _at247r = atan2(r, i); _at247i = 0;
    double _c48r = 0, _c48i = 0;
    _c48r = 4.0; _c48i = 0;
    double _div49r = 0, _div49i = 0;
    c_div(M_PI, 0, _c48r, _c48i, &_div49r, &_div49i);
    double _add50r = 0, _add50i = 0;
    _add50r = _at247r + _div49r; _add50i = _at247i + _div49i;
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c46r, _c46i, _add50r, _add50i, &_mul51r, &_mul51i);
    double _exp52r = 0, _exp52i = 0;
    c_exp2(_mul51r, _mul51i, &_exp52r, &_exp52i);
    double _mul53r = 0, _mul53i = 0;
    c_mul(_sqrt45r, _sqrt45i, _exp52r, _exp52i, &_mul53r, &_mul53i);
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    double _c54r = 0, _c54i = 0;
    _c54r = 2.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_mul(r, 0, r, 0, &_pow55r, &_pow55i);
    double _c56r = 0, _c56i = 0;
    _c56r = 2.0; _c56i = 0;
    double _pow57r = 0, _pow57i = 0;
    c_mul(i, 0, i, 0, &_pow57r, &_pow57i);
    double _add58r = 0, _add58i = 0;
    _add58r = _pow55r + _pow57r; _add58i = _pow55i + _pow57i;
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    double _c59r = 0, _c59i = 0;
    _c59r = 0.0; _c59i = 1.0;
    double _mul60r = 0, _mul60i = 0;
    c_mul(_c59r, _c59i, i, 0, &_mul60r, &_mul60i);
    double _add61r = 0, _add61i = 0;
    _add61r = r + _mul60r; _add61i = 0 + _mul60i;
    double _abs62r = 0, _abs62i = 0;
    _abs62r = c_abs(_add61r, _add61i); _abs62i = 0;
    double _c63r = 0, _c63i = 0;
    _c63r = 1.5; _c63i = 0;
    double _pow64r = 0, _pow64i = 0;
    c_powr(_abs62r, _abs62i, 1.5, &_pow64r, &_pow64i);
    double _c65r = 0, _c65i = 0;
    _c65r = 0.0; _c65i = 1.0;
    double _at266r = 0, _at266i = 0;
    _at266r = atan2(r, i); _at266i = 0;
    double _c67r = 0, _c67i = 0;
    _c67r = 4.0; _c67i = 0;
    double _div68r = 0, _div68i = 0;
    c_div(M_PI, 0, _c67r, _c67i, &_div68r, &_div68i);
    double _add69r = 0, _add69i = 0;
    _add69r = _at266r + _div68r; _add69i = _at266i + _div68i;
    double _mul70r = 0, _mul70i = 0;
    c_mul(_c65r, _c65i, _add69r, _add69i, &_mul70r, &_mul70i);
    double _exp71r = 0, _exp71i = 0;
    c_exp2(_mul70r, _mul70i, &_exp71r, &_exp71i);
    double _mul72r = 0, _mul72i = 0;
    c_mul(_pow64r, _pow64i, _exp71r, _exp71i, &_mul72r, &_mul72i);
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul72r; cIm[_idx] = _mul72i; } }
    double _mul73r = 0, _mul73i = 0;
    c_mul(r, 0, i, 0, &_mul73r, &_mul73i);
    double _sub74r = 0, _sub74i = 0;
    _sub74r = r - i; _sub74i = 0 - 0;
    double _abs75r = 0, _abs75i = 0;
    _abs75r = c_abs(_sub74r, _sub74i); _abs75i = 0;
    double _mul76r = 0, _mul76i = 0;
    c_mul(_mul73r, _mul73i, _abs75r, _abs75i, &_mul76r, &_mul76i);
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul76r; cIm[_idx] = _mul76i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_739_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _log2r = 0, _log2i = 0;
    c_log(_add1r, _add1i, &_log2r, &_log2i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _log2r; cIm[_idx] = _log2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 1.0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c3r, _c3i, x1r, x1i, &_mul4r, &_mul4i);
    double _exp5r = 0, _exp5i = 0;
    c_exp2(_mul4r, _mul4i, &_exp5r, &_exp5i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp5r; cIm[_idx] = _exp5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 1.0;
    double _neg7r = 0, _neg7i = 0;
    _neg7r = -(_c6r); _neg7i = -(_c6i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_neg7r, _neg7i, x2r, x2i, &_mul8r, &_mul8i);
    double _exp9r = 0, _exp9i = 0;
    c_exp2(_mul8r, _mul8i, &_exp9r, &_exp9i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp9r; cIm[_idx] = _exp9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 2.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow11r, &_pow11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 2.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow13r, &_pow13i);
    double _sub14r = 0, _sub14i = 0;
    _sub14r = _pow11r - _pow13r; _sub14i = _pow11i - _pow13i;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub14r; cIm[_idx] = _sub14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 1.0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c15r, _c15i, x1r, x1i, &_mul16r, &_mul16i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(_mul16r, _mul16i, x2r, x2i, &_mul17r, &_mul17i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul17r; cIm[_idx] = _mul17i; } }
    double _sin18r = 0, _sin18i = 0;
    c_sin(x1r, x1i, &_sin18r, &_sin18i);
    double _cos19r = 0, _cos19i = 0;
    c_cos(x2r, x2i, &_cos19r, &_cos19i);
    double _add20r = 0, _add20i = 0;
    _add20r = _sin18r + _cos19r; _add20i = _sin18i + _cos19i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    double _cos21r = 0, _cos21i = 0;
    c_cos(x1r, x1i, &_cos21r, &_cos21i);
    double _sin22r = 0, _sin22i = 0;
    c_sin(x2r, x2i, &_sin22r, &_sin22i);
    double _sub23r = 0, _sub23i = 0;
    _sub23r = _cos21r - _sin22r; _sub23i = _cos21i - _sin22i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub23r; cIm[_idx] = _sub23i; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c24r, _c24i, x2r, x2i, &_mul25r, &_mul25i);
    double _add26r = 0, _add26i = 0;
    _add26r = x1r + _mul25r; _add26i = x1i + _mul25i;
    double _c27r = 0, _c27i = 0;
    _c27r = 2.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(_add26r, _add26i, _add26r, _add26i, &_pow28r, &_pow28i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _pow28r; cIm[_idx] = _pow28i; } }
    double _c29r = 0, _c29i = 0;
    _c29r = 0.0; _c29i = 1.0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c29r, _c29i, x1r, x1i, &_mul30r, &_mul30i);
    double _sub31r = 0, _sub31i = 0;
    _sub31r = x2r - _mul30r; _sub31i = x2i - _mul30i;
    double _c32r = 0, _c32i = 0;
    _c32r = 3.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(_sub31r, _sub31i, _sub31r, _sub31i, &_pow33r, &_pow33i);
    c_mul(_pow33r, _pow33i, _sub31r, _sub31i, &_pow33r, &_pow33i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _pow33r; cIm[_idx] = _pow33i; } }
    double _c34r = 0, _c34i = 0;
    _c34r = 2.0; _c34i = 0;
    double _pow35r = 0, _pow35i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow35r, &_pow35i);
    double _c36r = 0, _c36i = 0;
    _c36r = 2.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow37r, &_pow37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _pow35r + _pow37r; _add38i = _pow35i + _pow37i;
    double _sqrt39r = 0, _sqrt39i = 0;
    c_powr(_add38r, _add38i, 0.5, &_sqrt39r, &_sqrt39i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sqrt39r; cIm[_idx] = _sqrt39i; } }
    double _mul40r = 0, _mul40i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul40r, &_mul40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = x1r - x2r; _sub41i = x1i - x2i;
    double _mul42r = 0, _mul42i = 0;
    c_mul(_mul40r, _mul40i, _sub41r, _sub41i, &_mul42r, &_mul42i);
    double _c43r = 0, _c43i = 0;
    _c43r = 0.0; _c43i = 1.0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c43r, _c43i, x2r, x2i, &_mul44r, &_mul44i);
    double _add45r = 0, _add45i = 0;
    _add45r = x1r + _mul44r; _add45i = x1i + _mul44i;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_mul42r, _mul42i, _add45r, _add45i, &_mul46r, &_mul46i);
    double _c47r = 0, _c47i = 0;
    _c47r = 0.0; _c47i = 1.0;
    double _mul48r = 0, _mul48i = 0;
    c_mul(_c47r, _c47i, x1r, x1i, &_mul48r, &_mul48i);
    double _sub49r = 0, _sub49i = 0;
    _sub49r = x2r - _mul48r; _sub49i = x2i - _mul48i;
    double _mul50r = 0, _mul50i = 0;
    c_mul(_mul46r, _mul46i, _sub49r, _sub49i, &_mul50r, &_mul50i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_740_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double r_1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double r_2 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double i_1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double i_2 = _attr4r; /* +_attr4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 3.0; _c5i = 0;
    double _pow6r = 0, _pow6i = 0;
    c_mul(i_1, 0, i_1, 0, &_pow6r, &_pow6i);
    c_mul(_pow6r, _pow6i, i_1, 0, &_pow6r, &_pow6i);
    double _c7r = 0, _c7i = 0;
    _c7r = 3.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(i_2, 0, i_2, 0, &_pow8r, &_pow8i);
    c_mul(_pow8r, _pow8i, i_2, 0, &_pow8r, &_pow8i);
    double _sub9r = 0, _sub9i = 0;
    _sub9r = _pow6r - _pow8r; _sub9i = _pow6i - _pow8i;
    double _c10r = 0, _c10i = 0;
    _c10r = 2.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(r_1, 0, r_1, 0, &_pow11r, &_pow11i);
    double _add12r = 0, _add12i = 0;
    _add12r = _sub9r + _pow11r; _add12i = _sub9i + _pow11i;
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(r_2, 0, r_2, 0, &_pow14r, &_pow14i);
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _add12r - _pow14r; _sub15i = _add12i - _pow14i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub15r; cIm[_idx] = _sub15i; } }
    double _mul16r = 0, _mul16i = 0;
    c_mul(r_1, 0, i_1, 0, &_mul16r, &_mul16i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(_mul16r, _mul16i, r_2, 0, &_mul17r, &_mul17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_mul17r, _mul17i, i_2, 0, &_mul18r, &_mul18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 0.0; _c19i = 1.0;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c19r, _c19i, i_1, 0, &_mul20r, &_mul20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = r_1 - _mul20r; _sub21i = 0 - _mul20i;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_mul18r, _mul18i, _sub21r, _sub21i, &_mul22r, &_mul22i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(i_1, 0, i_1, 0, &_pow24r, &_pow24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(r_2, 0, r_2, 0, &_pow26r, &_pow26i);
    double _add27r = 0, _add27i = 0;
    _add27r = _pow24r + _pow26r; _add27i = _pow24i + _pow26i;
    double _c28r = 0, _c28i = 0;
    _c28r = 2.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(i_2, 0, i_2, 0, &_pow29r, &_pow29i);
    double _sub30r = 0, _sub30i = 0;
    _sub30r = _add27r - _pow29r; _sub30i = _add27i - _pow29i;
    double _sub31r = 0, _sub31i = 0;
    _sub31r = r_2 - i_1; _sub31i = 0 - 0;
    double _add32r = 0, _add32i = 0;
    _add32r = _sub30r + _sub31r; _add32i = _sub30i + _sub31i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(r_1, 0, r_1, 0, &_pow34r, &_pow34i);
    double _c35r = 0, _c35i = 0;
    _c35r = 2.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(i_1, 0, i_1, 0, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_pow34r, _pow34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _c38r = 0, _c38i = 0;
    _c38r = 2.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(r_2, 0, r_2, 0, &_pow39r, &_pow39i);
    double _c40r = 0, _c40i = 0;
    _c40r = 2.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(i_2, 0, i_2, 0, &_pow41r, &_pow41i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_pow39r, _pow39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _mul37r - _mul42r; _sub43i = _mul37i - _mul42i;
    double _mul44r = 0, _mul44i = 0;
    c_mul(r_1, 0, _sub43r, _sub43i, &_mul44r, &_mul44i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 3.0; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_mul(r_1, 0, r_1, 0, &_pow46r, &_pow46i);
    c_mul(_pow46r, _pow46i, r_1, 0, &_pow46r, &_pow46i);
    double _c47r = 0, _c47i = 0;
    _c47r = 2.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(r_1, 0, r_1, 0, &_pow48r, &_pow48i);
    double _add49r = 0, _add49i = 0;
    _add49r = _pow46r + _pow48r; _add49i = _pow46i + _pow48i;
    double _c50r = 0, _c50i = 0;
    _c50r = 3.0; _c50i = 0;
    double _pow51r = 0, _pow51i = 0;
    c_mul(i_2, 0, i_2, 0, &_pow51r, &_pow51i);
    c_mul(_pow51r, _pow51i, i_2, 0, &_pow51r, &_pow51i);
    double _add52r = 0, _add52i = 0;
    _add52r = _add49r + _pow51r; _add52i = _add49i + _pow51i;
    double _c53r = 0, _c53i = 0;
    _c53r = 2.0; _c53i = 0;
    double _pow54r = 0, _pow54i = 0;
    c_mul(i_1, 0, i_1, 0, &_pow54r, &_pow54i);
    double _add55r = 0, _add55i = 0;
    _add55r = _add52r + _pow54r; _add55i = _add52i + _pow54i;
    double _c56r = 0, _c56i = 0;
    _c56r = 10.0; _c56i = 0;
    double _sub57r = 0, _sub57i = 0;
    _sub57r = _add55r - _c56r; _sub57i = _add55i - _c56i;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub57r; cIm[_idx] = _sub57i; } }
    double _mul58r = 0, _mul58i = 0;
    c_mul(i_1, 0, i_2, 0, &_mul58r, &_mul58i);
    double _c59r = 0, _c59i = 0;
    _c59r = 2.0; _c59i = 0;
    double _pow60r = 0, _pow60i = 0;
    c_mul(i_1, 0, i_1, 0, &_pow60r, &_pow60i);
    double _c61r = 0, _c61i = 0;
    _c61r = 2.0; _c61i = 0;
    double _pow62r = 0, _pow62i = 0;
    c_mul(r_2, 0, r_2, 0, &_pow62r, &_pow62i);
    double _mul63r = 0, _mul63i = 0;
    c_mul(_pow60r, _pow60i, _pow62r, _pow62i, &_mul63r, &_mul63i);
    double _c64r = 0, _c64i = 0;
    _c64r = 2.0; _c64i = 0;
    double _pow65r = 0, _pow65i = 0;
    c_mul(i_2, 0, i_2, 0, &_pow65r, &_pow65i);
    double _c66r = 0, _c66i = 0;
    _c66r = 2.0; _c66i = 0;
    double _pow67r = 0, _pow67i = 0;
    c_mul(r_1, 0, r_1, 0, &_pow67r, &_pow67i);
    double _mul68r = 0, _mul68i = 0;
    c_mul(_pow65r, _pow65i, _pow67r, _pow67i, &_mul68r, &_mul68i);
    double _sub69r = 0, _sub69i = 0;
    _sub69r = _mul63r - _mul68r; _sub69i = _mul63i - _mul68i;
    double _mul70r = 0, _mul70i = 0;
    c_mul(_mul58r, _mul58i, _sub69r, _sub69i, &_mul70r, &_mul70i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul70r; cIm[_idx] = _mul70i; } }
    double _c71r = 0, _c71i = 0;
    _c71r = 0.5; _c71i = 0;
    double _pow72r = 0, _pow72i = 0;
    c_powr(r_1, 0, 0.5, &_pow72r, &_pow72i);
    double _c73r = 0, _c73i = 0;
    _c73r = 0.5; _c73i = 0;
    double _pow74r = 0, _pow74i = 0;
    c_powr(i_2, 0, 0.5, &_pow74r, &_pow74i);
    double _sub75r = 0, _sub75i = 0;
    _sub75r = _pow72r - _pow74r; _sub75i = _pow72i - _pow74i;
    double _c76r = 0, _c76i = 0;
    _c76r = 0.5; _c76i = 0;
    double _pow77r = 0, _pow77i = 0;
    c_powr(r_2, 0, 0.5, &_pow77r, &_pow77i);
    double _add78r = 0, _add78i = 0;
    _add78r = _sub75r + _pow77r; _add78i = _sub75i + _pow77i;
    double _c79r = 0, _c79i = 0;
    _c79r = 0.5; _c79i = 0;
    double _pow80r = 0, _pow80i = 0;
    c_powr(i_1, 0, 0.5, &_pow80r, &_pow80i);
    double _sub81r = 0, _sub81i = 0;
    _sub81r = _add78r - _pow80r; _sub81i = _add78i - _pow80i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub81r; cIm[_idx] = _sub81i; } }
    double _c82r = 0, _c82i = 0;
    _c82r = 2.0; _c82i = 0;
    double _pow83r = 0, _pow83i = 0;
    c_mul(i_1, 0, i_1, 0, &_pow83r, &_pow83i);
    double _mul84r = 0, _mul84i = 0;
    c_mul(r_1, 0, _pow83r, _pow83i, &_mul84r, &_mul84i);
    double _mul85r = 0, _mul85i = 0;
    c_mul(r_2, 0, i_2, 0, &_mul85r, &_mul85i);
    double _c86r = 0, _c86i = 0;
    _c86r = 0.0; _c86i = 1.0;
    double _mul87r = 0, _mul87i = 0;
    c_mul(_c86r, _c86i, i_1, 0, &_mul87r, &_mul87i);
    double _sub88r = 0, _sub88i = 0;
    _sub88r = r_1 - _mul87r; _sub88i = 0 - _mul87i;
    double _mul89r = 0, _mul89i = 0;
    c_mul(_mul85r, _mul85i, _sub88r, _sub88i, &_mul89r, &_mul89i);
    double _sub90r = 0, _sub90i = 0;
    _sub90r = _mul84r - _mul89r; _sub90i = _mul84i - _mul89i;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub90r; cIm[_idx] = _sub90i; } }
    double _c91r = 0, _c91i = 0;
    _c91r = 3.0; _c91i = 0;
    double _pow92r = 0, _pow92i = 0;
    c_mul(i_1, 0, i_1, 0, &_pow92r, &_pow92i);
    c_mul(_pow92r, _pow92i, i_1, 0, &_pow92r, &_pow92i);
    double _c93r = 0, _c93i = 0;
    _c93r = 3.0; _c93i = 0;
    double _pow94r = 0, _pow94i = 0;
    c_mul(i_2, 0, i_2, 0, &_pow94r, &_pow94i);
    c_mul(_pow94r, _pow94i, i_2, 0, &_pow94r, &_pow94i);
    double _sub95r = 0, _sub95i = 0;
    _sub95r = _pow92r - _pow94r; _sub95i = _pow92i - _pow94i;
    double _c96r = 0, _c96i = 0;
    _c96r = 2.0; _c96i = 0;
    double _pow97r = 0, _pow97i = 0;
    c_mul(r_2, 0, r_2, 0, &_pow97r, &_pow97i);
    double _add98r = 0, _add98i = 0;
    _add98r = _sub95r + _pow97r; _add98i = _sub95i + _pow97i;
    double _c99r = 0, _c99i = 0;
    _c99r = 2.0; _c99i = 0;
    double _pow100r = 0, _pow100i = 0;
    c_mul(r_1, 0, r_1, 0, &_pow100r, &_pow100i);
    double _sub101r = 0, _sub101i = 0;
    _sub101r = _add98r - _pow100r; _sub101i = _add98i - _pow100i;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub101r; cIm[_idx] = _sub101i; } }
    double _mul102r = 0, _mul102i = 0;
    c_mul(r_1, 0, i_1, 0, &_mul102r, &_mul102i);
    double _mul103r = 0, _mul103i = 0;
    c_mul(_mul102r, _mul102i, r_2, 0, &_mul103r, &_mul103i);
    double _mul104r = 0, _mul104i = 0;
    c_mul(_mul103r, _mul103i, i_2, 0, &_mul104r, &_mul104i);
    double _sub105r = 0, _sub105i = 0;
    _sub105r = i_2 - r_1; _sub105i = 0 - 0;
    double _mul106r = 0, _mul106i = 0;
    c_mul(_mul104r, _mul104i, _sub105r, _sub105i, &_mul106r, &_mul106i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul106r; cIm[_idx] = _mul106i; } }
    double _c107r = 0, _c107i = 0;
    _c107r = 4.0; _c107i = 0;
    double _pow108r = 0, _pow108i = 0;
    c_mul(i_1, 0, i_1, 0, &_pow108r, &_pow108i);
    c_mul(_pow108r, _pow108i, _pow108r, _pow108i, &_pow108r, &_pow108i);
    double _c109r = 0, _c109i = 0;
    _c109r = 4.0; _c109i = 0;
    double _pow110r = 0, _pow110i = 0;
    c_mul(i_2, 0, i_2, 0, &_pow110r, &_pow110i);
    c_mul(_pow110r, _pow110i, _pow110r, _pow110i, &_pow110r, &_pow110i);
    double _add111r = 0, _add111i = 0;
    _add111r = _pow108r + _pow110r; _add111i = _pow108i + _pow110i;
    double _c112r = 0, _c112i = 0;
    _c112r = 4.0; _c112i = 0;
    double _pow113r = 0, _pow113i = 0;
    c_mul(r_1, 0, r_1, 0, &_pow113r, &_pow113i);
    c_mul(_pow113r, _pow113i, _pow113r, _pow113i, &_pow113r, &_pow113i);
    double _add114r = 0, _add114i = 0;
    _add114r = _add111r + _pow113r; _add114i = _add111i + _pow113i;
    double _c115r = 0, _c115i = 0;
    _c115r = 4.0; _c115i = 0;
    double _pow116r = 0, _pow116i = 0;
    c_mul(r_2, 0, r_2, 0, &_pow116r, &_pow116i);
    c_mul(_pow116r, _pow116i, _pow116r, _pow116i, &_pow116r, &_pow116i);
    double _add117r = 0, _add117i = 0;
    _add117r = _add114r + _pow116r; _add117i = _add114i + _pow116i;
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add117r; cIm[_idx] = _add117i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_741_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 7.0;
    double _add2r = 0, _add2i = 0;
    _add2r = x1r + _c1r; _add2i = x1i + _c1i;
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(_add2r, _add2i, _add2r, _add2i, &_pow4r, &_pow4i);
    double _re5r = 0, _re5i = 0;
    _re5r = _pow4r; _re5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 5.0;
    double _add7r = 0, _add7i = 0;
    _add7r = x2r + _c6r; _add7i = x2i + _c6i;
    double _c8r = 0, _c8i = 0;
    _c8r = 3.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_mul(_add7r, _add7i, _add7r, _add7i, &_pow9r, &_pow9i);
    c_mul(_pow9r, _pow9i, _add7r, _add7i, &_pow9r, &_pow9i);
    double _im10r = 0, _im10i = 0;
    _im10r = _pow9i; _im10i = 0;
    double _add11r = 0, _add11i = 0;
    _add11r = _re5r + _im10r; _add11i = _re5i + _im10i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add11r; cIm[_idx] = _add11i; } }
    double _c12r = 0, _c12i = 0;
    _c12r = 0.0; _c12i = 1.0;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c12r, _c12i, x1r, x1i, &_mul13r, &_mul13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_mul13r, _mul13i, x2r, x2i, &_mul14r, &_mul14i);
    double _exp15r = 0, _exp15i = 0;
    c_exp2(_mul14r, _mul14i, &_exp15r, &_exp15i);
    { int _idx = 1; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _exp15r; cIm[_idx] = _exp15i; } }
    for (int _si = 0; _si < 4; _si++) {
        int _si_idx = _si + 2;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, 0, 0, &_mul17r, &_mul17i);
        double _log18r = 0, _log18i = 0;
        c_log(_mul17r, _mul17i, &_log18r, &_log18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 0.0; _c19i = 1.0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c19r, _c19i, x1r, x1i, &_mul20r, &_mul20i);
        double _exp21r = 0, _exp21i = 0;
        c_exp2(_mul20r, _mul20i, &_exp21r, &_exp21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _log18r + _exp21r; _add22i = _log18i + _exp21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _neg24r = 0, _neg24i = 0;
        _neg24r = -(_c23r); _neg24i = -(_c23i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_neg24r, _neg24i, x2r, x2i, &_mul25r, &_mul25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add22r + _exp26r; _add27i = _add22i + _exp26i;
        cRe[_si_idx] = _add27r; cIm[_si_idx] = _add27i;
    }
    for (int _si = 0; _si < 4; _si++) {
        int _si_idx = _si + 6;
        double _sub28r = 0, _sub28i = 0;
        /* WARNING: unhandled subscript Subscript(value=Subscript(value=Name(id='cf', ctx=Load()), slice=Slice(lower=Constant(value=0), upper=Constant(value=4)), ctx=Load()), slice=Slice(step=UnaryOp(op=USub(), operand=Constant(value=1))), ctx=Load()) */
        cRe[_si_idx] = _sub28r; cIm[_si_idx] = _sub28i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_742_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 1.0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_c3r, _c3i, x2r, x2i, &_mul4r, &_mul4i);
        double _add5r = 0, _add5i = 0;
        _add5r = x1r + _mul4r; _add5i = x1i + _mul4i;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_add5r, _add5i, k, &_pow6r, &_pow6i);
        double _call7r = 0, _call7i = 0;
        /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='math', ctx=Load()), attr='factorial', ctx=Load()) */
        double _div8r = 0, _div8i = 0;
        c_div(_pow6r, _pow6i, _call7r, _call7i, &_div8r, &_div8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 1.0;
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(k, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_c9r, _c9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _exp14r = 0, _exp14i = 0;
        c_exp2(_mul13r, _mul13i, &_exp14r, &_exp14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_div8r, _div8i, _exp14r, _exp14i, &_mul15r, &_mul15i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    }
    double _c16r = 0, _c16i = 0;
    _c16r = 3.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow17r, &_pow17i);
    c_mul(_pow17r, _pow17i, x1r, x1i, &_pow17r, &_pow17i);
    double _c18r = 0, _c18i = 0;
    _c18r = 0.0; _c18i = 1.0;
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow20r, &_pow20i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c18r, _c18i, _pow20r, _pow20i, &_mul21r, &_mul21i);
    double _sub22r = 0, _sub22i = 0;
    _sub22r = _pow17r - _mul21r; _sub22i = _pow17i - _mul21i;
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow24r, &_pow24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _sub22r + _pow24r; _add25i = _sub22i + _pow24i;
    double _c26r = 0, _c26i = 0;
    _c26r = 0.0; _c26i = 1.0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c26r, _c26i, x2r, x2i, &_mul27r, &_mul27i);
    double _sub28r = 0, _sub28i = 0;
    _sub28r = _add25r - _mul27r; _sub28i = _add25i - _mul27i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub28r; cIm[_idx] = _sub28i; } }
    double _attr29r = 0, _attr29i = 0;
    _attr29r = x2r; _attr29i = 0;
    double _attr30r = 0, _attr30i = 0;
    _attr30r = x1i; _attr30i = 0;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_attr29r, _attr29i, _attr30r, _attr30i, &_mul31r, &_mul31i);
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 1.0;
    double _c33r = 0, _c33i = 0;
    _c33r = 3.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow34r, &_pow34i);
    c_mul(_pow34r, _pow34i, x2r, x2i, &_pow34r, &_pow34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c32r, _c32i, _pow34r, _pow34i, &_mul35r, &_mul35i);
    double _sub36r = 0, _sub36i = 0;
    _sub36r = _mul31r - _mul35r; _sub36i = _mul31i - _mul35i;
    { int _idx = 4; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    double _attr37r = 0, _attr37i = 0;
    _attr37r = x1r; _attr37i = 0;
    double _c38r = 0, _c38i = 0;
    _c38r = 2.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(_attr37r, _attr37i, _attr37r, _attr37i, &_pow39r, &_pow39i);
    double _attr40r = 0, _attr40i = 0;
    _attr40r = x2r; _attr40i = 0;
    double _c41r = 0, _c41i = 0;
    _c41r = 2.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_mul(_attr40r, _attr40i, _attr40r, _attr40i, &_pow42r, &_pow42i);
    double _mul43r = 0, _mul43i = 0;
    c_mul(_pow39r, _pow39i, _pow42r, _pow42i, &_mul43r, &_mul43i);
    double _c44r = 0, _c44i = 0;
    _c44r = 0.0; _c44i = 1.0;
    double _attr45r = 0, _attr45i = 0;
    _attr45r = x1r; _attr45i = 0;
    double _attr46r = 0, _attr46i = 0;
    _attr46r = x2r; _attr46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = _attr45r + _attr46r; _add47i = _attr45i + _attr46i;
    double _mul48r = 0, _mul48i = 0;
    c_mul(_c44r, _c44i, _add47r, _add47i, &_mul48r, &_mul48i);
    double _exp49r = 0, _exp49i = 0;
    c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
    double _mul50r = 0, _mul50i = 0;
    c_mul(_mul43r, _mul43i, _exp49r, _exp49i, &_mul50r, &_mul50i);
    { int _idx = 9; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_743_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _mul2r = 0, _mul2i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
    double _sin3r = 0, _sin3i = 0;
    c_sin(_mul2r, _mul2i, &_sin3r, &_sin3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 0.0; _c4i = 1.0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c4r, _c4i, x1r, x1i, &_mul5r, &_mul5i);
    double _exp6r = 0, _exp6i = 0;
    c_exp2(_mul5r, _mul5i, &_exp6r, &_exp6i);
    double _mul7r = 0, _mul7i = 0;
    c_mul(_sin3r, _sin3i, _exp6r, _exp6i, &_mul7r, &_mul7i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 2.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow9r, &_pow9i);
    double _c10r = 0, _c10i = 0;
    _c10r = 2.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow11r, &_pow11i);
    double _add12r = 0, _add12i = 0;
    _add12r = _pow9r + _pow11r; _add12i = _pow9i + _pow11i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
    double _c13r = 0, _c13i = 0;
    _c13r = 0.0; _c13i = 1.0;
    double _c14r = 0, _c14i = 0;
    _c14r = 2.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow15r, &_pow15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c13r, _c13i, _pow15r, _pow15i, &_mul16r, &_mul16i);
    double _exp17r = 0, _exp17i = 0;
    c_exp2(_mul16r, _mul16i, &_exp17r, &_exp17i);
    double _cos18r = 0, _cos18i = 0;
    c_cos(x2r, x2i, &_cos18r, &_cos18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_exp17r, _exp17i, _cos18r, _cos18i, &_mul19r, &_mul19i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 1.0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c20r, _c20i, x2r, x2i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = x1r + _mul21r; _add22i = x1i + _mul21i;
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c23r, _c23i, x2r, x2i, &_mul24r, &_mul24i);
    double _sub25r = 0, _sub25i = 0;
    _sub25r = x1r - _mul24r; _sub25i = x1i - _mul24i;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_add22r, _add22i, _sub25r, _sub25i, &_mul26r, &_mul26i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    double _c27r = 0, _c27i = 0;
    _c27r = 0.0; _c27i = 1.0;
    double _mul28r = 0, _mul28i = 0;
    c_mul(_c27r, _c27i, x1r, x1i, &_mul28r, &_mul28i);
    double _add29r = 0, _add29i = 0;
    _add29r = x2r + _mul28r; _add29i = x2i + _mul28i;
    double _c30r = 0, _c30i = 0;
    _c30r = 3.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(_add29r, _add29i, _add29r, _add29i, &_pow31r, &_pow31i);
    c_mul(_pow31r, _pow31i, _add29r, _add29i, &_pow31r, &_pow31i);
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 1.0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c32r, _c32i, x2r, x2i, &_mul33r, &_mul33i);
    double _sub34r = 0, _sub34i = 0;
    _sub34r = x1r - _mul33r; _sub34i = x1i - _mul33i;
    double _c35r = 0, _c35i = 0;
    _c35r = 3.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(_sub34r, _sub34i, _sub34r, _sub34i, &_pow36r, &_pow36i);
    c_mul(_pow36r, _pow36i, _sub34r, _sub34i, &_pow36r, &_pow36i);
    double _sub37r = 0, _sub37i = 0;
    _sub37r = _pow31r - _pow36r; _sub37i = _pow31i - _pow36i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub37r; cIm[_idx] = _sub37i; } }
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _neg39r = 0, _neg39i = 0;
    _neg39r = -(_c38r); _neg39i = -(_c38i);
    double _c40r = 0, _c40i = 0;
    _c40r = 2.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow41r, &_pow41i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_neg39r, _neg39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
    double _exp43r = 0, _exp43i = 0;
    c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
    double _sin44r = 0, _sin44i = 0;
    c_sin(x1r, x1i, &_sin44r, &_sin44i);
    double _mul45r = 0, _mul45i = 0;
    c_mul(_exp43r, _exp43i, _sin44r, _sin44i, &_mul45r, &_mul45i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    double _c46r = 0, _c46i = 0;
    _c46r = 2.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow47r, &_pow47i);
    double _c48r = 0, _c48i = 0;
    _c48r = 2.0; _c48i = 0;
    double _pow49r = 0, _pow49i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow49r, &_pow49i);
    double _mul50r = 0, _mul50i = 0;
    c_mul(_pow47r, _pow47i, _pow49r, _pow49i, &_mul50r, &_mul50i);
    double _c51r = 0, _c51i = 0;
    _c51r = 0.0; _c51i = 1.0;
    double _attr52r = 0, _attr52i = 0;
    _attr52r = x1r; _attr52i = 0;
    double _attr53r = 0, _attr53i = 0;
    _attr53r = x2i; _attr53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = _attr52r + _attr53r; _add54i = _attr52i + _attr53i;
    double _mul55r = 0, _mul55i = 0;
    c_mul(_c51r, _c51i, _add54r, _add54i, &_mul55r, &_mul55i);
    double _exp56r = 0, _exp56i = 0;
    c_exp2(_mul55r, _mul55i, &_exp56r, &_exp56i);
    double _mul57r = 0, _mul57i = 0;
    c_mul(_mul50r, _mul50i, _exp56r, _exp56i, &_mul57r, &_mul57i);
    double _re58r = 0, _re58i = 0;
    _re58r = _mul57r; _re58i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _re58r; cIm[_idx] = _re58i; } }
    double _c59r = 0, _c59i = 0;
    _c59r = 100.0; _c59i = 0;
    double _sin60r = 0, _sin60i = 0;
    c_sin(x1r, x1i, &_sin60r, &_sin60i);
    double _mul61r = 0, _mul61i = 0;
    c_mul(_c59r, _c59i, _sin60r, _sin60i, &_mul61r, &_mul61i);
    double _cos62r = 0, _cos62i = 0;
    c_cos(x2r, x2i, &_cos62r, &_cos62i);
    double _mul63r = 0, _mul63i = 0;
    c_mul(_mul61r, _mul61i, _cos62r, _cos62i, &_mul63r, &_mul63i);
    double _c64r = 0, _c64i = 0;
    _c64r = 0.0; _c64i = 100.0;
    double _sin65r = 0, _sin65i = 0;
    c_sin(x2r, x2i, &_sin65r, &_sin65i);
    double _mul66r = 0, _mul66i = 0;
    c_mul(_c64r, _c64i, _sin65r, _sin65i, &_mul66r, &_mul66i);
    double _cos67r = 0, _cos67i = 0;
    c_cos(x1r, x1i, &_cos67r, &_cos67i);
    double _mul68r = 0, _mul68i = 0;
    c_mul(_mul66r, _mul66i, _cos67r, _cos67i, &_mul68r, &_mul68i);
    double _sub69r = 0, _sub69i = 0;
    _sub69r = _mul63r - _mul68r; _sub69i = _mul63i - _mul68i;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub69r; cIm[_idx] = _sub69i; } }
    double _c70r = 0, _c70i = 0;
    _c70r = 0.0; _c70i = 1.0;
    double _c71r = 0, _c71i = 0;
    _c71r = 3.0; _c71i = 0;
    double _pow72r = 0, _pow72i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow72r, &_pow72i);
    c_mul(_pow72r, _pow72i, x1r, x1i, &_pow72r, &_pow72i);
    double _c73r = 0, _c73i = 0;
    _c73r = 3.0; _c73i = 0;
    double _pow74r = 0, _pow74i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow74r, &_pow74i);
    c_mul(_pow74r, _pow74i, x2r, x2i, &_pow74r, &_pow74i);
    double _sub75r = 0, _sub75i = 0;
    _sub75r = _pow72r - _pow74r; _sub75i = _pow72i - _pow74i;
    double _mul76r = 0, _mul76i = 0;
    c_mul(_c70r, _c70i, _sub75r, _sub75i, &_mul76r, &_mul76i);
    double _c77r = 0, _c77i = 0;
    _c77r = 100.0; _c77i = 0;
    double _c78r = 0, _c78i = 0;
    _c78r = 0.0; _c78i = 1.0;
    double _sub79r = 0, _sub79i = 0;
    _sub79r = _c77r - _c78r; _sub79i = _c77i - _c78i;
    double _add80r = 0, _add80i = 0;
    _add80r = _mul76r + _sub79r; _add80i = _mul76i + _sub79i;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add80r; cIm[_idx] = _add80i; } }
    double _add81r = 0, _add81i = 0;
    _add81r = x1r + x2r; _add81i = x1i + x2i;
    double _c82r = 0, _c82i = 0;
    _c82r = 0.0; _c82i = 2.0;
    double _mul83r = 0, _mul83i = 0;
    c_mul(_c82r, _c82i, x1r, x1i, &_mul83r, &_mul83i);
    double _mul84r = 0, _mul84i = 0;
    c_mul(_mul83r, _mul83i, x2r, x2i, &_mul84r, &_mul84i);
    double _exp85r = 0, _exp85i = 0;
    c_exp2(_mul84r, _mul84i, &_exp85r, &_exp85i);
    double _mul86r = 0, _mul86i = 0;
    c_mul(_add81r, _add81i, _exp85r, _exp85i, &_mul86r, &_mul86i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul86r; cIm[_idx] = _mul86i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_744_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 3.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow2r, &_pow2i);
    c_mul(_pow2r, _pow2i, x1r, x1i, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, x2r, x2i, &_pow4r, &_pow4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _pow2r + _pow4r; _add5i = _pow2i + _pow4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 11.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = x1r + x2r; _add7i = x1i + x2i;
    double _c8r = 0, _c8i = 0;
    _c8r = 9.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_powr(_add7r, _add7i, 9.0, &_pow9r, &_pow9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c6r, _c6i, _pow9r, _pow9i, &_mul10r, &_mul10i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 0.0; _c11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 0.0; _c12i = 1.0;
    double _add13r = 0, _add13i = 0;
    _add13r = _c11r + _c12r; _add13i = _c11i + _c12i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add13r; cIm[_idx] = _add13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 0.0; _c14i = 1.0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c14r, _c14i, x1r, x1i, &_mul15r, &_mul15i);
    double _exp16r = 0, _exp16i = 0;
    c_exp2(_mul15r, _mul15i, &_exp16r, &_exp16i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp16r; cIm[_idx] = _exp16i; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 100.0; _c17i = 0;
    double _sin18r = 0, _sin18i = 0;
    c_sin(x2r, x2i, &_sin18r, &_sin18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c17r, _c17i, _sin18r, _sin18i, &_mul19r, &_mul19i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    double _attr20r = 0, _attr20i = 0;
    _attr20r = x1r; _attr20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 1.0;
    double _attr22r = 0, _attr22i = 0;
    _attr22r = x2i; _attr22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c21r, _c21i, _attr22r, _attr22i, &_mul23r, &_mul23i);
    double _sub24r = 0, _sub24i = 0;
    _sub24r = _attr20r - _mul23r; _sub24i = _attr20i - _mul23i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub24r; cIm[_idx] = _sub24i; } }
    double _c25r = 0, _c25i = 0;
    _c25r = 0.0; _c25i = 11.0;
    double _attr26r = 0, _attr26i = 0;
    _attr26r = x2r; _attr26i = 0;
    double _attr27r = 0, _attr27i = 0;
    _attr27r = x1i; _attr27i = 0;
    double _c28r = 0, _c28i = 0;
    _c28r = 0.1; _c28i = 0;
    double _add29r = 0, _add29i = 0;
    _add29r = _attr27r + _c28r; _add29i = _attr27i + _c28i;
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(_add29r, _add29i); _abs30i = 0;
    double _div31r = 0, _div31i = 0;
    c_div(_attr26r, _attr26i, _abs30r, _abs30i, &_div31r, &_div31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c25r, _c25i, _div31r, _div31i, &_mul32r, &_mul32i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    double _attr33r = 0, _attr33i = 0;
    _attr33r = x1r; _attr33i = 0;
    double _attr34r = 0, _attr34i = 0;
    _attr34r = x1r; _attr34i = 0;
    double _attr35r = 0, _attr35i = 0;
    _attr35r = x2r; _attr35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _attr34r + _attr35r; _add36i = _attr34i + _attr35i;
    double _abs37r = 0, _abs37i = 0;
    _abs37r = c_abs(_add36r, _add36i); _abs37i = 0;
    double _c38r = 0, _c38i = 0;
    _c38r = 0.125; _c38i = 0;
    double _add39r = 0, _add39i = 0;
    _add39r = _abs37r + _c38r; _add39i = _abs37i + _c38i;
    double _div40r = 0, _div40i = 0;
    c_div(_attr33r, _attr33i, _add39r, _add39i, &_div40r, &_div40i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div40r; cIm[_idx] = _div40i; } }
    double _c41r = 0, _c41i = 0;
    _c41r = 0.0; _c41i = 1.0;
    double _attr42r = 0, _attr42i = 0;
    _attr42r = x1r; _attr42i = 0;
    double _mul43r = 0, _mul43i = 0;
    c_mul(_c41r, _c41i, _attr42r, _attr42i, &_mul43r, &_mul43i);
    double _attr44r = 0, _attr44i = 0;
    _attr44r = x2r; _attr44i = 0;
    double _mul45r = 0, _mul45i = 0;
    c_mul(_mul43r, _mul43i, _attr44r, _attr44i, &_mul45r, &_mul45i);
    double _exp46r = 0, _exp46i = 0;
    c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp46r; cIm[_idx] = _exp46i; } }
    double _mul47r = 0, _mul47i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul47r, &_mul47i);
    double _abs48r = 0, _abs48i = 0;
    _abs48r = c_abs(_mul47r, _mul47i); _abs48i = 0;
    double _c49r = 0, _c49i = 0;
    _c49r = 0.0; _c49i = 1.0;
    double _ang50r = 0, _ang50i = 0;
    _ang50r = c_arg(x1r, x1i); _ang50i = 0;
    double _ang51r = 0, _ang51i = 0;
    _ang51r = c_arg(x2r, x2i); _ang51i = 0;
    double _sub52r = 0, _sub52i = 0;
    _sub52r = _ang50r - _ang51r; _sub52i = _ang50i - _ang51i;
    double _mul53r = 0, _mul53i = 0;
    c_mul(_c49r, _c49i, _sub52r, _sub52i, &_mul53r, &_mul53i);
    double _exp54r = 0, _exp54i = 0;
    c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
    double _mul55r = 0, _mul55i = 0;
    c_mul(_abs48r, _abs48i, _exp54r, _exp54i, &_mul55r, &_mul55i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    double _attr56r = 0, _attr56i = 0;
    _attr56r = x1r; _attr56i = 0;
    double _attr57r = 0, _attr57i = 0;
    _attr57r = x2i; _attr57i = 0;
    double _mul58r = 0, _mul58i = 0;
    c_mul(_attr56r, _attr56i, _attr57r, _attr57i, &_mul58r, &_mul58i);
    double _c59r = 0, _c59i = 0;
    _c59r = 0.0; _c59i = 10.0;
    double _add60r = 0, _add60i = 0;
    _add60r = _mul58r + _c59r; _add60i = _mul58i + _c59i;
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add60r; cIm[_idx] = _add60i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_745_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2i; _attr2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_attr1r, _attr1i, _attr2r, _attr2i, &_mul3r, &_mul3i);
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2r; _attr5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_attr4r, _attr4i, _attr5r, _attr5i, &_mul6r, &_mul6i);
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul6r; cIm[_idx] = _mul6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 0.0; _c7i = 1.0;
    double _add8r = 0, _add8i = 0;
    _add8r = x1r + x2r; _add8i = x1i + x2i;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c7r, _c7i, _add8r, _add8i, &_mul9r, &_mul9i);
    double _exp10r = 0, _exp10i = 0;
    c_exp2(_mul9r, _mul9i, &_exp10r, &_exp10i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _exp10r; cIm[_idx] = _exp10i; } }
    double _add11r = 0, _add11i = 0;
    _add11r = x1r + x2r; _add11i = x1i + x2i;
    double _abs12r = 0, _abs12i = 0;
    _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
    double m = _abs12r; /* +_abs12ii */
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 1.0; _c14i = 0;
    double _add15r = 0, _add15i = 0;
    _add15r = m + _c14r; _add15i = 0 + _c14i;
    double _div16r = 0, _div16i = 0;
    c_div(_c13r, _c13i, _add15r, _add15i, &_div16r, &_div16i);
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _div16r; cIm[_idx] = _div16i; } }
    double _attr17r = 0, _attr17i = 0;
    _attr17r = x1r; _attr17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(_attr17r, _attr17i, _attr17r, _attr17i, &_pow19r, &_pow19i);
    double _attr20r = 0, _attr20i = 0;
    _attr20r = x1i; _attr20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(_attr20r, _attr20i, _attr20r, _attr20i, &_pow22r, &_pow22i);
    double _add23r = 0, _add23i = 0;
    _add23r = _pow19r + _pow22r; _add23i = _pow19i + _pow22i;
    double _sqrt24r = 0, _sqrt24i = 0;
    c_powr(_add23r, _add23i, 0.5, &_sqrt24r, &_sqrt24i);
    double _attr25r = 0, _attr25i = 0;
    _attr25r = x2r; _attr25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(_attr25r, _attr25i, _attr25r, _attr25i, &_pow27r, &_pow27i);
    double _attr28r = 0, _attr28i = 0;
    _attr28r = x2i; _attr28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_attr28r, _attr28i, _attr28r, _attr28i, &_pow30r, &_pow30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _pow27r + _pow30r; _add31i = _pow27i + _pow30i;
    double _sqrt32r = 0, _sqrt32i = 0;
    c_powr(_add31r, _add31i, 0.5, &_sqrt32r, &_sqrt32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_sqrt24r, _sqrt24i, _sqrt32r, _sqrt32i, &_mul33r, &_mul33i);
    double polar_coordinates = _mul33r; /* +_mul33ii */
    double _c34r = 0, _c34i = 0;
    _c34r = 0.0; _c34i = 1.0;
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c34r, _c34i, polar_coordinates, 0, &_mul35r, &_mul35i);
    double _exp36r = 0, _exp36i = 0;
    c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _exp36r; cIm[_idx] = _exp36i; } }
    double _c37r = 0, _c37i = 0;
    _c37r = 2.0; _c37i = 0;
    double _pow38r = 0, _pow38i = 0;
    c_mul(0, 0, 0, 0, &_pow38r, &_pow38i);
    double _attr39r = 0, _attr39i = 0;
    _attr39r = x1r; _attr39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_pow38r, _pow38i, _attr39r, _attr39i, &_mul40r, &_mul40i);
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    double _c41r = 0, _c41i = 0;
    _c41r = 10.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_powr(x2r, x2i, 10.0, &_pow42r, &_pow42i);
    double _cf43r = 0, _cf43i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 10.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_powr(_cf43r, _cf43i, 10.0, &_pow45r, &_pow45i);
    double _sub46r = 0, _sub46i = 0;
    _sub46r = _pow42r - _pow45r; _sub46i = _pow42i - _pow45i;
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub46r; cIm[_idx] = _sub46i; } }
    double _ang47r = 0, _ang47i = 0;
    _ang47r = c_arg(x1r, x1i); _ang47i = 0;
    double _cf48r = 0, _cf48i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
    double _mul49r = 0, _mul49i = 0;
    c_mul(_ang47r, _ang47i, _cf48r, _cf48i, &_mul49r, &_mul49i);
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    double _cf50r = 0, _cf50i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
    double _cf51r = 0, _cf51i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
    double _cf52r = 0, _cf52i = 0;
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
    double _mul53r = 0, _mul53i = 0;
    c_mul(_cf51r, _cf51i, _cf52r, _cf52i, &_mul53r, &_mul53i);
    double _add54r = 0, _add54i = 0;
    _add54r = _cf50r + _mul53r; _add54i = _cf50i + _mul53i;
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add54r; cIm[_idx] = _add54i; } }
    double _cf55r = 0, _cf55i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
    double _conj56r = 0, _conj56i = 0;
    _conj56r = _cf55r; _conj56i = -(_cf55i);
    double _cf57r = 0, _cf57i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
    double _mul58r = 0, _mul58i = 0;
    c_mul(_conj56r, _conj56i, _cf57r, _cf57i, &_mul58r, &_mul58i);
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_746_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _neg2r = 0, _neg2i = 0;
    _neg2r = -(_c1r); _neg2i = -(_c1i);
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _re4r = 0, _re4i = 0;
    _re4r = x1r; _re4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c3r, _c3i, _re4r, _re4i, &_mul5r, &_mul5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _neg2r + _mul5r; _add6i = _neg2i + _mul5i;
    double _c7r = 0, _c7i = 0;
    _c7r = 1.0; _c7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 2.0; _c8i = 0;
    double _im9r = 0, _im9i = 0;
    _im9r = x2i; _im9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c8r, _c8i, _im9r, _im9i, &_mul10r, &_mul10i);
    double _sub11r = 0, _sub11i = 0;
    _sub11r = _c7r - _mul10r; _sub11i = _c7i - _mul10i;
    double coeff_sequence[11];
    for (int _li = 0; _li < 11; _li++) {
        coeff_sequence[_li] = _add6r + (_sub11r - _add6r) * _li / 10.0;
    }
    for (int i = 0; i < 11; i++) {
        double _c12r = 0, _c12i = 0;
        _c12r = 0.0; _c12i = 1.0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = i + _c13r; _add14i = 0 + _c13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_c12r, _c12i, _add14r, _add14i, &_mul15r, &_mul15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_mul15r, _mul15i, x1r, x1i, &_mul16r, &_mul16i);
        double _exp17r = 0, _exp17i = 0;
        c_exp2(_mul16r, _mul16i, &_exp17r, &_exp17i);
        double _unk18r = 0, _unk18i = 0;
        /* WARNING: unhandled node Slice(upper=BinOp(left=Name(id='i', ctx=Load()), op=Add(), right=Constant(value=1))) */
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (int)(_unk18r); _arr19r = (_idx >= 0 && _idx < 11) ? coeff_sequence[_idx] : 0.0; _arr19i = 0; }
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x2r, x2i); _abs21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _c20r + _abs21r; _add22i = _c20i + _abs21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_mul(_log23r, _log23i, _log23r, _log23i, &_pow25r, &_pow25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_arr19r, _arr19i, _pow25r, _pow25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _exp17r + _mul26r; _add27i = _exp17i + _mul26i;
        { int _idx = i; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
    }
    double _cf28r = 0, _cf28i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf28r = cRe[_idx]; _cf28i = cIm[_idx]; } }
    double _cf29r = 0, _cf29i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
    double _mul30r = 0, _mul30i = 0;
    c_mul(_cf28r, _cf28i, _cf29r, _cf29i, &_mul30r, &_mul30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(_mul30r, _mul30i, x1r, x1i, &_mul31r, &_mul31i);
    double _sqrt32r = 0, _sqrt32i = 0;
    c_powr(_mul31r, _mul31i, 0.5, &_sqrt32r, &_sqrt32i);
    cRe[10] += _sqrt32r; cIm[10] += _sqrt32i;
    double _cf33r = 0, _cf33i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
    double _cf34r = 0, _cf34i = 0;
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
    double _mul35r = 0, _mul35i = 0;
    c_mul(_cf33r, _cf33i, _cf34r, _cf34i, &_mul35r, &_mul35i);
    double _mul36r = 0, _mul36i = 0;
    c_mul(_mul35r, _mul35i, x2r, x2i, &_mul36r, &_mul36i);
    double _sqrt37r = 0, _sqrt37i = 0;
    c_powr(_mul36r, _mul36i, 0.5, &_sqrt37r, &_sqrt37i);
    cRe[0] -= _sqrt37r; cIm[0] -= _sqrt37i;
    double _sum38r = 0, _sum38i = 0;
    _sum38r = 0; _sum38i = 0;
    for (int _si = 0; _si < 11; _si++) { _sum38r += cRe[_si]; _sum38i += cIm[_si]; }
    double _c39r = 0, _c39i = 0;
    _c39r = 11.0; _c39i = 0;
    double _div40r = 0, _div40i = 0;
    c_div(_sum38r, _sum38i, _c39r, _c39i, &_div40r, &_div40i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div40r; cIm[_idx] = _div40i; } }
    double _cf41r = 0, _cf41i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
    double _cf42r = 0, _cf42i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
    double _div43r = 0, _div43i = 0;
    c_div(_cf41r, _cf41i, _cf42r, _cf42i, &_div43r, &_div43i);
    { double _tr = cRe[2]*_div43r - cIm[2]*_div43i; cIm[2] = cRe[2]*_div43i + cIm[2]*_div43r; cRe[2] = _tr; }
    double _cf44r = 0, _cf44i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { _cf44r = cRe[_idx]; _cf44i = cIm[_idx]; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 2.0; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_mul(_cf44r, _cf44i, _cf44r, _cf44i, &_pow46r, &_pow46i);
    double _cf47r = 0, _cf47i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { _cf47r = cRe[_idx]; _cf47i = cIm[_idx]; } }
    double _sub48r = 0, _sub48i = 0;
    _sub48r = _pow46r - _cf47r; _sub48i = _pow46i - _cf47i;
    double _cf49r = 0, _cf49i = 0;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { _cf49r = cRe[_idx]; _cf49i = cIm[_idx]; } }
    double _add50r = 0, _add50i = 0;
    _add50r = _sub48r + _cf49r; _add50i = _sub48i + _cf49i;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_747_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 11; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 1.0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(x2r, x2i, _c1r, _c1i, &_mul2r, &_mul2i);
        double _add3r = 0, _add3i = 0;
        _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_add3r, _add3i, _add5r, &_pow6r, &_pow6i);
        double _exp7r = 0, _exp7i = 0;
        c_exp2(_pow6r, _pow6i, &_exp7r, &_exp7i);
        { int _idx = k; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp7r; cIm[_idx] = _exp7i; } }
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(k, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        if (_mod9r == _c10r) {
            double _im11r = 0, _im11i = 0;
            _im11r = x2i; _im11i = 0;
            double _add12r = 0, _add12i = 0;
            _add12r = k + _im11r; _add12i = 0 + _im11i;
            double _cos13r = 0, _cos13i = 0;
            c_cos(_add12r, _add12i, &_cos13r, &_cos13i);
            { double _tr = cRe[k]*_cos13r - cIm[k]*_cos13i; cIm[k] = cRe[k]*_cos13i + cIm[k]*_cos13r; cRe[k] = _tr; }
        } else {
            double _re14r = 0, _re14i = 0;
            _re14r = x1r; _re14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = k + _re14r; _add15i = 0 + _re14i;
            double _sin16r = 0, _sin16i = 0;
            c_sin(_add15r, _add15i, &_sin16r, &_sin16i);
            { double _tr = cRe[k]*_sin16r - cIm[k]*_sin16i; cIm[k] = cRe[k]*_sin16i + cIm[k]*_sin16r; cRe[k] = _tr; }
        }
        double _mul17r = 0, _mul17i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 1.0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul17r, _mul17i, _c18r, _c18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = k + _c20r; _add21i = 0 + _c20i;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(_add21r, _add21i, _add21r, _add21i, &_pow23r, &_pow23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul19r, _mul19i, _pow23r, _pow23i, &_mul24r, &_mul24i);
        cRe[k] += _mul24r; cIm[k] += _mul24i;
        double _cf25r = 0, _cf25i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 11) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        if (_cf25r == _c26r) {
            double _c27r = 0, _c27i = 0;
            _c27r = 0.0; _c27i = 1.0;
            double _neg28r = 0, _neg28i = 0;
            _neg28r = -(_c27r); _neg28i = -(_c27i);
            { int _idx = k; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _neg28r; cIm[_idx] = _neg28i; } }
        }
    }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_748_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(_add1r, _add1i, _add1r, _add1i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul4r, &_mul4i);
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(_mul4r, _mul4i); _abs5i = 0;
    double _sqrt6r = 0, _sqrt6i = 0;
    c_powr(_abs5r, _abs5i, 0.5, &_sqrt6r, &_sqrt6i);
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _pow3r - _sqrt6r; _sub7i = _pow3i - _sqrt6i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub7r; cIm[_idx] = _sub7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 1.0; _c8i = 0;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(x2r, x2i); _abs9i = 0;
    double _add10r = 0, _add10i = 0;
    _add10r = _c8r + _abs9r; _add10i = _c8i + _abs9i;
    double _log11r = 0, _log11i = 0;
    c_log(_add10r, _add10i, &_log11r, &_log11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 0.0; _c12i = 1.0;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c12r, _c12i, x1r, x1i, &_mul13r, &_mul13i);
    double _exp14r = 0, _exp14i = 0;
    c_exp2(_mul13r, _mul13i, &_exp14r, &_exp14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_log11r, _log11i, _exp14r, _exp14i, &_mul15r, &_mul15i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _re16r = 0, _re16i = 0;
    _re16r = x1r; _re16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(_re16r, _re16i, _re16r, _re16i, &_pow18r, &_pow18i);
    double _im19r = 0, _im19i = 0;
    _im19r = x2i; _im19i = 0;
    double _c20r = 0, _c20i = 0;
    _c20r = 2.0; _c20i = 0;
    double _pow21r = 0, _pow21i = 0;
    c_mul(_im19r, _im19i, _im19r, _im19i, &_pow21r, &_pow21i);
    double _sub22r = 0, _sub22i = 0;
    _sub22r = _pow18r - _pow21r; _sub22i = _pow18i - _pow21i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub22r; cIm[_idx] = _sub22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _re24r = 0, _re24i = 0;
    _re24r = x1r; _re24i = 0;
    double _re25r = 0, _re25i = 0;
    _re25r = x2r; _re25i = 0;
    double _add26r = 0, _add26i = 0;
    _add26r = _re24r + _re25r; _add26i = _re24i + _re25i;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c23r, _c23i, _add26r, _add26i, &_mul27r, &_mul27i);
    double _exp28r = 0, _exp28i = 0;
    c_exp2(_mul27r, _mul27i, &_exp28r, &_exp28i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp28r; cIm[_idx] = _exp28i; } }
    double _c29r = 0, _c29i = 0;
    _c29r = 100.0; _c29i = 0;
    double _sub30r = 0, _sub30i = 0;
    _sub30r = x1r - x2r; _sub30i = x1i - x2i;
    double _abs31r = 0, _abs31i = 0;
    _abs31r = c_abs(_sub30r, _sub30i); _abs31i = 0;
    double _neg32r = 0, _neg32i = 0;
    _neg32r = -(_abs31r); _neg32i = -(_abs31i);
    double _exp33r = 0, _exp33i = 0;
    c_exp2(_neg32r, _neg32i, &_exp33r, &_exp33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c29r, _c29i, _exp33r, _exp33i, &_mul34r, &_mul34i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    double _im35r = 0, _im35i = 0;
    _im35r = x1i; _im35i = 0;
    double _c36r = 0, _c36i = 0;
    _c36r = 3.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(_im35r, _im35i, _im35r, _im35i, &_pow37r, &_pow37i);
    c_mul(_pow37r, _pow37i, _im35r, _im35i, &_pow37r, &_pow37i);
    double _re38r = 0, _re38i = 0;
    _re38r = x2r; _re38i = 0;
    double _c39r = 0, _c39i = 0;
    _c39r = 3.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(_re38r, _re38i, _re38r, _re38i, &_pow40r, &_pow40i);
    c_mul(_pow40r, _pow40i, _re38r, _re38i, &_pow40r, &_pow40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _pow37r - _pow40r; _sub41i = _pow37i - _pow40i;
    double _c42r = 0, _c42i = 0;
    _c42r = 0.0; _c42i = 1.0;
    double _re43r = 0, _re43i = 0;
    _re43r = x1r; _re43i = 0;
    double _im44r = 0, _im44i = 0;
    _im44r = x2i; _im44i = 0;
    double _add45r = 0, _add45i = 0;
    _add45r = _re43r + _im44r; _add45i = _re43i + _im44i;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c42r, _c42i, _add45r, _add45i, &_mul46r, &_mul46i);
    double _exp47r = 0, _exp47i = 0;
    c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
    double _mul48r = 0, _mul48i = 0;
    c_mul(_sub41r, _sub41i, _exp47r, _exp47i, &_mul48r, &_mul48i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    double _sin49r = 0, _sin49i = 0;
    c_sin(x1r, x1i, &_sin49r, &_sin49i);
    double _cos50r = 0, _cos50i = 0;
    c_cos(x2r, x2i, &_cos50r, &_cos50i);
    double _sub51r = 0, _sub51i = 0;
    _sub51r = _sin49r - _cos50r; _sub51i = _sin49i - _cos50i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub51r; cIm[_idx] = _sub51i; } }
    double _c52r = 0, _c52i = 0;
    _c52r = 2.0; _c52i = 0;
    double _pow53r = 0, _pow53i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow53r, &_pow53i);
    double _c54r = 0, _c54i = 0;
    _c54r = 2.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow55r, &_pow55i);
    double _sub56r = 0, _sub56i = 0;
    _sub56r = _pow53r - _pow55r; _sub56i = _pow53i - _pow55i;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub56r; cIm[_idx] = _sub56i; } }
    double _c57r = 0, _c57i = 0;
    _c57r = 10.0; _c57i = 0;
    double _neg58r = 0, _neg58i = 0;
    _neg58r = -(_c57r); _neg58i = -(_c57i);
    double _c59r = 0, _c59i = 0;
    _c59r = 0.0; _c59i = 1.0;
    double _sub60r = 0, _sub60i = 0;
    _sub60r = x1r - x2r; _sub60i = x1i - x2i;
    double _c61r = 0, _c61i = 0;
    _c61r = 2.0; _c61i = 0;
    double _pow62r = 0, _pow62i = 0;
    c_mul(_sub60r, _sub60i, _sub60r, _sub60i, &_pow62r, &_pow62i);
    double _mul63r = 0, _mul63i = 0;
    c_mul(_c59r, _c59i, _pow62r, _pow62i, &_mul63r, &_mul63i);
    double _exp64r = 0, _exp64i = 0;
    c_exp2(_mul63r, _mul63i, &_exp64r, &_exp64i);
    double _mul65r = 0, _mul65i = 0;
    c_mul(_neg58r, _neg58i, _exp64r, _exp64i, &_mul65r, &_mul65i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul65r; cIm[_idx] = _mul65i; } }
    double _c66r = 0, _c66i = 0;
    _c66r = 0.0; _c66i = 1.0;
    double _mul67r = 0, _mul67i = 0;
    c_mul(_c66r, _c66i, x1r, x1i, &_mul67r, &_mul67i);
    double _mul68r = 0, _mul68i = 0;
    c_mul(_mul67r, _mul67i, x2r, x2i, &_mul68r, &_mul68i);
    double _exp69r = 0, _exp69i = 0;
    c_exp2(_mul68r, _mul68i, &_exp69r, &_exp69i);
    double _mul70r = 0, _mul70i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul70r, &_mul70i);
    double _sin71r = 0, _sin71i = 0;
    c_sin(_mul70r, _mul70i, &_sin71r, &_sin71i);
    double _sub72r = 0, _sub72i = 0;
    _sub72r = _exp69r - _sin71r; _sub72i = _exp69i - _sin71i;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub72r; cIm[_idx] = _sub72i; } }
    double _add73r = 0, _add73i = 0;
    _add73r = x1r + x2r; _add73i = x1i + x2i;
    double _c74r = 0, _c74i = 0;
    _c74r = 3.0; _c74i = 0;
    double _pow75r = 0, _pow75i = 0;
    c_mul(_add73r, _add73i, _add73r, _add73i, &_pow75r, &_pow75i);
    c_mul(_pow75r, _pow75i, _add73r, _add73i, &_pow75r, &_pow75i);
    double _c76r = 0, _c76i = 0;
    _c76r = 0.0; _c76i = 1.0;
    double _sub77r = 0, _sub77i = 0;
    _sub77r = x1r - x2r; _sub77i = x1i - x2i;
    double _mul78r = 0, _mul78i = 0;
    c_mul(_c76r, _c76i, _sub77r, _sub77i, &_mul78r, &_mul78i);
    double _exp79r = 0, _exp79i = 0;
    c_exp2(_mul78r, _mul78i, &_exp79r, &_exp79i);
    double _mul80r = 0, _mul80i = 0;
    c_mul(_pow75r, _pow75i, _exp79r, _exp79i, &_mul80r, &_mul80i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul80r; cIm[_idx] = _mul80i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_749_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _add2r = 0, _add2i = 0;
    _add2r = x1r + x2r; _add2i = x1i + x2i;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_c1r, _c1i, _add2r, _add2i, &_mul3r, &_mul3i);
    double _exp4r = 0, _exp4i = 0;
    c_exp2(_mul3r, _mul3i, &_exp4r, &_exp4i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _exp4r; cIm[_idx] = _exp4i; } }
    double _sin5r = 0, _sin5i = 0;
    c_sin(x1r, x1i, &_sin5r, &_sin5i);
    double _cos6r = 0, _cos6i = 0;
    c_cos(x2r, x2i, &_cos6r, &_cos6i);
    double _mul7r = 0, _mul7i = 0;
    c_mul(_sin5r, _sin5i, _cos6r, _cos6i, &_mul7r, &_mul7i);
    double _cos8r = 0, _cos8i = 0;
    c_cos(x1r, x1i, &_cos8r, &_cos8i);
    double _sin9r = 0, _sin9i = 0;
    c_sin(x2r, x2i, &_sin9r, &_sin9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_cos8r, _cos8i, _sin9r, _sin9i, &_mul10r, &_mul10i);
    double _sub11r = 0, _sub11i = 0;
    _sub11r = _mul7r - _mul10r; _sub11i = _mul7i - _mul10i;
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub11r; cIm[_idx] = _sub11i; } }
    for (int j = 2; j < 9; j++) {
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 3.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_add13r, _add13i, _c14r, _c14i, &_div15r, &_div15i);
        double _exp16r = 0, _exp16i = 0;
        c_exp2(_div15r, _div15i, &_exp16r, &_exp16i);
        double _add17r = 0, _add17i = 0;
        _add17r = x1r + x2r; _add17i = x1i + x2i;
        double _sin18r = 0, _sin18i = 0;
        c_sin(_add17r, _add17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_exp16r, _exp16i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _neg21r = 0, _neg21i = 0;
        _neg21r = -(_c20r); _neg21i = -(_c20i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = x1r - x2r; _sub22i = x1i - x2i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_neg21r, _neg21i, _sub22r, _sub22i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _div26r = 0, _div26i = 0;
        c_div(_mul23r, _mul23i, _add25r, _add25i, &_div26r, &_div26i);
        double _exp27r = 0, _exp27i = 0;
        c_exp2(_div26r, _div26i, &_exp27r, &_exp27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul19r, _mul19i, _exp27r, _exp27i, &_mul28r, &_mul28i);
        { int _idx = j; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow30r, &_pow30i);
    double _c31r = 0, _c31i = 0;
    _c31r = 2.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow32r, &_pow32i);
    double _add33r = 0, _add33i = 0;
    _add33r = _pow30r + _pow32r; _add33i = _pow30i + _pow32i;
    double _sqrt34r = 0, _sqrt34i = 0;
    c_powr(_add33r, _add33i, 0.5, &_sqrt34r, &_sqrt34i);
    double _np35r = 0, _np35i = 0;
    /* WARNING: unhandled np.log10 */
    double _sub36r = 0, _sub36i = 0;
    _sub36r = _sqrt34r - _np35r; _sub36i = _sqrt34i - _np35i;
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_750_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(_add1r, _add1i); _abs2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 100.0;
    double _add4r = 0, _add4i = 0;
    _add4r = _abs2r + _c3r; _add4i = _abs2i + _c3i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add4r; cIm[_idx] = _add4i; } }
    for (int k = 1; k < 10; k++) {
        double _cf5r = 0, _cf5i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 11) { _cf5r = cRe[_idx]; _cf5i = cIm[_idx]; } }
        double _c6r = 0, _c6i = 0;
        _c6r = 0.0; _c6i = 1.0;
        double _neg7r = 0, _neg7i = 0;
        _neg7r = -(_c6r); _neg7i = -(_c6i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_neg7r, _neg7i, x1r, x1i, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_c9r, _c9i, x2r, x2i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul8r + _mul10r; _add11i = _mul8i + _mul10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = k + _c12r; _add13i = 0 + _c12i;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_add11r, _add11i, _add13r, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_cf5r, _cf5i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = k + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_mul15r, _mul15i, _add17r, _add17i, &_div18r, &_div18i);
        { int _idx = k; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div18r; cIm[_idx] = _div18i; } }
    }
    double _cf19r = 0, _cf19i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 1.0;
    double _sum21r = 0, _sum21i = 0;
    _sum21r = 0; _sum21i = 0;
    for (int _si = 0; _si < -1; _si++) { _sum21r += cRe[_si]; _sum21i += cIm[_si]; }
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(_sum21r, _sum21i); _abs22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _abs22r, _abs22i, &_mul23r, &_mul23i);
    double _exp24r = 0, _exp24i = 0;
    c_exp2(_mul23r, _mul23i, &_exp24r, &_exp24i);
    double _mul25r = 0, _mul25i = 0;
    c_mul(_cf19r, _cf19i, _exp24r, _exp24i, &_mul25r, &_mul25i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_751_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_c2r, _c2i, M_PI, 0, &_mul3r, &_mul3i);
    double angle[11];
    for (int _li = 0; _li < 11; _li++) {
        angle[_li] = _c1r + (_mul3r - _c1r) * _li / 10.0;
    }
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + x2r; _add4i = x1i + x2i;
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(_add4r, _add4i); _abs5i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _abs5r; cIm[_idx] = _abs5i; } }
    double _sin6r = 0, _sin6i = 0;
    c_sin(x1r, x1i, &_sin6r, &_sin6i);
    double _cos7r = 0, _cos7i = 0;
    c_cos(x2r, x2i, &_cos7r, &_cos7i);
    double _c8r = 0, _c8i = 0;
    _c8r = 2.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_mul(_cos7r, _cos7i, _cos7r, _cos7i, &_pow9r, &_pow9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_sin6r, _sin6i, _pow9r, _pow9i, &_mul10r, &_mul10i);
    { int _idx = 5; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _abs11r = 0, _abs11i = 0;
    _abs11r = c_abs(x1r, x1i); _abs11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 1.0; _c12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
    double _log14r = 0, _log14i = 0;
    c_log(_add13r, _add13i, &_log14r, &_log14i);
    double _neg15r = 0, _neg15i = 0;
    _neg15r = -(_log14r); _neg15i = -(_log14i);
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 1.0;
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x2r, x2i); _abs17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 1.0; _c18i = 0;
    double _add19r = 0, _add19i = 0;
    _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
    double _log20r = 0, _log20i = 0;
    c_log(_add19r, _add19i, &_log20r, &_log20i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c16r, _c16i, _log20r, _log20i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _neg15r + _mul21r; _add22i = _neg15i + _mul21i;
    { int _idx = 7; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow24r, &_pow24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 0.0; _c25i = 1.0;
    double _add26r = 0, _add26i = 0;
    _add26r = x2r + _c25r; _add26i = x2i + _c25i;
    double _div27r = 0, _div27i = 0;
    c_div(_pow24r, _pow24i, _add26r, _add26i, &_div27r, &_div27i);
    { int _idx = 9; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div27r; cIm[_idx] = _div27i; } }
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _sqrt29r = 0, _sqrt29i = 0;
    c_powr(_mul28r, _mul28i, 0.5, &_sqrt29r, &_sqrt29i);
    { int _idx = 10; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sqrt29r; cIm[_idx] = _sqrt29i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_752_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(x1r, x1i); _abs2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_c1r, _c1i, _abs2r, _abs2i, &_mul3r, &_mul3i);
    double _exp4r = 0, _exp4i = 0;
    c_exp2(_mul3r, _mul3i, &_exp4r, &_exp4i);
    double _re5r = 0, _re5i = 0;
    _re5r = x2r; _re5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(_re5r, _re5i, _re5r, _re5i, &_pow7r, &_pow7i);
    double _cos8r = 0, _cos8i = 0;
    c_cos(_pow7r, _pow7i, &_cos8r, &_cos8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _exp4r + _cos8r; _add9i = _exp4i + _cos8i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 1.0;
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow12r, &_pow12i);
    double _im13r = 0, _im13i = 0;
    _im13r = _pow12i; _im13i = 0;
    double _sin14r = 0, _sin14i = 0;
    c_sin(_im13r, _im13i, &_sin14r, &_sin14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c10r, _c10i, _sin14r, _sin14i, &_mul15r, &_mul15i);
    double _sub16r = 0, _sub16i = 0;
    _sub16r = _mul15r - x2r; _sub16i = _mul15i - x2i;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub16r; cIm[_idx] = _sub16i; } }
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x1r, x1i); _abs17i = 0;
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(x2r, x2i); _abs18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_abs17r, _abs17i, _abs18r, _abs18i, &_mul19r, &_mul19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 3.0; _c20i = 0;
    double _div21r = 0, _div21i = 0;
    c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div21r; cIm[_idx] = _div21i; } }
    double _c22r = 0, _c22i = 0;
    _c22r = 0.0; _c22i = 1.0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c22r, _c22i, x2r, x2i, &_mul23r, &_mul23i);
    double _add24r = 0, _add24i = 0;
    _add24r = x1r + _mul23r; _add24i = x1i + _mul23i;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _div26r = 0, _div26i = 0;
    c_div(_add24r, _add24i, _c25r, _c25i, &_div26r, &_div26i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div26r; cIm[_idx] = _div26i; } }
    double _c27r = 0, _c27i = 0;
    _c27r = 0.0; _c27i = 1.0;
    double _mul28r = 0, _mul28i = 0;
    c_mul(_c27r, _c27i, x2r, x2i, &_mul28r, &_mul28i);
    double _sub29r = 0, _sub29i = 0;
    _sub29r = x1r - _mul28r; _sub29i = x1i - _mul28i;
    double _c30r = 0, _c30i = 0;
    _c30r = 2.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(_sub29r, _sub29i, _sub29r, _sub29i, &_pow31r, &_pow31i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _pow31r; cIm[_idx] = _pow31i; } }
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 1.0;
    double _re33r = 0, _re33i = 0;
    _re33r = x1r; _re33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c32r, _c32i, _re33r, _re33i, &_mul34r, &_mul34i);
    double _im35r = 0, _im35i = 0;
    _im35r = x2i; _im35i = 0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_mul34r, _mul34i, _im35r, _im35i, &_mul36r, &_mul36i);
    double _exp37r = 0, _exp37i = 0;
    c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
    double _sub38r = 0, _sub38i = 0;
    _sub38r = _exp37r - x1r; _sub38i = _exp37i - x1i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub38r; cIm[_idx] = _sub38i; } }
    double _c39r = 0, _c39i = 0;
    _c39r = 0.0; _c39i = 1.0;
    double _c40r = 0, _c40i = 0;
    _c40r = 3.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow41r, &_pow41i);
    c_mul(_pow41r, _pow41i, x2r, x2i, &_pow41r, &_pow41i);
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(_pow41r, _pow41i); _abs42i = 0;
    double _mul43r = 0, _mul43i = 0;
    c_mul(_c39r, _c39i, _abs42r, _abs42i, &_mul43r, &_mul43i);
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow45r, &_pow45i);
    double _add46r = 0, _add46i = 0;
    _add46r = _mul43r + _pow45r; _add46i = _mul43i + _pow45i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    double _re47r = 0, _re47i = 0;
    _re47r = x1r; _re47i = 0;
    double _c48r = 0, _c48i = 0;
    _c48r = 3.0; _c48i = 0;
    double _pow49r = 0, _pow49i = 0;
    c_mul(_re47r, _re47i, _re47r, _re47i, &_pow49r, &_pow49i);
    c_mul(_pow49r, _pow49i, _re47r, _re47i, &_pow49r, &_pow49i);
    double _c50r = 0, _c50i = 0;
    _c50r = 0.0; _c50i = 1.0;
    double _im51r = 0, _im51i = 0;
    _im51r = x2i; _im51i = 0;
    double _c52r = 0, _c52i = 0;
    _c52r = 3.0; _c52i = 0;
    double _pow53r = 0, _pow53i = 0;
    c_mul(_im51r, _im51i, _im51r, _im51i, &_pow53r, &_pow53i);
    c_mul(_pow53r, _pow53i, _im51r, _im51i, &_pow53r, &_pow53i);
    double _mul54r = 0, _mul54i = 0;
    c_mul(_c50r, _c50i, _pow53r, _pow53i, &_mul54r, &_mul54i);
    double _sub55r = 0, _sub55i = 0;
    _sub55r = _pow49r - _mul54r; _sub55i = _pow49i - _mul54i;
    double _exp56r = 0, _exp56i = 0;
    c_exp2(_sub55r, _sub55i, &_exp56r, &_exp56i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp56r; cIm[_idx] = _exp56i; } }
    double _sub57r = 0, _sub57i = 0;
    _sub57r = x1r - x2r; _sub57i = x1i - x2i;
    double _abs58r = 0, _abs58i = 0;
    _abs58r = c_abs(_sub57r, _sub57i); _abs58i = 0;
    double _add59r = 0, _add59i = 0;
    _add59r = x1r + x2r; _add59i = x1i + x2i;
    double _mul60r = 0, _mul60i = 0;
    c_mul(_abs58r, _abs58i, _add59r, _add59i, &_mul60r, &_mul60i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul60r; cIm[_idx] = _mul60i; } }
    double _abs61r = 0, _abs61i = 0;
    _abs61r = c_abs(x2r, x2i); _abs61i = 0;
    double _mul62r = 0, _mul62i = 0;
    c_mul(x1r, x1i, _abs61r, _abs61i, &_mul62r, &_mul62i);
    double _c63r = 0, _c63i = 0;
    _c63r = 0.0; _c63i = 1.0;
    double _mul64r = 0, _mul64i = 0;
    c_mul(_c63r, _c63i, x1r, x1i, &_mul64r, &_mul64i);
    double _mul65r = 0, _mul65i = 0;
    c_mul(_mul64r, _mul64i, x2r, x2i, &_mul65r, &_mul65i);
    double _sub66r = 0, _sub66i = 0;
    _sub66r = _mul62r - _mul65r; _sub66i = _mul62i - _mul65i;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub66r; cIm[_idx] = _sub66i; } }
    double _add67r = 0, _add67i = 0;
    _add67r = x1r + x2r; _add67i = x1i + x2i;
    double _c68r = 0, _c68i = 0;
    _c68r = 1.0; _c68i = 0;
    double _c69r = 0, _c69i = 0;
    _c69r = 2.0; _c69i = 0;
    double _pow70r = 0, _pow70i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow70r, &_pow70i);
    double _add71r = 0, _add71i = 0;
    _add71r = _c68r + _pow70r; _add71i = _c68i + _pow70i;
    double _c72r = 0, _c72i = 0;
    _c72r = 2.0; _c72i = 0;
    double _pow73r = 0, _pow73i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow73r, &_pow73i);
    double _add74r = 0, _add74i = 0;
    _add74r = _add71r + _pow73r; _add74i = _add71i + _pow73i;
    double _div75r = 0, _div75i = 0;
    c_div(_add67r, _add67i, _add74r, _add74i, &_div75r, &_div75i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div75r; cIm[_idx] = _div75i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_753_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 1.0;
    double _add7r = 0, _add7i = 0;
    _add7r = x1r + x2r; _add7i = x1i + x2i;
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(_add7r, _add7i); _abs8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(_abs8r, _abs8i, _abs8r, _abs8i, &_pow10r, &_pow10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c6r, _c6i, _pow10r, _pow10i, &_mul11r, &_mul11i);
    double _exp12r = 0, _exp12i = 0;
    c_exp2(_mul11r, _mul11i, &_exp12r, &_exp12i);
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _exp12r; cIm[_idx] = _exp12i; } }
    static const double prime_numbers[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0};
    for (int k = 2; k < 8; k++) {
        double _arr13r = 0, _arr13i = 0;
        { int _idx = k; _arr13r = (_idx >= 0 && _idx < 10) ? prime_numbers[_idx] : 0.0; _arr13i = 0; }
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = k + _c14r; _add15i = 0 + _c14i;
        double _div16r = 0, _div16i = 0;
        c_div(x1r, x1i, _add15r, _add15i, &_div16r, &_div16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_mul(_div16r, _div16i, _div16r, _div16i, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_arr13r, _arr13i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = k + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(x2r, x2i, _add22r, _add22i, &_div23r, &_div23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c20r, _c20i, _div23r, _div23i, &_mul24r, &_mul24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul19r, _mul19i, _exp25r, _exp25i, &_mul26r, &_mul26i);
        { int _idx = k; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    static const double fibonacci_sequence[] = {0.0, 1.0, 1.0, 2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0};
    double _arr27r = 0, _arr27i = 0;
    { int _idx = 10; _arr27r = (_idx >= 0 && _idx < 11) ? fibonacci_sequence[_idx] : 0.0; _arr27i = 0; }
    double _mul28r = 0, _mul28i = 0;
    c_mul(_arr27r, _arr27i, x1r, x1i, &_mul28r, &_mul28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_mul28r, _mul28i, x2r, x2i, &_mul29r, &_mul29i);
    double _c30r = 0, _c30i = 0;
    _c30r = 0.0; _c30i = 1.0;
    double _sub31r = 0, _sub31i = 0;
    _sub31r = x1r - x2r; _sub31i = x1i - x2i;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c30r, _c30i, _sub31r, _sub31i, &_mul32r, &_mul32i);
    double _exp33r = 0, _exp33i = 0;
    c_exp2(_mul32r, _mul32i, &_exp33r, &_exp33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_mul29r, _mul29i, _exp33r, _exp33i, &_mul34r, &_mul34i);
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 3.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow36r, &_pow36i);
    c_mul(_pow36r, _pow36i, x1r, x1i, &_pow36r, &_pow36i);
    double _c37r = 0, _c37i = 0;
    _c37r = 3.0; _c37i = 0;
    double _pow38r = 0, _pow38i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow38r, &_pow38i);
    c_mul(_pow38r, _pow38i, x2r, x2i, &_pow38r, &_pow38i);
    double _add39r = 0, _add39i = 0;
    _add39r = _pow36r + _pow38r; _add39i = _pow36i + _pow38i;
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _add39r - _c40r; _sub41i = _add39i - _c40i;
    double _c42r = 0, _c42i = 0;
    _c42r = 0.0; _c42i = 1.0;
    double _add43r = 0, _add43i = 0;
    _add43r = x1r + x2r; _add43i = x1i + x2i;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c42r, _c42i, _add43r, _add43i, &_mul44r, &_mul44i);
    double _exp45r = 0, _exp45i = 0;
    c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(_sub41r, _sub41i, _exp45r, _exp45i, &_mul46r, &_mul46i);
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_754_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _div7r = 0, _div7i = 0;
    c_div(_c5r, _c5i, _c6r, _c6i, &_div7r, &_div7i);
    double _neg8r = 0, _neg8i = 0;
    _neg8r = -(x2r); _neg8i = -(x2i);
    double _exp9r = 0, _exp9i = 0;
    c_exp2(_neg8r, _neg8i, &_exp9r, &_exp9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_div7r, _div7i, _exp9r, _exp9i, &_mul10r, &_mul10i);
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _mul11r = 0, _mul11i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul11r, &_mul11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 3.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow13r, &_pow13i);
    c_mul(_pow13r, _pow13i, x2r, x2i, &_pow13r, &_pow13i);
    double _sub14r = 0, _sub14i = 0;
    _sub14r = _mul11r - _pow13r; _sub14i = _mul11i - _pow13i;
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub14r; cIm[_idx] = _sub14i; } }
    double _cos15r = 0, _cos15i = 0;
    c_cos(x1r, x1i, &_cos15r, &_cos15i);
    double _cos16r = 0, _cos16i = 0;
    c_cos(x2r, x2i, &_cos16r, &_cos16i);
    double _add17r = 0, _add17i = 0;
    _add17r = _cos15r + _cos16r; _add17i = _cos15i + _cos16i;
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
    double _sin18r = 0, _sin18i = 0;
    c_sin(x1r, x1i, &_sin18r, &_sin18i);
    double _cos19r = 0, _cos19i = 0;
    c_cos(x2r, x2i, &_cos19r, &_cos19i);
    double _mul20r = 0, _mul20i = 0;
    c_mul(_sin18r, _sin18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul20r; cIm[_idx] = _mul20i; } }
    double _sub21r = 0, _sub21i = 0;
    _sub21r = x1r - x2r; _sub21i = x1i - x2i;
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(_sub21r, _sub21i); _abs22i = 0;
    double _c23r = 0, _c23i = 0;
    _c23r = 1.0; _c23i = 0;
    double _add24r = 0, _add24i = 0;
    _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
    double _log25r = 0, _log25i = 0;
    c_log(_add24r, _add24i, &_log25r, &_log25i);
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _log25r; cIm[_idx] = _log25i; } }
    double _add26r = 0, _add26i = 0;
    _add26r = x1r + x2r; _add26i = x1i + x2i;
    double _c27r = 0, _c27i = 0;
    _c27r = 2.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(_add26r, _add26i, _add26r, _add26i, &_pow28r, &_pow28i);
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _pow28r; cIm[_idx] = _pow28i; } }
    double _re29r = 0, _re29i = 0;
    _re29r = x1r; _re29i = 0;
    double _c30r = 0, _c30i = 0;
    _c30r = 3.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(_re29r, _re29i, _re29r, _re29i, &_pow31r, &_pow31i);
    c_mul(_pow31r, _pow31i, _re29r, _re29i, &_pow31r, &_pow31i);
    double _im32r = 0, _im32i = 0;
    _im32r = x2i; _im32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 3.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_im32r, _im32i, _im32r, _im32i, &_pow34r, &_pow34i);
    c_mul(_pow34r, _pow34i, _im32r, _im32i, &_pow34r, &_pow34i);
    double _add35r = 0, _add35i = 0;
    _add35r = _pow31r + _pow34r; _add35i = _pow31i + _pow34i;
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    double _mul36r = 0, _mul36i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul36r, &_mul36i);
    double _abs37r = 0, _abs37i = 0;
    _abs37r = c_abs(_mul36r, _mul36i); _abs37i = 0;
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _abs37r; cIm[_idx] = _abs37i; } }
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _mul39r = 0, _mul39i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul39r, &_mul39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c38r, _c38i, _mul39r, _mul39i, &_mul40r, &_mul40i);
    double _exp41r = 0, _exp41i = 0;
    c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _exp41r; cIm[_idx] = _exp41i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_755: too complex for auto-transpile, stubbed */
static void poly_755_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_756_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, x1r, x1i, &_mul2r, &_mul2i);
    double _mul3r = 0, _mul3i = 0;
    c_mul(_mul2r, _mul2i, x2r, x2i, &_mul3r, &_mul3i);
    double _exp4r = 0, _exp4i = 0;
    c_exp2(_mul3r, _mul3i, &_exp4r, &_exp4i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _exp4r; cIm[_idx] = _exp4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 100.0; _c5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
    double _sin7r = 0, _sin7i = 0;
    c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _sin7r, _sin7i, &_mul8r, &_mul8i);
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul8r; cIm[_idx] = _mul8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 0.0; _c9i = 100.0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul10r, &_mul10i);
    double _cos11r = 0, _cos11i = 0;
    c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c9r, _c9i, _cos11r, _cos11i, &_mul12r, &_mul12i);
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    double _c13r = 0, _c13i = 0;
    _c13r = 0.0; _c13i = 1.0;
    double _c14r = 0, _c14i = 0;
    _c14r = 3.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow15r, &_pow15i);
    c_mul(_pow15r, _pow15i, x1r, x1i, &_pow15r, &_pow15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c13r, _c13i, _pow15r, _pow15i, &_mul16r, &_mul16i);
    double _c17r = 0, _c17i = 0;
    _c17r = 0.0; _c17i = 2.0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c17r, _c17i, x1r, x1i, &_mul18r, &_mul18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(_mul18r, _mul18i, _pow20r, _pow20i, &_mul21r, &_mul21i);
    double _sub22r = 0, _sub22i = 0;
    _sub22r = _mul16r - _mul21r; _sub22i = _mul16i - _mul21i;
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub22r; cIm[_idx] = _sub22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 5.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_powr(x1r, x1i, 5.0, &_pow24r, &_pow24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 5.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_powr(x2r, x2i, 5.0, &_pow26r, &_pow26i);
    double _add27r = 0, _add27i = 0;
    _add27r = _pow24r + _pow26r; _add27i = _pow24i + _pow26i;
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 0.0; _c28i = 10.0;
    double _c29r = 0, _c29i = 0;
    _c29r = 4.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, _pow30r, _pow30i, &_pow30r, &_pow30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(_c28r, _c28i, _pow30r, _pow30i, &_mul31r, &_mul31i);
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 10.0;
    double _c33r = 0, _c33i = 0;
    _c33r = 4.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow34r, &_pow34i);
    c_mul(_pow34r, _pow34i, _pow34r, _pow34i, &_pow34r, &_pow34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c32r, _c32i, _pow34r, _pow34i, &_mul35r, &_mul35i);
    double _sub36r = 0, _sub36i = 0;
    _sub36r = _mul31r - _mul35r; _sub36i = _mul31i - _mul35i;
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    for (int k = 6; k < 10; k++) {
        double _cf37r = 0, _cf37i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 10) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_cf37r, _cf37i, _c38r, _c38i, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.8; _c40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = k + _c41r; _add42i = 0 + _c41i;
        double _pow43r = 0, _pow43i = 0;
        c_powr(_c40r, _c40i, _add42r, &_pow43r, &_pow43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_mul39r, _mul39i, _pow43r, _pow43i, &_mul44r, &_mul44i);
        { int _idx = k; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_757_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(x1r, x1i); _abs2i = 0;
    double _sqrt3r = 0, _sqrt3i = 0;
    c_powr(_abs2r, _abs2i, 0.5, &_sqrt3r, &_sqrt3i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sqrt3r; cIm[_idx] = _sqrt3i; } }
    double _cos4r = 0, _cos4i = 0;
    c_cos(x1r, x1i, &_cos4r, &_cos4i);
    double _sin5r = 0, _sin5i = 0;
    c_sin(x2r, x2i, &_sin5r, &_sin5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _cos4r + _sin5r; _add6i = _cos4i + _sin5i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add6r; cIm[_idx] = _add6i; } }
    double _abs7r = 0, _abs7i = 0;
    _abs7r = c_abs(x1r, x1i); _abs7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 1.0; _c8i = 0;
    double _add9r = 0, _add9i = 0;
    _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
    double _div10r = 0, _div10i = 0;
    c_div(x2r, x2i, _add9r, _add9i, &_div10r, &_div10i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div10r; cIm[_idx] = _div10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 0.0; _c11i = 1.0;
    double _ang12r = 0, _ang12i = 0;
    _ang12r = c_arg(x1r, x1i); _ang12i = 0;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c11r, _c11i, _ang12r, _ang12i, &_mul13r, &_mul13i);
    double _exp14r = 0, _exp14i = 0;
    c_exp2(_mul13r, _mul13i, &_exp14r, &_exp14i);
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 1.0;
    double _neg16r = 0, _neg16i = 0;
    _neg16r = -(_c15r); _neg16i = -(_c15i);
    double _ang17r = 0, _ang17i = 0;
    _ang17r = c_arg(x2r, x2i); _ang17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_neg16r, _neg16i, _ang17r, _ang17i, &_mul18r, &_mul18i);
    double _exp19r = 0, _exp19i = 0;
    c_exp2(_mul18r, _mul18i, &_exp19r, &_exp19i);
    double _sub20r = 0, _sub20i = 0;
    _sub20r = _exp14r - _exp19r; _sub20i = _exp14i - _exp19i;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub20r; cIm[_idx] = _sub20i; } }
    double _cf21r = 0, _cf21i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
    double _c22r = 0, _c22i = 0;
    _c22r = 2.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(_cf21r, _cf21i, _cf21r, _cf21i, &_pow23r, &_pow23i);
    double _cf24r = 0, _cf24i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(_cf24r, _cf24i, _cf24r, _cf24i, &_pow26r, &_pow26i);
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _pow23r - _pow26r; _sub27i = _pow23i - _pow26i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub27r; cIm[_idx] = _sub27i; } }
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _cf29r = 0, _cf29i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
    double _mul30r = 0, _mul30i = 0;
    c_mul(_mul28r, _mul28i, _cf29r, _cf29i, &_mul30r, &_mul30i);
    double _c31r = 0, _c31i = 0;
    _c31r = 1.0; _c31i = 0;
    double _cf32r = 0, _cf32i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_cf32r, _cf32i, _cf32r, _cf32i, &_pow34r, &_pow34i);
    double _add35r = 0, _add35i = 0;
    _add35r = _c31r + _pow34r; _add35i = _c31i + _pow34i;
    double _div36r = 0, _div36i = 0;
    c_div(_mul30r, _mul30i, _add35r, _add35i, &_div36r, &_div36i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div36r; cIm[_idx] = _div36i; } }
    double _re37r = 0, _re37i = 0;
    _re37r = x1r; _re37i = 0;
    double _im38r = 0, _im38i = 0;
    _im38r = x2i; _im38i = 0;
    double _add39r = 0, _add39i = 0;
    _add39r = _re37r + _im38r; _add39i = _re37i + _im38i;
    double _re40r = 0, _re40i = 0;
    _re40r = x2r; _re40i = 0;
    double _im41r = 0, _im41i = 0;
    _im41r = x1i; _im41i = 0;
    double _add42r = 0, _add42i = 0;
    _add42r = _re40r + _im41r; _add42i = _re40i + _im41i;
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _add39r - _add42r; _sub43i = _add39i - _add42i;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    double _cf44r = 0, _cf44i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { _cf44r = cRe[_idx]; _cf44i = cIm[_idx]; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 1.0; _c45i = 0;
    double _cf46r = 0, _cf46i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
    double _abs47r = 0, _abs47i = 0;
    _abs47r = c_abs(_cf46r, _cf46i); _abs47i = 0;
    double _add48r = 0, _add48i = 0;
    _add48r = _c45r + _abs47r; _add48i = _c45i + _abs47i;
    double _div49r = 0, _div49i = 0;
    c_div(_cf44r, _cf44i, _add48r, _add48i, &_div49r, &_div49i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div49r; cIm[_idx] = _div49i; } }
    double _cf50r = 0, _cf50i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
    double _cf51r = 0, _cf51i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
    double _mul52r = 0, _mul52i = 0;
    c_mul(_cf50r, _cf50i, _cf51r, _cf51i, &_mul52r, &_mul52i);
    double _cf53r = 0, _cf53i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { _cf53r = cRe[_idx]; _cf53i = cIm[_idx]; } }
    double _mul54r = 0, _mul54i = 0;
    c_mul(_mul52r, _mul52i, _cf53r, _cf53i, &_mul54r, &_mul54i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    double _cf55r = 0, _cf55i = 0;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
    double _c56r = 0, _c56i = 0;
    _c56r = 3.0; _c56i = 0;
    double _pow57r = 0, _pow57i = 0;
    c_mul(_cf55r, _cf55i, _cf55r, _cf55i, &_pow57r, &_pow57i);
    c_mul(_pow57r, _pow57i, _cf55r, _cf55i, &_pow57r, &_pow57i);
    double _cf58r = 0, _cf58i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { _cf58r = cRe[_idx]; _cf58i = cIm[_idx]; } }
    double _c59r = 0, _c59i = 0;
    _c59r = 2.0; _c59i = 0;
    double _pow60r = 0, _pow60i = 0;
    c_mul(_cf58r, _cf58i, _cf58r, _cf58i, &_pow60r, &_pow60i);
    double _add61r = 0, _add61i = 0;
    _add61r = _pow57r + _pow60r; _add61i = _pow57i + _pow60i;
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add61r; cIm[_idx] = _add61i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_758_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow4r, &_pow4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _pow2r + _pow4r; _add5i = _pow2i + _pow4i;
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 1.0;
    double _add7r = 0, _add7i = 0;
    _add7r = x1r + x2r; _add7i = x1i + x2i;
    double _ang8r = 0, _ang8i = 0;
    _ang8r = c_arg(_add7r, _add7i); _ang8i = 0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _ang8r, _ang8i, &_mul9r, &_mul9i);
    double _exp10r = 0, _exp10i = 0;
    c_exp2(_mul9r, _mul9i, &_exp10r, &_exp10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_add5r, _add5i, _exp10r, _exp10i, &_mul11r, &_mul11i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul11r; cIm[_idx] = _mul11i; } }
    double _c12r = 0, _c12i = 0;
    _c12r = 0.0; _c12i = 100.0;
    double _c13r = 0, _c13i = 0;
    _c13r = 3.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow14r, &_pow14i);
    c_mul(_pow14r, _pow14i, x1r, x1i, &_pow14r, &_pow14i);
    double _c15r = 0, _c15i = 0;
    _c15r = 2.0; _c15i = 0;
    double _pow16r = 0, _pow16i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow16r, &_pow16i);
    double _sub17r = 0, _sub17i = 0;
    _sub17r = _pow14r - _pow16r; _sub17i = _pow14i - _pow16i;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c12r, _c12i, _sub17r, _sub17i, &_mul18r, &_mul18i);
    double _sub19r = 0, _sub19i = 0;
    _sub19r = x1r - x2r; _sub19i = x1i - x2i;
    double _ang20r = 0, _ang20i = 0;
    _ang20r = c_arg(_sub19r, _sub19i); _ang20i = 0;
    double _cos21r = 0, _cos21i = 0;
    c_cos(_ang20r, _ang20i, &_cos21r, &_cos21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(_mul18r, _mul18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _sub23r = 0, _sub23i = 0;
    _sub23r = x1r - x2r; _sub23i = x1i - x2i;
    double _re24r = 0, _re24i = 0;
    _re24r = _sub23r; _re24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(_re24r, _re24i, _re24r, _re24i, &_pow26r, &_pow26i);
    double _add27r = 0, _add27i = 0;
    _add27r = x1r + x2r; _add27i = x1i + x2i;
    double _im28r = 0, _im28i = 0;
    _im28r = _add27i; _im28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_im28r, _im28i, _im28r, _im28i, &_pow30r, &_pow30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _pow26r + _pow30r; _add31i = _pow26i + _pow30i;
    double _c32r = 0, _c32i = 0;
    _c32r = 100.0; _c32i = 0;
    double _sub33r = 0, _sub33i = 0;
    _sub33r = _add31r - _c32r; _sub33i = _add31i - _c32i;
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub33r; cIm[_idx] = _sub33i; } }
    double _c34r = 0, _c34i = 0;
    _c34r = 42.0; _c34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 1.0; _c35i = 0;
    double _abs36r = 0, _abs36i = 0;
    _abs36r = c_abs(x1r, x1i); _abs36i = 0;
    double _add37r = 0, _add37i = 0;
    _add37r = _c35r + _abs36r; _add37i = _c35i + _abs36i;
    double _log38r = 0, _log38i = 0;
    c_log(_add37r, _add37i, &_log38r, &_log38i);
    double _c39r = 0, _c39i = 0;
    _c39r = 1.0; _c39i = 0;
    double _abs40r = 0, _abs40i = 0;
    _abs40r = c_abs(x2r, x2i); _abs40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = _c39r + _abs40r; _add41i = _c39i + _abs40i;
    double _log42r = 0, _log42i = 0;
    c_log(_add41r, _add41i, &_log42r, &_log42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _log38r + _log42r; _add43i = _log38i + _log42i;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c34r, _c34i, _add43r, _add43i, &_mul44r, &_mul44i);
    double _c45r = 0, _c45i = 0;
    _c45r = 0.0; _c45i = 1.0;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c45r, _c45i, M_PI, 0, &_mul46r, &_mul46i);
    double _c47r = 0, _c47i = 0;
    _c47r = 4.0; _c47i = 0;
    double _div48r = 0, _div48i = 0;
    c_div(_mul46r, _mul46i, _c47r, _c47i, &_div48r, &_div48i);
    double _exp49r = 0, _exp49i = 0;
    c_exp2(_div48r, _div48i, &_exp49r, &_exp49i);
    double _mul50r = 0, _mul50i = 0;
    c_mul(_mul44r, _mul44i, _exp49r, _exp49i, &_mul50r, &_mul50i);
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    double _abs51r = 0, _abs51i = 0;
    _abs51r = c_abs(x1r, x1i); _abs51i = 0;
    double _abs52r = 0, _abs52i = 0;
    _abs52r = c_abs(x2r, x2i); _abs52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = _abs51r + _abs52r; _add53i = _abs51i + _abs52i;
    double _sqrt54r = 0, _sqrt54i = 0;
    c_powr(_add53r, _add53i, 0.5, &_sqrt54r, &_sqrt54i);
    double _c55r = 0, _c55i = 0;
    _c55r = 0.0; _c55i = 1.0;
    double _mul56r = 0, _mul56i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul56r, &_mul56i);
    double _ang57r = 0, _ang57i = 0;
    _ang57r = c_arg(_mul56r, _mul56i); _ang57i = 0;
    double _c58r = 0, _c58i = 0;
    _c58r = 3.0; _c58i = 0;
    double _div59r = 0, _div59i = 0;
    c_div(M_PI, 0, _c58r, _c58i, &_div59r, &_div59i);
    double _sub60r = 0, _sub60i = 0;
    _sub60r = _ang57r - _div59r; _sub60i = _ang57i - _div59i;
    double _mul61r = 0, _mul61i = 0;
    c_mul(_c55r, _c55i, _sub60r, _sub60i, &_mul61r, &_mul61i);
    double _exp62r = 0, _exp62i = 0;
    c_exp2(_mul61r, _mul61i, &_exp62r, &_exp62i);
    double _mul63r = 0, _mul63i = 0;
    c_mul(_sqrt54r, _sqrt54i, _exp62r, _exp62i, &_mul63r, &_mul63i);
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul63r; cIm[_idx] = _mul63i; } }
    double _c64r = 0, _c64i = 0;
    _c64r = 0.1; _c64i = 0;
    double _add65r = 0, _add65i = 0;
    _add65r = x1r + x2r; _add65i = x1i + x2i;
    double _re66r = 0, _re66i = 0;
    _re66r = _add65r; _re66i = 0;
    double _mul67r = 0, _mul67i = 0;
    c_mul(_c64r, _c64i, _re66r, _re66i, &_mul67r, &_mul67i);
    double _sinh68r = 0, _sinh68i = 0;
    _sinh68r = sinh(_mul67r); _sinh68i = 0; /* approx real sinh */
    double _c69r = 0, _c69i = 0;
    _c69r = 0.1; _c69i = 0;
    double _sub70r = 0, _sub70i = 0;
    _sub70r = x1r - x2r; _sub70i = x1i - x2i;
    double _im71r = 0, _im71i = 0;
    _im71r = _sub70i; _im71i = 0;
    double _mul72r = 0, _mul72i = 0;
    c_mul(_c69r, _c69i, _im71r, _im71i, &_mul72r, &_mul72i);
    double _cosh73r = 0, _cosh73i = 0;
    _cosh73r = cosh(_mul72r); _cosh73i = 0; /* approx real cosh */
    double _add74r = 0, _add74i = 0;
    _add74r = _sinh68r + _cosh73r; _add74i = _sinh68i + _cosh73i;
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add74r; cIm[_idx] = _add74i; } }
    double _c75r = 0, _c75i = 0;
    _c75r = 1.0; _c75i = 0;
    double _c76r = 0, _c76i = 0;
    _c76r = 1.0; _c76i = 0;
    double _abs77r = 0, _abs77i = 0;
    _abs77r = c_abs(x1r, x1i); _abs77i = 0;
    double _neg78r = 0, _neg78i = 0;
    _neg78r = -(_abs77r); _neg78i = -(_abs77i);
    double _exp79r = 0, _exp79i = 0;
    c_exp2(_neg78r, _neg78i, &_exp79r, &_exp79i);
    double _add80r = 0, _add80i = 0;
    _add80r = _c76r + _exp79r; _add80i = _c76i + _exp79i;
    double _div81r = 0, _div81i = 0;
    c_div(_c75r, _c75i, _add80r, _add80i, &_div81r, &_div81i);
    double _c82r = 0, _c82i = 0;
    _c82r = 0.0; _c82i = 1.0;
    double _c83r = 0, _c83i = 0;
    _c83r = 1.0; _c83i = 0;
    double _abs84r = 0, _abs84i = 0;
    _abs84r = c_abs(x2r, x2i); _abs84i = 0;
    double _neg85r = 0, _neg85i = 0;
    _neg85r = -(_abs84r); _neg85i = -(_abs84i);
    double _exp86r = 0, _exp86i = 0;
    c_exp2(_neg85r, _neg85i, &_exp86r, &_exp86i);
    double _add87r = 0, _add87i = 0;
    _add87r = _c83r + _exp86r; _add87i = _c83i + _exp86i;
    double _div88r = 0, _div88i = 0;
    c_div(_c82r, _c82i, _add87r, _add87i, &_div88r, &_div88i);
    double _add89r = 0, _add89i = 0;
    _add89r = _div81r + _div88r; _add89i = _div81i + _div88i;
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add89r; cIm[_idx] = _add89i; } }
    double _np90r = 0, _np90i = 0;
    /* WARNING: unhandled np.arctan */
    double _c91r = 0, _c91i = 0;
    _c91r = 0.0; _c91i = 1.0;
    double _mul92r = 0, _mul92i = 0;
    c_mul(_np90r, _np90i, _c91r, _c91i, &_mul92r, &_mul92i);
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul92r; cIm[_idx] = _mul92i; } }
    double _c93r = 0, _c93i = 0;
    _c93r = 0.0; _c93i = 1.0;
    double _c94r = 0, _c94i = 0;
    _c94r = 3.0; _c94i = 0;
    double _pow95r = 0, _pow95i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow95r, &_pow95i);
    c_mul(_pow95r, _pow95i, x1r, x1i, &_pow95r, &_pow95i);
    double _mul96r = 0, _mul96i = 0;
    c_mul(_c93r, _c93i, _pow95r, _pow95i, &_mul96r, &_mul96i);
    double _c97r = 0, _c97i = 0;
    _c97r = 2.0; _c97i = 0;
    double _mul98r = 0, _mul98i = 0;
    c_mul(x2r, x2i, _c97r, _c97i, &_mul98r, &_mul98i);
    double _sub99r = 0, _sub99i = 0;
    _sub99r = _mul96r - _mul98r; _sub99i = _mul96i - _mul98i;
    double _c100r = 0, _c100i = 0;
    _c100r = 200.0; _c100i = 0;
    double _sub101r = 0, _sub101i = 0;
    _sub101r = _sub99r - _c100r; _sub101i = _sub99i - _c100i;
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub101r; cIm[_idx] = _sub101i; } }
    double _c102r = 0, _c102i = 0;
    _c102r = 0.0; _c102i = 1.0;
    double _mul103r = 0, _mul103i = 0;
    c_mul(_c102r, _c102i, x1r, x1i, &_mul103r, &_mul103i);
    double _mul104r = 0, _mul104i = 0;
    c_mul(_mul103r, _mul103i, x2r, x2i, &_mul104r, &_mul104i);
    double _exp105r = 0, _exp105i = 0;
    c_exp2(_mul104r, _mul104i, &_exp105r, &_exp105i);
    double _c106r = 0, _c106i = 0;
    _c106r = 1.0; _c106i = 0;
    double _add107r = 0, _add107i = 0;
    _add107r = _c106r + x1r; _add107i = _c106i + x1i;
    double _add108r = 0, _add108i = 0;
    _add108r = _add107r + x2r; _add108i = _add107i + x2i;
    double _div109r = 0, _div109i = 0;
    c_div(_exp105r, _exp105i, _add108r, _add108i, &_div109r, &_div109i);
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _div109r; cIm[_idx] = _div109i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_759_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _exp1r = 0, _exp1i = 0;
    c_exp2(x1r, x1i, &_exp1r, &_exp1i);
    double _cos2r = 0, _cos2i = 0;
    c_cos(x2r, x2i, &_cos2r, &_cos2i);
    double _mul3r = 0, _mul3i = 0;
    c_mul(_exp1r, _exp1i, _cos2r, _cos2i, &_mul3r, &_mul3i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 0.0; _c4i = 1.0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c4r, _c4i, x1r, x1i, &_mul5r, &_mul5i);
    double _exp6r = 0, _exp6i = 0;
    c_exp2(_mul5r, _mul5i, &_exp6r, &_exp6i);
    double _neg7r = 0, _neg7i = 0;
    _neg7r = -(_exp6r); _neg7i = -(_exp6i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _neg7r; cIm[_idx] = _neg7i; } }
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(x2r, x2i); _abs8i = 0;
    double _log9r = 0, _log9i = 0;
    c_log(_abs8r, _abs8i, &_log9r, &_log9i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _log9r; cIm[_idx] = _log9i; } }
    double _add10r = 0, _add10i = 0;
    _add10r = x1r + x2r; _add10i = x1i + x2i;
    double _c11r = 0, _c11i = 0;
    _c11r = 0.0; _c11i = 1.0;
    double _div12r = 0, _div12i = 0;
    c_div(_add10r, _add10i, _c11r, _c11i, &_div12r, &_div12i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div12r; cIm[_idx] = _div12i; } }
    double _c13r = 0, _c13i = 0;
    _c13r = 3.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow14r, &_pow14i);
    c_mul(_pow14r, _pow14i, x1r, x1i, &_pow14r, &_pow14i);
    double _c15r = 0, _c15i = 0;
    _c15r = 3.0; _c15i = 0;
    double _pow16r = 0, _pow16i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow16r, &_pow16i);
    c_mul(_pow16r, _pow16i, x2r, x2i, &_pow16r, &_pow16i);
    double _sub17r = 0, _sub17i = 0;
    _sub17r = _pow14r - _pow16r; _sub17i = _pow14i - _pow16i;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub17r; cIm[_idx] = _sub17i; } }
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _neg19r = 0, _neg19i = 0;
    _neg19r = -(_c18r); _neg19i = -(_c18i);
    double _exp20r = 0, _exp20i = 0;
    c_exp2(x2r, x2i, &_exp20r, &_exp20i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(_neg19r, _neg19i, _exp20r, _exp20i, &_mul21r, &_mul21i);
    double _sin22r = 0, _sin22i = 0;
    c_sin(x1r, x1i, &_sin22r, &_sin22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_mul21r, _mul21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    double _sub24r = 0, _sub24i = 0;
    _sub24r = x1r - x2r; _sub24i = x1i - x2i;
    double _abs25r = 0, _abs25i = 0;
    _abs25r = c_abs(_sub24r, _sub24i); _abs25i = 0;
    double _sqrt26r = 0, _sqrt26i = 0;
    c_powr(_abs25r, _abs25i, 0.5, &_sqrt26r, &_sqrt26i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sqrt26r; cIm[_idx] = _sqrt26i; } }
    double _c27r = 0, _c27i = 0;
    _c27r = 0.0; _c27i = 1.0;
    double _add28r = 0, _add28i = 0;
    _add28r = x1r + x2r; _add28i = x1i + x2i;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_add28r, _add28i, _add28r, _add28i, &_pow30r, &_pow30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(_c27r, _c27i, _pow30r, _pow30i, &_mul31r, &_mul31i);
    double _exp32r = 0, _exp32i = 0;
    c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp32r; cIm[_idx] = _exp32i; } }
    double _np33r = 0, _np33i = 0;
    /* WARNING: unhandled np.log1p */
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _np33r; cIm[_idx] = _np33i; } }
    double _sub34r = 0, _sub34i = 0;
    _sub34r = x1r - x2r; _sub34i = x1i - x2i;
    double _c35r = 0, _c35i = 0;
    _c35r = 0.0; _c35i = 1.0;
    double _div36r = 0, _div36i = 0;
    c_div(_sub34r, _sub34i, _c35r, _c35i, &_div36r, &_div36i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div36r; cIm[_idx] = _div36i; } }
    double _mul37r = 0, _mul37i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul37r, &_mul37i);
    double _exp38r = 0, _exp38i = 0;
    c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
    double _div39r = 0, _div39i = 0;
    c_div(_exp38r, _exp38i, x1r, x1i, &_div39r, &_div39i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div39r; cIm[_idx] = _div39i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_760_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _im2r = 0, _im2i = 0;
    _im2r = x2i; _im2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_im2r, _im2i, _c3r, _c3i, &_mul4r, &_mul4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _re1r + _mul4r; _add5i = _re1i + _mul4i;
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = x1r + x2r; _add7i = x1i + x2i;
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(_add7r, _add7i); _abs8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(_abs8r, _abs8i, _abs8r, _abs8i, &_pow10r, &_pow10i);
    double _add11r = 0, _add11i = 0;
    _add11r = _c6r + _pow10r; _add11i = _c6i + _pow10i;
    double _div12r = 0, _div12i = 0;
    c_div(_add5r, _add5i, _add11r, _add11i, &_div12r, &_div12i);
    double _sin13r = 0, _sin13i = 0;
    c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sin13r; cIm[_idx] = _sin13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 100.0; _c14i = 0;
    double _re15r = 0, _re15i = 0;
    _re15r = x1r; _re15i = 0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c14r, _c14i, _re15r, _re15i, &_mul16r, &_mul16i);
    double _im17r = 0, _im17i = 0;
    _im17r = x1i; _im17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_mul16r, _mul16i, _im17r, _im17i, &_mul18r, &_mul18i);
    double _re19r = 0, _re19i = 0;
    _re19r = x2r; _re19i = 0;
    double _cos20r = 0, _cos20i = 0;
    c_cos(_re19r, _re19i, &_cos20r, &_cos20i);
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(_cos20r, _cos20i); _abs21i = 0;
    double _c22r = 0, _c22i = 0;
    _c22r = 2.1; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_powr(_abs21r, _abs21i, 2.1, &_pow23r, &_pow23i);
    double _im24r = 0, _im24i = 0;
    _im24r = x2i; _im24i = 0;
    double _sin25r = 0, _sin25i = 0;
    c_sin(_im24r, _im24i, &_sin25r, &_sin25i);
    double _abs26r = 0, _abs26i = 0;
    _abs26r = c_abs(_sin25r, _sin25i); _abs26i = 0;
    double _c27r = 0, _c27i = 0;
    _c27r = 2.1; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_powr(_abs26r, _abs26i, 2.1, &_pow28r, &_pow28i);
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _pow23r - _pow28r; _sub29i = _pow23i - _pow28i;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_mul18r, _mul18i, _sub29r, _sub29i, &_mul30r, &_mul30i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double mm = 0; /* +0i */
    double vv = 0;
    double _call31r = 0, _call31i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='linalg', ctx=Load()), attr='det', ctx=Load()) */
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_call31r, _call31i); _abs32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 1e-10; _c33i = 0;
    if (_abs32r < _c33r) {
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 0;
        vv = _c34r;
    } else {
        double _call35r = 0, _call35i = 0;
        /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='linalg', ctx=Load()), attr='inv', ctx=Load()) */
        vv = _call35r;
    }
    double _c36r = 0, _c36i = 0;
    _c36r = 1000.0; _c36i = 0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c36r, _c36i, vv, 0, &_mul37r, &_mul37i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    double _call38r = 0, _call38i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='fft', ctx=Load()), attr='fft', ctx=Load()) */
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _call38r; cIm[_idx] = _call38i; } }
    double _c39r = 0, _c39i = 0;
    _c39r = 10.0; _c39i = 0;
    double _c40r = 0, _c40i = 0;
    _c40r = 0.0; _c40i = 1.0;
    double _im41r = 0, _im41i = 0;
    _im41r = x1i; _im41i = 0;
    double _re42r = 0, _re42i = 0;
    _re42r = x1r; _re42i = 0;
    double _at243r = 0, _at243i = 0;
    _at243r = atan2(_im41r, _re42r); _at243i = 0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c40r, _c40i, _at243r, _at243i, &_mul44r, &_mul44i);
    double _exp45r = 0, _exp45i = 0;
    c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c39r, _c39i, _exp45r, _exp45i, &_mul46r, &_mul46i);
    double _re47r = 0, _re47i = 0;
    _re47r = x1r; _re47i = 0;
    double _c48r = 0, _c48i = 0;
    _c48r = 2.0; _c48i = 0;
    double _pow49r = 0, _pow49i = 0;
    c_mul(_re47r, _re47i, _re47r, _re47i, &_pow49r, &_pow49i);
    double _im50r = 0, _im50i = 0;
    _im50r = x1i; _im50i = 0;
    double _c51r = 0, _c51i = 0;
    _c51r = 2.0; _c51i = 0;
    double _pow52r = 0, _pow52i = 0;
    c_mul(_im50r, _im50i, _im50r, _im50i, &_pow52r, &_pow52i);
    double _add53r = 0, _add53i = 0;
    _add53r = _pow49r + _pow52r; _add53i = _pow49i + _pow52i;
    double _sqrt54r = 0, _sqrt54i = 0;
    c_powr(_add53r, _add53i, 0.5, &_sqrt54r, &_sqrt54i);
    double _mul55r = 0, _mul55i = 0;
    c_mul(_mul46r, _mul46i, _sqrt54r, _sqrt54i, &_mul55r, &_mul55i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    double _c56r = 0, _c56i = 0;
    _c56r = 1000.0; _c56i = 0;
    double _np57r = 0, _np57i = 0;
    /* WARNING: unhandled np.median */
    double _c58r = 0, _c58i = 0;
    _c58r = 2.0; _c58i = 0;
    double _pow59r = 0, _pow59i = 0;
    c_mul(_np57r, _np57i, _np57r, _np57i, &_pow59r, &_pow59i);
    double _mul60r = 0, _mul60i = 0;
    c_mul(_c56r, _c56i, _pow59r, _pow59i, &_mul60r, &_mul60i);
    double _c61r = 0, _c61i = 0;
    _c61r = 500.0; _c61i = 0;
    double _np62r = 0, _np62i = 0;
    /* WARNING: unhandled np.median */
    double _c63r = 0, _c63i = 0;
    _c63r = 3.0; _c63i = 0;
    double _pow64r = 0, _pow64i = 0;
    c_mul(_np62r, _np62i, _np62r, _np62i, &_pow64r, &_pow64i);
    c_mul(_pow64r, _pow64i, _np62r, _np62i, &_pow64r, &_pow64i);
    double _mul65r = 0, _mul65i = 0;
    c_mul(_c61r, _c61i, _pow64r, _pow64i, &_mul65r, &_mul65i);
    double _add66r = 0, _add66i = 0;
    _add66r = _mul60r + _mul65r; _add66i = _mul60i + _mul65i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add66r; cIm[_idx] = _add66i; } }
    double _re67r = 0, _re67i = 0;
    _re67r = x2r; _re67i = 0;
    double _c68r = 0, _c68i = 0;
    _c68r = 0.0; _c68i = 1.0;
    double _c69r = 0, _c69i = 0;
    _c69r = 2.0; _c69i = 0;
    double _mul70r = 0, _mul70i = 0;
    c_mul(_c68r, _c68i, _c69r, _c69i, &_mul70r, &_mul70i);
    double _im71r = 0, _im71i = 0;
    _im71r = x2i; _im71i = 0;
    double _abs72r = 0, _abs72i = 0;
    _abs72r = c_abs(_im71r, _im71i); _abs72i = 0;
    double _sqrt73r = 0, _sqrt73i = 0;
    c_powr(_abs72r, _abs72i, 0.5, &_sqrt73r, &_sqrt73i);
    double _mul74r = 0, _mul74i = 0;
    c_mul(_mul70r, _mul70i, _sqrt73r, _sqrt73i, &_mul74r, &_mul74i);
    double _add75r = 0, _add75i = 0;
    _add75r = _re67r + _mul74r; _add75i = _re67i + _mul74i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add75r; cIm[_idx] = _add75i; } }
    double _c76r = 0, _c76i = 0;
    _c76r = 100.0; _c76i = 0;
    double _np77r = 0, _np77i = 0;
    /* WARNING: unhandled np.log10 */
    double _mul78r = 0, _mul78i = 0;
    c_mul(_c76r, _c76i, _np77r, _np77i, &_mul78r, &_mul78i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul78r; cIm[_idx] = _mul78i; } }
    double _mul79r = 0, _mul79i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul79r, &_mul79i);
    double _abs80r = 0, _abs80i = 0;
    _abs80r = c_abs(_mul79r, _mul79i); _abs80i = 0;
    double _sqrt81r = 0, _sqrt81i = 0;
    c_powr(_abs80r, _abs80i, 0.5, &_sqrt81r, &_sqrt81i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sqrt81r; cIm[_idx] = _sqrt81i; } }
    double _c82r = 0, _c82i = 0;
    _c82r = 1000.0; _c82i = 0;
    double _im83r = 0, _im83i = 0;
    _im83r = x1i; _im83i = 0;
    double _c84r = 0, _c84i = 0;
    _c84r = 3.0; _c84i = 0;
    double _pow85r = 0, _pow85i = 0;
    c_mul(_im83r, _im83i, _im83r, _im83i, &_pow85r, &_pow85i);
    c_mul(_pow85r, _pow85i, _im83r, _im83i, &_pow85r, &_pow85i);
    double _c86r = 0, _c86i = 0;
    _c86r = 3.0; _c86i = 0;
    double _im87r = 0, _im87i = 0;
    _im87r = x1i; _im87i = 0;
    double _mul88r = 0, _mul88i = 0;
    c_mul(_c86r, _c86i, _im87r, _im87i, &_mul88r, &_mul88i);
    double _re89r = 0, _re89i = 0;
    _re89r = x1r; _re89i = 0;
    double _c90r = 0, _c90i = 0;
    _c90r = 2.0; _c90i = 0;
    double _pow91r = 0, _pow91i = 0;
    c_mul(_re89r, _re89i, _re89r, _re89i, &_pow91r, &_pow91i);
    double _mul92r = 0, _mul92i = 0;
    c_mul(_mul88r, _mul88i, _pow91r, _pow91i, &_mul92r, &_mul92i);
    double _sub93r = 0, _sub93i = 0;
    _sub93r = _pow85r - _mul92r; _sub93i = _pow85i - _mul92i;
    double _mul94r = 0, _mul94i = 0;
    c_mul(_c82r, _c82i, _sub93r, _sub93i, &_mul94r, &_mul94i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul94r; cIm[_idx] = _mul94i; } }
    double _c95r = 0, _c95i = 0;
    _c95r = 0.0; _c95i = 2.0;
    double _re96r = 0, _re96i = 0;
    _re96r = x1r; _re96i = 0;
    double _im97r = 0, _im97i = 0;
    _im97r = x1i; _im97i = 0;
    double _sub98r = 0, _sub98i = 0;
    _sub98r = _re96r - _im97r; _sub98i = _re96i - _im97i;
    double _mul99r = 0, _mul99i = 0;
    c_mul(_c95r, _c95i, _sub98r, _sub98i, &_mul99r, &_mul99i);
    double _c100r = 0, _c100i = 0;
    _c100r = 2.0; _c100i = 0;
    double _re101r = 0, _re101i = 0;
    _re101r = x1r; _re101i = 0;
    double _im102r = 0, _im102i = 0;
    _im102r = x1i; _im102i = 0;
    double _add103r = 0, _add103i = 0;
    _add103r = _re101r + _im102r; _add103i = _re101i + _im102i;
    double _mul104r = 0, _mul104i = 0;
    c_mul(_c100r, _c100i, _add103r, _add103i, &_mul104r, &_mul104i);
    double _add105r = 0, _add105i = 0;
    _add105r = _mul99r + _mul104r; _add105i = _mul99i + _mul104i;
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add105r; cIm[_idx] = _add105i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_761_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    for (int k = 0; k < (int)(n); k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 100.0; _c1i = 0;
        double _re2r = 0, _re2i = 0;
        _re2r = x1r; _re2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = k + _c3r; _add4i = 0 + _c3i;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_re2r, _re2i, _add4r, _add4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c1r, _c1i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _im8r = 0, _im8i = 0;
        _im8r = x2i; _im8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = k + _c9r; _add10i = 0 + _c9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_im8r, _im8i, _add10r, _add10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_mul7r, _mul7i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 0.0; _c14i = 1.0;
        double _re15r = 0, _re15i = 0;
        _re15r = x2r; _re15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = k + _c16r; _add17i = 0 + _c16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_re15r, _re15i, _add17r, _add17i, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 100.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c14r, _c14i, _div20r, _div20i, &_mul21r, &_mul21i);
        double _exp22r = 0, _exp22i = 0;
        c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul13r, _mul13i, _exp22r, _exp22i, &_mul23r, &_mul23i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    }
    double _c24r = 0, _c24i = 0;
    _c24r = 10.0; _c24i = 0;
    { double _tr = cRe[0]*_c24r - cIm[0]*_c24i; cIm[0] = cRe[0]*_c24i + cIm[0]*_c24r; cRe[0] = _tr; }
    double _mul25r = 0, _mul25i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul25r, &_mul25i);
    double _sin26r = 0, _sin26i = 0;
    c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
    { double _tr = cRe[4]*_sin26r - cIm[4]*_sin26i; cIm[4] = cRe[4]*_sin26i + cIm[4]*_sin26r; cRe[4] = _tr; }
    double _mul27r = 0, _mul27i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul27r, &_mul27i);
    double _cos28r = 0, _cos28i = 0;
    c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
    double _c29r = 0, _c29i = 0;
    _c29r = 0.0; _c29i = 1.0;
    double _re30r = 0, _re30i = 0;
    _re30r = x1r; _re30i = 0;
    double _re31r = 0, _re31i = 0;
    _re31r = x2r; _re31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = _re30r + _re31r; _add32i = _re30i + _re31i;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c29r, _c29i, _add32r, _add32i, &_mul33r, &_mul33i);
    double _exp34r = 0, _exp34i = 0;
    c_exp2(_mul33r, _mul33i, &_exp34r, &_exp34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_cos28r, _cos28i, _exp34r, _exp34i, &_mul35r, &_mul35i);
    cRe[9] += _mul35r; cIm[9] += _mul35i;
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_762_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double primes[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0};
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int _si = 0; _si < 8; _si++) {
        int _si_idx = _si + 1;
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=1), upper=Constant(value=9)) */
        double _arr3r = 0, _arr3i = 0;
        { int _idx = (int)(_unk2r); _arr3r = (_idx >= 0 && _idx < 10) ? primes[_idx] : 0.0; _arr3i = 0; }
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow5r, &_pow5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow7r, &_pow7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow5r + _pow7r; _add8i = _pow5i + _pow7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 1.0;
        double _add10r = 0, _add10i = 0;
        _add10r = _add8r + _c9r; _add10i = _add8i + _c9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_arr3r, _arr3i, _add10r, _add10i, &_mul11r, &_mul11i);
        cRe[_si_idx] = _mul11r; cIm[_si_idx] = _mul11i;
    }
    double _sum12r = 0, _sum12i = 0;
    /* WARNING: unhandled np.sum */
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow14r, &_pow14i);
    double _c15r = 0, _c15i = 0;
    _c15r = 2.0; _c15i = 0;
    double _pow16r = 0, _pow16i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow16r, &_pow16i);
    double _add17r = 0, _add17i = 0;
    _add17r = _pow14r + _pow16r; _add17i = _pow14i + _pow16i;
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(_add17r, _add17i); _abs18i = 0;
    double _add19r = 0, _add19i = 0;
    _add19r = _sum12r + _abs18r; _add19i = _sum12i + _abs18i;
    { int _idx = 9; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_763_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow4r, &_pow4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _pow2r + _pow4r; _add5i = _pow2i + _pow4i;
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 1.0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_add5r, _add5i, _c6r, _c6i, &_mul7r, &_mul7i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 10.0; _c8i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _c8r; cIm[_idx] = _c8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 100.0; _c9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(x1r, x1i, _c9r, _c9i, &_mul10r, &_mul10i);
    double _abs11r = 0, _abs11i = 0;
    _abs11r = c_abs(_mul10r, _mul10i); _abs11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 0.5; _c12i = 0;
    double _sub13r = 0, _sub13i = 0;
    _sub13r = _abs11r - _c12r; _sub13i = _abs11i - _c12i;
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub13r; cIm[_idx] = _sub13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 100.0; _c14i = 0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(x2r, x2i, _c14r, _c14i, &_mul15r, &_mul15i);
    double _abs16r = 0, _abs16i = 0;
    _abs16r = c_abs(_mul15r, _mul15i); _abs16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 0.5; _c17i = 0;
    double _sub18r = 0, _sub18i = 0;
    _sub18r = _abs16r - _c17r; _sub18i = _abs16i - _c17i;
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub18r; cIm[_idx] = _sub18i; } }
    double _c19r = 0, _c19i = 0;
    _c19r = 10.0; _c19i = 0;
    double _neg20r = 0, _neg20i = 0;
    _neg20r = -(_c19r); _neg20i = -(_c19i);
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _neg20r; cIm[_idx] = _neg20i; } }
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow22r, &_pow22i);
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow24r, &_pow24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _pow22r + _pow24r; _add25i = _pow22i + _pow24i;
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add25r; cIm[_idx] = _add25i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_764_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, x1r, x1i, &_mul2r, &_mul2i);
    double _exp3r = 0, _exp3i = 0;
    c_exp2(_mul2r, _mul2i, &_exp3r, &_exp3i);
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _exp3r; cIm[_idx] = _exp3i; } }
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + x2r; _add4i = x1i + x2i;
    double _cos5r = 0, _cos5i = 0;
    c_cos(x1r, x1i, &_cos5r, &_cos5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_add4r, _add4i, _cos5r, _cos5i, &_mul6r, &_mul6i);
    double _c7r = 0, _c7i = 0;
    _c7r = 0.0; _c7i = 1.0;
    double _sin8r = 0, _sin8i = 0;
    c_sin(x2r, x2i, &_sin8r, &_sin8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c7r, _c7i, _sin8r, _sin8i, &_mul9r, &_mul9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _mul6r + _mul9r; _add10i = _mul6i + _mul9i;
    { int _idx = 1; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add10r; cIm[_idx] = _add10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 3.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, x1r, x1i, &_pow12r, &_pow12i);
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow14r, &_pow14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_pow12r, _pow12i, _pow14r, _pow14i, &_mul15r, &_mul15i);
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 1.0;
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 3.0; _c20i = 0;
    double _pow21r = 0, _pow21i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow21r, &_pow21i);
    c_mul(_pow21r, _pow21i, x2r, x2i, &_pow21r, &_pow21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(_mul19r, _mul19i, _pow21r, _pow21i, &_mul22r, &_mul22i);
    double _sub23r = 0, _sub23i = 0;
    _sub23r = _mul15r - _mul22r; _sub23i = _mul15i - _mul22i;
    { int _idx = 2; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub23r; cIm[_idx] = _sub23i; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c24r, _c24i, x2r, x2i, &_mul25r, &_mul25i);
    double _add26r = 0, _add26i = 0;
    _add26r = x1r + _mul25r; _add26i = x1i + _mul25i;
    double _log27r = 0, _log27i = 0;
    c_log(_add26r, _add26i, &_log27r, &_log27i);
    { int _idx = 3; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _log27r; cIm[_idx] = _log27i; } }
    double _cos28r = 0, _cos28i = 0;
    c_cos(x1r, x1i, &_cos28r, &_cos28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(x1r, x1i, _cos28r, _cos28i, &_mul29r, &_mul29i);
    double _sin30r = 0, _sin30i = 0;
    c_sin(x2r, x2i, &_sin30r, &_sin30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(x2r, x2i, _sin30r, _sin30i, &_mul31r, &_mul31i);
    double _add32r = 0, _add32i = 0;
    _add32r = _mul29r + _mul31r; _add32i = _mul29i + _mul31i;
    { int _idx = 4; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow34r, &_pow34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_pow34r, _pow34i, x2r, x2i, &_mul35r, &_mul35i);
    double _c36r = 0, _c36i = 0;
    _c36r = 2.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow37r, &_pow37i);
    double _mul38r = 0, _mul38i = 0;
    c_mul(x1r, x1i, _pow37r, _pow37i, &_mul38r, &_mul38i);
    double _sub39r = 0, _sub39i = 0;
    _sub39r = _mul35r - _mul38r; _sub39i = _mul35i - _mul38i;
    { int _idx = 5; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub39r; cIm[_idx] = _sub39i; } }
    double _c40r = 0, _c40i = 0;
    _c40r = 0.0; _c40i = 1.0;
    double _c41r = 0, _c41i = 0;
    _c41r = 3.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow42r, &_pow42i);
    c_mul(_pow42r, _pow42i, x1r, x1i, &_pow42r, &_pow42i);
    double _mul43r = 0, _mul43i = 0;
    c_mul(_c40r, _c40i, _pow42r, _pow42i, &_mul43r, &_mul43i);
    double _c44r = 0, _c44i = 0;
    _c44r = 3.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow45r, &_pow45i);
    c_mul(_pow45r, _pow45i, x2r, x2i, &_pow45r, &_pow45i);
    double _add46r = 0, _add46i = 0;
    _add46r = _mul43r + _pow45r; _add46i = _mul43i + _pow45i;
    { int _idx = 6; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 0.0; _c47i = 1.0;
    double _mul48r = 0, _mul48i = 0;
    c_mul(_c47r, _c47i, x2r, x2i, &_mul48r, &_mul48i);
    double _add49r = 0, _add49i = 0;
    _add49r = x1r + _mul48r; _add49i = x1i + _mul48i;
    double _c50r = 0, _c50i = 0;
    _c50r = 3.0; _c50i = 0;
    double _pow51r = 0, _pow51i = 0;
    c_mul(_add49r, _add49i, _add49r, _add49i, &_pow51r, &_pow51i);
    c_mul(_pow51r, _pow51i, _add49r, _add49i, &_pow51r, &_pow51i);
    double _mul52r = 0, _mul52i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul52r, &_mul52i);
    double _sub53r = 0, _sub53i = 0;
    _sub53r = _pow51r - _mul52r; _sub53i = _pow51i - _mul52i;
    { int _idx = 7; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub53r; cIm[_idx] = _sub53i; } }
    double _mul54r = 0, _mul54i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul54r, &_mul54i);
    double _sub55r = 0, _sub55i = 0;
    _sub55r = x1r - x2r; _sub55i = x1i - x2i;
    double _mul56r = 0, _mul56i = 0;
    c_mul(_mul54r, _mul54i, _sub55r, _sub55i, &_mul56r, &_mul56i);
    double _add57r = 0, _add57i = 0;
    _add57r = x1r + x2r; _add57i = x1i + x2i;
    double _mul58r = 0, _mul58i = 0;
    c_mul(_mul56r, _mul56i, _add57r, _add57i, &_mul58r, &_mul58i);
    { int _idx = 8; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    double _c59r = 0, _c59i = 0;
    _c59r = 3.0; _c59i = 0;
    double _pow60r = 0, _pow60i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow60r, &_pow60i);
    c_mul(_pow60r, _pow60i, x1r, x1i, &_pow60r, &_pow60i);
    double _c61r = 0, _c61i = 0;
    _c61r = 2.0; _c61i = 0;
    double _pow62r = 0, _pow62i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow62r, &_pow62i);
    double _mul63r = 0, _mul63i = 0;
    c_mul(_pow60r, _pow60i, _pow62r, _pow62i, &_mul63r, &_mul63i);
    double _c64r = 0, _c64i = 0;
    _c64r = 0.0; _c64i = 1.0;
    double _sub65r = 0, _sub65i = 0;
    _sub65r = x1r - x2r; _sub65i = x1i - x2i;
    double _mul66r = 0, _mul66i = 0;
    c_mul(_c64r, _c64i, _sub65r, _sub65i, &_mul66r, &_mul66i);
    double _exp67r = 0, _exp67i = 0;
    c_exp2(_mul66r, _mul66i, &_exp67r, &_exp67i);
    double _mul68r = 0, _mul68i = 0;
    c_mul(_mul63r, _mul63i, _exp67r, _exp67i, &_mul68r, &_mul68i);
    { int _idx = 9; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul68r; cIm[_idx] = _mul68i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_765_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 100.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_powr(x1r, x1i, x2r, &_pow3r, &_pow3i);
    double _re4r = 0, _re4i = 0;
    _re4r = _pow3r; _re4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _re4r, _re4i, &_mul5r, &_mul5i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 10.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 3.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow8r, &_pow8i);
    c_mul(_pow8r, _pow8i, x2r, x2i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(x1r, x1i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _re10r = 0, _re10i = 0;
    _re10r = _mul9r; _re10i = 0;
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c6r, _c6i, _re10r, _re10i, &_mul11r, &_mul11i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul11r; cIm[_idx] = _mul11i; } }
    double _c12r = 0, _c12i = 0;
    _c12r = 200.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_powr(x1r, x1i, x2r, &_pow13r, &_pow13i);
    double _im14r = 0, _im14i = 0;
    _im14r = _pow13i; _im14i = 0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c12r, _c12i, _im14r, _im14i, &_mul15r, &_mul15i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 1.0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_c16r, _c16i, x1r, x1i, &_mul17r, &_mul17i);
    double _exp18r = 0, _exp18i = 0;
    c_exp2(_mul17r, _mul17i, &_exp18r, &_exp18i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp18r; cIm[_idx] = _exp18i; } }
    double _re19r = 0, _re19i = 0;
    _re19r = x1r; _re19i = 0;
    double _sgn20r = 0, _sgn20i = 0;
    _sgn20r = (_re19r > 0) ? 1.0 : (_re19r < 0) ? -1.0 : 0.0; _sgn20i = 0;
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(x2r, x2i); _abs21i = 0;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_sgn20r, _sgn20i, _abs21r, _abs21i, &_mul22r, &_mul22i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _re23r = 0, _re23i = 0;
    _re23r = x1r; _re23i = 0;
    double _c24r = 0, _c24i = 0;
    _c24r = 2.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(_re23r, _re23i, _re23r, _re23i, &_pow25r, &_pow25i);
    double _im26r = 0, _im26i = 0;
    _im26r = x2i; _im26i = 0;
    double _c27r = 0, _c27i = 0;
    _c27r = 2.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(_im26r, _im26i, _im26r, _im26i, &_pow28r, &_pow28i);
    double _add29r = 0, _add29i = 0;
    _add29r = _pow25r + _pow28r; _add29i = _pow25i + _pow28i;
    double _c30r = 0, _c30i = 0;
    _c30r = 0.0; _c30i = 1.0;
    double _ang31r = 0, _ang31i = 0;
    _ang31r = c_arg(x1r, x1i); _ang31i = 0;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c30r, _c30i, _ang31r, _ang31i, &_mul32r, &_mul32i);
    double _exp33r = 0, _exp33i = 0;
    c_exp2(_mul32r, _mul32i, &_exp33r, &_exp33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_add29r, _add29i, _exp33r, _exp33i, &_mul34r, &_mul34i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    double _np35r = 0, _np35i = 0;
    /* WARNING: unhandled np.where */
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _np35r; cIm[_idx] = _np35i; } }
    double _c36r = 0, _c36i = 0;
    _c36r = 0.0; _c36i = 1.0;
    double _add37r = 0, _add37i = 0;
    _add37r = x1r + _c36r; _add37i = x1i + _c36i;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    double _c38r = 0, _c38i = 0;
    _c38r = 10.0; _c38i = 0;
    double _c39r = 0, _c39i = 0;
    _c39r = 0.1; _c39i = 0;
    double _c40r = 0, _c40i = 0;
    _c40r = 0.0; _c40i = 1.0;
    double _mul41r = 0, _mul41i = 0;
    c_mul(_c40r, _c40i, x2r, x2i, &_mul41r, &_mul41i);
    double _exp42r = 0, _exp42i = 0;
    c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _c39r + _exp42r; _add43i = _c39i + _exp42i;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c38r, _c38i, _add43r, _add43i, &_mul44r, &_mul44i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 0.001; _c45i = 0;
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 1.0;
    double _c47r = 0, _c47i = 0;
    _c47r = 3.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow48r, &_pow48i);
    c_mul(_pow48r, _pow48i, x2r, x2i, &_pow48r, &_pow48i);
    double _mul49r = 0, _mul49i = 0;
    c_mul(_c46r, _c46i, _pow48r, _pow48i, &_mul49r, &_mul49i);
    double _add50r = 0, _add50i = 0;
    _add50r = _c45r + _mul49r; _add50i = _c45i + _mul49i;
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_766_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 1.0;
    double _abs3r = 0, _abs3i = 0;
    _abs3r = c_abs(x1r, x1i); _abs3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c2r, _c2i, _abs3r, _abs3i, &_mul4r, &_mul4i);
    double _exp5r = 0, _exp5i = 0;
    c_exp2(_mul4r, _mul4i, &_exp5r, &_exp5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c1r, _c1i, _exp5r, _exp5i, &_mul6r, &_mul6i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul6r; cIm[_idx] = _mul6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 10.0; _c7i = 0;
    double _re8r = 0, _re8i = 0;
    _re8r = x1r; _re8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(_re8r, _re8i, _re8r, _re8i, &_pow10r, &_pow10i);
    double _im11r = 0, _im11i = 0;
    _im11r = x2i; _im11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 2.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_mul(_im11r, _im11i, _im11r, _im11i, &_pow13r, &_pow13i);
    double _sub14r = 0, _sub14i = 0;
    _sub14r = _pow10r - _pow13r; _sub14i = _pow10i - _pow13i;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c7r, _c7i, _sub14r, _sub14i, &_mul15r, &_mul15i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 1.0;
    double _re17r = 0, _re17i = 0;
    _re17r = x2r; _re17i = 0;
    double _im18r = 0, _im18i = 0;
    _im18r = x1i; _im18i = 0;
    double _add19r = 0, _add19i = 0;
    _add19r = _re17r + _im18r; _add19i = _re17i + _im18i;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c16r, _c16i, _add19r, _add19i, &_mul20r, &_mul20i);
    double _re21r = 0, _re21i = 0;
    _re21r = x1r; _re21i = 0;
    double _im22r = 0, _im22i = 0;
    _im22r = x2i; _im22i = 0;
    double _sub23r = 0, _sub23i = 0;
    _sub23r = _re21r - _im22r; _sub23i = _re21i - _im22i;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_mul20r, _mul20i, _sub23r, _sub23i, &_mul24r, &_mul24i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    double _add25r = 0, _add25i = 0;
    _add25r = x1r + x2r; _add25i = x1i + x2i;
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(_add25r, _add25i, _add25r, _add25i, &_pow27r, &_pow27i);
    double _sub28r = 0, _sub28i = 0;
    _sub28r = x1r - x2r; _sub28i = x1i - x2i;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_sub28r, _sub28i, _sub28r, _sub28i, &_pow30r, &_pow30i);
    double _sub31r = 0, _sub31i = 0;
    _sub31r = _pow27r - _pow30r; _sub31i = _pow27i - _pow30i;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub31r; cIm[_idx] = _sub31i; } }
    double _c32r = 0, _c32i = 0;
    _c32r = 100.0; _c32i = 0;
    double _add33r = 0, _add33i = 0;
    _add33r = x1r + x2r; _add33i = x1i + x2i;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c32r, _c32i, _add33r, _add33i, &_mul34r, &_mul34i);
    double _c35r = 0, _c35i = 0;
    _c35r = 1.0; _c35i = 0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul36r, &_mul36i);
    double _abs37r = 0, _abs37i = 0;
    _abs37r = c_abs(_mul36r, _mul36i); _abs37i = 0;
    double _add38r = 0, _add38i = 0;
    _add38r = _c35r + _abs37r; _add38i = _c35i + _abs37i;
    double _div39r = 0, _div39i = 0;
    c_div(_mul34r, _mul34i, _add38r, _add38i, &_div39r, &_div39i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div39r; cIm[_idx] = _div39i; } }
    double _unk40r = 0, _unk40i = 0;
    /* WARNING: unhandled node GeneratorExp(elt=BinOp(left=Attribute(value=BinOp(left=Name(id='t1', ctx=Load()), op=Pow(), right=Name(id='k', ctx=Load())), attr='real', ctx=Load()), op=Mult(), right=Attribute(value=BinOp(left=Name(id='t2', ctx=Load()), op=Pow(), right=Name(id='k', ctx=Load())), attr='imag', ctx=Load())), generators=[comprehension(target=Name(id='k', ctx=Store()), iter=Call(func=Name(id='range', ctx=Load()), args=[Constant(value=1), Constant(value=6)]), is_async=0)]) */
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _unk40r; cIm[_idx] = _unk40i; } }
    double _c41r = 0, _c41i = 0;
    _c41r = 2.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow42r, &_pow42i);
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow44r, &_pow44i);
    double _sub45r = 0, _sub45i = 0;
    _sub45r = _pow42r - _pow44r; _sub45i = _pow42i - _pow44i;
    double _abs46r = 0, _abs46i = 0;
    _abs46r = c_abs(_sub45r, _sub45i); _abs46i = 0;
    double _sqrt47r = 0, _sqrt47i = 0;
    c_powr(_abs46r, _abs46i, 0.5, &_sqrt47r, &_sqrt47i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sqrt47r; cIm[_idx] = _sqrt47i; } }
    double _mul48r = 0, _mul48i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul48r, &_mul48i);
    double _c49r = 0, _c49i = 0;
    _c49r = 1.0; _c49i = 0;
    double _sub50r = 0, _sub50i = 0;
    _sub50r = x1r - x2r; _sub50i = x1i - x2i;
    double _abs51r = 0, _abs51i = 0;
    _abs51r = c_abs(_sub50r, _sub50i); _abs51i = 0;
    double _add52r = 0, _add52i = 0;
    _add52r = _c49r + _abs51r; _add52i = _c49i + _abs51i;
    double _div53r = 0, _div53i = 0;
    c_div(_mul48r, _mul48i, _add52r, _add52i, &_div53r, &_div53i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div53r; cIm[_idx] = _div53i; } }
    double _attr54r = 0, _attr54i = 0;
    _attr54r = x1r; _attr54i = 0;
    double _attr55r = 0, _attr55i = 0;
    _attr55r = x2i; _attr55i = 0;
    double _math56r = 0, _math56i = 0;
    /* WARNING: unhandled math.prod */
    double _math57r = 0, _math57i = 0;
    /* WARNING: unhandled math.prod */
    double _tern58r = 0, _tern58i = 0;
    if (_attr54r <= _attr55r) { _tern58r = _math56r; _tern58i = _math56i; }
    else { _tern58r = _math57r; _tern58i = _math57i; }
    double _c59r = 0, _c59i = 0;
    _c59r = 0.0; _c59i = 1.0;
    double _attr60r = 0, _attr60i = 0;
    _attr60r = x2r; _attr60i = 0;
    double _attr61r = 0, _attr61i = 0;
    _attr61r = x1i; _attr61i = 0;
    double _math62r = 0, _math62i = 0;
    /* WARNING: unhandled math.prod */
    double _math63r = 0, _math63i = 0;
    /* WARNING: unhandled math.prod */
    double _tern64r = 0, _tern64i = 0;
    if (_attr60r <= _attr61r) { _tern64r = _math62r; _tern64i = _math62i; }
    else { _tern64r = _math63r; _tern64i = _math63i; }
    double _mul65r = 0, _mul65i = 0;
    c_mul(_c59r, _c59i, _tern64r, _tern64i, &_mul65r, &_mul65i);
    double _add66r = 0, _add66i = 0;
    _add66r = _tern58r + _mul65r; _add66i = _tern58i + _mul65i;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add66r; cIm[_idx] = _add66i; } }
    double _c67r = 0, _c67i = 0;
    _c67r = 0.0; _c67i = 1.0;
    double _re68r = 0, _re68i = 0;
    _re68r = x1r; _re68i = 0;
    double _im69r = 0, _im69i = 0;
    _im69r = x2i; _im69i = 0;
    double _sub70r = 0, _sub70i = 0;
    _sub70r = _re68r - _im69r; _sub70i = _re68i - _im69i;
    double _mul71r = 0, _mul71i = 0;
    c_mul(_c67r, _c67i, _sub70r, _sub70i, &_mul71r, &_mul71i);
    double _exp72r = 0, _exp72i = 0;
    c_exp2(_mul71r, _mul71i, &_exp72r, &_exp72i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp72r; cIm[_idx] = _exp72i; } }
    double _c73r = 0, _c73i = 0;
    _c73r = 0.0; _c73i = 1.0;
    double _neg74r = 0, _neg74i = 0;
    _neg74r = -(_c73r); _neg74i = -(_c73i);
    double _re75r = 0, _re75i = 0;
    _re75r = x2r; _re75i = 0;
    double _im76r = 0, _im76i = 0;
    _im76r = x1i; _im76i = 0;
    double _sub77r = 0, _sub77i = 0;
    _sub77r = _re75r - _im76r; _sub77i = _re75i - _im76i;
    double _mul78r = 0, _mul78i = 0;
    c_mul(_neg74r, _neg74i, _sub77r, _sub77i, &_mul78r, &_mul78i);
    double _exp79r = 0, _exp79i = 0;
    c_exp2(_mul78r, _mul78i, &_exp79r, &_exp79i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp79r; cIm[_idx] = _exp79i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_767_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _im2r = 0, _im2i = 0;
    _im2r = x2i; _im2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_re1r, _re1i, _im2r, _im2i, &_mul3r, &_mul3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul4r, &_mul4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _mul3r + _mul4r; _add5i = _mul3i + _mul4i;
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 1.0;
    double _add7r = 0, _add7i = 0;
    _add7r = _add5r + _c6r; _add7i = _add5i + _c6i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 3.0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c8r, _c8i, _pow10r, _pow10i, &_mul11r, &_mul11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 0.0; _c12i = 2.0;
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow14r, &_pow14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c12r, _c12i, _pow14r, _pow14i, &_mul15r, &_mul15i);
    double _add16r = 0, _add16i = 0;
    _add16r = _mul11r + _mul15r; _add16i = _mul11i + _mul15i;
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c17r, _c17i, x2r, x2i, &_mul18r, &_mul18i);
    double _sub19r = 0, _sub19i = 0;
    _sub19r = _add16r - _mul18r; _sub19i = _add16i - _mul18i;
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 1.0;
    double _add21r = 0, _add21i = 0;
    _add21r = _sub19r + _c20r; _add21i = _sub19i + _c20i;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    double _c22r = 0, _c22i = 0;
    _c22r = 3.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow23r, &_pow23i);
    c_mul(_pow23r, _pow23i, x1r, x1i, &_pow23r, &_pow23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 3.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow25r, &_pow25i);
    c_mul(_pow25r, _pow25i, x2r, x2i, &_pow25r, &_pow25i);
    double _add26r = 0, _add26i = 0;
    _add26r = _pow23r + _pow25r; _add26i = _pow23i + _pow25i;
    double _sin27r = 0, _sin27i = 0;
    c_sin(_add26r, _add26i, &_sin27r, &_sin27i);
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _cos29r = 0, _cos29i = 0;
    c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
    double _add30r = 0, _add30i = 0;
    _add30r = _sin27r + _cos29r; _add30i = _sin27i + _cos29i;
    double _c31r = 0, _c31i = 0;
    _c31r = 2.0; _c31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = _add30r + _c31r; _add32i = _add30i + _c31i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 0.0; _c33i = 1.0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c33r, _c33i, x1r, x1i, &_mul34r, &_mul34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_mul34r, _mul34i, x2r, x2i, &_mul35r, &_mul35i);
    double _exp36r = 0, _exp36i = 0;
    c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
    double _c37r = 0, _c37i = 0;
    _c37r = 0.0; _c37i = 2.0;
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c37r, _c37i, x1r, x1i, &_mul38r, &_mul38i);
    double _add39r = 0, _add39i = 0;
    _add39r = _exp36r + _mul38r; _add39i = _exp36i + _mul38i;
    double _sub40r = 0, _sub40i = 0;
    _sub40r = _add39r - x2r; _sub40i = _add39i - x2i;
    double _c41r = 0, _c41i = 0;
    _c41r = 2.0; _c41i = 0;
    double _add42r = 0, _add42i = 0;
    _add42r = _sub40r + _c41r; _add42i = _sub40i + _c41i;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    double _c43r = 0, _c43i = 0;
    _c43r = 3.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow44r, &_pow44i);
    c_mul(_pow44r, _pow44i, x1r, x1i, &_pow44r, &_pow44i);
    double _c45r = 0, _c45i = 0;
    _c45r = 3.0; _c45i = 0;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c45r, _c45i, x2r, x2i, &_mul46r, &_mul46i);
    double _sub47r = 0, _sub47i = 0;
    _sub47r = _pow44r - _mul46r; _sub47i = _pow44i - _mul46i;
    double _add48r = 0, _add48i = 0;
    _add48r = x1r + x2r; _add48i = x1i + x2i;
    double _im49r = 0, _im49i = 0;
    _im49r = _add48i; _im49i = 0;
    double _c50r = 0, _c50i = 0;
    _c50r = 2.0; _c50i = 0;
    double _pow51r = 0, _pow51i = 0;
    c_mul(_im49r, _im49i, _im49r, _im49i, &_pow51r, &_pow51i);
    double _add52r = 0, _add52i = 0;
    _add52r = _sub47r + _pow51r; _add52i = _sub47i + _pow51i;
    double _c53r = 0, _c53i = 0;
    _c53r = 0.0; _c53i = 1.0;
    double _add54r = 0, _add54i = 0;
    _add54r = _add52r + _c53r; _add54i = _add52i + _c53i;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add54r; cIm[_idx] = _add54i; } }
    double _re55r = 0, _re55i = 0;
    _re55r = x1r; _re55i = 0;
    double _sin56r = 0, _sin56i = 0;
    c_sin(x2r, x2i, &_sin56r, &_sin56i);
    double _mul57r = 0, _mul57i = 0;
    c_mul(_re55r, _re55i, _sin56r, _sin56i, &_mul57r, &_mul57i);
    double _c58r = 0, _c58i = 0;
    _c58r = 1.0; _c58i = 0;
    double _add59r = 0, _add59i = 0;
    _add59r = x2r + _c58r; _add59i = x2i + _c58i;
    double _div60r = 0, _div60i = 0;
    c_div(x1r, x1i, _add59r, _add59i, &_div60r, &_div60i);
    double _add61r = 0, _add61i = 0;
    _add61r = _mul57r + _div60r; _add61i = _mul57i + _div60i;
    double _c62r = 0, _c62i = 0;
    _c62r = 0.0; _c62i = 2.0;
    double _add63r = 0, _add63i = 0;
    _add63r = _add61r + _c62r; _add63i = _add61i + _c62i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add63r; cIm[_idx] = _add63i; } }
    double _cos64r = 0, _cos64i = 0;
    c_cos(x1r, x1i, &_cos64r, &_cos64i);
    double _c65r = 0, _c65i = 0;
    _c65r = 1.0; _c65i = 0;
    double _abs66r = 0, _abs66i = 0;
    _abs66r = c_abs(x2r, x2i); _abs66i = 0;
    double _add67r = 0, _add67i = 0;
    _add67r = _c65r + _abs66r; _add67i = _c65i + _abs66i;
    double _div68r = 0, _div68i = 0;
    c_div(_cos64r, _cos64i, _add67r, _add67i, &_div68r, &_div68i);
    double _add69r = 0, _add69i = 0;
    _add69r = x1r + x2r; _add69i = x1i + x2i;
    double _sin70r = 0, _sin70i = 0;
    c_sin(_add69r, _add69i, &_sin70r, &_sin70i);
    double _add71r = 0, _add71i = 0;
    _add71r = _div68r + _sin70r; _add71i = _div68i + _sin70i;
    double _c72r = 0, _c72i = 0;
    _c72r = 2.0; _c72i = 0;
    double _add73r = 0, _add73i = 0;
    _add73r = _add71r + _c72r; _add73i = _add71i + _c72i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add73r; cIm[_idx] = _add73i; } }
    double _c74r = 0, _c74i = 0;
    _c74r = 0.0; _c74i = 1.0;
    double _mul75r = 0, _mul75i = 0;
    c_mul(_c74r, _c74i, x1r, x1i, &_mul75r, &_mul75i);
    double _exp76r = 0, _exp76i = 0;
    c_exp2(_mul75r, _mul75i, &_exp76r, &_exp76i);
    double _c77r = 0, _c77i = 0;
    _c77r = 0.0; _c77i = 1.0;
    double _mul78r = 0, _mul78i = 0;
    c_mul(_c77r, _c77i, x2r, x2i, &_mul78r, &_mul78i);
    double _exp79r = 0, _exp79i = 0;
    c_exp2(_mul78r, _mul78i, &_exp79r, &_exp79i);
    double _sub80r = 0, _sub80i = 0;
    _sub80r = _exp76r - _exp79r; _sub80i = _exp76i - _exp79i;
    double _add81r = 0, _add81i = 0;
    _add81r = x1r + x2r; _add81i = x1i + x2i;
    double _abs82r = 0, _abs82i = 0;
    _abs82r = c_abs(_add81r, _add81i); _abs82i = 0;
    double _sqrt83r = 0, _sqrt83i = 0;
    c_powr(_abs82r, _abs82i, 0.5, &_sqrt83r, &_sqrt83i);
    double _add84r = 0, _add84i = 0;
    _add84r = _sub80r + _sqrt83r; _add84i = _sub80i + _sqrt83i;
    double _c85r = 0, _c85i = 0;
    _c85r = 0.0; _c85i = 1.0;
    double _add86r = 0, _add86i = 0;
    _add86r = _add84r + _c85r; _add86i = _add84i + _c85i;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add86r; cIm[_idx] = _add86i; } }
    double _abs87r = 0, _abs87i = 0;
    _abs87r = c_abs(x1r, x1i); _abs87i = 0;
    double _abs88r = 0, _abs88i = 0;
    _abs88r = c_abs(x2r, x2i); _abs88i = 0;
    double _mul89r = 0, _mul89i = 0;
    c_mul(_abs87r, _abs87i, _abs88r, _abs88i, &_mul89r, &_mul89i);
    double _re90r = 0, _re90i = 0;
    _re90r = x1r; _re90i = 0;
    double _sin91r = 0, _sin91i = 0;
    c_sin(_re90r, _re90i, &_sin91r, &_sin91i);
    double _im92r = 0, _im92i = 0;
    _im92r = x2i; _im92i = 0;
    double _cos93r = 0, _cos93i = 0;
    c_cos(_im92r, _im92i, &_cos93r, &_cos93i);
    double _mul94r = 0, _mul94i = 0;
    c_mul(_sin91r, _sin91i, _cos93r, _cos93i, &_mul94r, &_mul94i);
    double _sub95r = 0, _sub95i = 0;
    _sub95r = _mul89r - _mul94r; _sub95i = _mul89i - _mul94i;
    double _c96r = 0, _c96i = 0;
    _c96r = 0.0; _c96i = 1.0;
    double _add97r = 0, _add97i = 0;
    _add97r = _sub95r + _c96r; _add97i = _sub95i + _c96i;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add97r; cIm[_idx] = _add97i; } }
    double _c98r = 0, _c98i = 0;
    _c98r = 2.0; _c98i = 0;
    double _pow99r = 0, _pow99i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow99r, &_pow99i);
    double _c100r = 0, _c100i = 0;
    _c100r = 1.0; _c100i = 0;
    double _add101r = 0, _add101i = 0;
    _add101r = x2r + _c100r; _add101i = x2i + _c100i;
    double _mul102r = 0, _mul102i = 0;
    c_mul(_pow99r, _pow99i, _add101r, _add101i, &_mul102r, &_mul102i);
    double _c103r = 0, _c103i = 0;
    _c103r = 1.0; _c103i = 0;
    double _add104r = 0, _add104i = 0;
    _add104r = x1r + _c103r; _add104i = x1i + _c103i;
    double _c105r = 0, _c105i = 0;
    _c105r = 2.0; _c105i = 0;
    double _pow106r = 0, _pow106i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow106r, &_pow106i);
    double _mul107r = 0, _mul107i = 0;
    c_mul(_add104r, _add104i, _pow106r, _pow106i, &_mul107r, &_mul107i);
    double _sub108r = 0, _sub108i = 0;
    _sub108r = _mul102r - _mul107r; _sub108i = _mul102i - _mul107i;
    double _c109r = 0, _c109i = 0;
    _c109r = 0.0; _c109i = 1.0;
    double _add110r = 0, _add110i = 0;
    _add110r = _sub108r + _c109r; _add110i = _sub108i + _c109i;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add110r; cIm[_idx] = _add110i; } }
    double _c111r = 0, _c111i = 0;
    _c111r = 0.0; _c111i = 1.0;
    double _mul112r = 0, _mul112i = 0;
    c_mul(_c111r, _c111i, x2r, x2i, &_mul112r, &_mul112i);
    double _re113r = 0, _re113i = 0;
    _re113r = x1r; _re113i = 0;
    double _add114r = 0, _add114i = 0;
    _add114r = _mul112r + _re113r; _add114i = _mul112i + _re113i;
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add114r; cIm[_idx] = _add114i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_768_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 4.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, _pow3r, _pow3i, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 4.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    c_mul(_pow5r, _pow5i, _pow5r, _pow5i, &_pow5r, &_pow5i);
    double _sub6r = 0, _sub6i = 0;
    _sub6r = _pow3r - _pow5r; _sub6i = _pow3i - _pow5i;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c1r, _c1i, _sub6r, _sub6i, &_mul7r, &_mul7i);
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 1.0;
    double _sub9r = 0, _sub9i = 0;
    _sub9r = x1r - x2r; _sub9i = x1i - x2i;
    double _c10r = 0, _c10i = 0;
    _c10r = 4.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(_sub9r, _sub9i, _sub9r, _sub9i, &_pow11r, &_pow11i);
    c_mul(_pow11r, _pow11i, _pow11r, _pow11i, &_pow11r, &_pow11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c8r, _c8i, _pow11r, _pow11i, &_mul12r, &_mul12i);
    double _exp13r = 0, _exp13i = 0;
    c_exp2(_mul12r, _mul12i, &_exp13r, &_exp13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_mul7r, _mul7i, _exp13r, _exp13i, &_mul14r, &_mul14i);
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 50.0;
    double _c16r = 0, _c16i = 0;
    _c16r = 3.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow17r, &_pow17i);
    c_mul(_pow17r, _pow17i, x2r, x2i, &_pow17r, &_pow17i);
    double _add18r = 0, _add18i = 0;
    _add18r = x1r + _pow17r; _add18i = x1i + _pow17i;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c15r, _c15i, _add18r, _add18i, &_mul19r, &_mul19i);
    { int _idx = 1; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 100.0; _c20i = 0;
    double _sub21r = 0, _sub21i = 0;
    _sub21r = x1r - x2r; _sub21i = x1i - x2i;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c20r, _c20i, _sub21r, _sub21i, &_mul22r, &_mul22i);
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _neg24r = 0, _neg24i = 0;
    _neg24r = -(_c23r); _neg24i = -(_c23i);
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_neg24r, _neg24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _exp28r = 0, _exp28i = 0;
    c_exp2(_mul27r, _mul27i, &_exp28r, &_exp28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_mul22r, _mul22i, _exp28r, _exp28i, &_mul29r, &_mul29i);
    double _c30r = 0, _c30i = 0;
    _c30r = 50.0; _c30i = 0;
    double _add31r = 0, _add31i = 0;
    _add31r = x1r + x2r; _add31i = x1i + x2i;
    double _c32r = 0, _c32i = 0;
    _c32r = 2.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(_add31r, _add31i, _add31r, _add31i, &_pow33r, &_pow33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c30r, _c30i, _pow33r, _pow33i, &_mul34r, &_mul34i);
    double _add35r = 0, _add35i = 0;
    _add35r = _mul29r + _mul34r; _add35i = _mul29i + _mul34i;
    { int _idx = 2; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    double _c36r = 0, _c36i = 0;
    _c36r = 0.0; _c36i = 25.0;
    double _neg37r = 0, _neg37i = 0;
    _neg37r = -(_c36r); _neg37i = -(_c36i);
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _mul39r = 0, _mul39i = 0;
    c_mul(_c38r, _c38i, x2r, x2i, &_mul39r, &_mul39i);
    double _sub40r = 0, _sub40i = 0;
    _sub40r = x1r - _mul39r; _sub40i = x1i - _mul39i;
    double _c41r = 0, _c41i = 0;
    _c41r = 3.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_mul(_sub40r, _sub40i, _sub40r, _sub40i, &_pow42r, &_pow42i);
    c_mul(_pow42r, _pow42i, _sub40r, _sub40i, &_pow42r, &_pow42i);
    double _mul43r = 0, _mul43i = 0;
    c_mul(_neg37r, _neg37i, _pow42r, _pow42i, &_mul43r, &_mul43i);
    { int _idx = 3; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 0.0; _c44i = 3.0;
    double _mul45r = 0, _mul45i = 0;
    c_mul(_c44r, _c44i, x2r, x2i, &_mul45r, &_mul45i);
    double _add46r = 0, _add46i = 0;
    _add46r = x1r + _mul45r; _add46i = x1i + _mul45i;
    double _c47r = 0, _c47i = 0;
    _c47r = 4.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(_add46r, _add46i, _add46r, _add46i, &_pow48r, &_pow48i);
    c_mul(_pow48r, _pow48i, _pow48r, _pow48i, &_pow48r, &_pow48i);
    double _c49r = 0, _c49i = 0;
    _c49r = 0.0; _c49i = 50.0;
    double _c50r = 0, _c50i = 0;
    _c50r = 0.0; _c50i = 2.0;
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c50r, _c50i, x2r, x2i, &_mul51r, &_mul51i);
    double _add52r = 0, _add52i = 0;
    _add52r = x1r + _mul51r; _add52i = x1i + _mul51i;
    double _c53r = 0, _c53i = 0;
    _c53r = 2.0; _c53i = 0;
    double _pow54r = 0, _pow54i = 0;
    c_mul(_add52r, _add52i, _add52r, _add52i, &_pow54r, &_pow54i);
    double _mul55r = 0, _mul55i = 0;
    c_mul(_c49r, _c49i, _pow54r, _pow54i, &_mul55r, &_mul55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _pow48r + _mul55r; _add56i = _pow48i + _mul55i;
    { int _idx = 4; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    double _c57r = 0, _c57i = 0;
    _c57r = 75.0; _c57i = 0;
    double _c58r = 0, _c58i = 0;
    _c58r = 0.0; _c58i = 1.0;
    double _sub59r = 0, _sub59i = 0;
    _sub59r = x1r - _c58r; _sub59i = x1i - _c58i;
    double _c60r = 0, _c60i = 0;
    _c60r = 4.0; _c60i = 0;
    double _pow61r = 0, _pow61i = 0;
    c_mul(_sub59r, _sub59i, _sub59r, _sub59i, &_pow61r, &_pow61i);
    c_mul(_pow61r, _pow61i, _pow61r, _pow61i, &_pow61r, &_pow61i);
    double _mul62r = 0, _mul62i = 0;
    c_mul(_c57r, _c57i, _pow61r, _pow61i, &_mul62r, &_mul62i);
    double _c63r = 0, _c63i = 0;
    _c63r = 100.0; _c63i = 0;
    double _c64r = 0, _c64i = 0;
    _c64r = 0.0; _c64i = 2.0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(_c64r, _c64i, x2r, x2i, &_mul65r, &_mul65i);
    double _add66r = 0, _add66i = 0;
    _add66r = _mul65r + x1r; _add66i = _mul65i + x1i;
    double _c67r = 0, _c67i = 0;
    _c67r = 2.0; _c67i = 0;
    double _pow68r = 0, _pow68i = 0;
    c_mul(_add66r, _add66i, _add66r, _add66i, &_pow68r, &_pow68i);
    double _mul69r = 0, _mul69i = 0;
    c_mul(_c63r, _c63i, _pow68r, _pow68i, &_mul69r, &_mul69i);
    double _add70r = 0, _add70i = 0;
    _add70r = _mul62r + _mul69r; _add70i = _mul62i + _mul69i;
    { int _idx = 5; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add70r; cIm[_idx] = _add70i; } }
    double _c71r = 0, _c71i = 0;
    _c71r = 0.0; _c71i = 100.0;
    double _neg72r = 0, _neg72i = 0;
    _neg72r = -(_c71r); _neg72i = -(_c71i);
    double _c73r = 0, _c73i = 0;
    _c73r = 0.0; _c73i = 1.0;
    double _mul74r = 0, _mul74i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul74r, &_mul74i);
    double _abs75r = 0, _abs75i = 0;
    _abs75r = c_abs(_mul74r, _mul74i); _abs75i = 0;
    double _mul76r = 0, _mul76i = 0;
    c_mul(_c73r, _c73i, _abs75r, _abs75i, &_mul76r, &_mul76i);
    double _exp77r = 0, _exp77i = 0;
    c_exp2(_mul76r, _mul76i, &_exp77r, &_exp77i);
    double _mul78r = 0, _mul78i = 0;
    c_mul(_neg72r, _neg72i, _exp77r, _exp77i, &_mul78r, &_mul78i);
    { int _idx = 6; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul78r; cIm[_idx] = _mul78i; } }
    double _c79r = 0, _c79i = 0;
    _c79r = 0.0; _c79i = 50.0;
    double _c80r = 0, _c80i = 0;
    _c80r = 2.0; _c80i = 0;
    double _pow81r = 0, _pow81i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow81r, &_pow81i);
    double _c82r = 0, _c82i = 0;
    _c82r = 2.0; _c82i = 0;
    double _pow83r = 0, _pow83i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow83r, &_pow83i);
    double _add84r = 0, _add84i = 0;
    _add84r = _pow81r + _pow83r; _add84i = _pow81i + _pow83i;
    double _c85r = 0, _c85i = 0;
    _c85r = 2.0; _c85i = 0;
    double _pow86r = 0, _pow86i = 0;
    c_mul(_add84r, _add84i, _add84r, _add84i, &_pow86r, &_pow86i);
    double _mul87r = 0, _mul87i = 0;
    c_mul(_c79r, _c79i, _pow86r, _pow86i, &_mul87r, &_mul87i);
    double _c88r = 0, _c88i = 0;
    _c88r = 25.0; _c88i = 0;
    double _c89r = 0, _c89i = 0;
    _c89r = 0.0; _c89i = 1.0;
    double _sub90r = 0, _sub90i = 0;
    _sub90r = x1r - _c89r; _sub90i = x1i - _c89i;
    double _c91r = 0, _c91i = 0;
    _c91r = 4.0; _c91i = 0;
    double _pow92r = 0, _pow92i = 0;
    c_mul(_sub90r, _sub90i, _sub90r, _sub90i, &_pow92r, &_pow92i);
    c_mul(_pow92r, _pow92i, _pow92r, _pow92i, &_pow92r, &_pow92i);
    double _mul93r = 0, _mul93i = 0;
    c_mul(_c88r, _c88i, _pow92r, _pow92i, &_mul93r, &_mul93i);
    double _add94r = 0, _add94i = 0;
    _add94r = _mul87r + _mul93r; _add94i = _mul87i + _mul93i;
    { int _idx = 7; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add94r; cIm[_idx] = _add94i; } }
    double _c95r = 0, _c95i = 0;
    _c95r = 75.0; _c95i = 0;
    double _c96r = 0, _c96i = 0;
    _c96r = 0.0; _c96i = 1.0;
    double _neg97r = 0, _neg97i = 0;
    _neg97r = -(_c96r); _neg97i = -(_c96i);
    double _c98r = 0, _c98i = 0;
    _c98r = 2.0; _c98i = 0;
    double _pow99r = 0, _pow99i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow99r, &_pow99i);
    double _mul100r = 0, _mul100i = 0;
    c_mul(_neg97r, _neg97i, _pow99r, _pow99i, &_mul100r, &_mul100i);
    double _exp101r = 0, _exp101i = 0;
    c_exp2(_mul100r, _mul100i, &_exp101r, &_exp101i);
    double _mul102r = 0, _mul102i = 0;
    c_mul(_c95r, _c95i, _exp101r, _exp101i, &_mul102r, &_mul102i);
    double _c103r = 0, _c103i = 0;
    _c103r = 0.0; _c103i = 100.0;
    double _c104r = 0, _c104i = 0;
    _c104r = 4.0; _c104i = 0;
    double _pow105r = 0, _pow105i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow105r, &_pow105i);
    c_mul(_pow105r, _pow105i, _pow105r, _pow105i, &_pow105r, &_pow105i);
    double _mul106r = 0, _mul106i = 0;
    c_mul(_c103r, _c103i, _pow105r, _pow105i, &_mul106r, &_mul106i);
    double _add107r = 0, _add107i = 0;
    _add107r = _mul102r + _mul106r; _add107i = _mul102i + _mul106i;
    { int _idx = 8; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add107r; cIm[_idx] = _add107i; } }
    double _c108r = 0, _c108i = 0;
    _c108r = 100.0; _c108i = 0;
    double _mul109r = 0, _mul109i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul109r, &_mul109i);
    double _c110r = 0, _c110i = 0;
    _c110r = 4.0; _c110i = 0;
    double _pow111r = 0, _pow111i = 0;
    c_mul(_mul109r, _mul109i, _mul109r, _mul109i, &_pow111r, &_pow111i);
    c_mul(_pow111r, _pow111i, _pow111r, _pow111i, &_pow111r, &_pow111i);
    double _mul112r = 0, _mul112i = 0;
    c_mul(_c108r, _c108i, _pow111r, _pow111i, &_mul112r, &_mul112i);
    double _c113r = 0, _c113i = 0;
    _c113r = 0.0; _c113i = 25.0;
    double _add114r = 0, _add114i = 0;
    _add114r = x1r + x2r; _add114i = x1i + x2i;
    double _c115r = 0, _c115i = 0;
    _c115r = 3.0; _c115i = 0;
    double _pow116r = 0, _pow116i = 0;
    c_mul(_add114r, _add114i, _add114r, _add114i, &_pow116r, &_pow116i);
    c_mul(_pow116r, _pow116i, _add114r, _add114i, &_pow116r, &_pow116i);
    double _c117r = 0, _c117i = 0;
    _c117r = 2.0; _c117i = 0;
    double _pow118r = 0, _pow118i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow118r, &_pow118i);
    double _c119r = 0, _c119i = 0;
    _c119r = 2.0; _c119i = 0;
    double _pow120r = 0, _pow120i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow120r, &_pow120i);
    double _mul121r = 0, _mul121i = 0;
    c_mul(_pow118r, _pow118i, _pow120r, _pow120i, &_mul121r, &_mul121i);
    double _add122r = 0, _add122i = 0;
    _add122r = _pow116r + _mul121r; _add122i = _pow116i + _mul121i;
    double _mul123r = 0, _mul123i = 0;
    c_mul(_c113r, _c113i, _add122r, _add122i, &_mul123r, &_mul123i);
    double _sub124r = 0, _sub124i = 0;
    _sub124r = _mul112r - _mul123r; _sub124i = _mul112i - _mul123i;
    { int _idx = 9; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub124r; cIm[_idx] = _sub124i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_769_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    double _sin2r = 0, _sin2i = 0;
    c_sin(x1r, x1i, &_sin2r, &_sin2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(_sin2r, _sin2i, _sin2r, _sin2i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, _sin2r, _sin2i, &_pow4r, &_pow4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c1r, _c1i, _pow4r, _pow4i, &_mul5r, &_mul5i);
    double _cos6r = 0, _cos6i = 0;
    c_cos(x2r, x2i, &_cos6r, &_cos6i);
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(_cos6r, _cos6i, _cos6r, _cos6i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_mul5r, _mul5i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 100.0; _c10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 0.0; _c11i = 1.0;
    double _add12r = 0, _add12i = 0;
    _add12r = x1r + x2r; _add12i = x1i + x2i;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c11r, _c11i, _add12r, _add12i, &_mul13r, &_mul13i);
    double _exp14r = 0, _exp14i = 0;
    c_exp2(_mul13r, _mul13i, &_exp14r, &_exp14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c10r, _c10i, _exp14r, _exp14i, &_mul15r, &_mul15i);
    double _c16r = 0, _c16i = 0;
    _c16r = 10.0; _c16i = 0;
    double _sub17r = 0, _sub17i = 0;
    _sub17r = x1r - x2r; _sub17i = x1i - x2i;
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(_sub17r, _sub17i, _sub17r, _sub17i, &_pow19r, &_pow19i);
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c16r, _c16i, _pow19r, _pow19i, &_mul20r, &_mul20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = _mul15r - _mul20r; _sub21i = _mul15i - _mul20i;
    { int _idx = 1; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub21r; cIm[_idx] = _sub21i; } }
    double _mul22r = 0, _mul22i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul22r, &_mul22i);
    double _sub23r = 0, _sub23i = 0;
    _sub23r = x1r - x2r; _sub23i = x1i - x2i;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_mul22r, _mul22i, _sub23r, _sub23i, &_mul24r, &_mul24i);
    double _abs25r = 0, _abs25i = 0;
    _abs25r = c_abs(x1r, x1i); _abs25i = 0;
    double _abs26r = 0, _abs26i = 0;
    _abs26r = c_abs(x2r, x2i); _abs26i = 0;
    double _add27r = 0, _add27i = 0;
    _add27r = _abs25r + _abs26r; _add27i = _abs25i + _abs26i;
    double _c28r = 0, _c28i = 0;
    _c28r = 1.0; _c28i = 0;
    double _add29r = 0, _add29i = 0;
    _add29r = _add27r + _c28r; _add29i = _add27i + _c28i;
    double _div30r = 0, _div30i = 0;
    c_div(_mul24r, _mul24i, _add29r, _add29i, &_div30r, &_div30i);
    { int _idx = 2; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div30r; cIm[_idx] = _div30i; } }
    double _mul31r = 0, _mul31i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul31r, &_mul31i);
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 1.0;
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow34r, &_pow34i);
    double _c35r = 0, _c35i = 0;
    _c35r = 2.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow36r, &_pow36i);
    double _sub37r = 0, _sub37i = 0;
    _sub37r = _pow34r - _pow36r; _sub37i = _pow34i - _pow36i;
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c32r, _c32i, _sub37r, _sub37i, &_mul38r, &_mul38i);
    double _exp39r = 0, _exp39i = 0;
    c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_mul31r, _mul31i, _exp39r, _exp39i, &_mul40r, &_mul40i);
    double _c41r = 0, _c41i = 0;
    _c41r = 3.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_mul(_mul40r, _mul40i, _mul40r, _mul40i, &_pow42r, &_pow42i);
    c_mul(_pow42r, _pow42i, _mul40r, _mul40i, &_pow42r, &_pow42i);
    { int _idx = 4; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _pow42r; cIm[_idx] = _pow42i; } }
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(x1r, x1i); _abs43i = 0;
    double _sqrt44r = 0, _sqrt44i = 0;
    c_powr(_abs43r, _abs43i, 0.5, &_sqrt44r, &_sqrt44i);
    double _abs45r = 0, _abs45i = 0;
    _abs45r = c_abs(x2r, x2i); _abs45i = 0;
    double _sqrt46r = 0, _sqrt46i = 0;
    c_powr(_abs45r, _abs45i, 0.5, &_sqrt46r, &_sqrt46i);
    double _sub47r = 0, _sub47i = 0;
    _sub47r = _sqrt44r - _sqrt46r; _sub47i = _sqrt44i - _sqrt46i;
    double _c48r = 0, _c48i = 0;
    _c48r = 0.0; _c48i = 1.0;
    double _mul49r = 0, _mul49i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul49r, &_mul49i);
    double _sin50r = 0, _sin50i = 0;
    c_sin(_mul49r, _mul49i, &_sin50r, &_sin50i);
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c48r, _c48i, _sin50r, _sin50i, &_mul51r, &_mul51i);
    double _add52r = 0, _add52i = 0;
    _add52r = _sub47r + _mul51r; _add52i = _sub47i + _mul51i;
    { int _idx = 6; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    double _c53r = 0, _c53i = 0;
    _c53r = 50.0; _c53i = 0;
    double _sub54r = 0, _sub54i = 0;
    _sub54r = x1r - x2r; _sub54i = x1i - x2i;
    double _abs55r = 0, _abs55i = 0;
    _abs55r = c_abs(_sub54r, _sub54i); _abs55i = 0;
    double _mul56r = 0, _mul56i = 0;
    c_mul(_c53r, _c53i, _abs55r, _abs55i, &_mul56r, &_mul56i);
    double _c57r = 0, _c57i = 0;
    _c57r = 0.0; _c57i = 1.0;
    double _add58r = 0, _add58i = 0;
    _add58r = x1r + x2r; _add58i = x1i + x2i;
    double _abs59r = 0, _abs59i = 0;
    _abs59r = c_abs(_add58r, _add58i); _abs59i = 0;
    double _mul60r = 0, _mul60i = 0;
    c_mul(_c57r, _c57i, _abs59r, _abs59i, &_mul60r, &_mul60i);
    double _exp61r = 0, _exp61i = 0;
    c_exp2(_mul60r, _mul60i, &_exp61r, &_exp61i);
    double _mul62r = 0, _mul62i = 0;
    c_mul(_mul56r, _mul56i, _exp61r, _exp61i, &_mul62r, &_mul62i);
    { int _idx = 7; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul62r; cIm[_idx] = _mul62i; } }
    double _np63r = 0, _np63i = 0;
    /* WARNING: unhandled np.where */
    { int _idx = 8; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _np63r; cIm[_idx] = _np63i; } }
    double _c64r = 0, _c64i = 0;
    _c64r = 0.0; _c64i = 1.0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(_c64r, _c64i, x1r, x1i, &_mul65r, &_mul65i);
    double _mul66r = 0, _mul66i = 0;
    c_mul(_mul65r, _mul65i, x2r, x2i, &_mul66r, &_mul66i);
    double _c67r = 0, _c67i = 0;
    _c67r = 0.1; _c67i = 0;
    double _mul68r = 0, _mul68i = 0;
    c_mul(_c67r, _c67i, x1r, x1i, &_mul68r, &_mul68i);
    double _mul69r = 0, _mul69i = 0;
    c_mul(_mul68r, _mul68i, x2r, x2i, &_mul69r, &_mul69i);
    double _pow70r = 0, _pow70i = 0;
    c_powr(_mul66r, _mul66i, _mul69r, &_pow70r, &_pow70i);
    { int _idx = 9; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _pow70r; cIm[_idx] = _pow70i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_770_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
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
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub6r; cIm[_idx] = _sub6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 3.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow8r, &_pow8i);
    c_mul(_pow8r, _pow8i, x1r, x1i, &_pow8r, &_pow8i);
    double _c9r = 0, _c9i = 0;
    _c9r = 3.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow10r, &_pow10i);
    c_mul(_pow10r, _pow10i, x2r, x2i, &_pow10r, &_pow10i);
    double _add11r = 0, _add11i = 0;
    _add11r = _pow8r + _pow10r; _add11i = _pow8i + _pow10i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add11r; cIm[_idx] = _add11i; } }
    double _sin12r = 0, _sin12i = 0;
    c_sin(x1r, x1i, &_sin12r, &_sin12i);
    double _cos13r = 0, _cos13i = 0;
    c_cos(x2r, x2i, &_cos13r, &_cos13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_sin12r, _sin12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 1.0;
    double _sub16r = 0, _sub16i = 0;
    _sub16r = x1r - x2r; _sub16i = x1i - x2i;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_c15r, _c15i, _sub16r, _sub16i, &_mul17r, &_mul17i);
    double _exp18r = 0, _exp18i = 0;
    c_exp2(_mul17r, _mul17i, &_exp18r, &_exp18i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp18r; cIm[_idx] = _exp18i; } }
    double _add19r = 0, _add19i = 0;
    _add19r = x1r + x2r; _add19i = x1i + x2i;
    double _abs20r = 0, _abs20i = 0;
    _abs20r = c_abs(_add19r, _add19i); _abs20i = 0;
    double _log21r = 0, _log21i = 0;
    c_log(_abs20r, _abs20i, &_log21r, &_log21i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _log21r; cIm[_idx] = _log21i; } }
    double _c22r = 0, _c22i = 0;
    _c22r = 4.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow23r, &_pow23i);
    c_mul(_pow23r, _pow23i, _pow23r, _pow23i, &_pow23r, &_pow23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _c25r = 0, _c25i = 0;
    _c25r = 4.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow26r, &_pow26i);
    c_mul(_pow26r, _pow26i, _pow26r, _pow26i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c24r, _c24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _pow23r + _mul27r; _add28i = _pow23i + _mul27i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add28r; cIm[_idx] = _add28i; } }
    double _mul29r = 0, _mul29i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul29r, &_mul29i);
    double _c30r = 0, _c30i = 0;
    _c30r = 2.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(_mul29r, _mul29i, _mul29r, _mul29i, &_pow31r, &_pow31i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _pow31r; cIm[_idx] = _pow31i; } }
    double _add32r = 0, _add32i = 0;
    _add32r = x1r + x2r; _add32i = x1i + x2i;
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _div34r = 0, _div34i = 0;
    c_div(_add32r, _add32i, _c33r, _c33i, &_div34r, &_div34i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div34r; cIm[_idx] = _div34i; } }
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
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub39r; cIm[_idx] = _sub39i; } }
    double _c40r = 0, _c40i = 0;
    _c40r = 0.0; _c40i = 1.0;
    double _mul41r = 0, _mul41i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul41r, &_mul41i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c40r, _c40i, _mul41r, _mul41i, &_mul42r, &_mul42i);
    double _exp43r = 0, _exp43i = 0;
    c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp43r; cIm[_idx] = _exp43i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_771_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    _c5r = 0.0; _c5i = 200.0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _mul4r + _mul8r; _add9i = _mul4i + _mul8i;
    double _c10r = 0, _c10i = 0;
    _c10r = 100.0; _c10i = 0;
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c10r, _c10i, x1r, x1i, &_mul11r, &_mul11i);
    double _sub12r = 0, _sub12i = 0;
    _sub12r = _add9r - _mul11r; _sub12i = _add9i - _mul11i;
    double _c13r = 0, _c13i = 0;
    _c13r = 0.0; _c13i = 55.0;
    double _sub14r = 0, _sub14i = 0;
    _sub14r = _sub12r - _c13r; _sub14i = _sub12i - _c13i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub14r; cIm[_idx] = _sub14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 1.0;
    double _c16r = 0, _c16i = 0;
    _c16r = 2.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow17r, &_pow17i);
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow19r, &_pow19i);
    double _add20r = 0, _add20i = 0;
    _add20r = _pow17r + _pow19r; _add20i = _pow17i + _pow19i;
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(_add20r, _add20i, _add20r, _add20i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, _add20r, _add20i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c15r, _c15i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 2.0;
    double _c25r = 0, _c25i = 0;
    _c25r = 5.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_powr(x2r, x2i, 5.0, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c24r, _c24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _mul23r + _mul27r; _add28i = _mul23i + _mul27i;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add28r; cIm[_idx] = _add28i; } }
    double _add29r = 0, _add29i = 0;
    _add29r = x1r + x2r; _add29i = x1i + x2i;
    double _c30r = 0, _c30i = 0;
    _c30r = 3.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(_add29r, _add29i, _add29r, _add29i, &_pow31r, &_pow31i);
    c_mul(_pow31r, _pow31i, _add29r, _add29i, &_pow31r, &_pow31i);
    double _sub32r = 0, _sub32i = 0;
    _sub32r = x1r - x2r; _sub32i = x1i - x2i;
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_sub32r, _sub32i, _sub32r, _sub32i, &_pow34r, &_pow34i);
    double _sub35r = 0, _sub35i = 0;
    _sub35r = _pow31r - _pow34r; _sub35i = _pow31i - _pow34i;
    double _add36r = 0, _add36i = 0;
    _add36r = x1r + x2r; _add36i = x1i + x2i;
    double _sin37r = 0, _sin37i = 0;
    c_sin(_add36r, _add36i, &_sin37r, &_sin37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _sub35r + _sin37r; _add38i = _sub35i + _sin37i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    double _c39r = 0, _c39i = 0;
    _c39r = 0.0; _c39i = 1.0;
    double _c40r = 0, _c40i = 0;
    _c40r = 2.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow41r, &_pow41i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c39r, _c39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
    double _c43r = 0, _c43i = 0;
    _c43r = 3.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow44r, &_pow44i);
    c_mul(_pow44r, _pow44i, x2r, x2i, &_pow44r, &_pow44i);
    double _mul45r = 0, _mul45i = 0;
    c_mul(_mul42r, _mul42i, _pow44r, _pow44i, &_mul45r, &_mul45i);
    double _exp46r = 0, _exp46i = 0;
    c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp46r; cIm[_idx] = _exp46i; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 1.0; _c47i = 0;
    double _mul48r = 0, _mul48i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul48r, &_mul48i);
    double _abs49r = 0, _abs49i = 0;
    _abs49r = c_abs(_mul48r, _mul48i); _abs49i = 0;
    double _add50r = 0, _add50i = 0;
    _add50r = _c47r + _abs49r; _add50i = _c47i + _abs49i;
    double _log51r = 0, _log51i = 0;
    c_log(_add50r, _add50i, &_log51r, &_log51i);
    double _c52r = 0, _c52i = 0;
    _c52r = 50.0; _c52i = 0;
    double _mul53r = 0, _mul53i = 0;
    c_mul(_c52r, _c52i, x1r, x1i, &_mul53r, &_mul53i);
    double _mul54r = 0, _mul54i = 0;
    c_mul(_mul53r, _mul53i, x2r, x2i, &_mul54r, &_mul54i);
    double _sub55r = 0, _sub55i = 0;
    _sub55r = _log51r - _mul54r; _sub55i = _log51i - _mul54i;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub55r; cIm[_idx] = _sub55i; } }
    double _sub56r = 0, _sub56i = 0;
    _sub56r = x1r - x2r; _sub56i = x1i - x2i;
    double _abs57r = 0, _abs57i = 0;
    _abs57r = c_abs(_sub56r, _sub56i); _abs57i = 0;
    double _add58r = 0, _add58i = 0;
    _add58r = x1r + x2r; _add58i = x1i + x2i;
    double _mul59r = 0, _mul59i = 0;
    c_mul(_abs57r, _abs57i, _add58r, _add58i, &_mul59r, &_mul59i);
    double _c60r = 0, _c60i = 0;
    _c60r = 1.0; _c60i = 0;
    double _add61r = 0, _add61i = 0;
    _add61r = x1r + x2r; _add61i = x1i + x2i;
    double _abs62r = 0, _abs62i = 0;
    _abs62r = c_abs(_add61r, _add61i); _abs62i = 0;
    double _add63r = 0, _add63i = 0;
    _add63r = _c60r + _abs62r; _add63i = _c60i + _abs62i;
    double _log64r = 0, _log64i = 0;
    c_log(_add63r, _add63i, &_log64r, &_log64i);
    double _mul65r = 0, _mul65i = 0;
    c_mul(_mul59r, _mul59i, _log64r, _log64i, &_mul65r, &_mul65i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul65r; cIm[_idx] = _mul65i; } }
    double _c66r = 0, _c66i = 0;
    _c66r = 0.0; _c66i = 1.0;
    double _neg67r = 0, _neg67i = 0;
    _neg67r = -(_c66r); _neg67i = -(_c66i);
    double _c68r = 0, _c68i = 0;
    _c68r = 2.0; _c68i = 0;
    double _pow69r = 0, _pow69i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow69r, &_pow69i);
    double _mul70r = 0, _mul70i = 0;
    c_mul(_neg67r, _neg67i, _pow69r, _pow69i, &_mul70r, &_mul70i);
    double _exp71r = 0, _exp71i = 0;
    c_exp2(_mul70r, _mul70i, &_exp71r, &_exp71i);
    double _add72r = 0, _add72i = 0;
    _add72r = x1r + x2r; _add72i = x1i + x2i;
    double _c73r = 0, _c73i = 0;
    _c73r = 3.0; _c73i = 0;
    double _pow74r = 0, _pow74i = 0;
    c_mul(_add72r, _add72i, _add72r, _add72i, &_pow74r, &_pow74i);
    c_mul(_pow74r, _pow74i, _add72r, _add72i, &_pow74r, &_pow74i);
    double _mul75r = 0, _mul75i = 0;
    c_mul(_exp71r, _exp71i, _pow74r, _pow74i, &_mul75r, &_mul75i);
    double _sub76r = 0, _sub76i = 0;
    _sub76r = x1r - x2r; _sub76i = x1i - x2i;
    double _c77r = 0, _c77i = 0;
    _c77r = 3.0; _c77i = 0;
    double _pow78r = 0, _pow78i = 0;
    c_mul(_sub76r, _sub76i, _sub76r, _sub76i, &_pow78r, &_pow78i);
    c_mul(_pow78r, _pow78i, _sub76r, _sub76i, &_pow78r, &_pow78i);
    double _sub79r = 0, _sub79i = 0;
    _sub79r = _mul75r - _pow78r; _sub79i = _mul75i - _pow78i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub79r; cIm[_idx] = _sub79i; } }
    double _c80r = 0, _c80i = 0;
    _c80r = 100.0; _c80i = 0;
    double _c81r = 0, _c81i = 0;
    _c81r = 3.0; _c81i = 0;
    double _pow82r = 0, _pow82i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow82r, &_pow82i);
    c_mul(_pow82r, _pow82i, x2r, x2i, &_pow82r, &_pow82i);
    double _mul83r = 0, _mul83i = 0;
    c_mul(_c80r, _c80i, _pow82r, _pow82i, &_mul83r, &_mul83i);
    double _c84r = 0, _c84i = 0;
    _c84r = 0.0; _c84i = 200.0;
    double _c85r = 0, _c85i = 0;
    _c85r = 2.0; _c85i = 0;
    double _pow86r = 0, _pow86i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow86r, &_pow86i);
    double _mul87r = 0, _mul87i = 0;
    c_mul(_c84r, _c84i, _pow86r, _pow86i, &_mul87r, &_mul87i);
    double _add88r = 0, _add88i = 0;
    _add88r = _mul83r + _mul87r; _add88i = _mul83i + _mul87i;
    double _c89r = 0, _c89i = 0;
    _c89r = 100.0; _c89i = 0;
    double _mul90r = 0, _mul90i = 0;
    c_mul(_c89r, _c89i, x2r, x2i, &_mul90r, &_mul90i);
    double _sub91r = 0, _sub91i = 0;
    _sub91r = _add88r - _mul90r; _sub91i = _add88i - _mul90i;
    double _c92r = 0, _c92i = 0;
    _c92r = 0.0; _c92i = 55.0;
    double _sub93r = 0, _sub93i = 0;
    _sub93r = _sub91r - _c92r; _sub93i = _sub91i - _c92i;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub93r; cIm[_idx] = _sub93i; } }
    double _c94r = 0, _c94i = 0;
    _c94r = 2.0; _c94i = 0;
    double _pow95r = 0, _pow95i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow95r, &_pow95i);
    double _c96r = 0, _c96i = 0;
    _c96r = 2.0; _c96i = 0;
    double _pow97r = 0, _pow97i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow97r, &_pow97i);
    double _add98r = 0, _add98i = 0;
    _add98r = _pow95r + _pow97r; _add98i = _pow95i + _pow97i;
    double _mul99r = 0, _mul99i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul99r, &_mul99i);
    double _sin100r = 0, _sin100i = 0;
    c_sin(_mul99r, _mul99i, &_sin100r, &_sin100i);
    double _mul101r = 0, _mul101i = 0;
    c_mul(_add98r, _add98i, _sin100r, _sin100i, &_mul101r, &_mul101i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul101r; cIm[_idx] = _mul101i; } }
    double _c102r = 0, _c102i = 0;
    _c102r = 2.0; _c102i = 0;
    double _abs103r = 0, _abs103i = 0;
    _abs103r = c_abs(x1r, x1i); _abs103i = 0;
    double _abs104r = 0, _abs104i = 0;
    _abs104r = c_abs(x2r, x2i); _abs104i = 0;
    double _add105r = 0, _add105i = 0;
    _add105r = _abs103r + _abs104r; _add105i = _abs103i + _abs104i;
    double _c106r = 0, _c106i = 0;
    _c106r = 4.0; _c106i = 0;
    double _pow107r = 0, _pow107i = 0;
    c_mul(_add105r, _add105i, _add105r, _add105i, &_pow107r, &_pow107i);
    c_mul(_pow107r, _pow107i, _pow107r, _pow107i, &_pow107r, &_pow107i);
    double _mul108r = 0, _mul108i = 0;
    c_mul(_c102r, _c102i, _pow107r, _pow107i, &_mul108r, &_mul108i);
    double _c109r = 0, _c109i = 0;
    _c109r = 0.0; _c109i = 1.0;
    double _c110r = 0, _c110i = 0;
    _c110r = 2.0; _c110i = 0;
    double _pow111r = 0, _pow111i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow111r, &_pow111i);
    double _c112r = 0, _c112i = 0;
    _c112r = 2.0; _c112i = 0;
    double _pow113r = 0, _pow113i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow113r, &_pow113i);
    double _sub114r = 0, _sub114i = 0;
    _sub114r = _pow111r - _pow113r; _sub114i = _pow111i - _pow113i;
    double _mul115r = 0, _mul115i = 0;
    c_mul(_c109r, _c109i, _sub114r, _sub114i, &_mul115r, &_mul115i);
    double _add116r = 0, _add116i = 0;
    _add116r = _mul108r + _mul115r; _add116i = _mul108i + _mul115i;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add116r; cIm[_idx] = _add116i; } }
    double _c117r = 0, _c117i = 0;
    _c117r = 50.0; _c117i = 0;
    double _add118r = 0, _add118i = 0;
    _add118r = x1r + x2r; _add118i = x1i + x2i;
    double _c119r = 0, _c119i = 0;
    _c119r = 2.0; _c119i = 0;
    double _pow120r = 0, _pow120i = 0;
    c_mul(_add118r, _add118i, _add118r, _add118i, &_pow120r, &_pow120i);
    double _mul121r = 0, _mul121i = 0;
    c_mul(_c117r, _c117i, _pow120r, _pow120i, &_mul121r, &_mul121i);
    double _c122r = 0, _c122i = 0;
    _c122r = 50.0; _c122i = 0;
    double _mul123r = 0, _mul123i = 0;
    c_mul(_c122r, _c122i, x1r, x1i, &_mul123r, &_mul123i);
    double _mul124r = 0, _mul124i = 0;
    c_mul(_mul123r, _mul123i, x2r, x2i, &_mul124r, &_mul124i);
    double _add125r = 0, _add125i = 0;
    _add125r = _mul121r + _mul124r; _add125i = _mul121i + _mul124i;
    double _c126r = 0, _c126i = 0;
    _c126r = 0.0; _c126i = 50.0;
    double _sub127r = 0, _sub127i = 0;
    _sub127r = x1r - x2r; _sub127i = x1i - x2i;
    double _mul128r = 0, _mul128i = 0;
    c_mul(_c126r, _c126i, _sub127r, _sub127i, &_mul128r, &_mul128i);
    double _sub129r = 0, _sub129i = 0;
    _sub129r = _add125r - _mul128r; _sub129i = _add125i - _mul128i;
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub129r; cIm[_idx] = _sub129i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_772: too complex for auto-transpile, stubbed */
static void poly_772_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_773_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _sin1r = 0, _sin1i = 0;
    c_sin(x1r, x1i, &_sin1r, &_sin1i);
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 1.0;
    double _cos3r = 0, _cos3i = 0;
    c_cos(x2r, x2i, &_cos3r, &_cos3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c2r, _c2i, _cos3r, _cos3i, &_mul4r, &_mul4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _sin1r + _mul4r; _add5i = _sin1i + _mul4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 1.0;
    double _neg7r = 0, _neg7i = 0;
    _neg7r = -(_c6r); _neg7i = -(_c6i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_neg7r, _neg7i, x1r, x1i, &_mul8r, &_mul8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _mul8r + x2r; _add9i = _mul8i + x2i;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 1.0;
    double _sub11r = 0, _sub11i = 0;
    _sub11r = x1r - x2r; _sub11i = x1i - x2i;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c10r, _c10i, _sub11r, _sub11i, &_mul12r, &_mul12i);
    double _exp13r = 0, _exp13i = 0;
    c_exp2(_mul12r, _mul12i, &_exp13r, &_exp13i);
    double _c14r = 0, _c14i = 0;
    _c14r = 1.0; _c14i = 0;
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(x1r, x1i); _abs15i = 0;
    double _add16r = 0, _add16i = 0;
    _add16r = _c14r + _abs15r; _add16i = _c14i + _abs15i;
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x2r, x2i); _abs17i = 0;
    double _add18r = 0, _add18i = 0;
    _add18r = _add16r + _abs17r; _add18i = _add16i + _abs17i;
    double _log19r = 0, _log19i = 0;
    c_log(_add18r, _add18i, &_log19r, &_log19i);
    double _add20r = 0, _add20i = 0;
    _add20r = _exp13r + _log19r; _add20i = _exp13i + _log19i;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    double _mul21r = 0, _mul21i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul21r, &_mul21i);
    double _c22r = 0, _c22i = 0;
    _c22r = 2.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(_mul21r, _mul21i, _mul21r, _mul21i, &_pow23r, &_pow23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _sub25r = 0, _sub25i = 0;
    _sub25r = x1r - x2r; _sub25i = x1i - x2i;
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(_sub25r, _sub25i, _sub25r, _sub25i, &_pow27r, &_pow27i);
    double _mul28r = 0, _mul28i = 0;
    c_mul(_c24r, _c24i, _pow27r, _pow27i, &_mul28r, &_mul28i);
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _pow23r - _mul28r; _sub29i = _pow23i - _mul28i;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 0.0; _c30i = 100.0;
    double _sin31r = 0, _sin31i = 0;
    c_sin(x2r, x2i, &_sin31r, &_sin31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c30r, _c30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
    double _c33r = 0, _c33i = 0;
    _c33r = 100.0; _c33i = 0;
    double _cos34r = 0, _cos34i = 0;
    c_cos(x1r, x1i, &_cos34r, &_cos34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c33r, _c33i, _cos34r, _cos34i, &_mul35r, &_mul35i);
    double _add36r = 0, _add36i = 0;
    _add36r = _mul32r + _mul35r; _add36i = _mul32i + _mul35i;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    double _add37r = 0, _add37i = 0;
    _add37r = x1r + x2r; _add37i = x1i + x2i;
    double _c38r = 0, _c38i = 0;
    _c38r = 3.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(_add37r, _add37i, _add37r, _add37i, &_pow39r, &_pow39i);
    c_mul(_pow39r, _pow39i, _add37r, _add37i, &_pow39r, &_pow39i);
    double _c40r = 0, _c40i = 0;
    _c40r = 0.0; _c40i = 1.0;
    double _sub41r = 0, _sub41i = 0;
    _sub41r = x1r - x2r; _sub41i = x1i - x2i;
    double _c42r = 0, _c42i = 0;
    _c42r = 3.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_mul(_sub41r, _sub41i, _sub41r, _sub41i, &_pow43r, &_pow43i);
    c_mul(_pow43r, _pow43i, _sub41r, _sub41i, &_pow43r, &_pow43i);
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c40r, _c40i, _pow43r, _pow43i, &_mul44r, &_mul44i);
    double _sub45r = 0, _sub45i = 0;
    _sub45r = _pow39r - _mul44r; _sub45i = _pow39i - _mul44i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 1.0;
    double _mul47r = 0, _mul47i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul47r, &_mul47i);
    double _mul48r = 0, _mul48i = 0;
    c_mul(_c46r, _c46i, _mul47r, _mul47i, &_mul48r, &_mul48i);
    double _exp49r = 0, _exp49i = 0;
    c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _exp49r; cIm[_idx] = _exp49i; } }
    double _c50r = 0, _c50i = 0;
    _c50r = 1.0; _c50i = 0;
    double _mul51r = 0, _mul51i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul51r, &_mul51i);
    double _abs52r = 0, _abs52i = 0;
    _abs52r = c_abs(_mul51r, _mul51i); _abs52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = _c50r + _abs52r; _add53i = _c50i + _abs52i;
    double _log54r = 0, _log54i = 0;
    c_log(_add53r, _add53i, &_log54r, &_log54i);
    double _c55r = 0, _c55i = 0;
    _c55r = 0.0; _c55i = 1.0;
    double _sub56r = 0, _sub56i = 0;
    _sub56r = x1r - x2r; _sub56i = x1i - x2i;
    double _mul57r = 0, _mul57i = 0;
    c_mul(_c55r, _c55i, _sub56r, _sub56i, &_mul57r, &_mul57i);
    double _add58r = 0, _add58i = 0;
    _add58r = _log54r + _mul57r; _add58i = _log54i + _mul57i;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    double _c59r = 0, _c59i = 0;
    _c59r = 2.0; _c59i = 0;
    double _pow60r = 0, _pow60i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow60r, &_pow60i);
    double _c61r = 0, _c61i = 0;
    _c61r = 2.0; _c61i = 0;
    double _pow62r = 0, _pow62i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow62r, &_pow62i);
    double _add63r = 0, _add63i = 0;
    _add63r = _pow60r + _pow62r; _add63i = _pow60i + _pow62i;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add63r; cIm[_idx] = _add63i; } }
    double _c64r = 0, _c64i = 0;
    _c64r = 0.0; _c64i = 1.0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(_c64r, _c64i, x2r, x2i, &_mul65r, &_mul65i);
    double _add66r = 0, _add66i = 0;
    _add66r = x1r + _mul65r; _add66i = x1i + _mul65i;
    double _c67r = 0, _c67i = 0;
    _c67r = 3.0; _c67i = 0;
    double _pow68r = 0, _pow68i = 0;
    c_mul(_add66r, _add66i, _add66r, _add66i, &_pow68r, &_pow68i);
    c_mul(_pow68r, _pow68i, _add66r, _add66i, &_pow68r, &_pow68i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _pow68r; cIm[_idx] = _pow68i; } }
    double _np69r = 0, _np69i = 0;
    /* WARNING: unhandled np.linspace */
    double _cos70r = 0, _cos70i = 0;
    c_cos(_np69r, _np69i, &_cos70r, &_cos70i);
    double _c71r = 0, _c71i = 0;
    _c71r = 0.0; _c71i = 1.0;
    double _np72r = 0, _np72i = 0;
    /* WARNING: unhandled np.linspace */
    double _sin73r = 0, _sin73i = 0;
    c_sin(_np72r, _np72i, &_sin73r, &_sin73i);
    double _mul74r = 0, _mul74i = 0;
    c_mul(_c71r, _c71i, _sin73r, _sin73i, &_mul74r, &_mul74i);
    double _add75r = 0, _add75i = 0;
    _add75r = _cos70r + _mul74r; _add75i = _cos70i + _mul74i;
    double _mul76r = 0, _mul76i = 0;
    c_mul(_add75r, _add75i, x1r, x1i, &_mul76r, &_mul76i);
    double _mul77r = 0, _mul77i = 0;
    c_mul(_mul76r, _mul76i, x2r, x2i, &_mul77r, &_mul77i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul77r; cIm[_idx] = _mul77i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_774_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double complex_sum = _add1r; /* +_add1ii */
    double _mul2r = 0, _mul2i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
    double complex_product = _mul2r; /* +_mul2ii */
    double _sub3r = 0, _sub3i = 0;
    _sub3r = x1r - x2r; _sub3i = x1i - x2i;
    double complex_diff = _sub3r; /* +_sub3ii */
    double _c4r = 0, _c4i = 0;
    _c4r = 0.0; _c4i = 1.0;
    double _ang5r = 0, _ang5i = 0;
    _ang5r = c_arg(complex_sum, 0); _ang5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c4r, _c4i, _ang5r, _ang5i, &_mul6r, &_mul6i);
    double _exp7r = 0, _exp7i = 0;
    c_exp2(_mul6r, _mul6i, &_exp7r, &_exp7i);
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(complex_sum, 0); _abs8i = 0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_exp7r, _exp7i, _abs8r, _abs8i, &_mul9r, &_mul9i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
    double _re10r = 0, _re10i = 0;
    _re10r = complex_product; _re10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 0.0; _c11i = 1.0;
    double _im12r = 0, _im12i = 0;
    _im12r = 0; _im12i = 0;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c11r, _c11i, _im12r, _im12i, &_mul13r, &_mul13i);
    double _add14r = 0, _add14i = 0;
    _add14r = _re10r + _mul13r; _add14i = _re10i + _mul13i;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add14r; cIm[_idx] = _add14i; } }
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(complex_product, 0); _abs15i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _abs15r; cIm[_idx] = _abs15i; } }
    double _cf16r = 0, _cf16i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
    double _cf17r = 0, _cf17i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf17r = cRe[_idx]; _cf17i = cIm[_idx]; } }
    double _add18r = 0, _add18i = 0;
    _add18r = _cf16r + _cf17r; _add18i = _cf16i + _cf17i;
    double _cf19r = 0, _cf19i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
    double _add20r = 0, _add20i = 0;
    _add20r = _add18r + _cf19r; _add20i = _add18i + _cf19i;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    double _cf21r = 0, _cf21i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
    double _cf22r = 0, _cf22i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
    double _mul23r = 0, _mul23i = 0;
    c_mul(_cf21r, _cf21i, _cf22r, _cf22i, &_mul23r, &_mul23i);
    double _cf24r = 0, _cf24i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
    double _cf25r = 0, _cf25i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
    double _mul26r = 0, _mul26i = 0;
    c_mul(_cf24r, _cf24i, _cf25r, _cf25i, &_mul26r, &_mul26i);
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _mul23r - _mul26r; _sub27i = _mul23i - _mul26i;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub27r; cIm[_idx] = _sub27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 10.0; _c28i = 0;
    double _cf29r = 0, _cf29i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
    double _sin30r = 0, _sin30i = 0;
    c_sin(_cf29r, _cf29i, &_sin30r, &_sin30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _c28r + _sin30r; _add31i = _c28i + _sin30i;
    double _cf32r = 0, _cf32i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
    double _sin33r = 0, _sin33i = 0;
    c_sin(_cf32r, _cf32i, &_sin33r, &_sin33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _add31r + _sin33r; _add34i = _add31i + _sin33i;
    double _cf35r = 0, _cf35i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
    double _cos36r = 0, _cos36i = 0;
    c_cos(_cf35r, _cf35i, &_cos36r, &_cos36i);
    double _add37r = 0, _add37i = 0;
    _add37r = _add34r + _cos36r; _add37i = _add34i + _cos36i;
    double _cf38r = 0, _cf38i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
    double _cos39r = 0, _cos39i = 0;
    c_cos(_cf38r, _cf38i, &_cos39r, &_cos39i);
    double _add40r = 0, _add40i = 0;
    _add40r = _add37r + _cos39r; _add40i = _add37i + _cos39i;
    double _cf41r = 0, _cf41i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
    double _sin42r = 0, _sin42i = 0;
    c_sin(_cf41r, _cf41i, &_sin42r, &_sin42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _add40r + _sin42r; _add43i = _add40i + _sin42i;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 0.0; _c44i = 2.0;
    double _cf45r = 0, _cf45i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf45r = cRe[_idx]; _cf45i = cIm[_idx]; } }
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c44r, _c44i, _cf45r, _cf45i, &_mul46r, &_mul46i);
    double _cf47r = 0, _cf47i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf47r = cRe[_idx]; _cf47i = cIm[_idx]; } }
    double _cf48r = 0, _cf48i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
    double _div49r = 0, _div49i = 0;
    c_div(_cf47r, _cf47i, _cf48r, _cf48i, &_div49r, &_div49i);
    double _add50r = 0, _add50i = 0;
    _add50r = _mul46r + _div49r; _add50i = _mul46i + _div49i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    double _cf51r = 0, _cf51i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
    double _cf52r = 0, _cf52i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
    double _mul53r = 0, _mul53i = 0;
    c_mul(_cf51r, _cf51i, _cf52r, _cf52i, &_mul53r, &_mul53i);
    double _cf54r = 0, _cf54i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { _cf54r = cRe[_idx]; _cf54i = cIm[_idx]; } }
    double _mul55r = 0, _mul55i = 0;
    c_mul(_mul53r, _mul53i, _cf54r, _cf54i, &_mul55r, &_mul55i);
    double _cf56r = 0, _cf56i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf56r = cRe[_idx]; _cf56i = cIm[_idx]; } }
    double _add57r = 0, _add57i = 0;
    _add57r = _mul55r + _cf56r; _add57i = _mul55i + _cf56i;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add57r; cIm[_idx] = _add57i; } }
    double _c58r = 0, _c58i = 0;
    _c58r = 2.0; _c58i = 0;
    double _cf59r = 0, _cf59i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { _cf59r = cRe[_idx]; _cf59i = cIm[_idx]; } }
    double _mul60r = 0, _mul60i = 0;
    c_mul(_c58r, _c58i, _cf59r, _cf59i, &_mul60r, &_mul60i);
    double _cf61r = 0, _cf61i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { _cf61r = cRe[_idx]; _cf61i = cIm[_idx]; } }
    double _cf62r = 0, _cf62i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { _cf62r = cRe[_idx]; _cf62i = cIm[_idx]; } }
    double _mul63r = 0, _mul63i = 0;
    c_mul(_cf61r, _cf61i, _cf62r, _cf62i, &_mul63r, &_mul63i);
    double _sub64r = 0, _sub64i = 0;
    _sub64r = _mul60r - _mul63r; _sub64i = _mul60i - _mul63i;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub64r; cIm[_idx] = _sub64i; } }
    double _cf65r = 0, _cf65i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf65r = cRe[_idx]; _cf65i = cIm[_idx]; } }
    double _cf66r = 0, _cf66i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf66r = cRe[_idx]; _cf66i = cIm[_idx]; } }
    double _mul67r = 0, _mul67i = 0;
    c_mul(_cf65r, _cf65i, _cf66r, _cf66i, &_mul67r, &_mul67i);
    double _cf68r = 0, _cf68i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { _cf68r = cRe[_idx]; _cf68i = cIm[_idx]; } }
    double _mul69r = 0, _mul69i = 0;
    c_mul(_mul67r, _mul67i, _cf68r, _cf68i, &_mul69r, &_mul69i);
    double _cf70r = 0, _cf70i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { _cf70r = cRe[_idx]; _cf70i = cIm[_idx]; } }
    double _mul71r = 0, _mul71i = 0;
    c_mul(_mul69r, _mul69i, _cf70r, _cf70i, &_mul71r, &_mul71i);
    double _cf72r = 0, _cf72i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { _cf72r = cRe[_idx]; _cf72i = cIm[_idx]; } }
    double _mul73r = 0, _mul73i = 0;
    c_mul(_mul71r, _mul71i, _cf72r, _cf72i, &_mul73r, &_mul73i);
    double _cf74r = 0, _cf74i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { _cf74r = cRe[_idx]; _cf74i = cIm[_idx]; } }
    double _mul75r = 0, _mul75i = 0;
    c_mul(_mul73r, _mul73i, _cf74r, _cf74i, &_mul75r, &_mul75i);
    double _cf76r = 0, _cf76i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { _cf76r = cRe[_idx]; _cf76i = cIm[_idx]; } }
    double _mul77r = 0, _mul77i = 0;
    c_mul(_mul75r, _mul75i, _cf76r, _cf76i, &_mul77r, &_mul77i);
    double _cf78r = 0, _cf78i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { _cf78r = cRe[_idx]; _cf78i = cIm[_idx]; } }
    double _mul79r = 0, _mul79i = 0;
    c_mul(_mul77r, _mul77i, _cf78r, _cf78i, &_mul79r, &_mul79i);
    double _cf80r = 0, _cf80i = 0;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { _cf80r = cRe[_idx]; _cf80i = cIm[_idx]; } }
    double _mul81r = 0, _mul81i = 0;
    c_mul(_mul79r, _mul79i, _cf80r, _cf80i, &_mul81r, &_mul81i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul81r; cIm[_idx] = _mul81i; } }
    double _cf82r = 0, _cf82i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { _cf82r = cRe[_idx]; _cf82i = cIm[_idx]; } }
    double _cf83r = 0, _cf83i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { _cf83r = cRe[_idx]; _cf83i = cIm[_idx]; } }
    double _add84r = 0, _add84i = 0;
    _add84r = _cf82r + _cf83r; _add84i = _cf82i + _cf83i;
    double _cf85r = 0, _cf85i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { _cf85r = cRe[_idx]; _cf85i = cIm[_idx]; } }
    double _sub86r = 0, _sub86i = 0;
    _sub86r = _add84r - _cf85r; _sub86i = _add84i - _cf85i;
    double _cf87r = 0, _cf87i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { _cf87r = cRe[_idx]; _cf87i = cIm[_idx]; } }
    double _add88r = 0, _add88i = 0;
    _add88r = _sub86r + _cf87r; _add88i = _sub86i + _cf87i;
    double _cf89r = 0, _cf89i = 0;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { _cf89r = cRe[_idx]; _cf89i = cIm[_idx]; } }
    double _sub90r = 0, _sub90i = 0;
    _sub90r = _add88r - _cf89r; _sub90i = _add88i - _cf89i;
    double _cf91r = 0, _cf91i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { _cf91r = cRe[_idx]; _cf91i = cIm[_idx]; } }
    double _cf92r = 0, _cf92i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { _cf92r = cRe[_idx]; _cf92i = cIm[_idx]; } }
    double _sub93r = 0, _sub93i = 0;
    _sub93r = _cf91r - _cf92r; _sub93i = _cf91i - _cf92i;
    double _cf94r = 0, _cf94i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { _cf94r = cRe[_idx]; _cf94i = cIm[_idx]; } }
    double _add95r = 0, _add95i = 0;
    _add95r = _sub93r + _cf94r; _add95i = _sub93i + _cf94i;
    double _cf96r = 0, _cf96i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { _cf96r = cRe[_idx]; _cf96i = cIm[_idx]; } }
    double _sub97r = 0, _sub97i = 0;
    _sub97r = _add95r - _cf96r; _sub97i = _add95i - _cf96i;
    double _cf98r = 0, _cf98i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { _cf98r = cRe[_idx]; _cf98i = cIm[_idx]; } }
    double _add99r = 0, _add99i = 0;
    _add99r = _sub97r + _cf98r; _add99i = _sub97i + _cf98i;
    double _div100r = 0, _div100i = 0;
    c_div(_sub90r, _sub90i, _add99r, _add99i, &_div100r, &_div100i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _div100r; cIm[_idx] = _div100i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_775_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, x1r, x1i, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _pow3r + _pow5r; _add6i = _pow3i + _pow5i;
    double _c7r = 0, _c7i = 0;
    _c7r = 0.0; _c7i = 1.0;
    double _mul8r = 0, _mul8i = 0;
    c_mul(_add6r, _add6i, _c7r, _c7i, &_mul8r, &_mul8i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul8r; cIm[_idx] = _mul8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
    double _abs11r = 0, _abs11i = 0;
    _abs11r = c_abs(x2r, x2i); _abs11i = 0;
    double _cos12r = 0, _cos12i = 0;
    c_cos(_abs11r, _abs11i, &_cos12r, &_cos12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_pow10r, _pow10i, _cos12r, _cos12i, &_mul13r, &_mul13i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 3.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow15r, &_pow15i);
    c_mul(_pow15r, _pow15i, x2r, x2i, &_pow15r, &_pow15i);
    double _ang16r = 0, _ang16i = 0;
    _ang16r = c_arg(x1r, x1i); _ang16i = 0;
    double _sin17r = 0, _sin17i = 0;
    c_sin(_ang16r, _ang16i, &_sin17r, &_sin17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_pow15r, _pow15i, _sin17r, _sin17i, &_mul18r, &_mul18i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul18r; cIm[_idx] = _mul18i; } }
    double _c19r = 0, _c19i = 0;
    _c19r = 0.0; _c19i = 1.0;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c19r, _c19i, x1r, x1i, &_mul20r, &_mul20i);
    double _exp21r = 0, _exp21i = 0;
    c_exp2(_mul20r, _mul20i, &_exp21r, &_exp21i);
    double _c22r = 0, _c22i = 0;
    _c22r = 0.0; _c22i = 1.0;
    double _neg23r = 0, _neg23i = 0;
    _neg23r = -(_c22r); _neg23i = -(_c22i);
    double _mul24r = 0, _mul24i = 0;
    c_mul(_neg23r, _neg23i, x2r, x2i, &_mul24r, &_mul24i);
    double _exp25r = 0, _exp25i = 0;
    c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_exp21r, _exp21i, _exp25r, _exp25i, &_mul26r, &_mul26i);
    double _re27r = 0, _re27i = 0;
    _re27r = _mul26r; _re27i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _re27r; cIm[_idx] = _re27i; } }
    double _add28r = 0, _add28i = 0;
    _add28r = x1r + x2r; _add28i = x1i + x2i;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_add28r, _add28i, _add28r, _add28i, &_pow30r, &_pow30i);
    double _c31r = 0, _c31i = 0;
    _c31r = 0.0; _c31i = 1.0;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_pow30r, _pow30i, _c31r, _c31i, &_mul32r, &_mul32i);
    double _im33r = 0, _im33i = 0;
    _im33r = _mul32i; _im33i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _im33r; cIm[_idx] = _im33i; } }
    double _sub34r = 0, _sub34i = 0;
    _sub34r = x1r - x2r; _sub34i = x1i - x2i;
    double _abs35r = 0, _abs35i = 0;
    _abs35r = c_abs(_sub34r, _sub34i); _abs35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = x1r + x2r; _add36i = x1i + x2i;
    double _cos37r = 0, _cos37i = 0;
    c_cos(_add36r, _add36i, &_cos37r, &_cos37i);
    double _mul38r = 0, _mul38i = 0;
    c_mul(_abs35r, _abs35i, _cos37r, _cos37i, &_mul38r, &_mul38i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    double _mul39r = 0, _mul39i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul39r, &_mul39i);
    double _ang40r = 0, _ang40i = 0;
    _ang40r = c_arg(_mul39r, _mul39i); _ang40i = 0;
    double _mul41r = 0, _mul41i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul41r, &_mul41i);
    double _sin42r = 0, _sin42i = 0;
    c_sin(_mul41r, _mul41i, &_sin42r, &_sin42i);
    double _mul43r = 0, _mul43i = 0;
    c_mul(_ang40r, _ang40i, _sin42r, _sin42i, &_mul43r, &_mul43i);
    double _c44r = 0, _c44i = 0;
    _c44r = 0.0; _c44i = 1.0;
    double _mul45r = 0, _mul45i = 0;
    c_mul(_mul43r, _mul43i, _c44r, _c44i, &_mul45r, &_mul45i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    double _add46r = 0, _add46i = 0;
    _add46r = x1r + x2r; _add46i = x1i + x2i;
    double _c47r = 0, _c47i = 0;
    _c47r = 3.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(_add46r, _add46i, _add46r, _add46i, &_pow48r, &_pow48i);
    c_mul(_pow48r, _pow48i, _add46r, _add46i, &_pow48r, &_pow48i);
    double _c49r = 0, _c49i = 0;
    _c49r = 2.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow50r, &_pow50i);
    double _c51r = 0, _c51i = 0;
    _c51r = 3.0; _c51i = 0;
    double _pow52r = 0, _pow52i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow52r, &_pow52i);
    c_mul(_pow52r, _pow52i, x2r, x2i, &_pow52r, &_pow52i);
    double _mul53r = 0, _mul53i = 0;
    c_mul(_pow50r, _pow50i, _pow52r, _pow52i, &_mul53r, &_mul53i);
    double _c54r = 0, _c54i = 0;
    _c54r = 0.0; _c54i = 1.0;
    double _mul55r = 0, _mul55i = 0;
    c_mul(_mul53r, _mul53i, _c54r, _c54i, &_mul55r, &_mul55i);
    double _re56r = 0, _re56i = 0;
    _re56r = _mul55r; _re56i = 0;
    double _sub57r = 0, _sub57i = 0;
    _sub57r = _pow48r - _re56r; _sub57i = _pow48i - _re56i;
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub57r; cIm[_idx] = _sub57i; } }
    double _c58r = 0, _c58i = 0;
    _c58r = 2.0; _c58i = 0;
    double _pow59r = 0, _pow59i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow59r, &_pow59i);
    double _c60r = 0, _c60i = 0;
    _c60r = 2.0; _c60i = 0;
    double _pow61r = 0, _pow61i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow61r, &_pow61i);
    double _sub62r = 0, _sub62i = 0;
    _sub62r = _pow59r - _pow61r; _sub62i = _pow59i - _pow61i;
    double _abs63r = 0, _abs63i = 0;
    _abs63r = c_abs(_sub62r, _sub62i); _abs63i = 0;
    double _c64r = 0, _c64i = 0;
    _c64r = 0.0; _c64i = 1.0;
    double _add65r = 0, _add65i = 0;
    _add65r = x1r + x2r; _add65i = x1i + x2i;
    double _ang66r = 0, _ang66i = 0;
    _ang66r = c_arg(_add65r, _add65i); _ang66i = 0;
    double _mul67r = 0, _mul67i = 0;
    c_mul(_c64r, _c64i, _ang66r, _ang66i, &_mul67r, &_mul67i);
    double _exp68r = 0, _exp68i = 0;
    c_exp2(_mul67r, _mul67i, &_exp68r, &_exp68i);
    double _mul69r = 0, _mul69i = 0;
    c_mul(_abs63r, _abs63i, _exp68r, _exp68i, &_mul69r, &_mul69i);
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul69r; cIm[_idx] = _mul69i; } }
    double _re70r = 0, _re70i = 0;
    _re70r = x1r; _re70i = 0;
    double _im71r = 0, _im71i = 0;
    _im71r = x2i; _im71i = 0;
    double _add72r = 0, _add72i = 0;
    _add72r = _re70r + _im71r; _add72i = _re70i + _im71i;
    double _c73r = 0, _c73i = 0;
    _c73r = 2.0; _c73i = 0;
    double _pow74r = 0, _pow74i = 0;
    c_mul(_add72r, _add72i, _add72r, _add72i, &_pow74r, &_pow74i);
    double _re75r = 0, _re75i = 0;
    _re75r = x1r; _re75i = 0;
    double _im76r = 0, _im76i = 0;
    _im76r = x2i; _im76i = 0;
    double _sub77r = 0, _sub77i = 0;
    _sub77r = _re75r - _im76r; _sub77i = _re75i - _im76i;
    double _abs78r = 0, _abs78i = 0;
    _abs78r = c_abs(_sub77r, _sub77i); _abs78i = 0;
    double _sin79r = 0, _sin79i = 0;
    c_sin(_abs78r, _abs78i, &_sin79r, &_sin79i);
    double _mul80r = 0, _mul80i = 0;
    c_mul(_pow74r, _pow74i, _sin79r, _sin79i, &_mul80r, &_mul80i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul80r; cIm[_idx] = _mul80i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_776_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = x1r + x2r; _add2i = x1i + x2i;
    double _sin3r = 0, _sin3i = 0;
    c_sin(_add2r, _add2i, &_sin3r, &_sin3i);
    double _add4r = 0, _add4i = 0;
    _add4r = _c1r + _sin3r; _add4i = _c1i + _sin3i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add4r; cIm[_idx] = _add4i; } }
    double _mul5r = 0, _mul5i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul5r, &_mul5i);
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 1.0;
    double _neg7r = 0, _neg7i = 0;
    _neg7r = -(_c6r); _neg7i = -(_c6i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_neg7r, _neg7i, x2r, x2i, &_mul8r, &_mul8i);
    double _exp9r = 0, _exp9i = 0;
    c_exp2(_mul8r, _mul8i, &_exp9r, &_exp9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_mul5r, _mul5i, _exp9r, _exp9i, &_mul10r, &_mul10i);
    { int _idx = 1; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _np12r = 0, _np12i = 0;
    /* WARNING: unhandled np.where */
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c11r, _c11i, _np12r, _np12i, &_mul13r, &_mul13i);
    { int _idx = 2; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 200.0; _c14i = 0;
    double _re15r = 0, _re15i = 0;
    _re15r = x1r; _re15i = 0;
    double _im16r = 0, _im16i = 0;
    _im16r = x1i; _im16i = 0;
    double _sub17r = 0, _sub17i = 0;
    _sub17r = _re15r - _im16r; _sub17i = _re15i - _im16i;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c14r, _c14i, _sub17r, _sub17i, &_mul18r, &_mul18i);
    { int _idx = 3; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul18r; cIm[_idx] = _mul18i; } }
    double _c19r = 0, _c19i = 0;
    _c19r = 0.0; _c19i = 100.0;
    double _neg20r = 0, _neg20i = 0;
    _neg20r = -(_c19r); _neg20i = -(_c19i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(_neg20r, _neg20i, x2r, x2i, &_mul21r, &_mul21i);
    double _c22r = 0, _c22i = 0;
    _c22r = 100.0; _c22i = 0;
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow24r, &_pow24i);
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c22r, _c22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
    double _add26r = 0, _add26i = 0;
    _add26r = _mul21r + _mul25r; _add26i = _mul21i + _mul25i;
    { int _idx = 4; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add26r; cIm[_idx] = _add26i; } }
    double _c27r = 0, _c27i = 0;
    _c27r = 1.0; _c27i = 0;
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(x2r, x2i); _abs28i = 0;
    double _add29r = 0, _add29i = 0;
    _add29r = _c27r + _abs28r; _add29i = _c27i + _abs28i;
    double _div30r = 0, _div30i = 0;
    c_div(x1r, x1i, _add29r, _add29i, &_div30r, &_div30i);
    double _tsin31r = 0, _tsin31i = 0;
    double _tcos32r = 0, _tcos32i = 0;
    double _tan33r = 0, _tan33i = 0;
    c_sin(_div30r, _div30i, &_tsin31r, &_tsin31i);
    c_cos(_div30r, _div30i, &_tcos32r, &_tcos32i);
    c_div(_tsin31r, _tsin31i, _tcos32r, _tcos32i, &_tan33r, &_tan33i);
    { int _idx = 5; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _tan33r; cIm[_idx] = _tan33i; } }
    double _c34r = 0, _c34i = 0;
    _c34r = 0.0; _c34i = 100.0;
    double _mul35r = 0, _mul35i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul35r, &_mul35i);
    double _c36r = 0, _c36i = 0;
    _c36r = 0.0; _c36i = 1.0;
    double _sub37r = 0, _sub37i = 0;
    _sub37r = x1r - x2r; _sub37i = x1i - x2i;
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c36r, _c36i, _sub37r, _sub37i, &_mul38r, &_mul38i);
    double _exp39r = 0, _exp39i = 0;
    c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_mul35r, _mul35i, _exp39r, _exp39i, &_mul40r, &_mul40i);
    double _re41r = 0, _re41i = 0;
    _re41r = _mul40r; _re41i = 0;
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c34r, _c34i, _re41r, _re41i, &_mul42r, &_mul42i);
    { int _idx = 6; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    double _c43r = 0, _c43i = 0;
    _c43r = 100.0; _c43i = 0;
    double _neg44r = 0, _neg44i = 0;
    _neg44r = -(_c43r); _neg44i = -(_c43i);
    double _sin45r = 0, _sin45i = 0;
    c_sin(x2r, x2i, &_sin45r, &_sin45i);
    double _c46r = 0, _c46i = 0;
    _c46r = 3.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(_sin45r, _sin45i, _sin45r, _sin45i, &_pow47r, &_pow47i);
    c_mul(_pow47r, _pow47i, _sin45r, _sin45i, &_pow47r, &_pow47i);
    double _mul48r = 0, _mul48i = 0;
    c_mul(_neg44r, _neg44i, _pow47r, _pow47i, &_mul48r, &_mul48i);
    double _c49r = 0, _c49i = 0;
    _c49r = 2.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow50r, &_pow50i);
    double _cos51r = 0, _cos51i = 0;
    c_cos(x2r, x2i, &_cos51r, &_cos51i);
    double _mul52r = 0, _mul52i = 0;
    c_mul(_pow50r, _pow50i, _cos51r, _cos51i, &_mul52r, &_mul52i);
    double _sin53r = 0, _sin53i = 0;
    c_sin(x1r, x1i, &_sin53r, &_sin53i);
    double _mul54r = 0, _mul54i = 0;
    c_mul(_mul52r, _mul52i, _sin53r, _sin53i, &_mul54r, &_mul54i);
    double _add55r = 0, _add55i = 0;
    _add55r = _mul48r + _mul54r; _add55i = _mul48i + _mul54i;
    { int _idx = 7; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    double _add56r = 0, _add56i = 0;
    _add56r = x1r + x2r; _add56i = x1i + x2i;
    double _c57r = 0, _c57i = 0;
    _c57r = 4.0; _c57i = 0;
    double _pow58r = 0, _pow58i = 0;
    c_mul(_add56r, _add56i, _add56r, _add56i, &_pow58r, &_pow58i);
    c_mul(_pow58r, _pow58i, _pow58r, _pow58i, &_pow58r, &_pow58i);
    double _sub59r = 0, _sub59i = 0;
    _sub59r = x1r - x2r; _sub59i = x1i - x2i;
    double _c60r = 0, _c60i = 0;
    _c60r = 4.0; _c60i = 0;
    double _pow61r = 0, _pow61i = 0;
    c_mul(_sub59r, _sub59i, _sub59r, _sub59i, &_pow61r, &_pow61i);
    c_mul(_pow61r, _pow61i, _pow61r, _pow61i, &_pow61r, &_pow61i);
    double _sub62r = 0, _sub62i = 0;
    _sub62r = _pow58r - _pow61r; _sub62i = _pow58i - _pow61i;
    { int _idx = 8; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub62r; cIm[_idx] = _sub62i; } }
    double _cf63r = 0, _cf63i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 36) { _cf63r = cRe[_idx]; _cf63i = cIm[_idx]; } }
    double _cf64r = 0, _cf64i = 0;
    { int _idx = 7; if (_idx >= 0 && _idx < 36) { _cf64r = cRe[_idx]; _cf64i = cIm[_idx]; } }
    double _mul65r = 0, _mul65i = 0;
    c_mul(_cf63r, _cf63i, _cf64r, _cf64i, &_mul65r, &_mul65i);
    double _cf66r = 0, _cf66i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { _cf66r = cRe[_idx]; _cf66i = cIm[_idx]; } }
    double _cf67r = 0, _cf67i = 0;
    { int _idx = 8; if (_idx >= 0 && _idx < 36) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
    double _mul68r = 0, _mul68i = 0;
    c_mul(_cf66r, _cf66i, _cf67r, _cf67i, &_mul68r, &_mul68i);
    double _sub69r = 0, _sub69i = 0;
    _sub69r = _mul65r - _mul68r; _sub69i = _mul65i - _mul68i;
    { int _idx = 9; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub69r; cIm[_idx] = _sub69i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_777_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int k = 2; k < 26; k++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 1.0;
        double _cf3r = 0, _cf3i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf3r = cRe[_idx]; _cf3i = cIm[_idx]; } }
        double _add4r = 0, _add4i = 0;
        _add4r = _cf3r + x2r; _add4i = _cf3i + x2i;
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(_add4r, _add4i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_c2r, _c2i, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _exp7r = 0, _exp7i = 0;
        c_exp2(_mul6r, _mul6i, &_exp7r, &_exp7i);
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _mul9r = 0, _mul9i = 0;
        c_mul(k, 0, _cf8r, _cf8i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul9r + x1r; _add10i = _mul9i + x1i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_exp7r, _exp7i, _abs11r, _abs11i, &_mul12r, &_mul12i);
        double v = _mul12r; /* +_mul12ii */
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = v; cIm[_idx] = 0; } }
    }
    double _re13r = 0, _re13i = 0;
    _re13r = x1r; _re13i = 0;
    double _im14r = 0, _im14i = 0;
    _im14r = x2i; _im14i = 0;
    double _add15r = 0, _add15i = 0;
    _add15r = _re13r + _im14r; _add15i = _re13i + _im14i;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add15r; cIm[_idx] = _add15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 1.0;
    double _re17r = 0, _re17i = 0;
    _re17r = x1r; _re17i = 0;
    double _im18r = 0, _im18i = 0;
    _im18r = x2i; _im18i = 0;
    double _add19r = 0, _add19i = 0;
    _add19r = _re17r + _im18r; _add19i = _re17i + _im18i;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c16r, _c16i, _add19r, _add19i, &_mul20r, &_mul20i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul20r; cIm[_idx] = _mul20i; } }
    double _mul21r = 0, _mul21i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul21r, &_mul21i);
    double _re22r = 0, _re22i = 0;
    _re22r = _mul21r; _re22i = 0;
    double _c23r = 0, _c23i = 0;
    _c23r = 1.0; _c23i = 0;
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _add25r = 0, _add25i = 0;
    _add25r = _c23r + _c24r; _add25i = _c23i + _c24i;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_re22r, _re22i, _add25r, _add25i, &_mul26r, &_mul26i);
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    double _abs27r = 0, _abs27i = 0;
    _abs27r = c_abs(x1r, x1i); _abs27i = 0;
    double _c28r = 0, _c28i = 0;
    _c28r = 2.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(_abs27r, _abs27i, _abs27r, _abs27i, &_pow29r, &_pow29i);
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(x2r, x2i); _abs30i = 0;
    double _c31r = 0, _c31i = 0;
    _c31r = 2.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(_abs30r, _abs30i, _abs30r, _abs30i, &_pow32r, &_pow32i);
    double _add33r = 0, _add33i = 0;
    _add33r = _pow29r + _pow32r; _add33i = _pow29i + _pow32i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_778_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(_add1r, _add1i); _abs2i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs2r; cIm[_idx] = _abs2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _re4r = 0, _re4i = 0;
    _re4r = x1r; _re4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c3r, _c3i, _re4r, _re4i, &_mul5r, &_mul5i);
    double _im6r = 0, _im6i = 0;
    _im6r = x2i; _im6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_mul5r, _mul5i, _im6r, _im6i, &_mul7r, &_mul7i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _add8r = 0, _add8i = 0;
    _add8r = x1r + x2r; _add8i = x1i + x2i;
    double _ang9r = 0, _ang9i = 0;
    _ang9r = c_arg(_add8r, _add8i); _ang9i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _ang9r; cIm[_idx] = _ang9i; } }
    double _conj10r = 0, _conj10i = 0;
    _conj10r = x1r; _conj10i = -(x1i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_conj10r, _conj10i, x2r, x2i, &_mul11r, &_mul11i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul11r; cIm[_idx] = _mul11i; } }
    double _ang12r = 0, _ang12i = 0;
    _ang12r = c_arg(x1r, x1i); _ang12i = 0;
    double _ang13r = 0, _ang13i = 0;
    _ang13r = c_arg(x2r, x2i); _ang13i = 0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_ang12r, _ang12i, _ang13r, _ang13i, &_mul14r, &_mul14i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    for (int k = 5; k < 22; k++) {
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _neg16r = 0, _neg16i = 0;
        _neg16r = -(_c15r); _neg16i = -(_c15i);
        double _pow17r = 0, _pow17i = 0;
        c_powr(_neg16r, _neg16i, k, &_pow17r, &_pow17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_pow17r, _pow17i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = k + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_mul20r, _mul20i, _add22r, _add22i, &_div23r, &_div23i);
        double _add24r = 0, _add24i = 0;
        _add24r = x1r + _div23r; _add24i = x1i + _div23i;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _neg26r = 0, _neg26i = 0;
        _neg26r = -(_c25r); _neg26i = -(_c25i);
        double _pow27r = 0, _pow27i = 0;
        c_powr(_neg26r, _neg26i, k, &_pow27r, &_pow27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow29r, &_pow29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow27r, _pow27i, _pow29r, _pow29i, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = k + _c31r; _add32i = 0 + _c31i;
        double _div33r = 0, _div33i = 0;
        c_div(_mul30r, _mul30i, _add32r, _add32i, &_div33r, &_div33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _add24r + _div33r; _add34i = _add24i + _div33i;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(_add34r, _add34i); _abs35i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs35r; cIm[_idx] = _abs35i; } }
    }
    double _cf36r = 0, _cf36i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf36r = cRe[_idx]; _cf36i = cIm[_idx]; } }
    double _cf37r = 0, _cf37i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
    double _add38r = 0, _add38i = 0;
    _add38r = _cf36r + _cf37r; _add38i = _cf36i + _cf37i;
    double _cf39r = 0, _cf39i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
    double _sub40r = 0, _sub40i = 0;
    _sub40r = _add38r - _cf39r; _sub40i = _add38i - _cf39i;
    double _cf41r = 0, _cf41i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
    double _add42r = 0, _add42i = 0;
    _add42r = _sub40r + _cf41r; _add42i = _sub40i + _cf41i;
    { int _idx = 21; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    double _cf43r = 0, _cf43i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
    double _cf44r = 0, _cf44i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { _cf44r = cRe[_idx]; _cf44i = cIm[_idx]; } }
    double _mul45r = 0, _mul45i = 0;
    c_mul(_cf43r, _cf43i, _cf44r, _cf44i, &_mul45r, &_mul45i);
    double _cf46r = 0, _cf46i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
    double _mul47r = 0, _mul47i = 0;
    c_mul(_mul45r, _mul45i, _cf46r, _cf46i, &_mul47r, &_mul47i);
    double _cf48r = 0, _cf48i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
    double _mul49r = 0, _mul49i = 0;
    c_mul(_mul47r, _mul47i, _cf48r, _cf48i, &_mul49r, &_mul49i);
    double _abs50r = 0, _abs50i = 0;
    _abs50r = c_abs(_mul49r, _mul49i); _abs50i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs50r; cIm[_idx] = _abs50i; } }
    double _c51r = 0, _c51i = 0;
    _c51r = 1.0; _c51i = 0;
    double _conj52r = 0, _conj52i = 0;
    _conj52r = x1r; _conj52i = -(x1i);
    double _mul53r = 0, _mul53i = 0;
    c_mul(_conj52r, _conj52i, x2r, x2i, &_mul53r, &_mul53i);
    double _re54r = 0, _re54i = 0;
    _re54r = _mul53r; _re54i = 0;
    double _add55r = 0, _add55i = 0;
    _add55r = _c51r + _re54r; _add55i = _c51i + _re54i;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    double _c56r = 0, _c56i = 0;
    _c56r = 0.0; _c56i = 1.0;
    double _conj57r = 0, _conj57i = 0;
    _conj57r = x1r; _conj57i = -(x1i);
    double _mul58r = 0, _mul58i = 0;
    c_mul(_conj57r, _conj57i, x2r, x2i, &_mul58r, &_mul58i);
    double _im59r = 0, _im59i = 0;
    _im59r = _mul58i; _im59i = 0;
    double _add60r = 0, _add60i = 0;
    _add60r = _c56r + _im59r; _add60i = _c56i + _im59i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add60r; cIm[_idx] = _add60i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_779_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs1r, _abs1i, _add3r, &_pow4r, &_pow4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 1.0;
        double _add6r = 0, _add6i = 0;
        _add6r = x2r + _c5r; _add6i = x2i + _c5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = k + _c7r; _add8i = 0 + _c7i;
        double _pow9r = 0, _pow9i = 0;
        c_powr(_add6r, _add6i, _add8r, &_pow9r, &_pow9i);
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(_pow9r, _pow9i); _ang10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _pow4r + _ang10r; _add11i = _pow4i + _ang10i;
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, x1r, x1i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(k, 0, x2r, x2i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sin13r, _sin13i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_mul16r, _mul16i); _abs17i = 0;
        double _log18r = 0, _log18i = 0;
        c_log(_abs17r, _abs17i, &_log18r, &_log18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _add11r + _log18r; _add19i = _add11i + _log18i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    }
    double _pow20r = 0, _pow20i = 0;
    c_powr(x1r, x1i, x2r, &_pow20r, &_pow20i);
    double _conj21r = 0, _conj21i = 0;
    _conj21r = x2r; _conj21i = -(x2i);
    double _c22r = 0, _c22i = 0;
    _c22r = 3.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(_conj21r, _conj21i, _conj21r, _conj21i, &_pow23r, &_pow23i);
    c_mul(_pow23r, _pow23i, _conj21r, _conj21i, &_pow23r, &_pow23i);
    double _add24r = 0, _add24i = 0;
    _add24r = _pow20r + _pow23r; _add24i = _pow20i + _pow23i;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add24r; cIm[_idx] = _add24i; } }
    double _re25r = 0, _re25i = 0;
    _re25r = x1r; _re25i = 0;
    double _cos26r = 0, _cos26i = 0;
    c_cos(_re25r, _re25i, &_cos26r, &_cos26i);
    double _im27r = 0, _im27i = 0;
    _im27r = x2i; _im27i = 0;
    double _sin28r = 0, _sin28i = 0;
    c_sin(_im27r, _im27i, &_sin28r, &_sin28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_cos26r, _cos26i, _sin28r, _sin28i, &_mul29r, &_mul29i);
    double _c30r = 0, _c30i = 0;
    _c30r = 0.0; _c30i = 1.0;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_c30r, _c30i, x2r, x2i, &_mul31r, &_mul31i);
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_mul31r, _mul31i); _abs32i = 0;
    double _log33r = 0, _log33i = 0;
    c_log(_abs32r, _abs32i, &_log33r, &_log33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _mul29r + _log33r; _add34i = _mul29i + _log33i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add34r; cIm[_idx] = _add34i; } }
    double _conj35r = 0, _conj35i = 0;
    _conj35r = x2r; _conj35i = -(x2i);
    double _pow36r = 0, _pow36i = 0;
    c_powr(_conj35r, _conj35i, x1r, &_pow36r, &_pow36i);
    double _re37r = 0, _re37i = 0;
    _re37r = _pow36r; _re37i = 0;
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _c39r = 0, _c39i = 0;
    _c39r = 3.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow40r, &_pow40i);
    c_mul(_pow40r, _pow40i, x1r, x1i, &_pow40r, &_pow40i);
    double _mul41r = 0, _mul41i = 0;
    c_mul(_c38r, _c38i, _pow40r, _pow40i, &_mul41r, &_mul41i);
    double _im42r = 0, _im42i = 0;
    _im42r = _mul41i; _im42i = 0;
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _re37r - _im42r; _sub43i = _re37i - _im42i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_780_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow4r, &_pow4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _pow2r + _pow4r; _add5i = _pow2i + _pow4i;
    double _mul6r = 0, _mul6i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _add5r - _mul6r; _sub7i = _add5i - _mul6i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub7r; cIm[_idx] = _sub7i; } }
    for (int k = 1; k < 25; k++) {
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _add9r = 0, _add9i = 0;
        _add9r = x1r + x2r; _add9i = x1i + x2i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_cf8r, _cf8i, _add9r, _add9i, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _cf12r = 0, _cf12i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf12r = cRe[_idx]; _cf12i = cIm[_idx]; } }
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_cf12r, _cf12i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _c11r + _abs13r; _add14i = _c11i + _abs13i;
        double _div15r = 0, _div15i = 0;
        c_div(_mul10r, _mul10i, _add14r, _add14i, &_div15r, &_div15i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
    }
    double _unk16r = 0, _unk16i = 0;
    /* WARNING: unhandled node List(elts=[Constant(value=2), Constant(value=5), Constant(value=8), Constant(value=11), Constant(value=14), Constant(value=17), Constant(value=20), Constant(value=23)], ctx=Load()) */
    double _c17r = 0, _c17i = 0;
    _c17r = 0.0; _c17i = 1.0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c17r, _c17i, x2r, x2i, &_mul18r, &_mul18i);
    double _add19r = 0, _add19i = 0;
    _add19r = x1r + _mul18r; _add19i = x1i + _mul18i;
    cRe[(int)(_unk16r)] += _add19r; cIm[(int)(_unk16r)] += _add19i;
    double _unk20r = 0, _unk20i = 0;
    /* WARNING: unhandled node List(elts=[Constant(value=1), Constant(value=4), Constant(value=7), Constant(value=10), Constant(value=13), Constant(value=16), Constant(value=19), Constant(value=22), Constant(value=24)], ctx=Load()) */
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 1.0;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c21r, _c21i, x1r, x1i, &_mul22r, &_mul22i);
    double _add23r = 0, _add23i = 0;
    _add23r = x2r + _mul22r; _add23i = x2i + _mul22i;
    cRe[(int)(_unk20r)] -= _add23r; cIm[(int)(_unk20r)] -= _add23i;
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_781_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _pow3r + _pow5r; _add6i = _pow3i + _pow5i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add6r; cIm[_idx] = _add6i; } }
    for (int k = 2; k < 13; k++) {
        double _cf7r = 0, _cf7i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf7r = cRe[_idx]; _cf7i = cIm[_idx]; } }
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_cf7r, _cf7i, _cf8r, _cf8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _c10r + _abs11r; _add12i = _c10i + _abs11i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _add12r + _abs13r; _add14i = _add12i + _abs13i;
        double _div15r = 0, _div15i = 0;
        c_div(_mul9r, _mul9i, _add14r, _add14i, &_div15r, &_div15i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
    }
    for (int l = 13; l < 25; l++) {
        double _cf16r = 0, _cf16i = 0;
        { int _idx = (l - 1); if (_idx >= 0 && _idx < 25) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
        double _cf17r = 0, _cf17i = 0;
        { int _idx = (l - 13); if (_idx >= 0 && _idx < 25) { _cf17r = cRe[_idx]; _cf17i = cIm[_idx]; } }
        double _add18r = 0, _add18i = 0;
        _add18r = _cf16r + _cf17r; _add18i = _cf16i + _cf17i;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _re20r = 0, _re20i = 0;
        _re20r = x1r; _re20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_mul(_re20r, _re20i, _re20r, _re20i, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _c19r + _pow22r; _add23i = _c19i + _pow22i;
        double _im24r = 0, _im24i = 0;
        _im24r = x2i; _im24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(_im24r, _im24i, _im24r, _im24i, &_pow26r, &_pow26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add23r + _pow26r; _add27i = _add23i + _pow26i;
        double _div28r = 0, _div28i = 0;
        c_div(_add18r, _add18i, _add27r, _add27i, &_div28r, &_div28i);
        { int _idx = l; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div28r; cIm[_idx] = _div28i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_781_v1_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _pow3r + _pow5r; _add6i = _pow3i + _pow5i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add6r; cIm[_idx] = _add6i; } }
    for (int k = 3; k < 14; k++) {
        double _cf7r = 0, _cf7i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf7r = cRe[_idx]; _cf7i = cIm[_idx]; } }
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_cf7r, _cf7i, _cf8r, _cf8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _c10r + _abs11r; _add12i = _c10i + _abs11i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _add12r + _abs13r; _add14i = _add12i + _abs13i;
        double _div15r = 0, _div15i = 0;
        c_div(_mul9r, _mul9i, _add14r, _add14i, &_div15r, &_div15i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
    }
    for (int l = 14; l < 25; l++) {
        double _cf16r = 0, _cf16i = 0;
        { int _idx = (l - 1); if (_idx >= 0 && _idx < 25) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
        double _cf17r = 0, _cf17i = 0;
        { int _idx = (l - 13); if (_idx >= 0 && _idx < 25) { _cf17r = cRe[_idx]; _cf17i = cIm[_idx]; } }
        double _add18r = 0, _add18i = 0;
        _add18r = _cf16r + _cf17r; _add18i = _cf16i + _cf17i;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _re20r = 0, _re20i = 0;
        _re20r = x1r; _re20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_mul(_re20r, _re20i, _re20r, _re20i, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _c19r + _pow22r; _add23i = _c19i + _pow22i;
        double _im24r = 0, _im24i = 0;
        _im24r = x2i; _im24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(_im24r, _im24i, _im24r, _im24i, &_pow26r, &_pow26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add23r + _pow26r; _add27i = _add23i + _pow26i;
        double _div28r = 0, _div28i = 0;
        c_div(_add18r, _add18i, _add27r, _add27i, &_div28r, &_div28i);
        { int _idx = l; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div28r; cIm[_idx] = _div28i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_782_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 1.0;
    double _conj3r = 0, _conj3i = 0;
    _conj3r = x2r; _conj3i = -(x2i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(x1r, x1i, _conj3r, _conj3i, &_mul4r, &_mul4i);
    double _ang5r = 0, _ang5i = 0;
    _ang5r = c_arg(_mul4r, _mul4i); _ang5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c2r, _c2i, _ang5r, _ang5i, &_mul6r, &_mul6i);
    double _exp7r = 0, _exp7i = 0;
    c_exp2(_mul6r, _mul6i, &_exp7r, &_exp7i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _exp7r; cIm[_idx] = _exp7i; } }
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(x1r, x1i); _abs8i = 0;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(x2r, x2i); _abs9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_abs8r, _abs8i, _abs9r, _abs9i, &_mul10r, &_mul10i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    for (int k = 3; k < 26; k++) {
        double _cf11r = 0, _cf11i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _re12r = 0, _re12i = 0;
        _re12r = _cf11r; _re12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 1.0;
        double _cf14r = 0, _cf14i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
        double _im15r = 0, _im15i = 0;
        _im15r = _cf14i; _im15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c13r, _c13i, _im15r, _im15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _re12r + _mul16r; _add17i = _re12i + _mul16i;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 1.0;
        double _cf19r = 0, _cf19i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(_cf19r, _cf19i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c18r, _c18i, _ang20r, _ang20i, &_mul21r, &_mul21i);
        double _exp22r = 0, _exp22i = 0;
        c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_add17r, _add17i, _exp22r, _exp22i, &_mul23r, &_mul23i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
        double _cf24r = 0, _cf24i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _im25r = 0, _im25i = 0;
        _im25r = _cf24i; _im25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        if (_im25r == _c26r) {
            double _c27r = 0, _c27i = 0;
            _c27r = 1e-10; _c27i = 0;
            cRe[k] += _c27r; cIm[k] += _c27i;
        }
        double _cf28r = 0, _cf28i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf28r = cRe[_idx]; _cf28i = cIm[_idx]; } }
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(_cf28r, _cf28i); _abs29i = 0;
        double _log30r = 0, _log30i = 0;
        c_log(_abs29r, _abs29i, &_log30r, &_log30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_log30r, _log30i, _c31r, _c31i, &_div32r, &_div32i);
        double _cf33r = 0, _cf33i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_cf33r, _cf33i, _c34r, _c34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _div32r + _mul35r; _add36i = _div32i + _mul35i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_783_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(_mul1r, _mul1i); _abs2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 1.0;
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + x2r; _add4i = x1i + x2i;
    double _ang5r = 0, _ang5i = 0;
    _ang5r = c_arg(_add4r, _add4i); _ang5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c3r, _c3i, _ang5r, _ang5i, &_mul6r, &_mul6i);
    double _exp7r = 0, _exp7i = 0;
    c_exp2(_mul6r, _mul6i, &_exp7r, &_exp7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_abs2r, _abs2i, _exp7r, _exp7i, &_mul8r, &_mul8i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul8r; cIm[_idx] = _mul8i; } }
    double _mul9r = 0, _mul9i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul9r, &_mul9i);
    double _ang10r = 0, _ang10i = 0;
    _ang10r = c_arg(_mul9r, _mul9i); _ang10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 0.0; _c11i = 1.0;
    double _sub12r = 0, _sub12i = 0;
    _sub12r = x1r - x2r; _sub12i = x1i - x2i;
    double _abs13r = 0, _abs13i = 0;
    _abs13r = c_abs(_sub12r, _sub12i); _abs13i = 0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c11r, _c11i, _abs13r, _abs13i, &_mul14r, &_mul14i);
    double _exp15r = 0, _exp15i = 0;
    c_exp2(_mul14r, _mul14i, &_exp15r, &_exp15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_ang10r, _ang10i, _exp15r, _exp15i, &_mul16r, &_mul16i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul16r; cIm[_idx] = _mul16i; } }
    for (int k = 2; k < 25; k++) {
        double _c17r = 0, _c17i = 0;
        _c17r = 0.0; _c17i = 1.0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = k + _c18r; _add19i = 0 + _c18i;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_c17r, _c17i, _add19r, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(x2r, x2i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = x1r + _mul21r; _add22i = x1i + _mul21i;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_add22r, _add22i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _cf25r = 0, _cf25i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _cf26r = 0, _cf26i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
        double _mul27r = 0, _mul27i = 0;
        c_mul(x1r, x1i, _cf26r, _cf26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _cf25r + _mul27r; _add28i = _cf25i + _mul27i;
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(_add28r, _add28i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c24r, _c24i, _ang29r, _ang29i, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_abs23r, _abs23i, _exp31r, _exp31i, &_mul32r, &_mul32i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_784_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _re2r = 0, _re2i = 0;
    _re2r = _add1r; _re2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul3r, &_mul3i);
    double _conj4r = 0, _conj4i = 0;
    _conj4r = _mul3r; _conj4i = -(_mul3i);
    double _add5r = 0, _add5i = 0;
    _add5r = _re2r + _conj4r; _add5i = _re2i + _conj4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    for (int k = 1; k < 25; k++) {
        double _cf6r = 0, _cf6i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf6r = cRe[_idx]; _cf6i = cIm[_idx]; } }
        double _mul7r = 0, _mul7i = 0;
        c_mul(x1r, x1i, _cf6r, _cf6i, &_mul7r, &_mul7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_mul7r, _mul7i); _abs8i = 0;
        double n = _abs8r; /* +_abs8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 0;
        if (n != _c9r) {
            double _ang10r = 0, _ang10i = 0;
            _ang10r = c_arg(x1r, x1i); _ang10i = 0;
            double _sin11r = 0, _sin11i = 0;
            c_sin(_ang10r, _ang10i, &_sin11r, &_sin11i);
            double _abs12r = 0, _abs12i = 0;
            _abs12r = c_abs(n, 0); _abs12i = 0;
            double _log13r = 0, _log13i = 0;
            c_log(_abs12r, _abs12i, &_log13r, &_log13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(_sin11r, _sin11i, _log13r, _log13i, &_mul14r, &_mul14i);
            double _ang15r = 0, _ang15i = 0;
            _ang15r = c_arg(x2r, x2i); _ang15i = 0;
            double _cos16r = 0, _cos16i = 0;
            c_cos(_ang15r, _ang15i, &_cos16r, &_cos16i);
            double _c17r = 0, _c17i = 0;
            _c17r = 1.0; _c17i = 0;
            double _div18r = 0, _div18i = 0;
            c_div(_c17r, _c17i, n, 0, &_div18r, &_div18i);
            double _abs19r = 0, _abs19i = 0;
            _abs19r = c_abs(_div18r, _div18i); _abs19i = 0;
            double _log20r = 0, _log20i = 0;
            c_log(_abs19r, _abs19i, &_log20r, &_log20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(_cos16r, _cos16i, _log20r, _log20i, &_mul21r, &_mul21i);
            double _add22r = 0, _add22i = 0;
            _add22r = _mul14r + _mul21r; _add22i = _mul14i + _mul21i;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
        } else {
            double _c23r = 0, _c23i = 0;
            _c23r = 0.0; _c23i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c23r; cIm[_idx] = _c23i; } }
        }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_785_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _abop3r = 0, _abop3i = 0;
    c_mul(_c2r, _c2i, _c1r, _c1i, &_abop3r, &_abop3i);
    cRe[0] = _abop3r; cIm[0] = _abop3i;
    double _abop4r = 0, _abop4i = 0;
    c_mul(x1r, x1i, _c1r, _c1i, &_abop4r, &_abop4i);
    cRe[1] = _abop4r; cIm[1] = _abop4i;
    double _abop5r = 0, _abop5i = 0;
    c_mul(x2r, x2i, _c1r, _c1i, &_abop5r, &_abop5i);
    cRe[2] = _abop5r; cIm[2] = _abop5i;
    double _mul6r = 0, _mul6i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
    double _abop7r = 0, _abop7i = 0;
    c_mul(_mul6r, _mul6i, _c1r, _c1i, &_abop7r, &_abop7i);
    cRe[3] = _abop7r; cIm[3] = _abop7i;
    double _add8r = 0, _add8i = 0;
    _add8r = x1r + x2r; _add8i = x1i + x2i;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
    double _abop10r = 0, _abop10i = 0;
    c_mul(_abs9r, _abs9i, _c1r, _c1i, &_abop10r, &_abop10i);
    cRe[4] = _abop10r; cIm[4] = _abop10i;
    for (int k = 5; k < 25; k++) {
        double _c11r = 0, _c11i = 0;
        _c11r = 3.0; _c11i = 0;
        double _cf12r = 0, _cf12i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf12r = cRe[_idx]; _cf12i = cIm[_idx]; } }
        double _mul13r = 0, _mul13i = 0;
        c_mul(_c11r, _c11i, _cf12r, _cf12i, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _cf15r = 0, _cf15i = 0;
        { int _idx = (k - 5); if (_idx >= 0 && _idx < 25) { _cf15r = cRe[_idx]; _cf15i = cIm[_idx]; } }
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c14r, _c14i, _cf15r, _cf15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul13r + _mul16r; _add17i = _mul13i + _mul16i;
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = k + _c19r; _add20i = 0 + _c19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c18r, _c18i, _add20r, _add20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _add17r + _mul21r; _add22i = _add17i + _mul21i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    }
    for (int _si = 0; _si < 6; _si++) {
        int _si_idx = _si + 9;
        double _unk23r = 0, _unk23i = 0;
        /* WARNING: unhandled node Slice(upper=Constant(value=6)) */
        double _cf24r = 0, _cf24i = 0;
        { int _idx = (int)(_unk23r); if (_idx >= 0 && _idx < 25) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _conj25r = 0, _conj25i = 0;
        _conj25r = _cf24r; _conj25i = -(_cf24i);
        cRe[_si_idx] = _conj25r; cIm[_si_idx] = _conj25i;
    }
    for (int _si = 0; _si < 6; _si++) {
        int _si_idx = _si + 19;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _unk27r = 0, _unk27i = 0;
        /* WARNING: unhandled node Slice(upper=Constant(value=6)) */
        double _cf28r = 0, _cf28i = 0;
        { int _idx = (int)(_unk27r); if (_idx >= 0 && _idx < 25) { _cf28r = cRe[_idx]; _cf28i = cIm[_idx]; } }
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(_cf28r, _cf28i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c26r, _c26i, _ang29r, _ang29i, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        cRe[_si_idx] = _exp31r; cIm[_si_idx] = _exp31i;
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_786_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _div3r = 0, _div3i = 0;
        c_div(k, 0, _c2r, _c2i, &_div3r, &_div3i);
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs1r, _abs1i, _div3r, &_pow4r, &_pow4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(k, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 1.0;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_c8r, _c8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _cos7r + _mul12r; _add13i = _cos7i + _mul12i;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_pow4r, _pow4i, _add13r, _add13i, &_mul14r, &_mul14i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    }
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(x1r, x1i); _abs15i = 0;
    double _log16r = 0, _log16i = 0;
    c_log(_abs15r, _abs15i, &_log16r, &_log16i);
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x2r, x2i); _abs17i = 0;
    double _log18r = 0, _log18i = 0;
    c_log(_abs17r, _abs17i, &_log18r, &_log18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _log16r + _log18r; _add19i = _log16i + _log18i;
    double _c20r = 0, _c20i = 0;
    _c20r = 2.0; _c20i = 0;
    double _div21r = 0, _div21i = 0;
    c_div(_add19r, _add19i, _c20r, _c20i, &_div21r, &_div21i);
    cRe[4] += _div21r; cIm[4] += _div21i;
    double _mul22r = 0, _mul22i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul22r, &_mul22i);
    double _conj23r = 0, _conj23i = 0;
    _conj23r = _mul22r; _conj23i = -(_mul22i);
    cRe[9] += _conj23r; cIm[9] += _conj23i;
    double _sub24r = 0, _sub24i = 0;
    _sub24r = x2r - x1r; _sub24i = x2i - x1i;
    double _abs25r = 0, _abs25i = 0;
    _abs25r = c_abs(_sub24r, _sub24i); _abs25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(_abs25r, _abs25i, _abs25r, _abs25i, &_pow27r, &_pow27i);
    cRe[14] += _pow27r; cIm[14] += _pow27i;
    double _ang28r = 0, _ang28i = 0;
    _ang28r = c_arg(x1r, x1i); _ang28i = 0;
    double _sin29r = 0, _sin29i = 0;
    c_sin(_ang28r, _ang28i, &_sin29r, &_sin29i);
    double _ang30r = 0, _ang30i = 0;
    _ang30r = c_arg(x2r, x2i); _ang30i = 0;
    double _cos31r = 0, _cos31i = 0;
    c_cos(_ang30r, _ang30i, &_cos31r, &_cos31i);
    double _div32r = 0, _div32i = 0;
    c_div(_sin29r, _sin29i, _cos31r, _cos31i, &_div32r, &_div32i);
    double _c33r = 0, _c33i = 0;
    _c33r = 3.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_div32r, _div32i, _div32r, _div32i, &_pow34r, &_pow34i);
    c_mul(_pow34r, _pow34i, _div32r, _div32i, &_pow34r, &_pow34i);
    cRe[19] += _pow34r; cIm[19] += _pow34i;
    double _c35r = 0, _c35i = 0;
    _c35r = 0.0; _c35i = 1.0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c35r, _c35i, x1r, x1i, &_mul36r, &_mul36i);
    double _sub37r = 0, _sub37i = 0;
    _sub37r = _mul36r - x2r; _sub37i = _mul36i - x2i;
    double _c38r = 0, _c38i = 0;
    _c38r = 2.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(_sub37r, _sub37i, _sub37r, _sub37i, &_pow39r, &_pow39i);
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = x1r + x2r; _add41i = x1i + x2i;
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(_add41r, _add41i); _abs42i = 0;
    double _c43r = 0, _c43i = 0;
    _c43r = 3.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(_abs42r, _abs42i, _abs42r, _abs42i, &_pow44r, &_pow44i);
    c_mul(_pow44r, _pow44i, _abs42r, _abs42i, &_pow44r, &_pow44i);
    double _add45r = 0, _add45i = 0;
    _add45r = _c40r + _pow44r; _add45i = _c40i + _pow44i;
    double _div46r = 0, _div46i = 0;
    c_div(_pow39r, _pow39i, _add45r, _add45i, &_div46r, &_div46i);
    double _c47r = 0, _c47i = 0;
    _c47r = 4.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(_div46r, _div46i, _div46r, _div46i, &_pow48r, &_pow48i);
    c_mul(_pow48r, _pow48i, _pow48r, _pow48i, &_pow48r, &_pow48i);
    cRe[24] += _pow48r; cIm[24] += _pow48i;
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_787_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _re2r = 0, _re2i = 0;
    _re2r = _add1r; _re2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 1.0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul4r, &_mul4i);
    double _im5r = 0, _im5i = 0;
    _im5r = _mul4i; _im5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c3r, _c3i, _im5r, _im5i, &_mul6r, &_mul6i);
    double _add7r = 0, _add7i = 0;
    _add7r = _re2r + _mul6r; _add7i = _re2i + _mul6i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    for (int i = 1; i < 24; i++) {
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _sub9r = 0, _sub9i = 0;
        _sub9r = x1r - x2r; _sub9i = x1i - x2i;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_cf8r, _cf8i, _sub9r, &_pow10r, &_pow10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_pow10r, _pow10i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 0.0; _c12i = 1.0;
        double _cf13r = 0, _cf13i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(_cf13r, _cf13i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_c12r, _c12i, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _abs11r + _mul15r; _add16i = _abs11i + _mul15i;
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add16r; cIm[_idx] = _add16i; } }
        double _np17r = 0, _np17i = 0;
        /* WARNING: unhandled np.isinf */
        double _np18r = 0, _np18i = 0;
        /* WARNING: unhandled np.isnan */
        if (((_np17r != 0 || _np17i != 0)) || ((_np18r != 0 || _np18i != 0))) {
            double _c19r = 0, _c19i = 0;
            _c19r = 0.0; _c19i = 1.0;
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c19r; cIm[_idx] = _c19i; } }
        }
    }
    double _cf20r = 0, _cf20i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(_cf20r, _cf20i, _cf20r, _cf20i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, _cf20r, _cf20i, &_pow22r, &_pow22i);
    double _cf23r = 0, _cf23i = 0;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 2.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(_cf23r, _cf23i, _cf23r, _cf23i, &_pow25r, &_pow25i);
    double _add26r = 0, _add26i = 0;
    _add26r = _pow22r + _pow25r; _add26i = _pow22i + _pow25i;
    double _cf27r = 0, _cf27i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { _cf27r = cRe[_idx]; _cf27i = cIm[_idx]; } }
    double _add28r = 0, _add28i = 0;
    _add28r = _add26r + _cf27r; _add28i = _add26i + _cf27i;
    double _cf29r = 0, _cf29i = 0;
    { int _idx = 21; if (_idx >= 0 && _idx < 25) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
    double _sub30r = 0, _sub30i = 0;
    _sub30r = _add28r - _cf29r; _sub30i = _add28i - _cf29i;
    double _cf31r = 0, _cf31i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
    double _conj32r = 0, _conj32i = 0;
    _conj32r = _cf31r; _conj32i = -(_cf31i);
    double _add33r = 0, _add33i = 0;
    _add33r = _sub30r + _conj32r; _add33i = _sub30i + _conj32i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_788_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, x2r, x2i, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 1.0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c5r, _c5i, x2r, x2i, &_mul6r, &_mul6i);
    double _add7r = 0, _add7i = 0;
    _add7r = x1r + _mul6r; _add7i = x1i + _mul6i;
    double _re8r = 0, _re8i = 0;
    _re8r = _add7r; _re8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(_re8r, _re8i, _re8r, _re8i, &_pow10r, &_pow10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c4r, _c4i, _pow10r, _pow10i, &_mul11r, &_mul11i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul11r; cIm[_idx] = _mul11i; } }
    double _c12r = 0, _c12i = 0;
    _c12r = 3.0; _c12i = 0;
    double _c13r = 0, _c13i = 0;
    _c13r = 0.0; _c13i = 1.0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c13r, _c13i, x2r, x2i, &_mul14r, &_mul14i);
    double _add15r = 0, _add15i = 0;
    _add15r = x1r + _mul14r; _add15i = x1i + _mul14i;
    double _im16r = 0, _im16i = 0;
    _im16r = _add15i; _im16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(_im16r, _im16i, _im16r, _im16i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, _im16r, _im16i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c12r, _c12i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    for (int k = 3; k < 15; k++) {
        double _cf20r = 0, _cf20i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = k + _c21r; _add22i = 0 + _c21i;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_cf20r, _cf20i, _add22r, &_pow23r, &_pow23i);
        double _cf24r = 0, _cf24i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _pow25r = 0, _pow25i = 0;
        c_powr(_cf24r, _cf24i, k, &_pow25r, &_pow25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _pow23r + _pow25r; _add26i = _pow23i + _pow25i;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_add26r, _add26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(k, 0, k, 0, &_pow29r, &_pow29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _pow29r + _c30r; _add31i = _pow29i + _c30i;
        double _div32r = 0, _div32i = 0;
        c_div(_abs27r, _abs27i, _add31r, _add31i, &_div32r, &_div32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x1r, x1i); _ang33i = 0;
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _ang33r + _ang34r; _add35i = _ang33i + _ang34i;
        double _add36r = 0, _add36i = 0;
        _add36r = _div32r + _add35r; _add36i = _div32i + _add35i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 15;
        double _unk37r = 0, _unk37i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=10), upper=Constant(value=15)) */
        double _cf38r = 0, _cf38i = 0;
        { int _idx = (int)(_unk37r); if (_idx >= 0 && _idx < 25) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
        double _unk39r = 0, _unk39i = 0;
        /* WARNING: unhandled node Slice(upper=Constant(value=5)) */
        double _cf40r = 0, _cf40i = 0;
        { int _idx = (int)(_unk39r); if (_idx >= 0 && _idx < 25) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
        double _add41r = 0, _add41i = 0;
        _add41r = _cf38r + _cf40r; _add41i = _cf38i + _cf40i;
        cRe[_si_idx] = _add41r; cIm[_si_idx] = _add41i;
    }
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(x1r, x1i); _abs42i = 0;
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(_abs42r, _abs42i, _abs42r, _abs42i, &_pow44r, &_pow44i);
    double _abs45r = 0, _abs45i = 0;
    _abs45r = c_abs(x2r, x2i); _abs45i = 0;
    double _c46r = 0, _c46i = 0;
    _c46r = 2.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(_abs45r, _abs45i, _abs45r, _abs45i, &_pow47r, &_pow47i);
    double _sub48r = 0, _sub48i = 0;
    _sub48r = _pow44r - _pow47r; _sub48i = _pow44i - _pow47i;
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub48r; cIm[_idx] = _sub48i; } }
    double _ang49r = 0, _ang49i = 0;
    _ang49r = c_arg(x1r, x1i); _ang49i = 0;
    double _ang50r = 0, _ang50i = 0;
    _ang50r = c_arg(x2r, x2i); _ang50i = 0;
    double _add51r = 0, _add51i = 0;
    _add51r = _ang49r + _ang50r; _add51i = _ang49i + _ang50i;
    { int _idx = 21; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    double _c52r = 0, _c52i = 0;
    _c52r = 3.0; _c52i = 0;
    double _pow53r = 0, _pow53i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow53r, &_pow53i);
    c_mul(_pow53r, _pow53i, x1r, x1i, &_pow53r, &_pow53i);
    double _c54r = 0, _c54i = 0;
    _c54r = 3.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow55r, &_pow55i);
    c_mul(_pow55r, _pow55i, x2r, x2i, &_pow55r, &_pow55i);
    double _sub56r = 0, _sub56i = 0;
    _sub56r = _pow53r - _pow55r; _sub56i = _pow53i - _pow55i;
    double _re57r = 0, _re57i = 0;
    _re57r = _sub56r; _re57i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _re57r; cIm[_idx] = _re57i; } }
    double _mul58r = 0, _mul58i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul58r, &_mul58i);
    double _sub59r = 0, _sub59i = 0;
    _sub59r = x1r - x2r; _sub59i = x1i - x2i;
    double _mul60r = 0, _mul60i = 0;
    c_mul(_mul58r, _mul58i, _sub59r, _sub59i, &_mul60r, &_mul60i);
    double _im61r = 0, _im61i = 0;
    _im61r = _mul60i; _im61i = 0;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _im61r; cIm[_idx] = _im61i; } }
    double _mul62r = 0, _mul62i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul62r, &_mul62i);
    double _sub63r = 0, _sub63i = 0;
    _sub63r = x1r - x2r; _sub63i = x1i - x2i;
    double _mul64r = 0, _mul64i = 0;
    c_mul(_mul62r, _mul62i, _sub63r, _sub63i, &_mul64r, &_mul64i);
    double _abs65r = 0, _abs65i = 0;
    _abs65r = c_abs(_mul64r, _mul64i); _abs65i = 0;
    double _c66r = 0, _c66i = 0;
    _c66r = 0.5; _c66i = 0;
    double _pow67r = 0, _pow67i = 0;
    c_powr(_abs65r, _abs65i, 0.5, &_pow67r, &_pow67i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow67r; cIm[_idx] = _pow67i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_789_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 0;
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _arange2r = 0, _arange2i = 0;
        _arange2r = (double)(_si + 1); _arange2i = 0;
        double _smul3r = 0, _smul3i = 0;
        c_mul(_re1r, _re1i, _arange2r, _arange2i, &_smul3r, &_smul3i);
        double _im4r = 0, _im4i = 0;
        _im4r = x2i; _im4i = 0;
        double _arange5r = 0, _arange5i = 0;
        _arange5r = (double)(_si + 1); _arange5i = 0;
        double _smul6r = 0, _smul6i = 0;
        c_mul(_im4r, _im4i, _arange5r, _arange5i, &_smul6r, &_smul6i);
        double _ssub7r = 0, _ssub7i = 0;
        _ssub7r = _smul3r - _smul6r; _ssub7i = _smul3i - _smul6i;
        cRe[_si_idx] = _ssub7r; cIm[_si_idx] = _ssub7i;
    }
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(x1r, x1i); _abs8i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs8r; cIm[_idx] = _abs8i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 6;
        double _add9r = 0, _add9i = 0;
        _add9r = x1r + x2r; _add9i = x1i + x2i;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(_add9r, _add9i); _ang10i = 0;
        double _arange11r = 0, _arange11i = 0;
        _arange11r = (double)(_si + 6); _arange11i = 0;
        double _smul12r = 0, _smul12i = 0;
        c_mul(_ang10r, _ang10i, _arange11r, _arange11i, &_smul12r, &_smul12i);
        cRe[_si_idx] = _smul12r; cIm[_si_idx] = _smul12i;
    }
    double _conj13r = 0, _conj13i = 0;
    _conj13r = x1r; _conj13i = -(x1i);
    double _conj14r = 0, _conj14i = 0;
    _conj14r = x2r; _conj14i = -(x2i);
    double _add15r = 0, _add15i = 0;
    _add15r = _conj13r + _conj14r; _add15i = _conj13i + _conj14i;
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add15r; cIm[_idx] = _add15i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 13;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, x2r, x2i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = x1r + _mul17r; _add18i = x1i + _mul17i;
        double _re19r = 0, _re19i = 0;
        _re19r = _add18r; _re19i = 0;
        double _arange20r = 0, _arange20i = 0;
        _arange20r = (double)(_si + 1); _arange20i = 0;
        double _smul21r = 0, _smul21i = 0;
        c_mul(_re19r, _re19i, _arange20r, _arange20i, &_smul21r, &_smul21i);
        cRe[_si_idx] = _smul21r; cIm[_si_idx] = _smul21i;
    }
    double _ang22r = 0, _ang22i = 0;
    _ang22r = c_arg(x1r, x1i); _ang22i = 0;
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _ang22r; cIm[_idx] = _ang22i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 19;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, x2r, x2i, &_mul24r, &_mul24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = x1r - _mul24r; _sub25i = x1i - _mul24i;
        double _im26r = 0, _im26i = 0;
        _im26r = _sub25i; _im26i = 0;
        double _arange27r = 0, _arange27i = 0;
        _arange27r = (double)(_si + 1); _arange27i = 0;
        double _smul28r = 0, _smul28i = 0;
        c_mul(_im26r, _im26i, _arange27r, _arange27i, &_smul28r, &_smul28i);
        cRe[_si_idx] = _smul28r; cIm[_si_idx] = _smul28i;
    }
    double _mul29r = 0, _mul29i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul29r, &_mul29i);
    double _conj30r = 0, _conj30i = 0;
    _conj30r = _mul29r; _conj30i = -(_mul29i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _conj30r; cIm[_idx] = _conj30i; } }
    double _cf31r = 0, _cf31i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_cf31r, _cf31i); _abs32i = 0;
    double _cf33r = 0, _cf33i = 0;
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
    double _ang34r = 0, _ang34i = 0;
    _ang34r = c_arg(_cf33r, _cf33i); _ang34i = 0;
    double _add35r = 0, _add35i = 0;
    _add35r = _abs32r + _ang34r; _add35i = _abs32i + _ang34i;
    { int _idx = 25; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_790_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int k = 1; k < 25; k++) {
        double _cf2r = 0, _cf2i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf2r = cRe[_idx]; _cf2i = cIm[_idx]; } }
        double _mul3r = 0, _mul3i = 0;
        c_mul(k, 0, _cf2r, _cf2i, &_mul3r, &_mul3i);
        double _pow4r = 0, _pow4i = 0;
        c_powr(x2r, x2i, k, &_pow4r, &_pow4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(_pow4r, _pow4i); _ang5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul3r + _ang5r; _add6i = _mul3i + _ang5i;
        double _sin7r = 0, _sin7i = 0;
        c_sin(_add6r, _add6i, &_sin7r, &_sin7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(k, 0, _abs8r, _abs8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _sin7r + _cos10r; _add11i = _sin7i + _cos10i;
        double v = _add11r; /* +_add11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(v, 0); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1e-10; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _div15r = 0, _div15i = 0;
        c_div(v, 0, _add14r, _add14i, &_div15r, &_div15i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
    }
    double _mul16r = 0, _mul16i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul16r, &_mul16i);
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x2r, x2i); _abs17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(_abs17r, _abs17i, _abs17r, _abs17i, &_pow19r, &_pow19i);
    double _sub20r = 0, _sub20i = 0;
    _sub20r = _mul16r - _pow19r; _sub20i = _mul16i - _pow19i;
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 1.0;
    double _ang22r = 0, _ang22i = 0;
    _ang22r = c_arg(x1r, x1i); _ang22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c21r, _c21i, _ang22r, _ang22i, &_mul23r, &_mul23i);
    double _add24r = 0, _add24i = 0;
    _add24r = _sub20r + _mul23r; _add24i = _sub20i + _mul23i;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add24r; cIm[_idx] = _add24i; } }
    double _conj25r = 0, _conj25i = 0;
    _conj25r = x1r; _conj25i = -(x1i);
    double _c26r = 0, _c26i = 0;
    _c26r = 3.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(_conj25r, _conj25i, _conj25r, _conj25i, &_pow27r, &_pow27i);
    c_mul(_pow27r, _pow27i, _conj25r, _conj25i, &_pow27r, &_pow27i);
    double _ang28r = 0, _ang28i = 0;
    _ang28r = c_arg(x2r, x2i); _ang28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 3.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_ang28r, _ang28i, _ang28r, _ang28i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, _ang28r, _ang28i, &_pow30r, &_pow30i);
    double _sub31r = 0, _sub31i = 0;
    _sub31r = _pow27r - _pow30r; _sub31i = _pow27i - _pow30i;
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 1.0;
    double _abs33r = 0, _abs33i = 0;
    _abs33r = c_abs(x2r, x2i); _abs33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c32r, _c32i, _abs33r, _abs33i, &_mul34r, &_mul34i);
    double _add35r = 0, _add35i = 0;
    _add35r = _sub31r + _mul34r; _add35i = _sub31i + _mul34i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    double _abs36r = 0, _abs36i = 0;
    _abs36r = c_abs(x2r, x2i); _abs36i = 0;
    double _c37r = 0, _c37i = 0;
    _c37r = 3.0; _c37i = 0;
    double _pow38r = 0, _pow38i = 0;
    c_mul(_abs36r, _abs36i, _abs36r, _abs36i, &_pow38r, &_pow38i);
    c_mul(_pow38r, _pow38i, _abs36r, _abs36i, &_pow38r, &_pow38i);
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow40r, &_pow40i);
    double _add41r = 0, _add41i = 0;
    _add41r = _pow38r + _pow40r; _add41i = _pow38i + _pow40i;
    double _c42r = 0, _c42i = 0;
    _c42r = 2.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow43r, &_pow43i);
    double _add44r = 0, _add44i = 0;
    _add44r = _add41r + _pow43r; _add44i = _add41i + _pow43i;
    double _c45r = 0, _c45i = 0;
    _c45r = 0.0; _c45i = 1.0;
    double _ang46r = 0, _ang46i = 0;
    _ang46r = c_arg(x2r, x2i); _ang46i = 0;
    double _c47r = 0, _c47i = 0;
    _c47r = 2.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(_ang46r, _ang46i, _ang46r, _ang46i, &_pow48r, &_pow48i);
    double _mul49r = 0, _mul49i = 0;
    c_mul(_c45r, _c45i, _pow48r, _pow48i, &_mul49r, &_mul49i);
    double _add50r = 0, _add50i = 0;
    _add50r = _add44r + _mul49r; _add50i = _add44i + _mul49i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    double _mul51r = 0, _mul51i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul51r, &_mul51i);
    double _abs52r = 0, _abs52i = 0;
    _abs52r = c_abs(_mul51r, _mul51i); _abs52i = 0;
    double _ang53r = 0, _ang53i = 0;
    _ang53r = c_arg(x1r, x1i); _ang53i = 0;
    double _c54r = 0, _c54i = 0;
    _c54r = 5.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_powr(_ang53r, _ang53i, 5.0, &_pow55r, &_pow55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _abs52r + _pow55r; _add56i = _abs52i + _pow55i;
    double _c57r = 0, _c57i = 0;
    _c57r = 0.0; _c57i = 1.0;
    double _abs58r = 0, _abs58i = 0;
    _abs58r = c_abs(x1r, x1i); _abs58i = 0;
    double _c59r = 0, _c59i = 0;
    _c59r = 5.0; _c59i = 0;
    double _pow60r = 0, _pow60i = 0;
    c_powr(_abs58r, _abs58i, 5.0, &_pow60r, &_pow60i);
    double _mul61r = 0, _mul61i = 0;
    c_mul(_c57r, _c57i, _pow60r, _pow60i, &_mul61r, &_mul61i);
    double _add62r = 0, _add62i = 0;
    _add62r = _add56r + _mul61r; _add62i = _add56i + _mul61i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add62r; cIm[_idx] = _add62i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_791_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 0;
        double _arange1r = 0, _arange1i = 0;
        _arange1r = (double)(_si + 1); _arange1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
        double _sadd4r = 0, _sadd4i = 0;
        _sadd4r = _arange1r + _pow3r; _sadd4i = _arange1i + _pow3i;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow6r, &_pow6i);
        double _sadd7r = 0, _sadd7i = 0;
        _sadd7r = _sadd4r + _pow6r; _sadd7i = _sadd4i + _pow6i;
        cRe[_si_idx] = _sadd7r; cIm[_si_idx] = _sadd7i;
    }
    for (int i = 10; i < 20; i++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _cf9r = 0, _cf9i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf9r = cRe[_idx]; _cf9i = cIm[_idx]; } }
        double _mul10r = 0, _mul10i = 0;
        c_mul(_c8r, _c8i, _cf9r, _cf9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = x1r + x2r; _add11i = x1i + x2i;
        double _sin12r = 0, _sin12i = 0;
        c_sin(_add11r, _add11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_mul10r, _mul10i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    }
    double _add14r = 0, _add14i = 0;
    _add14r = x1r + x2r; _add14i = x1i + x2i;
    double _sum15r = 0, _sum15i = 0;
    _sum15r = 0; _sum15i = 0;
    for (int _si = 0; _si < 20; _si++) { _sum15r += cRe[_si]; _sum15i += cIm[_si]; }
    double _abop16r = 0, _abop16i = 0;
    c_mul(_sum15r, _sum15i, _add14r, _add14i, &_abop16r, &_abop16i);
    cRe[20] = _abop16r; cIm[20] = _abop16i;
    double _prod17r = 0, _prod17i = 0;
    _prod17r = 1; _prod17i = 0;
    for (int _pi = 0; _pi < 20; _pi++) { double _pr = _prod17r*cRe[_pi]-_prod17i*cIm[_pi]; double _pp = _prod17r*cIm[_pi]+_prod17i*cRe[_pi]; _prod17r=_pr; _prod17i=_pp; }
    double _abop18r = 0, _abop18i = 0;
    c_mul(_prod17r, _prod17i, _add14r, _add14i, &_abop18r, &_abop18i);
    cRe[21] = _abop18r; cIm[21] = _abop18i;
    double _mul19r = 0, _mul19i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul19r, &_mul19i);
    double _abs20r = 0, _abs20i = 0;
    _abs20r = c_abs(_mul19r, _mul19i); _abs20i = 0;
    double _log21r = 0, _log21i = 0;
    c_log(_abs20r, _abs20i, &_log21r, &_log21i);
    double _c22r = 0, _c22i = 0;
    _c22r = 2.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow23r, &_pow23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 2.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow25r, &_pow25i);
    double _add26r = 0, _add26i = 0;
    _add26r = _pow23r + _pow25r; _add26i = _pow23i + _pow25i;
    double _ang27r = 0, _ang27i = 0;
    _ang27r = c_arg(_add26r, _add26i); _ang27i = 0;
    double _cf28r = 0, _cf28i = 0;
    { int _idx = 21; if (_idx >= 0 && _idx < 25) { _cf28r = cRe[_idx]; _cf28i = cIm[_idx]; } }
    double _mul29r = 0, _mul29i = 0;
    c_mul(_ang27r, _ang27i, _cf28r, _cf28i, &_mul29r, &_mul29i);
    double _add30r = 0, _add30i = 0;
    _add30r = _log21r + _mul29r; _add30i = _log21i + _mul29i;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    double _sub31r = 0, _sub31i = 0;
    _sub31r = x1r - x2r; _sub31i = x1i - x2i;
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_sub31r, _sub31i); _abs32i = 0;
    double _cf33r = 0, _cf33i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(_cf33r, _cf33i); _abs34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 1.0; _c35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _abs34r + _c35r; _add36i = _abs34i + _c35i;
    double _div37r = 0, _div37i = 0;
    c_div(_abs32r, _abs32i, _add36r, _add36i, &_div37r, &_div37i);
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div37r; cIm[_idx] = _div37i; } }
    double _cf38r = 0, _cf38i = 0;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
    double _cf39r = 0, _cf39i = 0;
    { int _idx = 21; if (_idx >= 0 && _idx < 25) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
    double _mul40r = 0, _mul40i = 0;
    c_mul(_cf38r, _cf38i, _cf39r, _cf39i, &_mul40r, &_mul40i);
    double _conj41r = 0, _conj41i = 0;
    _conj41r = _mul40r; _conj41i = -(_mul40i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _conj41r; cIm[_idx] = _conj41i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_792_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _sin2r = 0, _sin2i = 0;
    c_sin(_add1r, _add1i, &_sin2r, &_sin2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 1.0;
    double _sub4r = 0, _sub4i = 0;
    _sub4r = x1r - x2r; _sub4i = x1i - x2i;
    double _cos5r = 0, _cos5i = 0;
    c_cos(_sub4r, _sub4i, &_cos5r, &_cos5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c3r, _c3i, _cos5r, _cos5i, &_mul6r, &_mul6i);
    double _add7r = 0, _add7i = 0;
    _add7r = _sin2r + _mul6r; _add7i = _sin2i + _mul6i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    for (int k = 1; k < 25; k++) {
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _mul9r = 0, _mul9i = 0;
        c_mul(k, 0, _cf8r, _cf8i, &_mul9r, &_mul9i);
        double _exp10r = 0, _exp10i = 0;
        c_exp2(_mul9r, _mul9i, &_exp10r, &_exp10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _log12r = 0, _log12i = 0;
        c_log(_abs11r, _abs11i, &_log12r, &_log12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _exp10r + _log12r; _add13i = _exp10i + _log12i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _log15r = 0, _log15i = 0;
        c_log(_abs14r, _abs14i, &_log15r, &_log15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = _add13r - _log15r; _sub16i = _add13i - _log15i;
        double v = _sub16r; /* +_sub16ii */
        double _np17r = 0, _np17i = 0;
        /* WARNING: unhandled np.isnan */
        double _np18r = 0, _np18i = 0;
        /* WARNING: unhandled np.isinf */
        if ((!((_np17r != 0 || _np17i != 0))) && (!((_np18r != 0 || _np18i != 0)))) {
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = v; cIm[_idx] = 0; } }
        } else {
            double _c19r = 0, _c19i = 0;
            _c19r = 1.0; _c19i = 0;
            double _c20r = 0, _c20i = 0;
            _c20r = 0.0; _c20i = 1.0;
            double _add21r = 0, _add21i = 0;
            _add21r = _c19r + _c20r; _add21i = _c19i + _c20i;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
        }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_793_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(x1r, x1i); _abs2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_abs2r, _abs2i, x2r, x2i, &_mul3r, &_mul3i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    double _conj4r = 0, _conj4i = 0;
    _conj4r = x1r; _conj4i = -(x1i);
    double _re5r = 0, _re5i = 0;
    _re5r = x2r; _re5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = _conj4r + _re5r; _add6i = _conj4i + _re5i;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add6r; cIm[_idx] = _add6i; } }
    double _abs7r = 0, _abs7i = 0;
    _abs7r = c_abs(x1r, x1i); _abs7i = 0;
    double _im8r = 0, _im8i = 0;
    _im8r = x2i; _im8i = 0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_abs7r, _abs7i, _im8r, _im8i, &_mul9r, &_mul9i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
    double _ang10r = 0, _ang10i = 0;
    _ang10r = c_arg(x1r, x1i); _ang10i = 0;
    double _conj11r = 0, _conj11i = 0;
    _conj11r = x2r; _conj11i = -(x2i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_ang10r, _ang10i, _conj11r, _conj11i, &_mul12r, &_mul12i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    for (int k = 5; k < 26; k++) {
        double _cf13r = 0, _cf13i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
        double _mul14r = 0, _mul14i = 0;
        c_mul(_cf13r, _cf13i, x1r, x1i, &_mul14r, &_mul14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(_mul14r, _mul14i); _abs15i = 0;
        double _cf16r = 0, _cf16i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
        double _mul17r = 0, _mul17i = 0;
        c_mul(_cf16r, _cf16i, x2r, x2i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(_mul17r, _mul17i); _ang18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs15r + _ang18r; _add19i = _abs15i + _ang18i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_794_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_abs1r, _abs1i, k, &_pow2r, &_pow2i);
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_ang3r, _ang3i, k, &_pow4r, &_pow4i);
        double _add5r = 0, _add5i = 0;
        _add5r = _pow2r + _pow4r; _add5i = _pow2i + _pow4i;
        double _c6r = 0, _c6i = 0;
        _c6r = 0.0; _c6i = 1.0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_c6r, _c6i, k, &_pow7r, &_pow7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(x1r, x1i, _pow7r, _pow7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _add5r + _mul8r; _add9i = _add5i + _mul8i;
        double _conj10r = 0, _conj10i = 0;
        _conj10r = x2r; _conj10i = -(x2i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = k + _c11r; _add12i = 0 + _c11i;
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_add12r, _add12i, _c13r, _c13i, &_div14r, &_div14i);
        double _pow15r = 0, _pow15i = 0;
        c_powr(_conj10r, _conj10i, _div14r, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _add9r + _pow15r; _add16i = _add9i + _pow15i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add16r; cIm[_idx] = _add16i; } }
    }
    double _mul17r = 0, _mul17i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul17r, &_mul17i);
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(_mul17r, _mul17i); _abs18i = 0;
    double _log19r = 0, _log19i = 0;
    c_log(_abs18r, _abs18i, &_log19r, &_log19i);
    cRe[9] += _log19r; cIm[9] += _log19i;
    double _np20r = 0, _np20i = 0;
    /* WARNING: unhandled np.isinf */
    double _np21r = 0, _np21i = 0;
    /* WARNING: unhandled np.isnan */
    if ((!((_np20r != 0 || _np20i != 0))) && (!((_np21r != 0 || _np21i != 0)))) {
        double _add22r = 0, _add22i = 0;
        _add22r = x1r + x2r; _add22i = x1i + x2i;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _add24r = 0, _add24i = 0;
        _add24r = _add22r + _c23r; _add24i = _add22i + _c23i;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_add24r, _add24i); _abs25i = 0;
    }
    double _c26r = 0, _c26i = 0;
    _c26r = 0.0; _c26i = 1.0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c26r, _c26i, x2r, x2i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = x1r + _mul27r; _add28i = x1i + _mul27i;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_add28r, _add28i, _add28r, _add28i, &_pow30r, &_pow30i);
    cRe[19] += _pow30r; cIm[19] += _pow30i;
    double _np31r = 0, _np31i = 0;
    /* WARNING: unhandled np.isinf */
    double _np32r = 0, _np32i = 0;
    /* WARNING: unhandled np.isnan */
    if ((!((_np31r != 0 || _np31i != 0))) && (!((_np32r != 0 || _np32i != 0)))) {
        double _add33r = 0, _add33i = 0;
        _add33r = x1r + x2r; _add33i = x1i + x2i;
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _add35r = 0, _add35i = 0;
        _add35r = _add33r + _c34r; _add35i = _add33i + _c34i;
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(_add35r, _add35i); _abs36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 2.0; _c37i = 0;
        double _pow38r = 0, _pow38i = 0;
        c_mul(_abs36r, _abs36i, _abs36r, _abs36i, &_pow38r, &_pow38i);
    }
    double _cf39r = 0, _cf39i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
    double _c40r = 0, _c40i = 0;
    _c40r = 2.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(_cf39r, _cf39i, _cf39r, _cf39i, &_pow41r, &_pow41i);
    double _cf42r = 0, _cf42i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(_cf42r, _cf42i, _cf42r, _cf42i, &_pow44r, &_pow44i);
    double _add45r = 0, _add45i = 0;
    _add45r = _pow41r + _pow44r; _add45i = _pow41i + _pow44i;
    double _cf46r = 0, _cf46i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 2.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(_cf46r, _cf46i, _cf46r, _cf46i, &_pow48r, &_pow48i);
    double _add49r = 0, _add49i = 0;
    _add49r = _add45r + _pow48r; _add49i = _add45i + _pow48i;
    double _c50r = 0, _c50i = 0;
    _c50r = 0.5; _c50i = 0;
    double _pow51r = 0, _pow51i = 0;
    c_powr(_add49r, _add49i, 0.5, &_pow51r, &_pow51i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow51r; cIm[_idx] = _pow51i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_795_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 25; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_mul(i, 0, i, 0, &_pow2r, &_pow2i);
        double _add3r = 0, _add3i = 0;
        _add3r = _pow2r + x1r; _add3i = _pow2i + x1i;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0.0;
        double _add6r = 0, _add6i = 0;
        _add6r = _c4r + _c5r; _add6i = _c4i + _c5i;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_add6r, _add6i, i, 0, &_mul7r, &_mul7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_mul7r, _mul7i, x2r, x2i, &_mul8r, &_mul8i);
        double _exp9r = 0, _exp9i = 0;
        c_exp2(_mul8r, _mul8i, &_exp9r, &_exp9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_add3r, _add3i, _exp9r, _exp9i, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul12r, &_mul12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_mul12r, _mul12i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _c11r + _abs13r; _add14i = _c11i + _abs13i;
        double _div15r = 0, _div15i = 0;
        c_div(_mul10r, _mul10i, _add14r, _add14i, &_div15r, &_div15i);
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
    }
    for (int _si = 0; _si < 11; _si++) {
        int _si_idx = _si + 4;
        double _unk16r = 0, _unk16i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=4), upper=Constant(value=15)) */
        double _cf17r = 0, _cf17i = 0;
        { int _idx = (int)(_unk16r); if (_idx >= 0 && _idx < 25) { _cf17r = cRe[_idx]; _cf17i = cIm[_idx]; } }
        double _re18r = 0, _re18i = 0;
        _re18r = _cf17r; _re18i = 0;
        double _unk19r = 0, _unk19i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=4), upper=Constant(value=15)) */
        double _cf20r = 0, _cf20i = 0;
        { int _idx = (int)(_unk19r); if (_idx >= 0 && _idx < 25) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
        double _im21r = 0, _im21i = 0;
        _im21r = _cf20i; _im21i = 0;
        double _cos22r = 0, _cos22i = 0;
        c_cos(_im21r, _im21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_re18r, _re18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        cRe[_si_idx] = _mul23r; cIm[_si_idx] = _mul23i;
    }
    for (int _si = 0; _si < 9; _si++) {
        int _si_idx = _si + 16;
        double _unk24r = 0, _unk24i = 0;
        /* WARNING: unhandled node Slice(lower=Constant(value=16), upper=Constant(value=25)) */
        double _cf25r = 0, _cf25i = 0;
        { int _idx = (int)(_unk24r); if (_idx >= 0 && _idx < 25) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_cf25r, _cf25i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _add28r = 0, _add28i = 0;
        _add28r = x1r + x2r; _add28i = x1i + x2i;
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(_add28r, _add28i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c27r, _c27i, _ang29r, _ang29i, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_abs26r, _abs26i, _exp31r, _exp31i, &_mul32r, &_mul32i);
        cRe[_si_idx] = _mul32r; cIm[_si_idx] = _mul32i;
    }
    double _c33r = 0, _c33i = 0;
    _c33r = 0.0; _c33i = 1.0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c33r, _c33i, x2r, x2i, &_mul34r, &_mul34i);
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + _mul34r; _add35i = x1i + _mul34i;
    double _c36r = 0, _c36i = 0;
    _c36r = 3.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(_add35r, _add35i, _add35r, _add35i, &_pow37r, &_pow37i);
    c_mul(_pow37r, _pow37i, _add35r, _add35i, &_pow37r, &_pow37i);
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _mul39r = 0, _mul39i = 0;
    c_mul(_c38r, _c38i, x2r, x2i, &_mul39r, &_mul39i);
    double _add40r = 0, _add40i = 0;
    _add40r = x1r + _mul39r; _add40i = x1i + _mul39i;
    double _conj41r = 0, _conj41i = 0;
    _conj41r = _add40r; _conj41i = -(_add40i);
    double _c42r = 0, _c42i = 0;
    _c42r = 3.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_mul(_conj41r, _conj41i, _conj41r, _conj41i, &_pow43r, &_pow43i);
    c_mul(_pow43r, _pow43i, _conj41r, _conj41i, &_pow43r, &_pow43i);
    double _sub44r = 0, _sub44i = 0;
    _sub44r = _pow37r - _pow43r; _sub44i = _pow37i - _pow43i;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub44r; cIm[_idx] = _sub44i; } }
    double _abs45r = 0, _abs45i = 0;
    _abs45r = c_abs(x1r, x1i); _abs45i = 0;
    double _c46r = 0, _c46i = 0;
    _c46r = 3.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(_abs45r, _abs45i, _abs45r, _abs45i, &_pow47r, &_pow47i);
    c_mul(_pow47r, _pow47i, _abs45r, _abs45i, &_pow47r, &_pow47i);
    double _abs48r = 0, _abs48i = 0;
    _abs48r = c_abs(x2r, x2i); _abs48i = 0;
    double _c49r = 0, _c49i = 0;
    _c49r = 3.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(_abs48r, _abs48i, _abs48r, _abs48i, &_pow50r, &_pow50i);
    c_mul(_pow50r, _pow50i, _abs48r, _abs48i, &_pow50r, &_pow50i);
    double _add51r = 0, _add51i = 0;
    _add51r = _pow47r + _pow50r; _add51i = _pow47i + _pow50i;
    double _c52r = 0, _c52i = 0;
    _c52r = 0.0; _c52i = 1.0;
    double _sub53r = 0, _sub53i = 0;
    _sub53r = x2r - x1r; _sub53i = x2i - x1i;
    double _ang54r = 0, _ang54i = 0;
    _ang54r = c_arg(_sub53r, _sub53i); _ang54i = 0;
    double _mul55r = 0, _mul55i = 0;
    c_mul(_c52r, _c52i, _ang54r, _ang54i, &_mul55r, &_mul55i);
    double _exp56r = 0, _exp56i = 0;
    c_exp2(_mul55r, _mul55i, &_exp56r, &_exp56i);
    double _mul57r = 0, _mul57i = 0;
    c_mul(_add51r, _add51i, _exp56r, _exp56i, &_mul57r, &_mul57i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    double _np58r = 0, _np58i = 0;
    /* WARNING: unhandled np.where */
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _np58r; cIm[_idx] = _np58i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_796_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _ang2r = 0, _ang2i = 0;
        _ang2r = c_arg(x2r, x2i); _ang2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(_ang1r, _ang1i, _ang2r, _ang2i, &_mul3r, &_mul3i);
        double phase = _mul3r; /* +_mul3ii */
        double _add4r = 0, _add4i = 0;
        _add4r = x1r + x2r; _add4i = x1i + x2i;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(_add4r, _add4i); _abs5i = 0;
        double modulus = _abs5r; /* +_abs5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = k + _c6r; _add7i = 0 + _c6i;
        double _pow8r = 0, _pow8i = 0;
        c_powr(modulus, 0, _add7r, &_pow8r, &_pow8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 1.0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_c9r, _c9i, phase, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = k + _c11r; _add12i = 0 + _c11i;
        double _div13r = 0, _div13i = 0;
        c_div(_mul10r, _mul10i, _add12r, _add12i, &_div13r, &_div13i);
        double _exp14r = 0, _exp14i = 0;
        c_exp2(_div13r, _div13i, &_exp14r, &_exp14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_pow8r, _pow8i, _exp14r, _exp14i, &_mul15r, &_mul15i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    }
    double _c16r = 0, _c16i = 0;
    _c16r = 5.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_powr(x1r, x1i, 5.0, &_pow17r, &_pow17i);
    double _c18r = 0, _c18i = 0;
    _c18r = 5.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_powr(x2r, x2i, 5.0, &_pow19r, &_pow19i);
    double _add20r = 0, _add20i = 0;
    _add20r = _pow17r + _pow19r; _add20i = _pow17i + _pow19i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    double _re21r = 0, _re21i = 0;
    _re21r = x1r; _re21i = 0;
    double _im22r = 0, _im22i = 0;
    _im22r = x2i; _im22i = 0;
    double _add23r = 0, _add23i = 0;
    _add23r = _re21r + _im22r; _add23i = _re21i + _im22i;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    double _mul24r = 0, _mul24i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul24r, &_mul24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 1.0; _c25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 0.0; _c26i = 1.0;
    double _add27r = 0, _add27i = 0;
    _add27r = _c25r + _c26r; _add27i = _c25i + _c26i;
    double _mul28r = 0, _mul28i = 0;
    c_mul(_mul24r, _mul24i, _add27r, _add27i, &_mul28r, &_mul28i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    double _add29r = 0, _add29i = 0;
    _add29r = x1r + x2r; _add29i = x1i + x2i;
    double _c30r = 0, _c30i = 0;
    _c30r = 0.0; _c30i = 1.0;
    double _add31r = 0, _add31i = 0;
    _add31r = _add29r + _c30r; _add31i = _add29i + _c30i;
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_add31r, _add31i); _abs32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 3.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_abs32r, _abs32i, _abs32r, _abs32i, &_pow34r, &_pow34i);
    c_mul(_pow34r, _pow34i, _abs32r, _abs32i, &_pow34r, &_pow34i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow34r; cIm[_idx] = _pow34i; } }
    double _conj35r = 0, _conj35i = 0;
    _conj35r = x1r; _conj35i = -(x1i);
    double _re36r = 0, _re36i = 0;
    _re36r = x2r; _re36i = 0;
    double _add37r = 0, _add37i = 0;
    _add37r = _conj35r + _re36r; _add37i = _conj35i + _re36i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    double _abs38r = 0, _abs38i = 0;
    _abs38r = c_abs(x1r, x1i); _abs38i = 0;
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(_abs38r, _abs38i, _abs38r, _abs38i, &_pow40r, &_pow40i);
    double _abs41r = 0, _abs41i = 0;
    _abs41r = c_abs(x2r, x2i); _abs41i = 0;
    double _c42r = 0, _c42i = 0;
    _c42r = 2.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_mul(_abs41r, _abs41i, _abs41r, _abs41i, &_pow43r, &_pow43i);
    double _add44r = 0, _add44i = 0;
    _add44r = _pow40r + _pow43r; _add44i = _pow40i + _pow43i;
    double _c45r = 0, _c45i = 0;
    _c45r = 0.5; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_powr(_add44r, _add44i, 0.5, &_pow46r, &_pow46i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow46r; cIm[_idx] = _pow46i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_797_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = _c1r + _mul2r; _add3i = _c1i + _mul2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + x2r; _add4i = x1i + x2i;
    double _neg5r = 0, _neg5i = 0;
    _neg5r = -(_add4r); _neg5i = -(_add4i);
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 1.0;
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _neg5r - _c6r; _sub7i = _neg5i - _c6i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub7r; cIm[_idx] = _sub7i; } }
    for (int k = 2; k < 25; k++) {
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _cf9r = 0, _cf9i = 0;
        { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf9r = cRe[_idx]; _cf9i = cIm[_idx]; } }
        double _mul10r = 0, _mul10i = 0;
        c_mul(_cf8r, _cf8i, _cf9r, _cf9i, &_mul10r, &_mul10i);
        double _cf11r = 0, _cf11i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _cf11r, _cf11i, &_div12r, &_div12i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div12r; cIm[_idx] = _div12i; } }
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = k + _c13r; _add14i = 0 + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powr(x2r, x2i, _add14r, &_pow15r, &_pow15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = k + _c17r; _add18i = 0 + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powr(x1r, x1i, _add18r, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c16r, _c16i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _pow15r + _mul20r; _add21i = _pow15i + _mul20i;
        cRe[k] += _add21r; cIm[k] += _add21i;
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _cf23r = 0, _cf23i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(_cf23r, _cf23i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c22r, _c22i, _ang24r, _ang24i, &_mul25r, &_mul25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
        { double _tr = cRe[k]*_exp26r - cIm[k]*_exp26i; cIm[k] = cRe[k]*_exp26i + cIm[k]*_exp26r; cRe[k] = _tr; }
        double _cf27r = 0, _cf27i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf27r = cRe[_idx]; _cf27i = cIm[_idx]; } }
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_cf27r, _cf27i); _abs28i = 0;
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_798_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _abs1r = 0, _abs1i = 0;
    _abs1r = c_abs(x1r, x1i); _abs1i = 0;
    double _ang2r = 0, _ang2i = 0;
    _ang2r = c_arg(x2r, x2i); _ang2i = 0;
    double _sin3r = 0, _sin3i = 0;
    c_sin(_ang2r, _ang2i, &_sin3r, &_sin3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_abs1r, _abs1i, _sin3r, _sin3i, &_mul4r, &_mul4i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(x2r, x2i); _abs5i = 0;
    double _ang6r = 0, _ang6i = 0;
    _ang6r = c_arg(x1r, x1i); _ang6i = 0;
    double _cos7r = 0, _cos7i = 0;
    c_cos(_ang6r, _ang6i, &_cos7r, &_cos7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_abs5r, _abs5i, _cos7r, _cos7i, &_mul8r, &_mul8i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul8r; cIm[_idx] = _mul8i; } }
    for (int k = 3; k < 26; k++) {
        double _cf9r = 0, _cf9i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf9r = cRe[_idx]; _cf9i = cIm[_idx]; } }
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_cf9r, _cf9i); _abs10i = 0;
        double _cf11r = 0, _cf11i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(_cf11r, _cf11i); _ang12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(_ang12r, _ang12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_abs10r, _abs10i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul14r + x1r; _add15i = _mul14i + x1i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add15r; cIm[_idx] = _add15i; } }
        double _cf16r = 0, _cf16i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_cf16r, _cf16i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 10000.0; _c18i = 0;
        if (_abs17r > _c18r) {
            double _cf19r = 0, _cf19i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
            double _abs20r = 0, _abs20i = 0;
            _abs20r = c_abs(_cf19r, _cf19i); _abs20i = 0;
        }
    }
    double _cf21r = 0, _cf21i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cf21r; cIm[_idx] = _cf21i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_799_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 25; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
        double _re3r = 0, _re3i = 0;
        _re3r = _mul2r; _re3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_c1r, _c1i, _re3r, _re3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 3.0; _c5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
        double _im7r = 0, _im7i = 0;
        _im7r = _mul6i; _im7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c5r, _c5i, _im7r, _im7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul4r + _mul8r; _add9i = _mul4i + _mul8i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = i + _c11r; _add12i = 0 + _c11i;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs10r, _abs10i, _add12r, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_add9r, _add9i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(_mul16r, _mul16i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c15r, _c15i, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x2r, x2i); _abs20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c19r, _c19i, _abs20r, _abs20i, &_mul21r, &_mul21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _mul18r - _mul21r; _sub22i = _mul18i - _mul21i;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = i + _c24r; _add25i = 0 + _c24i;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_abs23r, _abs23i, _add25r, &_pow26r, &_pow26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_sub22r, _sub22i, _pow26r, _pow26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul14r + _mul27r; _add28i = _mul14i + _mul27i;
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add28r; cIm[_idx] = _add28i; } }
        double _np29r = 0, _np29i = 0;
        /* WARNING: unhandled np.isfinite */
        if (!((_np29r != 0 || _np29i != 0))) {
            double _c30r = 0, _c30i = 0;
            _c30r = 0.0; _c30i = 0;
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c30r; cIm[_idx] = _c30i; } }
        }
    }
    double _mul31r = 0, _mul31i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul31r, &_mul31i);
    double _conj32r = 0, _conj32i = 0;
    _conj32r = _mul31r; _conj32i = -(_mul31i);
    cRe[0] += _conj32r; cIm[0] += _conj32i;
    double _np33r = 0, _np33i = 0;
    /* WARNING: unhandled np.isfinite */
    if (!((_np33r != 0 || _np33i != 0))) {
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 0;
        { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c34r; cIm[_idx] = _c34i; } }
    }
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _abs36r = 0, _abs36i = 0;
    _abs36r = c_abs(_add35r, _add35i); _abs36i = 0;
    double _log37r = 0, _log37i = 0;
    c_log(_abs36r, _abs36i, &_log37r, &_log37i);
    { double _tr = cRe[12]*_log37r - cIm[12]*_log37i; cIm[12] = cRe[12]*_log37i + cIm[12]*_log37r; cRe[12] = _tr; }
    double _np38r = 0, _np38i = 0;
    /* WARNING: unhandled np.isfinite */
    if (!((_np38r != 0 || _np38i != 0))) {
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 0;
        { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c39r; cIm[_idx] = _c39i; } }
    }
    double _cf40r = 0, _cf40i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
    double _mul41r = 0, _mul41i = 0;
    c_mul(_cf40r, _cf40i, x1r, x1i, &_mul41r, &_mul41i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_mul41r, _mul41i, x2r, x2i, &_mul42r, &_mul42i);
    double _add43r = 0, _add43i = 0;
    _add43r = x1r + x2r; _add43i = x1i + x2i;
    double _div44r = 0, _div44i = 0;
    c_div(_mul42r, _mul42i, _add43r, _add43i, &_div44r, &_div44i);
    cRe[24] -= _div44r; cIm[24] -= _div44i;
    double _np45r = 0, _np45i = 0;
    /* WARNING: unhandled np.isfinite */
    if (!((_np45r != 0 || _np45i != 0))) {
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 0;
        { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c46r; cIm[_idx] = _c46i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_800_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    for (int _si = 0; _si < 9; _si++) {
        int _si_idx = _si + 1;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 1.0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_c3r, _c3i, x2r, x2i, &_mul4r, &_mul4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(_mul4r, _mul4i); _ang5i = 0;
        double _arange6r = 0, _arange6i = 0;
        _arange6r = (double)(_si + 1); _arange6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 1.0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c7r, _c7i, x2r, x2i, &_mul8r, &_mul8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(_mul8r, _mul8i); _ang9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_ang9r, _ang9i, 0, &_pow10r, &_pow10i);
        double _smul11r = 0, _smul11i = 0;
        c_mul(_abs2r, _abs2i, _pow10r, _pow10i, &_smul11r, &_smul11i);
        cRe[_si_idx] = _smul11r; cIm[_si_idx] = _smul11i;
    }
    double _re12r = 0, _re12i = 0;
    _re12r = x1r; _re12i = 0;
    double _im13r = 0, _im13i = 0;
    _im13r = x2i; _im13i = 0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_re12r, _re12i, _im13r, _im13i, &_mul14r, &_mul14i);
    double _re15r = 0, _re15i = 0;
    _re15r = x2r; _re15i = 0;
    double _im16r = 0, _im16i = 0;
    _im16r = x1i; _im16i = 0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_re15r, _re15i, _im16r, _im16i, &_mul17r, &_mul17i);
    double _add18r = 0, _add18i = 0;
    _add18r = _mul14r + _mul17r; _add18i = _mul14i + _mul17i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add18r; cIm[_idx] = _add18i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 15;
        double _sub19r = 0, _sub19i = 0;
        _sub19r = x1r - x2r; _sub19i = x1i - x2i;
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _add21r = 0, _add21i = 0;
        _add21r = _sub19r + _c20r; _add21i = _sub19i + _c20i;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_add21r, _add21i); _abs22i = 0;
        double _arange23r = 0, _arange23i = 0;
        _arange23r = (double)(_si + 1); _arange23i = 0;
        double _sub24r = 0, _sub24i = 0;
        _sub24r = x1r - x2r; _sub24i = x1i - x2i;
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _add26r = 0, _add26i = 0;
        _add26r = _sub24r + _c25r; _add26i = _sub24i + _c25i;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_add26r, _add26i); _abs27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_powr(_abs27r, _abs27i, 0, &_pow28r, &_pow28i);
        cRe[_si_idx] = _pow28r; cIm[_si_idx] = _pow28i;
    }
    double _im29r = 0, _im29i = 0;
    _im29r = x1i; _im29i = 0;
    double _re30r = 0, _re30i = 0;
    _re30r = x2r; _re30i = 0;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_im29r, _im29i, _re30r, _re30i, &_mul31r, &_mul31i);
    double _re32r = 0, _re32i = 0;
    _re32r = x1r; _re32i = 0;
    double _im33r = 0, _im33i = 0;
    _im33r = x2i; _im33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_re32r, _re32i, _im33r, _im33i, &_mul34r, &_mul34i);
    double _add35r = 0, _add35i = 0;
    _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    for (int _si = 0; _si < 3; _si++) {
        int _si_idx = _si + 21;
        double _cf36r = 0, _cf36i = 0;
        { int _idx = 14; if (_idx >= 0 && _idx < 25) { _cf36r = cRe[_idx]; _cf36i = cIm[_idx]; } }
        double _mul37r = 0, _mul37i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cf36r + _mul37r; _add38i = _cf36i + _mul37i;
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(_add38r, _add38i); _abs39i = 0;
        double _arange40r = 0, _arange40i = 0;
        _arange40r = (double)(_si + 1); _arange40i = 0;
        double _cf41r = 0, _cf41i = 0;
        { int _idx = 14; if (_idx >= 0 && _idx < 25) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
        double _mul42r = 0, _mul42i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cf41r + _mul42r; _add43i = _cf41i + _mul42i;
        double _abs44r = 0, _abs44i = 0;
        _abs44r = c_abs(_add43r, _add43i); _abs44i = 0;
        double _pow45r = 0, _pow45i = 0;
        c_powr(_abs44r, _abs44i, 0, &_pow45r, &_pow45i);
        cRe[_si_idx] = _pow45r; cIm[_si_idx] = _pow45i;
    }
    double _cf46r = 0, _cf46i = 0;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
    double _abs47r = 0, _abs47i = 0;
    _abs47r = c_abs(_cf46r, _cf46i); _abs47i = 0;
    double _add48r = 0, _add48i = 0;
    _add48r = x1r + x2r; _add48i = x1i + x2i;
    double _c49r = 0, _c49i = 0;
    _c49r = 0.0; _c49i = 0.5;
    double _add50r = 0, _add50i = 0;
    _add50r = _add48r + _c49r; _add50i = _add48i + _c49i;
    double _abs51r = 0, _abs51i = 0;
    _abs51r = c_abs(_add50r, _add50i); _abs51i = 0;
    double _log52r = 0, _log52i = 0;
    c_log(_abs51r, _abs51i, &_log52r, &_log52i);
    double _add53r = 0, _add53i = 0;
    _add53r = _abs47r + _log52r; _add53i = _abs47i + _log52i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add53r; cIm[_idx] = _add53i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
