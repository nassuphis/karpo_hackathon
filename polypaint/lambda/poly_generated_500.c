/* AUTO-GENERATED from poly500.py — do not edit manually */
/* 102 coefficient functions */

static void poly_401_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double rec1 = _re1r; /* +_re1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double rec2 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double imc1 = _im3r; /* +_im3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc2 = _im4r; /* +_im4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(j, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _pow10r = 0, _pow10i = 0;
        c_powr(_abs7r, _abs7i, _div9r, &_pow10r, &_pow10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(j, 0); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_pow10r, _pow10i, _log14r, _log14i, &_mul15r, &_mul15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _sqrt17r = 0, _sqrt17i = 0;
        c_powr(j, 0, 0.5, &_sqrt17r, &_sqrt17i);
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs16r, _abs16i, _sqrt17r, &_pow18r, &_pow18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(j, 0, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_pow18r, _pow18i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul15r + _mul20r; _add21i = _mul15i + _mul20i;
        double magnitude = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _cos23r = 0, _cos23i = 0;
        c_cos(j, 0, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul24r + _mul29r; _add30i = _mul24i + _mul29i;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(j, 0); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _abs31r + _c32r; _add33i = _abs31i + _c32i;
        double _log34r = 0, _log34i = 0;
        c_log(_add33r, _add33i, &_log34r, &_log34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add30r + _log34r; _add35i = _add30i + _log34i;
        double angle = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, angle, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = n + _c40r; _add41i = 0 + _c40i;
    for (int k = 1; k < (int)(_add41r); k++) {
        double _c42r = 0, _c42i = 0;
        _c42r = 0.5; _c42i = 0;
        double _re43r = 0, _re43i = 0;
        _re43r = x1r; _re43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c42r, _c42i, _re43r, _re43i, &_mul44r, &_mul44i);
        double _sin45r = 0, _sin45i = 0;
        c_sin(k, 0, &_sin45r, &_sin45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_mul44r, _mul44i, _sin45r, _sin45i, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.3; _c47i = 0;
        double _im48r = 0, _im48i = 0;
        _im48r = x2i; _im48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c47r, _c47i, _im48r, _im48i, &_mul49r, &_mul49i);
        double _cos50r = 0, _cos50i = 0;
        c_cos(k, 0, &_cos50r, &_cos50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_mul49r, _mul49i, _cos50r, _cos50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul46r + _mul51r; _add52i = _mul46i + _mul51i;
        cRe[(k - 1)] += _add52r; cIm[(k - 1)] += _add52i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_402_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double temp_mag = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double temp_angle = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        for (int k = 1; k < (int)(_add6r); k++) {
            double _abs7r = 0, _abs7i = 0;
            _abs7r = c_abs(x1r, x1i); _abs7i = 0;
            double _mul8r = 0, _mul8i = 0;
            c_mul(_abs7r, _abs7i, k, 0, &_mul8r, &_mul8i);
            double _c9r = 0, _c9i = 0;
            _c9r = 1.0; _c9i = 0;
            double _add10r = 0, _add10i = 0;
            _add10r = _mul8r + _c9r; _add10i = _mul8i + _c9i;
            double _log11r = 0, _log11i = 0;
            c_log(_add10r, _add10i, &_log11r, &_log11i);
            double _re12r = 0, _re12i = 0;
            _re12r = x2r; _re12i = 0;
            double _mul13r = 0, _mul13i = 0;
            c_mul(k, 0, _re12r, _re12i, &_mul13r, &_mul13i);
            double _sin14r = 0, _sin14i = 0;
            c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
            double _mul15r = 0, _mul15i = 0;
            c_mul(_log11r, _log11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
            temp_mag += _mul15r;
            double _im16r = 0, _im16i = 0;
            _im16r = x1i; _im16i = 0;
            double _mul17r = 0, _mul17i = 0;
            c_mul(k, 0, _im16r, _im16i, &_mul17r, &_mul17i);
            double _cos18r = 0, _cos18i = 0;
            c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
            double _add19r = 0, _add19i = 0;
            _add19r = x2r + k; _add19i = x2i + 0;
            double _ang20r = 0, _ang20i = 0;
            _ang20r = c_arg(_add19r, _add19i); _ang20i = 0;
            double _mul21r = 0, _mul21i = 0;
            c_mul(_cos18r, _cos18i, _ang20r, _ang20i, &_mul21r, &_mul21i);
            temp_angle += _mul21r;
        }
        double _cos22r = 0, _cos22i = 0;
        c_cos(temp_angle, 0, &_cos22r, &_cos22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(temp_angle, 0, &_sin23r, &_sin23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin23r, _sin23i, _c24r, _c24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _cos22r + _mul25r; _add26i = _cos22i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(temp_mag, 0, _add26r, _add26i, &_mul27r, &_mul27i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    double _c28r = 0, _c28i = 0;
    _c28r = 1.0; _c28i = 0;
    double _add29r = 0, _add29i = 0;
    _add29r = n + _c28r; _add29i = 0 + _c28i;
    for (int r = 1; r < (int)(_add29r); r++) {
        double _cf30r = 0, _cf30i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x2r, x2i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(r, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _pow34r = 0, _pow34i = 0;
        c_powr(_abs31r, _abs31i, _div33r, &_pow34r, &_pow34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_cf30r, _cf30i, _pow34r, _pow34i, &_mul35r, &_mul35i);
        double _conj36r = 0, _conj36i = 0;
        _conj36r = x1r; _conj36i = -(x1i);
        double _pow37r = 0, _pow37i = 0;
        c_powr(_conj36r, _conj36i, r, &_pow37r, &_pow37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul35r + _pow37r; _add38i = _mul35i + _pow37i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_403_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 5.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin10r, _sin10i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = x1r + x2r; _add16i = x1i + x2i;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_add16r, _add16i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul15r + _log20r; _add21i = _mul15i + _log20i;
        double phase = _add21r; /* +_add21ii */
        double _re22r = 0, _re22i = 0;
        _re22r = x1r; _re22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_re22r, _re22i, j, &_pow23r, &_pow23i);
        double _im24r = 0, _im24i = 0;
        _im24r = x2i; _im24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_im24r, _im24i, j, &_pow25r, &_pow25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _pow23r + _pow25r; _add26i = _pow23i + _pow25i;
        double _sin27r = 0, _sin27i = 0;
        c_sin(j, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_add26r, _add26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 3.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul28r + _cos32r; _add33i = _mul28i + _cos32i;
        double magnitude = _add33r; /* +_add33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, phase, 0, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(magnitude, 0, _exp36r, _exp36i, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, phase, 0, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
        double _conj42r = 0, _conj42i = 0;
        _conj42r = _mul41r; _conj42i = -(_mul41i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul37r + _conj42r; _add43i = _mul37i + _conj42i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_404_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_re1r, _re1i, _log4r, _log4i, &_mul5r, &_mul5i);
        double mag_part1 = _mul5r; /* +_mul5ii */
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _sin7r = 0, _sin7i = 0;
        c_sin(j, 0, &_sin7r, &_sin7i);
        double _pow8r = 0, _pow8i = 0;
        c_powr(_abs6r, _abs6i, _sin7r, &_pow8r, &_pow8i);
        double mag_part2 = _pow8r; /* +_pow8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 35.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(j, 0, _c10r, _c10i, &_div11r, &_div11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _c9r + _div11r; _add12i = _c9i + _div11i;
        double mag_part3 = _add12r; /* +_add12ii */
        double _mul13r = 0, _mul13i = 0;
        c_mul(mag_part2, 0, mag_part3, 0, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = mag_part1 + _mul13r; _add14i = 0 + _mul13i;
        double magnitude = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang15r, _ang15i, j, 0, &_mul16r, &_mul16i);
        double ang_part1 = _mul16r; /* +_mul16ii */
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 7.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double ang_part2 = _cos20r; /* +_cos20ii */
        double _add21r = 0, _add21i = 0;
        _add21r = ang_part1 + ang_part2; _add21i = 0 + 0;
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
        c_mul(magnitude, 0, _add26r, _add26i, &_mul27r, &_mul27i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_405_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x1r, x1i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(k, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _sin5r + _cos8r; _add9i = _sin5i + _cos8i;
        double phase = _add9r; /* +_add9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs10r + k; _add11i = _abs10i + 0;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _neg13r = 0, _neg13i = 0;
        _neg13r = -(k); _neg13i = -(0);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _div17r = 0, _div17i = 0;
        c_div(_neg13r, _neg13i, _add16r, _add16i, &_div17r, &_div17i);
        double _exp18r = 0, _exp18i = 0;
        c_exp2(_div17r, _div17i, &_exp18r, &_exp18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log12r, _log12i, _exp18r, _exp18i, &_mul19r, &_mul19i);
        double _sqrt20r = 0, _sqrt20i = 0;
        c_powr(k, 0, 0.5, &_sqrt20r, &_sqrt20i);
        double _sub21r = 0, _sub21i = 0;
        _sub21r = x1r - x2r; _sub21i = x1i - x2i;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_sub21r, _sub21i); _abs22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sqrt20r, _sqrt20i, _abs22r, _abs22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul19r + _mul23r; _add24i = _mul19i + _mul23i;
        double magnitude = _add24r; /* +_add24ii */
        double _cos25r = 0, _cos25i = 0;
        c_cos(phase, 0, &_cos25r, &_cos25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(phase, 0, &_sin26r, &_sin26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin26r, _sin26i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos25r + _mul28r; _add29i = _cos25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(magnitude, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        double _conj31r = 0, _conj31i = 0;
        _conj31r = x1r; _conj31i = -(x1i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(k, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_conj31r, _conj31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(k, 0, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_mul33r, _mul33i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul30r + _mul35r; _add36i = _mul30i + _mul35i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_406_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int r = 1; r < (int)(_add6r); r++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(r, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 8.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _re11r = 0, _re11i = 0;
        _re11r = x1r; _re11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sin10r, _sin10i, _re11r, _re11i, &_mul12r, &_mul12i);
        double _arr13r = 0, _arr13i = 0;
        { int _idx = (r - 1); _arr13r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr13i = 0; }
        double _mul14r = 0, _mul14i = 0;
        c_mul(_mul12r, _mul12i, _arr13r, _arr13i, &_mul14r, &_mul14i);
        double term1 = _mul14r; /* +_mul14ii */
        double _mul15r = 0, _mul15i = 0;
        c_mul(r, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 6.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _im19r = 0, _im19i = 0;
        _im19r = x2i; _im19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_cos18r, _cos18i, _im19r, _im19i, &_mul20r, &_mul20i);
        double _arr21r = 0, _arr21i = 0;
        { int _idx = (r - 1); _arr21r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr21i = 0; }
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul20r, _mul20i, _arr21r, _arr21i, &_mul22r, &_mul22i);
        double term2 = _mul22r; /* +_mul22ii */
        double _arr23r = 0, _arr23i = 0;
        { int _idx = (r - 1); _arr23r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr23i = 0; }
        double _arr24r = 0, _arr24i = 0;
        { int _idx = (r - 1); _arr24r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr24i = 0; }
        double _add25r = 0, _add25i = 0;
        _add25r = _arr23r + _arr24r; _add25i = _arr23i + _arr24i;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_add25r, _add25i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double term3 = _log29r; /* +_log29ii */
        double _add30r = 0, _add30i = 0;
        _add30r = term1 + term2; _add30i = 0 + 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _add30r + term3; _add31i = _add30i + 0;
        double mag = _add31r; /* +_add31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(r, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 5.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang32r, _ang32i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(r, 0, M_PI, 0, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 7.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_mul39r, _mul39i, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang38r, _ang38i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul37r + _mul43r; _add44i = _mul37i + _mul43i;
        double _arr45r = 0, _arr45i = 0;
        { int _idx = (r - 1); _arr45r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr45i = 0; }
        double _c46r = 0, _c46i = 0;
        _c46r = 3.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(_arr45r, _arr45i, _c46r, _c46i, &_div47r, &_div47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _add44r + _div47r; _add48i = _add44i + _div47i;
        double angle = _add48r; /* +_add48ii */
        double _cos49r = 0, _cos49i = 0;
        c_cos(angle, 0, &_cos49r, &_cos49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(angle, 0, &_sin50r, &_sin50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(_sin50r, _sin50i, _c51r, _c51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _cos49r + _mul52r; _add53i = _cos49i + _mul52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(mag, 0, _add53r, _add53i, &_mul54r, &_mul54i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_407: too complex for auto-transpile, stubbed */
static void poly_407_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_408_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double rec_t1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double imc_t1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec_t2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_t2 = _attr4r; /* +_attr4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_mul7r, _mul7i, rec_t1, 0, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_mul10r, _mul10i, imc_t2, 0, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _sin9r + _cos12r; _add13i = _sin9i + _cos12i;
        double angle_part = _add13r; /* +_add13ii */
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, rec_t2, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _mul14r + _c15r; _add16i = _mul14i + _c15i;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_add16r, _add16i); _abs17i = 0;
        double _log18r = 0, _log18i = 0;
        c_log(_abs17r, _abs17i, &_log18r, &_log18i);
        double _sqrt19r = 0, _sqrt19i = 0;
        c_powr(j, 0, 0.5, &_sqrt19r, &_sqrt19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log18r, _log18i, _sqrt19r, _sqrt19i, &_mul20r, &_mul20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 0.5; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs21r, _abs21i, 0.5, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _pow23r; _add24i = _mul20i + _pow23i;
        double magnitude_part = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _ang26r, _ang26i, &_mul27r, &_mul27i);
        double _div28r = 0, _div28i = 0;
        c_div(_mul27r, _mul27i, j, 0, &_div28r, &_div28i);
        double phase_shift = _div28r; /* +_div28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _add30r = 0, _add30i = 0;
        _add30r = angle_part + phase_shift; _add30i = 0 + 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c29r, _c29i, _add30r, _add30i, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(magnitude_part, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        double _conj34r = 0, _conj34i = 0;
        _conj34r = x1r; _conj34i = -(x1i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, _ang35r, _ang35i, &_mul36r, &_mul36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_conj34r, _conj34i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul33r + _mul38r; _add39i = _mul33i + _mul38i;
        double _abs40r = 0, _abs40i = 0;
        _abs40r = c_abs(x1r, x1i); _abs40i = 0;
        double _cos41r = 0, _cos41i = 0;
        c_cos(_abs40r, _abs40i, &_cos41r, &_cos41i);
        double _sub42r = 0, _sub42i = 0;
        _sub42r = _add39r - _cos41r; _sub42i = _add39i - _cos41i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub42r; cIm[_idx] = _sub42i; } }
    }
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _fdiv44r = 0, _fdiv44i = 0;
    c_div(n, 0, _c43r, _c43i, &_fdiv44r, &_fdiv44i);
    _fdiv44r = floor(_fdiv44r); _fdiv44i = 0;
    double _c45r = 0, _c45i = 0;
    _c45r = 1.0; _c45i = 0;
    double _add46r = 0, _add46i = 0;
    _add46r = _fdiv44r + _c45r; _add46i = _fdiv44i + _c45i;
    for (int k = 1; k < (int)(_add46r); k++) {
        double _c47r = 0, _c47i = 0;
        _c47r = 2.0; _c47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(k, 0, _c47r, _c47i, &_mul48r, &_mul48i);
        double idx = _mul48r; /* +_mul48ii */
        double perturbation = 0;
        if (idx <= n) {
            double _c49r = 0, _c49i = 0;
            _c49r = 0.0; _c49i = 1.0;
            double _sin50r = 0, _sin50i = 0;
            c_sin(k, 0, &_sin50r, &_sin50i);
            double _cos51r = 0, _cos51i = 0;
            c_cos(k, 0, &_cos51r, &_cos51i);
            double _add52r = 0, _add52i = 0;
            _add52r = _sin50r + _cos51r; _add52i = _sin50i + _cos51i;
            double _mul53r = 0, _mul53i = 0;
            c_mul(_c49r, _c49i, _add52r, _add52i, &_mul53r, &_mul53i);
            double _exp54r = 0, _exp54i = 0;
            c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
            perturbation = _exp54r;
            double _cf55r = 0, _cf55i = 0;
            { int _idx = ((int)(idx) - 1); if (_idx >= 0 && _idx < 36) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
            double _mul56r = 0, _mul56i = 0;
            c_mul(_cf55r, _cf55i, perturbation, 0, &_mul56r, &_mul56i);
            double _cf57r = 0, _cf57i = 0;
            { int _idx = ((int)(idx) - 1); if (_idx >= 0 && _idx < 36) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
            double _abs58r = 0, _abs58i = 0;
            _abs58r = c_abs(_cf57r, _cf57i); _abs58i = 0;
            double _c59r = 0, _c59i = 0;
            _c59r = 1.0; _c59i = 0;
            double _add60r = 0, _add60i = 0;
            _add60r = _abs58r + _c59r; _add60i = _abs58i + _c59i;
            double _log61r = 0, _log61i = 0;
            c_log(_add60r, _add60i, &_log61r, &_log61i);
            double _add62r = 0, _add62i = 0;
            _add62r = _mul56r + _log61r; _add62i = _mul56i + _log61i;
            { int _idx = ((int)(idx) - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add62r; cIm[_idx] = _add62i; } }
        }
    }
    double _c63r = 0, _c63i = 0;
    _c63r = 1.0; _c63i = 0;
    double _add64r = 0, _add64i = 0;
    _add64r = n + _c63r; _add64i = 0 + _c63i;
    for (int r = 1; r < (int)(_add64r); r++) {
        double _c65r = 0, _c65i = 0;
        _c65r = 2.0; _c65i = 0;
        double _pow66r = 0, _pow66i = 0;
        c_mul(r, 0, r, 0, &_pow66r, &_pow66i);
        double _sqrt67r = 0, _sqrt67i = 0;
        c_powr(r, 0, 0.5, &_sqrt67r, &_sqrt67i);
        double _add68r = 0, _add68i = 0;
        _add68r = _pow66r + _sqrt67r; _add68i = _pow66i + _sqrt67i;
        double _abs69r = 0, _abs69i = 0;
        _abs69r = c_abs(x1r, x1i); _abs69i = 0;
        double _abs70r = 0, _abs70i = 0;
        _abs70r = c_abs(x2r, x2i); _abs70i = 0;
        double _add71r = 0, _add71i = 0;
        _add71r = _abs69r + _abs70r; _add71i = _abs69i + _abs70i;
        double _c72r = 0, _c72i = 0;
        _c72r = 1.0; _c72i = 0;
        double _add73r = 0, _add73i = 0;
        _add73r = _add71r + _c72r; _add73i = _add71i + _c72i;
        double _div74r = 0, _div74i = 0;
        c_div(_add68r, _add68i, _add73r, _add73i, &_div74r, &_div74i);
        double scaling_factor = _div74r; /* +_div74ii */
        double _cf75r = 0, _cf75i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf75r = cRe[_idx]; _cf75i = cIm[_idx]; } }
        double _mul76r = 0, _mul76i = 0;
        c_mul(_cf75r, _cf75i, scaling_factor, 0, &_mul76r, &_mul76i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul76r; cIm[_idx] = _mul76i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_409_c(double x1r, double x1i, double x2r, double x2i,
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
        double r_part = _sub9r; /* +_sub9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(j, 0); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_im10r, _im10i, _log14r, _log14i, &_mul15r, &_mul15i);
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 7.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_im16r, _im16i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul15r + _mul21r; _add22i = _mul15i + _mul21i;
        double i_part = _add22r; /* +_add22ii */
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(r_part, 0, r_part, 0, &_pow24r, &_pow24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(i_part, 0, i_part, 0, &_pow26r, &_pow26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _pow24r + _pow26r; _add27i = _pow24i + _pow26i;
        double _sqrt28r = 0, _sqrt28i = 0;
        c_powr(_add27r, _add27i, 0.5, &_sqrt28r, &_sqrt28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 0.1; _c30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, j, 0, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _c29r + _mul31r; _add32i = _c29i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_sqrt28r, _sqrt28i, _add32r, _add32i, &_mul33r, &_mul33i);
        double magnitude = _mul33r; /* +_mul33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _ang34r + _ang35r; _add36i = _ang34i + _ang35i;
        double _sin37r = 0, _sin37i = 0;
        c_sin(j, 0, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add36r + _sin37r; _add38i = _add36i + _sin37i;
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(j, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_410_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.5; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_powr(j, 0, 1.5, &_pow3r, &_pow3i);
        double _add4r = 0, _add4i = 0;
        _add4r = _abs1r + _pow3r; _add4i = _abs1i + _pow3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double mag_part1 = _log5r; /* +_log5ii */
        double _re6r = 0, _re6i = 0;
        _re6r = x2r; _re6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _re6r, _re6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _im9r = 0, _im9i = 0;
        _im9r = x1i; _im9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _im9r, _im9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sin8r, _sin8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double mag_part2 = _mul12r; /* +_mul12ii */
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(mag_part2, 0, mag_part2, 0, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c13r + _pow15r; _add16i = _c13i + _pow15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(mag_part1, 0, _add16r, _add16i, &_mul17r, &_mul17i);
        double magnitude = _mul17r; /* +_mul17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang19r, _ang19i, j, 0, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _ang18r + _mul20r; _add21i = _ang18i + _mul20i;
        double angle_part1 = _add21r; /* +_add21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul22r, &_mul22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_mul22r, _mul22i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _c24r; _add25i = _abs23i + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double angle_part2 = _log26r; /* +_log26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = angle_part1 + angle_part2; _add27i = 0 + 0;
        double angle = _add27r; /* +_add27ii */
        double _cos28r = 0, _cos28i = 0;
        c_cos(angle, 0, &_cos28r, &_cos28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(angle, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c29r, _c29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _cos28r + _mul31r; _add32i = _cos28i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(magnitude, 0, _add32r, _add32i, &_mul33r, &_mul33i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_411_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_re1r, _re1i, j, 0, &_mul2r, &_mul2i);
        double _re3r = 0, _re3i = 0;
        _re3r = x2r; _re3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 35.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_re3r, _re3i, _sub5r, _sub5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _mul2r + _mul6r; _add7i = _mul2i + _mul6i;
        double rec = _add7r; /* +_add7ii */
        double _im8r = 0, _im8i = 0;
        _im8r = x1i; _im8i = 0;
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
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _im8r - _mul14r; _sub15i = _im8i - _mul14i;
        double imc = _sub15r; /* +_sub15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 5.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 7.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sin23r + _cos27r; _add28i = _sin23i + _cos27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log19r, _log19i, _add28r, _add28i, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 6.0; _c31i = 0;
        double _mod32r = 0, _mod32i = 0;
        _mod32r = fmod(j, _c31r); _mod32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _c30r + _mod32r; _add33i = _c30i + _mod32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_mul29r, _mul29i, _add33r, _add33i, &_mul34r, &_mul34i);
        double mag = _mul34r; /* +_mul34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x1r, x1i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 9.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang35r, _ang35i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, M_PI, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 11.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang41r, _ang41i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _mul40r + _mul46r; _add47i = _mul40i + _mul46i;
        double _mul48r = 0, _mul48i = 0;
        c_mul(j, 0, M_PI, 0, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 13.0; _c49i = 0;
        double _div50r = 0, _div50i = 0;
        c_div(_mul48r, _mul48i, _c49r, _c49i, &_div50r, &_div50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(_div50r, _div50i, &_sin51r, &_sin51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _add47r + _sin51r; _add52i = _add47i + _sin51i;
        double ang = _add52r; /* +_add52ii */
        double _cos53r = 0, _cos53i = 0;
        c_cos(ang, 0, &_cos53r, &_cos53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 1.0;
        double _sin55r = 0, _sin55i = 0;
        c_sin(ang, 0, &_sin55r, &_sin55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c54r, _c54i, _sin55r, _sin55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _cos53r + _mul56r; _add57i = _cos53i + _mul56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(mag, 0, _add57r, _add57i, &_mul58r, &_mul58i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_412_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 7.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(j, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double k = _add6r; /* +_add6ii */
        double _c7r = 0, _c7i = 0;
        _c7r = 5.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(j, 0, _c7r, _c7i, &_div8r, &_div8i);
        double _flr9r = 0, _flr9i = 0;
        _flr9r = floor(_div8r); _flr9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _flr9r + _c10r; _add11i = _flr9i + _c10i;
        double r = _add11r; /* +_add11ii */
        double _re12r = 0, _re12i = 0;
        _re12r = x1r; _re12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(k, 0, _re12r, _re12i, &_mul13r, &_mul13i);
        double _im14r = 0, _im14i = 0;
        _im14r = x2i; _im14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(r, 0, _im14r, _im14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul13r + _mul15r; _add16i = _mul13i + _mul15i;
        double _sin17r = 0, _sin17i = 0;
        c_sin(_add16r, _add16i, &_sin17r, &_sin17i);
        double _im18r = 0, _im18i = 0;
        _im18r = x1i; _im18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(k, 0, _im18r, _im18i, &_mul19r, &_mul19i);
        double _re20r = 0, _re20i = 0;
        _re20r = x2r; _re20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(r, 0, _re20r, _re20i, &_mul21r, &_mul21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _mul19r - _mul21r; _sub22i = _mul19i - _mul21i;
        double _cos23r = 0, _cos23i = 0;
        c_cos(_sub22r, _sub22i, &_cos23r, &_cos23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _sin17r + _cos23r; _add24i = _sin17i + _cos23i;
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _ang26r, _ang26i, &_mul27r, &_mul27i);
        double _div28r = 0, _div28i = 0;
        c_div(_mul27r, _mul27i, j, 0, &_div28r, &_div28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add24r + _div28r; _add29i = _add24i + _div28i;
        double angle = _add29r; /* +_add29ii */
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _abs30r + _c31r; _add32i = _abs30i + _c31i;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.5; _c34i = 0;
        double _pow35r = 0, _pow35i = 0;
        c_powr(j, 0, 0.5, &_pow35r, &_pow35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _pow35r + r; _add36i = _pow35i + 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_log33r, _log33i, _add36r, _add36i, &_mul37r, &_mul37i);
        double _abs38r = 0, _abs38i = 0;
        _abs38r = c_abs(x2r, x2i); _abs38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 1.2; _c39i = 0;
        double _pow40r = 0, _pow40i = 0;
        c_powr(r, 0, 1.2, &_pow40r, &_pow40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_abs38r, _abs38i, _pow40r, _pow40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul37r + _mul41r; _add42i = _mul37i + _mul41i;
        double magnitude = _add42r; /* +_add42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, angle, 0, &_mul44r, &_mul44i);
        double _exp45r = 0, _exp45i = 0;
        c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _exp45r, _exp45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_413_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x1r, x1i); _ang3i = 0;
        double _sin4r = 0, _sin4i = 0;
        c_sin(j, 0, &_sin4r, &_sin4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_ang3r, _ang3i, _sin4r, _sin4i, &_mul5r, &_mul5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 3.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(j, 0, _c7r, _c7i, &_div8r, &_div8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_div8r, _div8i, &_cos9r, &_cos9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_ang6r, _ang6i, _cos9r, _cos9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul5r + _mul10r; _add11i = _mul5i + _mul10i;
        double angle = _add11r; /* +_add11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs12r, _abs12i, j, &_pow13r, &_pow13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = n - j; _sub18i = 0 - 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_log17r, _log17i, _sub18r, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_pow13r, _pow13i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 5.0; _c21i = 0;
        double _mod22r = 0, _mod22i = 0;
        _mod22r = fmod(j, _c21r); _mod22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _mod22r + _c23r; _add24i = _mod22i + _c23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul20r, _mul20i, _add24r, _add24i, &_mul25r, &_mul25i);
        double magnitude = _mul25r; /* +_mul25ii */
        double _cos26r = 0, _cos26i = 0;
        c_cos(angle, 0, &_cos26r, &_cos26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(angle, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c27r, _c27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _cos26r + _mul29r; _add30i = _cos26i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(magnitude, 0, _add30r, _add30i, &_mul31r, &_mul31i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    double _c32r = 0, _c32i = 0;
    _c32r = 2.0; _c32i = 0;
    double _div33r = 0, _div33i = 0;
    c_div(n, 0, _c32r, _c32i, &_div33r, &_div33i);
    double _int34r = 0, _int34i = 0;
    _int34r = (int)(_div33r); _int34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 1.0; _c35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _int34r + _c35r; _add36i = _int34i + _c35i;
    for (int k = 1; k < (int)(_add36r); k++) {
        double _c37r = 0, _c37i = 0;
        _c37r = 2.0; _c37i = 0;
        double _pow38r = 0, _pow38i = 0;
        c_mul(k, 0, k, 0, &_pow38r, &_pow38i);
        double _sqrt39r = 0, _sqrt39i = 0;
        c_powr(k, 0, 0.5, &_sqrt39r, &_sqrt39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _pow38r + _sqrt39r; _add40i = _pow38i + _sqrt39i;
        double r = _add40r; /* +_add40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, r, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        { double _tr = cRe[(k - 1)]*_exp43r - cIm[(k - 1)]*_exp43i; cIm[(k - 1)] = cRe[(k - 1)]*_exp43i + cIm[(k - 1)]*_exp43r; cRe[(k - 1)] = _tr; }
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, r, 0, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _conj47r = 0, _conj47i = 0;
        _conj47r = _exp46r; _conj47i = -(_exp46i);
        { double _tr = cRe[(-k)]*_conj47r - cIm[(-k)]*_conj47i; cIm[(-k)] = cRe[(-k)]*_conj47i + cIm[(-k)]*_conj47r; cRe[(-k)] = _tr; }
    }
    double _c48r = 0, _c48i = 0;
    _c48r = 1.0; _c48i = 0;
    double _add49r = 0, _add49i = 0;
    _add49r = n + _c48r; _add49i = 0 + _c48i;
    for (int r = 1; r < (int)(_add49r); r++) {
        double _c50r = 0, _c50i = 0;
        _c50r = 0.1; _c50i = 0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c50r, _c50i, r, 0, &_mul51r, &_mul51i);
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _neg53r = 0, _neg53i = 0;
        _neg53r = -(_c52r); _neg53i = -(_c52i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_neg53r, _neg53i, r, 0, &_mul54r, &_mul54i);
        double _div55r = 0, _div55i = 0;
        c_div(_mul54r, _mul54i, n, 0, &_div55r, &_div55i);
        double _exp56r = 0, _exp56i = 0;
        c_exp2(_div55r, _div55i, &_exp56r, &_exp56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_mul51r, _mul51i, _exp56r, _exp56i, &_mul57r, &_mul57i);
        cRe[(r - 1)] += _mul57r; cIm[(r - 1)] += _mul57i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_414_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(k, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double j = _add6r; /* +_add6ii */
        double _re7r = 0, _re7i = 0;
        _re7r = x1r; _re7i = 0;
        double _sin8r = 0, _sin8i = 0;
        c_sin(k, 0, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_re7r, _re7i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _re10r = 0, _re10i = 0;
        _re10r = x2r; _re10i = 0;
        double _cos11r = 0, _cos11i = 0;
        c_cos(j, 0, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_re10r, _re10i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul9r + _mul12r; _add13i = _mul9i + _mul12i;
        double _add14r = 0, _add14i = 0;
        _add14r = j + k; _add14i = 0 + 0;
        double _div15r = 0, _div15i = 0;
        c_div(_add13r, _add13i, _add14r, _add14i, &_div15r, &_div15i);
        double r = _div15r; /* +_div15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _abs17r; _add18i = _abs16i + _abs17i;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.5; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(k, 0, 1.5, &_pow20r, &_pow20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add18r + _pow20r; _add21i = _add18i + _pow20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(r, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _div24r = 0, _div24i = 0;
        c_div(_mul23r, _mul23i, j, 0, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log22r, _log22i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(r, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = k + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_mul27r, _mul27i, _add29r, _add29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_mul(_cos31r, _cos31i, _cos31r, _cos31i, &_pow33r, &_pow33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul26r + _pow33r; _add34i = _mul26i + _pow33i;
        double mag = _add34r; /* +_add34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x1r, x1i); _ang35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = k + _c36r; _add37i = 0 + _c36i;
        double _div38r = 0, _div38i = 0;
        c_div(j, 0, _add37r, _add37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang35r, _ang35i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _sin42r = 0, _sin42i = 0;
        c_sin(r, 0, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang41r, _ang41i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul40r + _mul43r; _add44i = _mul40i + _mul43i;
        double angle = _add44r; /* +_add44ii */
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
        c_mul(mag, 0, _add49r, _add49i, &_mul50r, &_mul50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 1.0; _c51i = 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 0.05; _c52i = 0;
        double _c53r = 0, _c53i = 0;
        _c53r = 2.0; _c53i = 0;
        double _pow54r = 0, _pow54i = 0;
        c_mul(k, 0, k, 0, &_pow54r, &_pow54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c52r, _c52i, _pow54r, _pow54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _c51r + _mul55r; _add56i = _c51i + _mul55i;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_mul50r, _mul50i, _add56r, _add56i, &_mul57r, &_mul57i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_415_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _abs4r; _add5i = _abs3i + _abs4i;
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + j; _add6i = _add5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 7.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _c8r + _sin12r; _add13i = _c8i + _sin12i;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log7r, _log7i, _add13r, _add13i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 5.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _c15r + _cos19r; _add20i = _c15i + _cos19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_mul14r, _mul14i, _add20r, _add20i, &_mul21r, &_mul21i);
        double mag = _mul21r; /* +_mul21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 3.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(j, 0, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang22r, _ang22i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 4.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(j, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang27r, _ang27i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul26r + _mul31r; _add32i = _mul26i + _mul31i;
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
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x1r; _conj39i = -(x1i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(j, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj39r, _conj39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul38r + _mul41r; _add42i = _mul38i + _mul41i;
        double _conj43r = 0, _conj43i = 0;
        _conj43r = x2r; _conj43i = -(x2i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(j, 0, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_conj43r, _conj43i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _sub46r = 0, _sub46i = 0;
        _sub46r = _add42r - _mul45r; _sub46i = _add42i - _mul45i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub46r; cIm[_idx] = _sub46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_416_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
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
        double mag_part1 = _log5r; /* +_log5ii */
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 3.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_sin9r, _sin9i, _abs10r, _abs10i, &_mul11r, &_mul11i);
        double mag_part2 = _mul11r; /* +_mul11ii */
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(j, 0, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(mag_part2, 0, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = mag_part1 + _mul15r; _add16i = 0 + _mul15i;
        double mag = _add16r; /* +_add16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang17r, _ang17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double angle_part1 = _mul21r; /* +_mul21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 5.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_sin25r, _sin25i, _ang26r, _ang26i, &_mul27r, &_mul27i);
        double angle_part2 = _mul27r; /* +_mul27ii */
        double _add28r = 0, _add28i = 0;
        _add28r = angle_part1 + angle_part2; _add28i = 0 + 0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(j, 0, &_sin29r, &_sin29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 3.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_sin29r, _sin29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _add28r + _mul33r; _add34i = _add28i + _mul33i;
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
        c_mul(mag, 0, _add39r, _add39i, &_mul40r, &_mul40i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_417_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _add7r = 0, _add7i = 0;
        _add7r = r1 + j; _add7i = 0 + 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_add7r, _add7i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.5; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(j, 0, 1.5, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, r2, 0, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _pow13r + _sin15r; _add16i = _pow13i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log11r, _log11i, _add16r, _add16i, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, i1, 0, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(_cos20r, _cos20i); _abs21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _c18r + _abs21r; _add22i = _c18i + _abs21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul17r, _mul17i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, r2, 0, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang24r, _ang24i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, i1, 0, &_mul29r, &_mul29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_mul29r, _mul29i, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang28r, _ang28i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul27r + _mul31r; _add32i = _mul27i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, i2, 0, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add32r + _sin34r; _add35i = _add32i + _sin34i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_418_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re1r + (_re2r - _re1r) * _li / 34.0;
    }
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im3r + (_im4r - _im3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 0;
        double sum_re = _c7r; /* +_c7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        double sum_im = _c8r; /* +_c8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        for (int k = 1; k < (int)(_add10r); k++) {
            double _arr11r = 0, _arr11i = 0;
            { int _idx = (k - 1); _arr11r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr11i = 0; }
            double _pow12r = 0, _pow12i = 0;
            c_powr(_arr11r, _arr11i, k, &_pow12r, &_pow12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(k, 0, M_PI, 0, &_mul13r, &_mul13i);
            double _c14r = 0, _c14i = 0;
            _c14r = 1.0; _c14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = j + _c14r; _add15i = 0 + _c14i;
            double _div16r = 0, _div16i = 0;
            c_div(_mul13r, _mul13i, _add15r, _add15i, &_div16r, &_div16i);
            double _cos17r = 0, _cos17i = 0;
            c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(_pow12r, _pow12i, _cos17r, _cos17i, &_mul18r, &_mul18i);
            sum_re += _mul18r;
            double _arr19r = 0, _arr19i = 0;
            { int _idx = (k - 1); _arr19r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr19i = 0; }
            double _pow20r = 0, _pow20i = 0;
            c_powr(_arr19r, _arr19i, k, &_pow20r, &_pow20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(k, 0, M_PI, 0, &_mul21r, &_mul21i);
            double _c22r = 0, _c22i = 0;
            _c22r = 1.0; _c22i = 0;
            double _add23r = 0, _add23i = 0;
            _add23r = j + _c22r; _add23i = 0 + _c22i;
            double _div24r = 0, _div24i = 0;
            c_div(_mul21r, _mul21i, _add23r, _add23i, &_div24r, &_div24i);
            double _sin25r = 0, _sin25i = 0;
            c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
            double _mul26r = 0, _mul26i = 0;
            c_mul(_pow20r, _pow20i, _sin25r, _sin25i, &_mul26r, &_mul26i);
            sum_im += _mul26r;
        }
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(sum_re, 0, sum_re, 0, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _c27r + _pow29r; _add30i = _c27i + _pow29i;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(sum_im, 0, sum_im, 0, &_pow32r, &_pow32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _add30r + _pow32r; _add33i = _add30i + _pow32i;
        double _log34r = 0, _log34i = 0;
        c_log(_add33r, _add33i, &_log34r, &_log34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 5.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_log34r, _log34i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(sum_re, 0, sum_im, 0, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _c40r + _mul41r; _add42i = _c40i + _mul41i;
        double _log43r = 0, _log43i = 0;
        c_log(_add42r, _add42i, &_log43r, &_log43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(j, 0, M_PI, 0, &_mul44r, &_mul44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 7.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(_mul44r, _mul44i, _c45r, _c45i, &_div46r, &_div46i);
        double _cos47r = 0, _cos47i = 0;
        c_cos(_div46r, _div46i, &_cos47r, &_cos47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_log43r, _log43i, _cos47r, _cos47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul39r + _mul48r; _add49i = _mul39i + _mul48i;
        double magnitude = _add49r; /* +_add49ii */
        double _ang50r = 0, _ang50i = 0;
        _ang50r = c_arg(x1r, x1i); _ang50i = 0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_ang50r, _ang50i, sum_re, 0, &_mul51r, &_mul51i);
        double _ang52r = 0, _ang52i = 0;
        _ang52r = c_arg(x2r, x2i); _ang52i = 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_ang52r, _ang52i, sum_im, 0, &_mul53r, &_mul53i);
        double _sub54r = 0, _sub54i = 0;
        _sub54r = _mul51r - _mul53r; _sub54i = _mul51i - _mul53i;
        double _mul55r = 0, _mul55i = 0;
        c_mul(j, 0, M_PI, 0, &_mul55r, &_mul55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 3.0; _c56i = 0;
        double _div57r = 0, _div57i = 0;
        c_div(_mul55r, _mul55i, _c56r, _c56i, &_div57r, &_div57i);
        double _sin58r = 0, _sin58i = 0;
        c_sin(_div57r, _div57i, &_sin58r, &_sin58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _sub54r + _sin58r; _add59i = _sub54i + _sin58i;
        double _mul60r = 0, _mul60i = 0;
        c_mul(j, 0, M_PI, 0, &_mul60r, &_mul60i);
        double _c61r = 0, _c61i = 0;
        _c61r = 7.0; _c61i = 0;
        double _div62r = 0, _div62i = 0;
        c_div(_mul60r, _mul60i, _c61r, _c61i, &_div62r, &_div62i);
        double _cos63r = 0, _cos63i = 0;
        c_cos(_div62r, _div62i, &_cos63r, &_cos63i);
        double _sub64r = 0, _sub64i = 0;
        _sub64r = _add59r - _cos63r; _sub64i = _add59i - _cos63i;
        double angle = _sub64r; /* +_sub64ii */
        double _c65r = 0, _c65i = 0;
        _c65r = 0.0; _c65i = 1.0;
        double _mul66r = 0, _mul66i = 0;
        c_mul(_c65r, _c65i, angle, 0, &_mul66r, &_mul66i);
        double _exp67r = 0, _exp67i = 0;
        c_exp2(_mul66r, _mul66i, &_exp67r, &_exp67i);
        double _mul68r = 0, _mul68i = 0;
        c_mul(magnitude, 0, _exp67r, _exp67i, &_mul68r, &_mul68i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul68r; cIm[_idx] = _mul68i; } }
    }
    double _c69r = 0, _c69i = 0;
    _c69r = 1.0; _c69i = 0;
    double _add70r = 0, _add70i = 0;
    _add70r = n + _c69r; _add70i = 0 + _c69i;
    for (int k = 1; k < (int)(_add70r); k++) {
        double _re71r = 0, _re71i = 0;
        _re71r = x1r; _re71i = 0;
        double _re72r = 0, _re72i = 0;
        _re72r = x2r; _re72i = 0;
        double _mul73r = 0, _mul73i = 0;
        c_mul(_re71r, _re71i, _re72r, _re72i, &_mul73r, &_mul73i);
        double _c74r = 0, _c74i = 0;
        _c74r = 1.0; _c74i = 0;
        double _add75r = 0, _add75i = 0;
        _add75r = k + _c74r; _add75i = 0 + _c74i;
        double _div76r = 0, _div76i = 0;
        c_div(_mul73r, _mul73i, _add75r, _add75i, &_div76r, &_div76i);
        double _mul77r = 0, _mul77i = 0;
        c_mul(k, 0, M_PI, 0, &_mul77r, &_mul77i);
        double _c78r = 0, _c78i = 0;
        _c78r = 6.0; _c78i = 0;
        double _div79r = 0, _div79i = 0;
        c_div(_mul77r, _mul77i, _c78r, _c78i, &_div79r, &_div79i);
        double _sin80r = 0, _sin80i = 0;
        c_sin(_div79r, _div79i, &_sin80r, &_sin80i);
        double _mul81r = 0, _mul81i = 0;
        c_mul(_div76r, _div76i, _sin80r, _sin80i, &_mul81r, &_mul81i);
        double _im82r = 0, _im82i = 0;
        _im82r = x1i; _im82i = 0;
        double _im83r = 0, _im83i = 0;
        _im83r = x2i; _im83i = 0;
        double _add84r = 0, _add84i = 0;
        _add84r = _im82r + _im83r; _add84i = _im82i + _im83i;
        double _mul85r = 0, _mul85i = 0;
        c_mul(k, 0, M_PI, 0, &_mul85r, &_mul85i);
        double _c86r = 0, _c86i = 0;
        _c86r = 8.0; _c86i = 0;
        double _div87r = 0, _div87i = 0;
        c_div(_mul85r, _mul85i, _c86r, _c86i, &_div87r, &_div87i);
        double _cos88r = 0, _cos88i = 0;
        c_cos(_div87r, _div87i, &_cos88r, &_cos88i);
        double _mul89r = 0, _mul89i = 0;
        c_mul(_add84r, _add84i, _cos88r, _cos88i, &_mul89r, &_mul89i);
        double _add90r = 0, _add90i = 0;
        _add90r = _mul81r + _mul89r; _add90i = _mul81i + _mul89i;
        cRe[(k - 1)] += _add90r; cIm[(k - 1)] += _add90i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_419_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
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
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 7.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log5r, _log5i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 5.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul10r + _cos14r; _add15i = _mul10i + _cos14i;
        double mag_part = _add15r; /* +_add15ii */
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
        double angle_part = _add24r; /* +_add24ii */
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c25r, _c25i, angle_part, 0, &_mul26r, &_mul26i);
        double _exp27r = 0, _exp27i = 0;
        c_exp2(_mul26r, _mul26i, &_exp27r, &_exp27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_part, 0, _exp27r, _exp27i, &_mul28r, &_mul28i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_420_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x1r, x1i); _ang7i = 0;
        double _sin8r = 0, _sin8i = 0;
        c_sin(j, 0, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_ang7r, _ang7i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x2r, x2i); _ang10i = 0;
        double _cos11r = 0, _cos11i = 0;
        c_cos(j, 0, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_ang10r, _ang10i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul9r + _mul12r; _add13i = _mul9i + _mul12i;
        double angle_component = _add13r; /* +_add13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 5.0; _c15i = 0;
        double _mod16r = 0, _mod16i = 0;
        _mod16r = fmod(j, _c15r); _mod16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _mod16r + _c17r; _add18i = _mod16i + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_abs14r, _abs14i, _add18r, &_pow19r, &_pow19i);
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x2r, x2i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log23r, _log23i, j, 0, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _pow19r + _mul24r; _add25i = _pow19i + _mul24i;
        double magnitude_component = _add25r; /* +_add25ii */
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 7.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _sin29r + _cos33r; _add34i = _sin29i + _cos33i;
        double phase_shift = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _add36r = 0, _add36i = 0;
        _add36r = angle_component + phase_shift; _add36i = 0 + 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c35r, _c35i, _add36r, _add36i, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude_component, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x1r; _conj40i = -(x1i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj40r, _conj40i, r2, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = j + _c42r; _add43i = 0 + _c42i;
        double _div44r = 0, _div44i = 0;
        c_div(_mul41r, _mul41i, _add43r, _add43i, &_div44r, &_div44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul39r + _div44r; _add45i = _mul39i + _div44i;
        double _add46r = 0, _add46i = 0;
        _add46r = i1 + i2; _add46i = 0 + 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 3.0; _c47i = 0;
        double _mod48r = 0, _mod48i = 0;
        _mod48r = fmod(j, _c47r); _mod48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_add46r, _add46i, _mod48r, _mod48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _add45r + _mul49r; _add50i = _add45i + _mul49i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    double _c51r = 0, _c51i = 0;
    _c51r = 1.0; _c51i = 0;
    double _add52r = 0, _add52i = 0;
    _add52r = n + _c51r; _add52i = 0 + _c51i;
    for (int k = 1; k < (int)(_add52r); k++) {
        double _c53r = 0, _c53i = 0;
        _c53r = 3.0; _c53i = 0;
        double _div54r = 0, _div54i = 0;
        c_div(n, 0, _c53r, _c53i, &_div54r, &_div54i);
        if (k <= _div54r) {
            double _c55r = 0, _c55i = 0;
            _c55r = 1.0; _c55i = 0;
            double _c56r = 0, _c56i = 0;
            _c56r = 0.5; _c56i = 0;
            double _sin57r = 0, _sin57i = 0;
            c_sin(k, 0, &_sin57r, &_sin57i);
            double _mul58r = 0, _mul58i = 0;
            c_mul(_c56r, _c56i, _sin57r, _sin57i, &_mul58r, &_mul58i);
            double _add59r = 0, _add59i = 0;
            _add59r = _c55r + _mul58r; _add59i = _c55i + _mul58i;
            { double _tr = cRe[(k - 1)]*_add59r - cIm[(k - 1)]*_add59i; cIm[(k - 1)] = cRe[(k - 1)]*_add59i + cIm[(k - 1)]*_add59r; cRe[(k - 1)] = _tr; }
        } else {
            double _c60r = 0, _c60i = 0;
            _c60r = 2.0; _c60i = 0;
            double _mul61r = 0, _mul61i = 0;
            c_mul(_c60r, _c60i, n, 0, &_mul61r, &_mul61i);
            double _c62r = 0, _c62i = 0;
            _c62r = 3.0; _c62i = 0;
            double _div63r = 0, _div63i = 0;
            c_div(_mul61r, _mul61i, _c62r, _c62i, &_div63r, &_div63i);
            if (k <= _div63r) {
                double _c64r = 0, _c64i = 0;
                _c64r = 1.0; _c64i = 0;
                double _c65r = 0, _c65i = 0;
                _c65r = 0.3; _c65i = 0;
                double _c66r = 0, _c66i = 0;
                _c66r = 2.0; _c66i = 0;
                double _mul67r = 0, _mul67i = 0;
                c_mul(k, 0, _c66r, _c66i, &_mul67r, &_mul67i);
                double _cos68r = 0, _cos68i = 0;
                c_cos(_mul67r, _mul67i, &_cos68r, &_cos68i);
                double _mul69r = 0, _mul69i = 0;
                c_mul(_c65r, _c65i, _cos68r, _cos68i, &_mul69r, &_mul69i);
                double _add70r = 0, _add70i = 0;
                _add70r = _c64r + _mul69r; _add70i = _c64i + _mul69i;
                { double _tr = cRe[(k - 1)]*_add70r - cIm[(k - 1)]*_add70i; cIm[(k - 1)] = cRe[(k - 1)]*_add70i + cIm[(k - 1)]*_add70r; cRe[(k - 1)] = _tr; }
            } else {
                double _c71r = 0, _c71i = 0;
                _c71r = 1.0; _c71i = 0;
                double _c72r = 0, _c72i = 0;
                _c72r = 0.2; _c72i = 0;
                double _c73r = 0, _c73i = 0;
                _c73r = 3.0; _c73i = 0;
                double _mul74r = 0, _mul74i = 0;
                c_mul(k, 0, _c73r, _c73i, &_mul74r, &_mul74i);
                double _sin75r = 0, _sin75i = 0;
                c_sin(_mul74r, _mul74i, &_sin75r, &_sin75i);
                double _mul76r = 0, _mul76i = 0;
                c_mul(_c72r, _c72i, _sin75r, _sin75i, &_mul76r, &_mul76i);
                double _cos77r = 0, _cos77i = 0;
                c_cos(k, 0, &_cos77r, &_cos77i);
                double _mul78r = 0, _mul78i = 0;
                c_mul(_mul76r, _mul76i, _cos77r, _cos77i, &_mul78r, &_mul78i);
                double _add79r = 0, _add79i = 0;
                _add79r = _c71r + _mul78r; _add79i = _c71i + _mul78i;
                { double _tr = cRe[(k - 1)]*_add79r - cIm[(k - 1)]*_add79i; cIm[(k - 1)] = cRe[(k - 1)]*_add79i + cIm[(k - 1)]*_add79r; cRe[(k - 1)] = _tr; }
            }
        }
    }
    for (int r = 1; r < (int)(n); r++) {
        double _c80r = 0, _c80i = 0;
        _c80r = 0.1; _c80i = 0;
        double _cf81r = 0, _cf81i = 0;
        { int _idx = r; if (_idx >= 0 && _idx < 36) { _cf81r = cRe[_idx]; _cf81i = cIm[_idx]; } }
        double _mul82r = 0, _mul82i = 0;
        c_mul(_c80r, _c80i, _cf81r, _cf81i, &_mul82r, &_mul82i);
        double _c83r = 0, _c83i = 0;
        _c83r = 0.0; _c83i = 1.0;
        double _cf84r = 0, _cf84i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf84r = cRe[_idx]; _cf84i = cIm[_idx]; } }
        double _ang85r = 0, _ang85i = 0;
        _ang85r = c_arg(_cf84r, _cf84i); _ang85i = 0;
        double _mul86r = 0, _mul86i = 0;
        c_mul(_c83r, _c83i, _ang85r, _ang85i, &_mul86r, &_mul86i);
        double _exp87r = 0, _exp87i = 0;
        c_exp2(_mul86r, _mul86i, &_exp87r, &_exp87i);
        double _mul88r = 0, _mul88i = 0;
        c_mul(_mul82r, _mul82i, _exp87r, _exp87i, &_mul88r, &_mul88i);
        cRe[(r - 1)] += _mul88r; cIm[(r - 1)] += _mul88i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_421_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double angle = 0;
        double mag = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 10.0; _c3i = 0;
        if (j <= _c3r) {
            double _abs4r = 0, _abs4i = 0;
            _abs4r = c_abs(x1r, x1i); _abs4i = 0;
            double _add5r = 0, _add5i = 0;
            _add5r = _abs4r + j; _add5i = _abs4i + 0;
            double _log6r = 0, _log6i = 0;
            c_log(_add5r, _add5i, &_log6r, &_log6i);
            double _c7r = 0, _c7i = 0;
            _c7r = 1.0; _c7i = 0;
            double _re8r = 0, _re8i = 0;
            _re8r = x1r; _re8i = 0;
            double _mul9r = 0, _mul9i = 0;
            c_mul(j, 0, _re8r, _re8i, &_mul9r, &_mul9i);
            double _sin10r = 0, _sin10i = 0;
            c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
            double _im11r = 0, _im11i = 0;
            _im11r = x2i; _im11i = 0;
            double _mul12r = 0, _mul12i = 0;
            c_mul(j, 0, _im11r, _im11i, &_mul12r, &_mul12i);
            double _cos13r = 0, _cos13i = 0;
            c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(_sin10r, _sin10i, _cos13r, _cos13i, &_mul14r, &_mul14i);
            double _add15r = 0, _add15i = 0;
            _add15r = _c7r + _mul14r; _add15i = _c7i + _mul14i;
            double _mul16r = 0, _mul16i = 0;
            c_mul(_log6r, _log6i, _add15r, _add15i, &_mul16r, &_mul16i);
            mag = _mul16r;
            double _ang17r = 0, _ang17i = 0;
            _ang17r = c_arg(x1r, x1i); _ang17i = 0;
            double _c18r = 0, _c18i = 0;
            _c18r = 2.0; _c18i = 0;
            double _div19r = 0, _div19i = 0;
            c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
            double _sin20r = 0, _sin20i = 0;
            c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(_ang17r, _ang17i, _sin20r, _sin20i, &_mul21r, &_mul21i);
            double _ang22r = 0, _ang22i = 0;
            _ang22r = c_arg(x2r, x2i); _ang22i = 0;
            double _c23r = 0, _c23i = 0;
            _c23r = 3.0; _c23i = 0;
            double _div24r = 0, _div24i = 0;
            c_div(j, 0, _c23r, _c23i, &_div24r, &_div24i);
            double _cos25r = 0, _cos25i = 0;
            c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
            double _mul26r = 0, _mul26i = 0;
            c_mul(_ang22r, _ang22i, _cos25r, _cos25i, &_mul26r, &_mul26i);
            double _add27r = 0, _add27i = 0;
            _add27r = _mul21r + _mul26r; _add27i = _mul21i + _mul26i;
            angle = _add27r;
        } else {
            double _c28r = 0, _c28i = 0;
            _c28r = 20.0; _c28i = 0;
            if (j <= _c28r) {
                double _mul29r = 0, _mul29i = 0;
                c_mul(x1r, x1i, x2r, x2i, &_mul29r, &_mul29i);
                double _abs30r = 0, _abs30i = 0;
                _abs30r = c_abs(_mul29r, _mul29i); _abs30i = 0;
                double _add31r = 0, _add31i = 0;
                _add31r = _abs30r + j; _add31i = _abs30i + 0;
                double _log32r = 0, _log32i = 0;
                c_log(_add31r, _add31i, &_log32r, &_log32i);
                double _c33r = 0, _c33i = 0;
                _c33r = 1.0; _c33i = 0;
                double _sin34r = 0, _sin34i = 0;
                c_sin(j, 0, &_sin34r, &_sin34i);
                double _c35r = 0, _c35i = 0;
                _c35r = 2.0; _c35i = 0;
                double _pow36r = 0, _pow36i = 0;
                c_mul(_sin34r, _sin34i, _sin34r, _sin34i, &_pow36r, &_pow36i);
                double _add37r = 0, _add37i = 0;
                _add37r = _c33r + _pow36r; _add37i = _c33i + _pow36i;
                double _cos38r = 0, _cos38i = 0;
                c_cos(j, 0, &_cos38r, &_cos38i);
                double _c39r = 0, _c39i = 0;
                _c39r = 2.0; _c39i = 0;
                double _pow40r = 0, _pow40i = 0;
                c_mul(_cos38r, _cos38i, _cos38r, _cos38i, &_pow40r, &_pow40i);
                double _sub41r = 0, _sub41i = 0;
                _sub41r = _add37r - _pow40r; _sub41i = _add37i - _pow40i;
                double _mul42r = 0, _mul42i = 0;
                c_mul(_log32r, _log32i, _sub41r, _sub41i, &_mul42r, &_mul42i);
                mag = _mul42r;
                double _add43r = 0, _add43i = 0;
                _add43r = x1r + x2r; _add43i = x1i + x2i;
                double _ang44r = 0, _ang44i = 0;
                _ang44r = c_arg(_add43r, _add43i); _ang44i = 0;
                double _c45r = 0, _c45i = 0;
                _c45r = 4.0; _c45i = 0;
                double _div46r = 0, _div46i = 0;
                c_div(j, 0, _c45r, _c45i, &_div46r, &_div46i);
                double _sin47r = 0, _sin47i = 0;
                c_sin(_div46r, _div46i, &_sin47r, &_sin47i);
                double _mul48r = 0, _mul48i = 0;
                c_mul(_ang44r, _ang44i, _sin47r, _sin47i, &_mul48r, &_mul48i);
                double _c49r = 0, _c49i = 0;
                _c49r = 1.0; _c49i = 0;
                double _add50r = 0, _add50i = 0;
                _add50r = j + _c49r; _add50i = 0 + _c49i;
                double _log51r = 0, _log51i = 0;
                c_log(_add50r, _add50i, &_log51r, &_log51i);
                double _add52r = 0, _add52i = 0;
                _add52r = _mul48r + _log51r; _add52i = _mul48i + _log51i;
                angle = _add52r;
            } else {
                double _abs53r = 0, _abs53i = 0;
                _abs53r = c_abs(x1r, x1i); _abs53i = 0;
                double _c54r = 0, _c54i = 0;
                _c54r = 2.0; _c54i = 0;
                double _pow55r = 0, _pow55i = 0;
                c_mul(_abs53r, _abs53i, _abs53r, _abs53i, &_pow55r, &_pow55i);
                double _abs56r = 0, _abs56i = 0;
                _abs56r = c_abs(x2r, x2i); _abs56i = 0;
                double _c57r = 0, _c57i = 0;
                _c57r = 2.0; _c57i = 0;
                double _pow58r = 0, _pow58i = 0;
                c_mul(_abs56r, _abs56i, _abs56r, _abs56i, &_pow58r, &_pow58i);
                double _add59r = 0, _add59i = 0;
                _add59r = _pow55r + _pow58r; _add59i = _pow55i + _pow58i;
                double _add60r = 0, _add60i = 0;
                _add60r = _add59r + j; _add60i = _add59i + 0;
                double _log61r = 0, _log61i = 0;
                c_log(_add60r, _add60i, &_log61r, &_log61i);
                double _c62r = 0, _c62i = 0;
                _c62r = 1.0; _c62i = 0;
                double _re63r = 0, _re63i = 0;
                _re63r = x1r; _re63i = 0;
                double _mul64r = 0, _mul64i = 0;
                c_mul(j, 0, _re63r, _re63i, &_mul64r, &_mul64i);
                double _re65r = 0, _re65i = 0;
                _re65r = x2r; _re65i = 0;
                double _mul66r = 0, _mul66i = 0;
                c_mul(j, 0, _re65r, _re65i, &_mul66r, &_mul66i);
                double _cos67r = 0, _cos67i = 0;
                c_cos(_mul66r, _mul66i, &_cos67r, &_cos67i);
                double _add68r = 0, _add68i = 0;
                _add68r = _mul64r + _cos67r; _add68i = _mul64i + _cos67i;
                double _sin69r = 0, _sin69i = 0;
                c_sin(_add68r, _add68i, &_sin69r, &_sin69i);
                double _add70r = 0, _add70i = 0;
                _add70r = _c62r + _sin69r; _add70i = _c62i + _sin69i;
                double _mul71r = 0, _mul71i = 0;
                c_mul(_log61r, _log61i, _add70r, _add70i, &_mul71r, &_mul71i);
                mag = _mul71r;
                double _ang72r = 0, _ang72i = 0;
                _ang72r = c_arg(x1r, x1i); _ang72i = 0;
                double _c73r = 0, _c73i = 0;
                _c73r = 5.0; _c73i = 0;
                double _div74r = 0, _div74i = 0;
                c_div(j, 0, _c73r, _c73i, &_div74r, &_div74i);
                double _cos75r = 0, _cos75i = 0;
                c_cos(_div74r, _div74i, &_cos75r, &_cos75i);
                double _mul76r = 0, _mul76i = 0;
                c_mul(_ang72r, _ang72i, _cos75r, _cos75i, &_mul76r, &_mul76i);
                double _ang77r = 0, _ang77i = 0;
                _ang77r = c_arg(x2r, x2i); _ang77i = 0;
                double _c78r = 0, _c78i = 0;
                _c78r = 6.0; _c78i = 0;
                double _div79r = 0, _div79i = 0;
                c_div(j, 0, _c78r, _c78i, &_div79r, &_div79i);
                double _sin80r = 0, _sin80i = 0;
                c_sin(_div79r, _div79i, &_sin80r, &_sin80i);
                double _mul81r = 0, _mul81i = 0;
                c_mul(_ang77r, _ang77i, _sin80r, _sin80i, &_mul81r, &_mul81i);
                double _add82r = 0, _add82i = 0;
                _add82r = _mul76r + _mul81r; _add82i = _mul76i + _mul81i;
                angle = _add82r;
            }
        }
        double _cos83r = 0, _cos83i = 0;
        c_cos(angle, 0, &_cos83r, &_cos83i);
        double _c84r = 0, _c84i = 0;
        _c84r = 0.0; _c84i = 1.0;
        double _sin85r = 0, _sin85i = 0;
        c_sin(angle, 0, &_sin85r, &_sin85i);
        double _mul86r = 0, _mul86i = 0;
        c_mul(_c84r, _c84i, _sin85r, _sin85i, &_mul86r, &_mul86i);
        double _add87r = 0, _add87i = 0;
        _add87r = _cos83r + _mul86r; _add87i = _cos83i + _mul86i;
        double _mul88r = 0, _mul88i = 0;
        c_mul(mag, 0, _add87r, _add87i, &_mul88r, &_mul88i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul88r; cIm[_idx] = _mul88i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_422_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, M_PI, 0, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 7.0; _c3i = 0;
        double _div4r = 0, _div4i = 0;
        c_div(_mul2r, _mul2i, _c3r, _c3i, &_div4r, &_div4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_div4r, _div4i, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_re1r, _re1i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _re7r = 0, _re7i = 0;
        _re7r = x2r; _re7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 5.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_div10r, _div10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_re7r, _re7i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul6r + _mul12r; _add13i = _mul6i + _mul12i;
        double r_part = _add13r; /* +_add13ii */
        double _im14r = 0, _im14i = 0;
        _im14r = x1i; _im14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 6.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_im14r, _im14i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _im20r = 0, _im20i = 0;
        _im20r = x2i; _im20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 8.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_im20r, _im20i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul19r - _mul25r; _sub26i = _mul19i - _mul25i;
        double im_part = _sub26r; /* +_sub26ii */
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x1r, x1i); _abs27i = 0;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x2r, x2i); _abs28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _abs27r + _abs28r; _add29i = _abs27i + _abs28i;
        double _add30r = 0, _add30i = 0;
        _add30r = _add29r + j; _add30i = _add29i + 0;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 4.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_log31r, _log31i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, M_PI, 0, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 3.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul36r + _cos40r; _add41i = _mul36i + _cos40i;
        double magnitude = _add41r; /* +_add41ii */
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x1r, x1i); _ang42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 3.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(j, 0, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang42r, _ang42i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x2r, x2i); _ang47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 4.0; _c48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(j, 0, _c48r, _c48i, &_div49r, &_div49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(_div49r, _div49i, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_ang47r, _ang47i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul46r + _mul51r; _add52i = _mul46i + _mul51i;
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
        double _conj59r = 0, _conj59i = 0;
        _conj59r = x1r; _conj59i = -(x1i);
        double _sin60r = 0, _sin60i = 0;
        c_sin(j, 0, &_sin60r, &_sin60i);
        double _mul61r = 0, _mul61i = 0;
        c_mul(_conj59r, _conj59i, _sin60r, _sin60i, &_mul61r, &_mul61i);
        double _add62r = 0, _add62i = 0;
        _add62r = _mul58r + _mul61r; _add62i = _mul58i + _mul61i;
        double _conj63r = 0, _conj63i = 0;
        _conj63r = x2r; _conj63i = -(x2i);
        double _cos64r = 0, _cos64i = 0;
        c_cos(j, 0, &_cos64r, &_cos64i);
        double _mul65r = 0, _mul65i = 0;
        c_mul(_conj63r, _conj63i, _cos64r, _cos64i, &_mul65r, &_mul65i);
        double _sub66r = 0, _sub66i = 0;
        _sub66r = _add62r - _mul65r; _sub66i = _add62i - _mul65i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub66r; cIm[_idx] = _sub66i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_423_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double angle = 0;
        double mag = 0;
        double _c1r = 0, _c1i = 0;
        _c1r = 10.0; _c1i = 0;
        if (j <= _c1r) {
            double _re2r = 0, _re2i = 0;
            _re2r = x1r; _re2i = 0;
            double _c3r = 0, _c3i = 0;
            _c3r = 2.0; _c3i = 0;
            double _pow4r = 0, _pow4i = 0;
            c_mul(j, 0, j, 0, &_pow4r, &_pow4i);
            double _mul5r = 0, _mul5i = 0;
            c_mul(_re2r, _re2i, _pow4r, _pow4i, &_mul5r, &_mul5i);
            double _abs6r = 0, _abs6i = 0;
            _abs6r = c_abs(x2r, x2i); _abs6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 1.0; _c7i = 0;
            double _add8r = 0, _add8i = 0;
            _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
            double _log9r = 0, _log9i = 0;
            c_log(_add8r, _add8i, &_log9r, &_log9i);
            double _re10r = 0, _re10i = 0;
            _re10r = x1r; _re10i = 0;
            double _mul11r = 0, _mul11i = 0;
            c_mul(j, 0, _re10r, _re10i, &_mul11r, &_mul11i);
            double _sin12r = 0, _sin12i = 0;
            c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_log9r, _log9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
            double _add14r = 0, _add14i = 0;
            _add14r = _mul5r + _mul13r; _add14i = _mul5i + _mul13i;
            mag = _add14r;
            double _ang15r = 0, _ang15i = 0;
            _ang15r = c_arg(x1r, x1i); _ang15i = 0;
            double _im16r = 0, _im16i = 0;
            _im16r = x2i; _im16i = 0;
            double _mul17r = 0, _mul17i = 0;
            c_mul(j, 0, _im16r, _im16i, &_mul17r, &_mul17i);
            double _cos18r = 0, _cos18i = 0;
            c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
            double _add19r = 0, _add19i = 0;
            _add19r = _ang15r + _cos18r; _add19i = _ang15i + _cos18i;
            angle = _add19r;
        } else {
            double _c20r = 0, _c20i = 0;
            _c20r = 25.0; _c20i = 0;
            if (j <= _c20r) {
                double _re21r = 0, _re21i = 0;
                _re21r = x2r; _re21i = 0;
                double _c22r = 0, _c22i = 0;
                _c22r = 1.0; _c22i = 0;
                double _add23r = 0, _add23i = 0;
                _add23r = j + _c22r; _add23i = 0 + _c22i;
                double _div24r = 0, _div24i = 0;
                c_div(_re21r, _re21i, _add23r, _add23i, &_div24r, &_div24i);
                double _exp25r = 0, _exp25i = 0;
                c_exp2(_div24r, _div24i, &_exp25r, &_exp25i);
                double _sqrt26r = 0, _sqrt26i = 0;
                c_powr(j, 0, 0.5, &_sqrt26r, &_sqrt26i);
                double _re27r = 0, _re27i = 0;
                _re27r = x1r; _re27i = 0;
                double _re28r = 0, _re28i = 0;
                _re28r = x2r; _re28i = 0;
                double _add29r = 0, _add29i = 0;
                _add29r = _re27r + _re28r; _add29i = _re27i + _re28i;
                double _mul30r = 0, _mul30i = 0;
                c_mul(_sqrt26r, _sqrt26i, _add29r, _add29i, &_mul30r, &_mul30i);
                double _add31r = 0, _add31i = 0;
                _add31r = _exp25r + _mul30r; _add31i = _exp25i + _mul30i;
                mag = _add31r;
                double _sin32r = 0, _sin32i = 0;
                c_sin(j, 0, &_sin32r, &_sin32i);
                double _ang33r = 0, _ang33i = 0;
                _ang33r = c_arg(x2r, x2i); _ang33i = 0;
                double _re34r = 0, _re34i = 0;
                _re34r = x1r; _re34i = 0;
                double _mul35r = 0, _mul35i = 0;
                c_mul(j, 0, _re34r, _re34i, &_mul35r, &_mul35i);
                double _cos36r = 0, _cos36i = 0;
                c_cos(_mul35r, _mul35i, &_cos36r, &_cos36i);
                double _mul37r = 0, _mul37i = 0;
                c_mul(_ang33r, _ang33i, _cos36r, _cos36i, &_mul37r, &_mul37i);
                double _add38r = 0, _add38i = 0;
                _add38r = _sin32r + _mul37r; _add38i = _sin32i + _mul37i;
                angle = _add38r;
            } else {
                double _abs39r = 0, _abs39i = 0;
                _abs39r = c_abs(x1r, x1i); _abs39i = 0;
                double _add40r = 0, _add40i = 0;
                _add40r = _abs39r + j; _add40i = _abs39i + 0;
                double _log41r = 0, _log41i = 0;
                c_log(_add40r, _add40i, &_log41r, &_log41i);
                double _re42r = 0, _re42i = 0;
                _re42r = x2r; _re42i = 0;
                double _neg43r = 0, _neg43i = 0;
                _neg43r = -(_re42r); _neg43i = -(_re42i);
                double _div44r = 0, _div44i = 0;
                c_div(_neg43r, _neg43i, j, 0, &_div44r, &_div44i);
                double _exp45r = 0, _exp45i = 0;
                c_exp2(_div44r, _div44i, &_exp45r, &_exp45i);
                double _mul46r = 0, _mul46i = 0;
                c_mul(_log41r, _log41i, _exp45r, _exp45i, &_mul46r, &_mul46i);
                double _im47r = 0, _im47i = 0;
                _im47r = x1i; _im47i = 0;
                double _mul48r = 0, _mul48i = 0;
                c_mul(j, 0, _im47r, _im47i, &_mul48r, &_mul48i);
                double _sin49r = 0, _sin49i = 0;
                c_sin(_mul48r, _mul48i, &_sin49r, &_sin49i);
                double _add50r = 0, _add50i = 0;
                _add50r = _mul46r + _sin49r; _add50i = _mul46i + _sin49i;
                mag = _add50r;
                double _mul51r = 0, _mul51i = 0;
                c_mul(x1r, x1i, x2r, x2i, &_mul51r, &_mul51i);
                double _ang52r = 0, _ang52i = 0;
                _ang52r = c_arg(_mul51r, _mul51i); _ang52i = 0;
                double _div53r = 0, _div53i = 0;
                c_div(_ang52r, _ang52i, j, 0, &_div53r, &_div53i);
                double _im54r = 0, _im54i = 0;
                _im54r = x2i; _im54i = 0;
                double _mul55r = 0, _mul55i = 0;
                c_mul(j, 0, _im54r, _im54i, &_mul55r, &_mul55i);
                double _cos56r = 0, _cos56i = 0;
                c_cos(_mul55r, _mul55i, &_cos56r, &_cos56i);
                double _add57r = 0, _add57i = 0;
                _add57r = _div53r + _cos56r; _add57i = _div53i + _cos56i;
                angle = _add57r;
            }
        }
        double _cos58r = 0, _cos58i = 0;
        c_cos(angle, 0, &_cos58r, &_cos58i);
        double _c59r = 0, _c59i = 0;
        _c59r = 0.0; _c59i = 1.0;
        double _sin60r = 0, _sin60i = 0;
        c_sin(angle, 0, &_sin60r, &_sin60i);
        double _mul61r = 0, _mul61i = 0;
        c_mul(_c59r, _c59i, _sin60r, _sin60i, &_mul61r, &_mul61i);
        double _add62r = 0, _add62i = 0;
        _add62r = _cos58r + _mul61r; _add62i = _cos58i + _mul61i;
        double _mul63r = 0, _mul63i = 0;
        c_mul(mag, 0, _add62r, _add62i, &_mul63r, &_mul63i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul63r; cIm[_idx] = _mul63i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_424_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_re3r, _re3i, _pow5r, _pow5i, &_mul6r, &_mul6i);
        double mag_part1 = _mul6r; /* +_mul6ii */
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log9r, _log9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double mag_part2 = _mul13r; /* +_mul13ii */
        double _re14r = 0, _re14i = 0;
        _re14r = x2r; _re14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _re14r, _re14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _sqrt17r = 0, _sqrt17i = 0;
        c_powr(j, 0, 0.5, &_sqrt17r, &_sqrt17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_cos16r, _cos16i, _sqrt17r, _sqrt17i, &_mul18r, &_mul18i);
        double mag_part3 = _mul18r; /* +_mul18ii */
        double _add19r = 0, _add19i = 0;
        _add19r = mag_part1 + mag_part2; _add19i = 0 + 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _add19r + mag_part3; _add20i = _add19i + 0;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _re22r = 0, _re22i = 0;
        _re22r = x1r; _re22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _re22r, _re22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _ang21r + _sin24r; _add25i = _ang21i + _sin24i;
        double angle_part1 = _add25r; /* +_add25ii */
        double _im26r = 0, _im26i = 0;
        _im26r = x2i; _im26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _im26r, _im26i, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_ang29r, _ang29i, j, 0, &_div30r, &_div30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _cos28r - _div30r; _sub31i = _cos28i - _div30i;
        double angle_part2 = _sub31r; /* +_sub31ii */
        double _add32r = 0, _add32i = 0;
        _add32r = angle_part1 + angle_part2; _add32i = 0 + 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_425_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re1r + (_re2r - _re1r) * _li / 34.0;
    }
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im3r + (_im4r - _im3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 6.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 8.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin10r, _sin10i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang16r, _ang16i, _log19r, _log19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul15r + _mul20r; _add21i = _mul15i + _mul20i;
        double angle_part = _add21r; /* +_add21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(j, 0, j, 0, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _abs22r + _pow24r; _add25i = _abs22i + _pow24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_cos27r, _cos27i); _abs28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log26r, _log26i, _abs28r, _abs28i, &_mul29r, &_mul29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs30r + j; _add31i = _abs30i + 0;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(j, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(_sin35r, _sin35i); _abs36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_log32r, _log32i, _abs36r, _abs36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul29r + _mul37r; _add38i = _mul29i + _mul37i;
        double magnitude_part = _add38r; /* +_add38ii */
        double _re39r = 0, _re39i = 0;
        _re39r = x1r; _re39i = 0;
        double _re40r = 0, _re40i = 0;
        _re40r = x2r; _re40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_re39r, _re39i, _re40r, _re40i, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = j + _c42r; _add43i = 0 + _c42i;
        double _div44r = 0, _div44i = 0;
        c_div(_mul41r, _mul41i, _add43r, _add43i, &_div44r, &_div44i);
        double _add45r = 0, _add45i = 0;
        _add45r = magnitude_part + _div44r; _add45i = 0 + _div44i;
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c46r, _c46i, angle_part, 0, &_mul47r, &_mul47i);
        double _exp48r = 0, _exp48i = 0;
        c_exp2(_mul47r, _mul47i, &_exp48r, &_exp48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_add45r, _add45i, _exp48r, _exp48i, &_mul49r, &_mul49i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
        double _c50r = 0, _c50i = 0;
        _c50r = 5.0; _c50i = 0;
        double _mod51r = 0, _mod51i = 0;
        _mod51r = fmod(j, _c50r); _mod51i = 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 0;
        if (_mod51r == _c52r) {
            double _cf53r = 0, _cf53i = 0;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf53r = cRe[_idx]; _cf53i = cIm[_idx]; } }
            double _conj54r = 0, _conj54i = 0;
            _conj54r = _cf53r; _conj54i = -(_cf53i);
            cRe[(j - 1)] += _conj54r; cIm[(j - 1)] += _conj54i;
        }
        double _c55r = 0, _c55i = 0;
        _c55r = 1.0; _c55i = 0;
        double _c56r = 0, _c56i = 0;
        _c56r = 0.1; _c56i = 0;
        double _sin57r = 0, _sin57i = 0;
        c_sin(j, 0, &_sin57r, &_sin57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_c56r, _c56i, _sin57r, _sin57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _c55r + _mul58r; _add59i = _c55i + _mul58i;
        { double _tr = cRe[(j - 1)]*_add59r - cIm[(j - 1)]*_add59i; cIm[(j - 1)] = cRe[(j - 1)]*_add59i + cIm[(j - 1)]*_add59r; cRe[(j - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_426_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 36.0; _c3i = 0;
        double _sub4r = 0, _sub4i = 0;
        _sub4r = _c3r - k; _sub4i = _c3i - 0;
        double j = _sub4r; /* +_sub4ii */
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _sin6r = 0, _sin6i = 0;
        c_sin(k, 0, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_re5r, _re5i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(j, 0, &_cos8r, &_cos8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_mul7r, _mul7i, _cos8r, _cos8i, &_mul9r, &_mul9i);
        double term1 = _mul9r; /* +_mul9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x2i; _im10i = 0;
        double _cos11r = 0, _cos11i = 0;
        c_cos(k, 0, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_im10r, _im10i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(j, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_mul12r, _mul12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double term2 = _mul14r; /* +_mul14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs15r + k; _add16i = _abs15i + 0;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double term3 = _log17r; /* +_log17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs18r + j; _add19i = _abs18i + 0;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double term4 = _log20r; /* +_log20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(k, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
        double angle = _add31r; /* +_add31ii */
        double _add32r = 0, _add32i = 0;
        _add32r = term1 + term2; _add32i = 0 + 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _add32r + term3; _add33i = _add32i + 0;
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _add33r - term4; _sub34i = _add33i - 0;
        double magnitude = _sub34r; /* +_sub34ii */
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_427_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = _re4r - _re5r; _sub6i = _re4i - _re5i;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_sub6r, _sub6i, j, 0, &_mul7r, &_mul7i);
        double _div8r = 0, _div8i = 0;
        c_div(_mul7r, _mul7i, n, 0, &_div8r, &_div8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _re3r + _div8r; _add9i = _re3i + _div8i;
        double rec = _add9r; /* +_add9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _im11r = 0, _im11i = 0;
        _im11r = x2i; _im11i = 0;
        double _im12r = 0, _im12i = 0;
        _im12r = x1i; _im12i = 0;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _im11r - _im12r; _sub13i = _im11i - _im12i;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sub13r, _sub13i, j, 0, &_mul14r, &_mul14i);
        double _div15r = 0, _div15i = 0;
        c_div(_mul14r, _mul14i, n, 0, &_div15r, &_div15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _im10r + _div15r; _add16i = _im10i + _div15i;
        double imc = _add16r; /* +_add16ii */
        double _conj17r = 0, _conj17i = 0;
        _conj17r = x1r; _conj17i = -(x1i);
        double conj_t1 = _conj17r; /* +_conj17ii */
        double _conj18r = 0, _conj18i = 0;
        _conj18r = x2r; _conj18i = -(x2i);
        double conj_t2 = _conj18r; /* +_conj18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(rec, 0); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log22r, _log22i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double mag_part1 = _mul27r; /* +_mul27ii */
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(imc, 0); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs28r + _c29r; _add30i = _abs28i + _c29i;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 4.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_log31r, _log31i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double mag_part2 = _mul36r; /* +_mul36ii */
        double _add37r = 0, _add37i = 0;
        _add37r = mag_part1 + mag_part2; _add37i = 0 + 0;
        double magnitude = _add37r; /* +_add37ii */
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang38r, _ang38i, j, 0, &_mul39r, &_mul39i);
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(x2r, x2i); _ang40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = j + _c41r; _add42i = 0 + _c41i;
        double _div43r = 0, _div43i = 0;
        c_div(_ang40r, _ang40i, _add42r, _add42i, &_div43r, &_div43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul39r + _div43r; _add44i = _mul39i + _div43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(j, 0, imc, 0, &_mul45r, &_mul45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_mul45r, _mul45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(j, 0, rec, 0, &_mul47r, &_mul47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_mul47r, _mul47i, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_sin46r, _sin46i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _add44r + _mul49r; _add50i = _add44i + _mul49i;
        double angle = _add50r; /* +_add50ii */
        double _cos51r = 0, _cos51i = 0;
        c_cos(angle, 0, &_cos51r, &_cos51i);
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _sin53r = 0, _sin53i = 0;
        c_sin(angle, 0, &_sin53r, &_sin53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c52r, _c52i, _sin53r, _sin53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _cos51r + _mul54r; _add55i = _cos51i + _mul54i;
        double _mul56r = 0, _mul56i = 0;
        c_mul(magnitude, 0, _add55r, _add55i, &_mul56r, &_mul56i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_428: too complex for auto-transpile, stubbed */
static void poly_428_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_429_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
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
        double _re6r = 0, _re6i = 0;
        _re6r = x2r; _re6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _re6r, _re6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _im10r, _im10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul9r + _cos12r; _add13i = _mul9i + _cos12i;
        double mag_part = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang14r, _ang14i, j, 0, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _sqrt17r = 0, _sqrt17i = 0;
        c_powr(j, 0, 0.5, &_sqrt17r, &_sqrt17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sqrt17r, _sqrt17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _mul18r; _add19i = _mul15i + _mul18i;
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin20r, _sin20i, _ang21r, _ang21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _add19r + _mul22r; _add23i = _add19i + _mul22i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    }
    for (int k = 1; k < 36; k++) {
        double _re30r = 0, _re30i = 0;
        _re30r = x1r; _re30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_re30r, _re30i, k, 0, &_mul31r, &_mul31i);
        double r = _mul31r; /* +_mul31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 4.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(k, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 6.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(k, 0, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul36r + _sin39r; _add40i = _mul36i + _sin39i;
        double angle = _add40r; /* +_add40ii */
        double _add41r = 0, _add41i = 0;
        _add41r = x1r + x2r; _add41i = x1i + x2i;
        double _abs42r = 0, _abs42i = 0;
        _abs42r = c_abs(_add41r, _add41i); _abs42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = _abs42r + _c43r; _add44i = _abs42i + _c43i;
        double _log45r = 0, _log45i = 0;
        c_log(_add44r, _add44i, &_log45r, &_log45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(angle, 0, &_cos46r, &_cos46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_log45r, _log45i, _cos46r, _cos46i, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _sub49r = 0, _sub49i = 0;
        _sub49r = x1r - x2r; _sub49i = x1i - x2i;
        double _abs50r = 0, _abs50i = 0;
        _abs50r = c_abs(_sub49r, _sub49i); _abs50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 1.0; _c51i = 0;
        double _add52r = 0, _add52i = 0;
        _add52r = _abs50r + _c51r; _add52i = _abs50i + _c51i;
        double _log53r = 0, _log53i = 0;
        c_log(_add52r, _add52i, &_log53r, &_log53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c48r, _c48i, _log53r, _log53i, &_mul54r, &_mul54i);
        double _sin55r = 0, _sin55i = 0;
        c_sin(angle, 0, &_sin55r, &_sin55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_mul54r, _mul54i, _sin55r, _sin55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _mul47r + _mul56r; _add57i = _mul47i + _mul56i;
        cRe[(k - 1)] += _add57r; cIm[(k - 1)] += _add57i;
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_430_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr7i = 0; }
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_arr7r, _arr7i, _arr8r, _arr8i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_mul9r, _mul9i); _abs10i = 0;
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
        _c21r = 1.0; _c21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 3.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _c21r + _cos25r; _add26i = _c21i + _cos25i;
        double _add27r = 0, _add27i = 0;
        _add27r = _mul20r + _add26r; _add27i = _mul20i + _add26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _ang28r + _ang29r; _add30i = _ang28i + _ang29i;
        double _sin31r = 0, _sin31i = 0;
        c_sin(j, 0, &_sin31r, &_sin31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _add30r + _sin31r; _add32i = _add30i + _sin31i;
        double _cos33r = 0, _cos33i = 0;
        c_cos(j, 0, &_cos33r, &_cos33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _add32r - _cos33r; _sub34i = _add32i - _cos33i;
        double ang = _sub34r; /* +_sub34ii */
        double _cos35r = 0, _cos35i = 0;
        c_cos(ang, 0, &_cos35r, &_cos35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _sin37r = 0, _sin37i = 0;
        c_sin(ang, 0, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c36r, _c36i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _cos35r + _mul38r; _add39i = _cos35i + _mul38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(mag, 0, _add39r, _add39i, &_mul40r, &_mul40i);
        double _conj41r = 0, _conj41i = 0;
        _conj41r = x1r; _conj41i = -(x1i);
        double _re42r = 0, _re42i = 0;
        _re42r = x2r; _re42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_conj41r, _conj41i, _re42r, _re42i, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 2.0; _c44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(j, 0, _c44r, _c44i, &_div45r, &_div45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_div45r, _div45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_mul43r, _mul43i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul40r + _mul47r; _add48i = _mul40i + _mul47i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_431_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re1r + (_re2r - _re1r) * _li / 34.0;
    }
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im3r + (_im4r - _im3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(_arr7r, _arr7i, j, 0, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _arr10r = 0, _arr10i = 0;
        { int _idx = (j - 1); _arr10r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr10i = 0; }
        double _c11r = 0, _c11i = 0;
        _c11r = 1.5; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(j, 0, 1.5, &_pow12r, &_pow12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_arr10r, _arr10i, _pow12r, _pow12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _sin9r + _cos14r; _add15i = _sin9i + _cos14i;
        double term_real = _add15r; /* +_add15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs16r + j; _add17i = _abs16i + 0;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr19i = 0; }
        double _mul20r = 0, _mul20i = 0;
        c_mul(_arr19r, _arr19i, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_mul20r, _mul20i, _add22r, _add22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _log18r - _sin24r; _sub25i = _log18i - _sin24i;
        double term_imag = _sub25r; /* +_sub25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_mul(term_real, 0, term_real, 0, &_pow27r, &_pow27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(term_imag, 0, term_imag, 0, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _pow27r + _pow29r; _add30i = _pow27i + _pow29i;
        double _sqrt31r = 0, _sqrt31i = 0;
        c_powr(_add30r, _add30i, 0.5, &_sqrt31r, &_sqrt31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 0.1; _c33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c33r, _c33i, j, 0, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _c32r + _mul34r; _add35i = _c32i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_sqrt31r, _sqrt31i, _add35r, _add35i, &_mul36r, &_mul36i);
        double mag = _mul36r; /* +_mul36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x1r, x1i); _ang37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = j + _c38r; _add39i = 0 + _c38i;
        double _log40r = 0, _log40i = 0;
        c_log(_add39r, _add39i, &_log40r, &_log40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang37r, _ang37i, _log40r, _log40i, &_mul41r, &_mul41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, M_PI, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 7.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul41r + _cos45r; _add46i = _mul41i + _cos45i;
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
        c_mul(mag, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_432_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _re6r = 0, _re6i = 0;
        _re6r = x2r; _re6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _re6r, _re6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double mag_part1 = _mul9r; /* +_mul9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _im10r, _im10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 10.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(j, 0, _c15r, _c15i, &_div16r, &_div16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _c14r + _div16r; _add17i = _c14i + _div16i;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs13r, _abs13i, _add17r, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_cos12r, _cos12i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double mag_part2 = _mul19r; /* +_mul19ii */
        double _add20r = 0, _add20i = 0;
        _add20r = mag_part1 + mag_part2; _add20i = 0 + 0;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 5.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double angle_part1 = _mul25r; /* +_mul25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double angle_part2 = _mul30r; /* +_mul30ii */
        double _add31r = 0, _add31i = 0;
        _add31r = angle_part1 + angle_part2; _add31i = 0 + 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    double _c38r = 0, _c38i = 0;
    _c38r = 1.0; _c38i = 0;
    double _add39r = 0, _add39i = 0;
    _add39r = n + _c38r; _add39i = 0 + _c38i;
    for (int k = 1; k < (int)(_add39r); k++) {
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _re41r = 0, _re41i = 0;
        _re41r = x1r; _re41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_re41r, _re41i, k, 0, &_mul42r, &_mul42i);
        double _div43r = 0, _div43i = 0;
        c_div(_mul42r, _mul42i, n, 0, &_div43r, &_div43i);
        double _im44r = 0, _im44i = 0;
        _im44r = x2i; _im44i = 0;
        double _sub45r = 0, _sub45i = 0;
        _sub45r = n - k; _sub45i = 0 - 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_im44r, _im44i, _sub45r, _sub45i, &_mul46r, &_mul46i);
        double _div47r = 0, _div47i = 0;
        c_div(_mul46r, _mul46i, n, 0, &_div47r, &_div47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _div43r + _div47r; _add48i = _div43i + _div47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c40r, _c40i, _add48r, _add48i, &_mul49r, &_mul49i);
        double _exp50r = 0, _exp50i = 0;
        c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
        { double _tr = cRe[(k - 1)]*_exp50r - cIm[(k - 1)]*_exp50i; cIm[(k - 1)] = cRe[(k - 1)]*_exp50i + cIm[(k - 1)]*_exp50r; cRe[(k - 1)] = _tr; }
    }
    double _c51r = 0, _c51i = 0;
    _c51r = 1.0; _c51i = 0;
    double _add52r = 0, _add52i = 0;
    _add52r = n + _c51r; _add52i = 0 + _c51i;
    for (int r = 1; r < (int)(_add52r); r++) {
        double _conj53r = 0, _conj53i = 0;
        _conj53r = x1r; _conj53i = -(x1i);
        double _conj54r = 0, _conj54i = 0;
        _conj54r = x2r; _conj54i = -(x2i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_conj53r, _conj53i, _conj54r, _conj54i, &_mul55r, &_mul55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 1.0; _c56i = 0;
        double _add57r = 0, _add57i = 0;
        _add57r = r + _c56r; _add57i = 0 + _c56i;
        double _div58r = 0, _div58i = 0;
        c_div(_mul55r, _mul55i, _add57r, _add57i, &_div58r, &_div58i);
        cRe[(r - 1)] += _div58r; cIm[(r - 1)] += _div58i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_433_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_re3r, _re3i, j, &_pow4r, &_pow4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_pow4r, _pow4i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _re9r = 0, _re9i = 0;
        _re9r = x1r; _re9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _re9r, _re9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs12r + j; _add13i = _abs12i + 0;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin11r, _sin11i, _log14r, _log14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul8r + _mul15r; _add16i = _mul8i + _mul15i;
        double real_component = _add16r; /* +_add16ii */
        double _im17r = 0, _im17i = 0;
        _im17r = x1i; _im17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_im17r, _im17i, _log20r, _log20i, &_mul21r, &_mul21i);
        double _im22r = 0, _im22i = 0;
        _im22r = x2i; _im22i = 0;
        double _cos23r = 0, _cos23i = 0;
        c_cos(_im22r, _im22i, &_cos23r, &_cos23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 0.5; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_abs24r, _abs24i, 0.5, &_pow26r, &_pow26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_cos23r, _cos23i, _pow26r, _pow26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul21r + _mul27r; _add28i = _mul21i + _mul27i;
        double imag_component = _add28r; /* +_add28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, imag_component, 0, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = real_component + _mul30r; _add31i = 0 + _mul30i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_434_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _re1r + (_re2r - _re1r) * _li / 34.0;
    }
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _im3r + (_im4r - _im3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_arr7r, _arr7i); _abs8i = 0;
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (j - 1); _arr9r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr9i = 0; }
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_arr9r, _arr9i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs8r + _abs10r; _add11i = _abs8i + _abs10i;
        double _add12r = 0, _add12i = 0;
        _add12r = _add11r + j; _add12i = _add11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _abs15r; _add16i = _abs14i + _abs15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 10.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _pow20r = 0, _pow20i = 0;
        c_powr(_log17r, _log17i, _div19r, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log13r, _log13i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double magnitude = _mul21r; /* +_mul21ii */
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin24r, _sin24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double angle_part = _sin29r; /* +_sin29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _ang30r + _ang31r; _add32i = _ang30i + _ang31i;
        double _add33r = 0, _add33i = 0;
        _add33r = _add32r + angle_part; _add33i = _add32i + 0;
        double angle = _add33r; /* +_add33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, angle, 0, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(magnitude, 0, _exp36r, _exp36i, &_mul37r, &_mul37i);
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x1r; _conj38i = -(x1i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(j, 0, &_sin39r, &_sin39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 2.0; _c40i = 0;
        double _pow41r = 0, _pow41i = 0;
        c_mul(_sin39r, _sin39i, _sin39r, _sin39i, &_pow41r, &_pow41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj38r, _conj38i, _pow41r, _pow41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul37r + _mul42r; _add43i = _mul37i + _mul42i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_435_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs3r, _abs3i, j, 0, &_mul4r, &_mul4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = n - j; _sub6i = 0 - 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_abs5r, _abs5i, _sub6r, &_pow7r, &_pow7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _mul4r + _pow7r; _add8i = _mul4i + _pow7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _add8r + _c9r; _add10i = _add8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double mag_part = _log11r; /* +_log11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 7.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang12r, _ang12i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang18r, _ang18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul17r + _mul23r; _add24i = _mul17i + _mul23i;
        double angle_part = _add24r; /* +_add24ii */
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 0;
        double real_sum = _c25r; /* +_c25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        double imag_sum = _c26r; /* +_c26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        for (int k = 1; k < (int)(_add28r); k++) {
            double _re29r = 0, _re29i = 0;
            _re29r = x1r; _re29i = 0;
            double _pow30r = 0, _pow30i = 0;
            c_powr(_re29r, _re29i, k, &_pow30r, &_pow30i);
            double _re31r = 0, _re31i = 0;
            _re31r = x2r; _re31i = 0;
            double _sub32r = 0, _sub32i = 0;
            _sub32r = j - k; _sub32i = 0 - 0;
            double _pow33r = 0, _pow33i = 0;
            c_powr(_re31r, _re31i, _sub32r, &_pow33r, &_pow33i);
            double _mul34r = 0, _mul34i = 0;
            c_mul(_pow30r, _pow30i, _pow33r, _pow33i, &_mul34r, &_mul34i);
            real_sum += _mul34r;
            double _im35r = 0, _im35i = 0;
            _im35r = x1i; _im35i = 0;
            double _pow36r = 0, _pow36i = 0;
            c_powr(_im35r, _im35i, k, &_pow36r, &_pow36i);
            double _im37r = 0, _im37i = 0;
            _im37r = x2i; _im37i = 0;
            double _sub38r = 0, _sub38i = 0;
            _sub38r = j - k; _sub38i = 0 - 0;
            double _pow39r = 0, _pow39i = 0;
            c_powr(_im37r, _im37i, _sub38r, &_pow39r, &_pow39i);
            double _mul40r = 0, _mul40i = 0;
            c_mul(_pow36r, _pow36i, _pow39r, _pow39i, &_mul40r, &_mul40i);
            imag_sum += _mul40r;
        }
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = j + _c42r; _add43i = 0 + _c42i;
        double _div44r = 0, _div44i = 0;
        c_div(real_sum, 0, _add43r, _add43i, &_div44r, &_div44i);
        double _sin45r = 0, _sin45i = 0;
        c_sin(_div44r, _div44i, &_sin45r, &_sin45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _c41r + _sin45r; _add46i = _c41i + _sin45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(mag_part, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        double intricate_mag = _mul47r; /* +_mul47ii */
        double _c48r = 0, _c48i = 0;
        _c48r = 1.0; _c48i = 0;
        double _add49r = 0, _add49i = 0;
        _add49r = j + _c48r; _add49i = 0 + _c48i;
        double _div50r = 0, _div50i = 0;
        c_div(imag_sum, 0, _add49r, _add49i, &_div50r, &_div50i);
        double _cos51r = 0, _cos51i = 0;
        c_cos(_div50r, _div50i, &_cos51r, &_cos51i);
        double _add52r = 0, _add52i = 0;
        _add52r = angle_part + _cos51r; _add52i = 0 + _cos51i;
        double intricate_angle = _add52r; /* +_add52ii */
        double _cos53r = 0, _cos53i = 0;
        c_cos(intricate_angle, 0, &_cos53r, &_cos53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 1.0;
        double _sin55r = 0, _sin55i = 0;
        c_sin(intricate_angle, 0, &_sin55r, &_sin55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c54r, _c54i, _sin55r, _sin55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _cos53r + _mul56r; _add57i = _cos53i + _mul56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(intricate_mag, 0, _add57r, _add57i, &_mul58r, &_mul58i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_436_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 3.0; _c4i = 0;
        double _div5r = 0, _div5i = 0;
        c_div(j, 0, _c4r, _c4i, &_div5r, &_div5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_div5r, _div5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double mag_part1 = _mul7r; /* +_mul7ii */
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 4.0; _c10i = 0;
        double _mod11r = 0, _mod11i = 0;
        _mod11r = fmod(j, _c10r); _mod11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _c9r + _mod11r; _add12i = _c9i + _mod11i;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs8r, _abs8i, _add12r, &_pow13r, &_pow13i);
        double mag_part2 = _pow13r; /* +_pow13ii */
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(j, 0, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(mag_part2, 0, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = mag_part1 + _mul17r; _add18i = 0 + _mul17i;
        double magnitude = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _cos20r = 0, _cos20i = 0;
        c_cos(j, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double angle_part1 = _mul21r; /* +_mul21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(j, 0, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang22r, _ang22i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double angle_part2 = _mul26r; /* +_mul26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = angle_part1 + angle_part2; _add27i = 0 + 0;
        double _re28r = 0, _re28i = 0;
        _re28r = x1r; _re28i = 0;
        double _im29r = 0, _im29i = 0;
        _im29r = x2i; _im29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_re28r, _re28i, _im29r, _im29i, &_mul30r, &_mul30i);
        double _div31r = 0, _div31i = 0;
        c_div(_mul30r, _mul30i, j, 0, &_div31r, &_div31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _add27r + _div31r; _add32i = _add27i + _div31i;
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
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x1r; _conj39i = -(x1i);
        double _c40r = 0, _c40i = 0;
        _c40r = 5.0; _c40i = 0;
        double _mod41r = 0, _mod41i = 0;
        _mod41r = fmod(j, _c40r); _mod41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = _mod41r + _c42r; _add43i = _mod41i + _c42i;
        double _pow44r = 0, _pow44i = 0;
        c_powr(_conj39r, _conj39i, _add43r, &_pow44r, &_pow44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul38r + _pow44r; _add45i = _mul38i + _pow44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_437_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul17r = 0, _mul17i = 0;
        c_mul(r, 0, im, 0, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_mul17r, _mul17i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _ang25r, _ang25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sin24r + _cos27r; _add28i = _sin24i + _cos27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log21r, _log21i, _add28r, _add28i, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(r, 0, r, 0, &_pow31r, &_pow31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_mul(im, 0, im, 0, &_pow33r, &_pow33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _pow31r + _pow33r; _add34i = _pow31i + _pow33i;
        double _sqrt35r = 0, _sqrt35i = 0;
        c_powr(_add34r, _add34i, 0.5, &_sqrt35r, &_sqrt35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul29r + _sqrt35r; _add36i = _mul29i + _sqrt35i;
        double mag = _add36r; /* +_add36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x1r, x1i); _ang37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 3.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(j, 0, _c38r, _c38i, &_div39r, &_div39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang37r, _ang37i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x2r, x2i); _ang42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 4.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(j, 0, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang42r, _ang42i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _sub47r = 0, _sub47i = 0;
        _sub47r = _mul41r - _mul46r; _sub47i = _mul41i - _mul46i;
        double angle = _sub47r; /* +_sub47ii */
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c48r, _c48i, angle, 0, &_mul49r, &_mul49i);
        double _exp50r = 0, _exp50i = 0;
        c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(mag, 0, _exp50r, _exp50i, &_mul51r, &_mul51i);
        double _conj52r = 0, _conj52i = 0;
        _conj52r = x1r; _conj52i = -(x1i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(im, 0, M_PI, 0, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 5.0; _c54i = 0;
        double _div55r = 0, _div55i = 0;
        c_div(_mul53r, _mul53i, _c54r, _c54i, &_div55r, &_div55i);
        double _cos56r = 0, _cos56i = 0;
        c_cos(_div55r, _div55i, &_cos56r, &_cos56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_conj52r, _conj52i, _cos56r, _cos56i, &_mul57r, &_mul57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(r, 0, M_PI, 0, &_mul58r, &_mul58i);
        double _c59r = 0, _c59i = 0;
        _c59r = 7.0; _c59i = 0;
        double _div60r = 0, _div60i = 0;
        c_div(_mul58r, _mul58i, _c59r, _c59i, &_div60r, &_div60i);
        double _sin61r = 0, _sin61i = 0;
        c_sin(_div60r, _div60i, &_sin61r, &_sin61i);
        double _mul62r = 0, _mul62i = 0;
        c_mul(_mul57r, _mul57i, _sin61r, _sin61i, &_mul62r, &_mul62i);
        double _add63r = 0, _add63i = 0;
        _add63r = _mul51r + _mul62r; _add63i = _mul51i + _mul62i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add63r; cIm[_idx] = _add63i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_438_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _sin2r = 0, _sin2i = 0;
        c_sin(j, 0, &_sin2r, &_sin2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(_ang1r, _ang1i, _sin2r, _sin2i, &_mul3r, &_mul3i);
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x2r, x2i); _ang4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(j, 0, _c5r, _c5i, &_div6r, &_div6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_div6r, _div6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_ang4r, _ang4i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul3r + _mul8r; _add9i = _mul3i + _mul8i;
        double angle = _add9r; /* +_add9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 4.0; _c11i = 0;
        double _mod12r = 0, _mod12i = 0;
        _mod12r = fmod(j, _c11r); _mod12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _mod12r + _c13r; _add14i = _mod12i + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs10r, _abs10i, _add14r, &_pow15r, &_pow15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _sqrt20r = 0, _sqrt20i = 0;
        c_powr(j, 0, 0.5, &_sqrt20r, &_sqrt20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log19r, _log19i, _sqrt20r, _sqrt20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _pow15r + _mul21r; _add22i = _pow15i + _mul21i;
        double magnitude = _add22r; /* +_add22ii */
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
        c_mul(magnitude, 0, _add27r, _add27i, &_mul28r, &_mul28i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    for (int k = 1; k < 36; k++) {
        double _cf29r = 0, _cf29i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
        double _conj30r = 0, _conj30i = 0;
        _conj30r = x1r; _conj30i = -(x1i);
        double _pow31r = 0, _pow31i = 0;
        c_powr(_conj30r, _conj30i, k, &_pow31r, &_pow31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(k, 0, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_pow31r, _pow31i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _cf29r + _mul33r; _add34i = _cf29i + _mul33i;
        double _re35r = 0, _re35i = 0;
        _re35r = x2r; _re35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(k, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_re35r, _re35i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _sub40r = 0, _sub40i = 0;
        _sub40r = _add34r - _mul39r; _sub40i = _add34i - _mul39i;
        double _add41r = 0, _add41i = 0;
        _add41r = x1r + x2r; _add41i = x1i + x2i;
        double _abs42r = 0, _abs42i = 0;
        _abs42r = c_abs(_add41r, _add41i); _abs42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 5.0; _c44i = 0;
        double _mod45r = 0, _mod45i = 0;
        _mod45r = fmod(k, _c44r); _mod45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = _c43r + _mod45r; _add46i = _c43i + _mod45i;
        double _pow47r = 0, _pow47i = 0;
        c_powr(_abs42r, _abs42i, _add46r, &_pow47r, &_pow47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _sub40r + _pow47r; _add48i = _sub40i + _pow47i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_439_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
    for (int j = 0; j < (int)(n); j++) {
        double _arr5r = 0, _arr5i = 0;
        { int _idx = j; _arr5r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr5i = 0; }
        double r = _arr5r; /* +_arr5ii */
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr6i = 0; }
        double im = _arr6r; /* +_arr6ii */
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 1.0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(im, 0, _c7r, _c7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = r + _mul8r; _add9i = 0 + _mul8i;
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
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = j + _c15r; _add16i = 0 + _c15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_add16r, _add16i, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _c14r + _sin20r; _add21i = _c14i + _sin20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log13r, _log13i, _add21r, _add21i, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_add25r, _add25i, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _c23r + _cos29r; _add30i = _c23i + _cos29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_mul22r, _mul22i, _add30r, _add30i, &_mul31r, &_mul31i);
        double mag = _mul31r; /* +_mul31ii */
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(im, 0, _c32r, _c32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = r + _mul33r; _add34i = 0 + _mul33i;
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(_add34r, _add34i); _ang35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = j + _c36r; _add37i = 0 + _c36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_add37r, _add37i, M_PI, 0, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 5.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _ang35r + _sin41r; _add42i = _ang35i + _sin41i;
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = j + _c43r; _add44i = 0 + _c43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_add44r, _add44i, M_PI, 0, &_mul45r, &_mul45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 6.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(_mul45r, _mul45i, _c46r, _c46i, &_div47r, &_div47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_div47r, _div47i, &_cos48r, &_cos48i);
        double _sub49r = 0, _sub49i = 0;
        _sub49r = _add42r - _cos48r; _sub49i = _add42i - _cos48i;
        double angle = _sub49r; /* +_sub49ii */
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
        c_mul(mag, 0, _add54r, _add54i, &_mul55r, &_mul55i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_440_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_re3r, _re3i, j, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(j, 0); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_pow4r, _pow4i, _log8r, _log8i, &_mul9r, &_mul9i);
        double _re10r = 0, _re10i = 0;
        _re10r = x2r; _re10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _re10r, _re10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_mul(j, 0, j, 0, &_pow14r, &_pow14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_pow14r, _pow14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sin12r, _sin12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul9r + _mul16r; _add17i = _mul9i + _mul16i;
        double real_part = _add17r; /* +_add17ii */
        double _im18r = 0, _im18i = 0;
        _im18r = x1i; _im18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.5; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(j, 0, 0.5, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_im18r, _im18i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double _im22r = 0, _im22i = 0;
        _im22r = x2i; _im22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _im22r, _im22i, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double _add25r = 0, _add25i = 0;
        _add25r = x1r + x2r; _add25i = x1i + x2i;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_add25r, _add25i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_cos24r, _cos24i, _log29r, _log29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul21r + _mul30r; _add31i = _mul21i + _mul30i;
        double imag_part = _add31r; /* +_add31ii */
        double _cplx32r = 0, _cplx32i = 0;
        _cplx32r = real_part; _cplx32i = imag_part;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 0.1; _c34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, j, 0, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _c33r + _mul35r; _add36i = _c33i + _mul35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_cplx32r, _cplx32i, _add36r, _add36i, &_mul37r, &_mul37i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_441: too complex for auto-transpile, stubbed */
static void poly_441_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_442_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _abs3r + _pow5r; _add6i = _abs3i + _pow5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 4.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log7r, _log7i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 6.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul12r + _cos16r; _add17i = _mul12i + _cos16i;
        double mag_part = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _sin19r = 0, _sin19i = 0;
        c_sin(j, 0, &_sin19r, &_sin19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(j, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin19r, _sin19i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _ang18r + _mul23r; _add24i = _ang18i + _mul23i;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add24r + _log28r; _add29i = _add24i + _log28i;
        double angle_part = _add29r; /* +_add29ii */
        double _cos30r = 0, _cos30i = 0;
        c_cos(angle_part, 0, &_cos30r, &_cos30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(angle_part, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c31r, _c31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _cos30r + _mul33r; _add34i = _cos30i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(mag_part, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    double _c36r = 0, _c36i = 0;
    _c36r = 1.0; _c36i = 0;
    double _add37r = 0, _add37i = 0;
    _add37r = n + _c36r; _add37i = 0 + _c36i;
    for (int k = 1; k < (int)(_add37r); k++) {
        double _c38r = 0, _c38i = 0;
        _c38r = 2.0; _c38i = 0;
        double _mod39r = 0, _mod39i = 0;
        _mod39r = fmod(k, _c38r); _mod39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 0;
        if (_mod39r == _c40r) {
            double _cf41r = 0, _cf41i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
            double _c42r = 0, _c42i = 0;
            _c42r = 0.0; _c42i = 1.0;
            double _ang43r = 0, _ang43i = 0;
            _ang43r = c_arg(x1r, x1i); _ang43i = 0;
            double _mul44r = 0, _mul44i = 0;
            c_mul(_c42r, _c42i, _ang43r, _ang43i, &_mul44r, &_mul44i);
            double _mul45r = 0, _mul45i = 0;
            c_mul(_mul44r, _mul44i, k, 0, &_mul45r, &_mul45i);
            double _c46r = 0, _c46i = 0;
            _c46r = 10.0; _c46i = 0;
            double _div47r = 0, _div47i = 0;
            c_div(_mul45r, _mul45i, _c46r, _c46i, &_div47r, &_div47i);
            double _exp48r = 0, _exp48i = 0;
            c_exp2(_div47r, _div47i, &_exp48r, &_exp48i);
            double _mul49r = 0, _mul49i = 0;
            c_mul(_cf41r, _cf41i, _exp48r, _exp48i, &_mul49r, &_mul49i);
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
        } else {
            double _cf50r = 0, _cf50i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
            double _c51r = 0, _c51i = 0;
            _c51r = 0.0; _c51i = 1.0;
            double _neg52r = 0, _neg52i = 0;
            _neg52r = -(_c51r); _neg52i = -(_c51i);
            double _ang53r = 0, _ang53i = 0;
            _ang53r = c_arg(x2r, x2i); _ang53i = 0;
            double _mul54r = 0, _mul54i = 0;
            c_mul(_neg52r, _neg52i, _ang53r, _ang53i, &_mul54r, &_mul54i);
            double _mul55r = 0, _mul55i = 0;
            c_mul(_mul54r, _mul54i, k, 0, &_mul55r, &_mul55i);
            double _c56r = 0, _c56i = 0;
            _c56r = 15.0; _c56i = 0;
            double _div57r = 0, _div57i = 0;
            c_div(_mul55r, _mul55i, _c56r, _c56i, &_div57r, &_div57i);
            double _exp58r = 0, _exp58i = 0;
            c_exp2(_div57r, _div57i, &_exp58r, &_exp58i);
            double _mul59r = 0, _mul59i = 0;
            c_mul(_cf50r, _cf50i, _exp58r, _exp58i, &_mul59r, &_mul59i);
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
        }
    }
    double _c60r = 0, _c60i = 0;
    _c60r = 1.0; _c60i = 0;
    double _add61r = 0, _add61i = 0;
    _add61r = n + _c60r; _add61i = 0 + _c60i;
    for (int r = 1; r < (int)(_add61r); r++) {
        double _cf62r = 0, _cf62i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf62r = cRe[_idx]; _cf62i = cIm[_idx]; } }
        double _cf63r = 0, _cf63i = 0;
        { int _idx = ((int)(n) - r); if (_idx >= 0 && _idx < 36) { _cf63r = cRe[_idx]; _cf63i = cIm[_idx]; } }
        double _conj64r = 0, _conj64i = 0;
        _conj64r = _cf63r; _conj64i = -(_cf63i);
        double _abs65r = 0, _abs65i = 0;
        _abs65r = c_abs(x1r, x1i); _abs65i = 0;
        double _c66r = 0, _c66i = 0;
        _c66r = 1.0; _c66i = 0;
        double _add67r = 0, _add67i = 0;
        _add67r = r + _c66r; _add67i = 0 + _c66i;
        double _div68r = 0, _div68i = 0;
        c_div(_abs65r, _abs65i, _add67r, _add67i, &_div68r, &_div68i);
        double _mul69r = 0, _mul69i = 0;
        c_mul(_conj64r, _conj64i, _div68r, _div68i, &_mul69r, &_mul69i);
        double _add70r = 0, _add70i = 0;
        _add70r = _cf62r + _mul69r; _add70i = _cf62i + _mul69i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add70r; cIm[_idx] = _add70i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_443: too complex for auto-transpile, stubbed */
static void poly_443_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_444_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs3r, _abs3i, j, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = n - j; _sub6i = 0 - 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_abs5r, _abs5i, _sub6r, &_pow7r, &_pow7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow4r + _pow7r; _add8i = _pow4i + _pow7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sin12r, _sin12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _log9r + _mul16r; _add17i = _log9i + _mul16i;
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang18r, _ang18i, j, 0, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = n - j; _sub21i = 0 - 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang20r, _ang20i, _sub21r, _sub21i, &_mul22r, &_mul22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _mul19r - _mul22r; _sub23i = _mul19i - _mul22i;
        double _sin24r = 0, _sin24i = 0;
        c_sin(j, 0, &_sin24r, &_sin24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _sub23r + _sin24r; _add25i = _sub23i + _sin24i;
        double _cos26r = 0, _cos26i = 0;
        c_cos(j, 0, &_cos26r, &_cos26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _add25r - _cos26r; _sub27i = _add25i - _cos26i;
        double angle = _sub27r; /* +_sub27ii */
        double _cos28r = 0, _cos28i = 0;
        c_cos(angle, 0, &_cos28r, &_cos28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(angle, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c29r, _c29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _cos28r + _mul31r; _add32i = _cos28i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(mag, 0, _add32r, _add32i, &_mul33r, &_mul33i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_445_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _c3r, _c3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 12.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(_add6r, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
        double k = _add10r; /* +_add10ii */
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _div13r = 0, _div13i = 0;
        c_div(_mul12r, _mul12i, k, 0, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr11r, _attr11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = k + _c18r; _add19i = 0 + _c18i;
        double _div20r = 0, _div20i = 0;
        c_div(_mul17r, _mul17i, _add19r, _add19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_attr16r, _attr16i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul15r + _mul22r; _add23i = _mul15i + _mul22i;
        double r_part = _add23r; /* +_add23ii */
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1i; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _div26r = 0, _div26i = 0;
        c_div(_mul25r, _mul25i, k, 0, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_attr24r, _attr24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = k + _c31r; _add32i = 0 + _c31i;
        double _div33r = 0, _div33i = 0;
        c_div(_mul30r, _mul30i, _add32r, _add32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_attr29r, _attr29i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _sub36r = 0, _sub36i = 0;
        _sub36r = _mul28r - _mul35r; _sub36i = _mul28i - _mul35i;
        double i_part = _sub36r; /* +_sub36ii */
        double _abs37r = 0, _abs37i = 0;
        _abs37r = c_abs(x1r, x1i); _abs37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _abs37r + j; _add38i = _abs37i + 0;
        double _log39r = 0, _log39i = 0;
        c_log(_add38r, _add38i, &_log39r, &_log39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, M_PI, 0, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 10.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(_mul40r, _mul40i, _c41r, _c41i, &_div42r, &_div42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_div42r, _div42i, &_sin43r, &_sin43i);
        double _abs44r = 0, _abs44i = 0;
        _abs44r = c_abs(_sin43r, _sin43i); _abs44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_log39r, _log39i, _abs44r, _abs44i, &_mul45r, &_mul45i);
        double magnitude = _mul45r; /* +_mul45ii */
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x1r, x1i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(j, 0, M_PI, 0, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 8.0; _c48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(_mul47r, _mul47i, _c48r, _c48i, &_div49r, &_div49i);
        double _cos50r = 0, _cos50i = 0;
        c_cos(_div49r, _div49i, &_cos50r, &_cos50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_ang46r, _ang46i, _cos50r, _cos50i, &_mul51r, &_mul51i);
        double _ang52r = 0, _ang52i = 0;
        _ang52r = c_arg(x2r, x2i); _ang52i = 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(j, 0, M_PI, 0, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 9.0; _c54i = 0;
        double _div55r = 0, _div55i = 0;
        c_div(_mul53r, _mul53i, _c54r, _c54i, &_div55r, &_div55i);
        double _sin56r = 0, _sin56i = 0;
        c_sin(_div55r, _div55i, &_sin56r, &_sin56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_ang52r, _ang52i, _sin56r, _sin56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _mul51r + _mul57r; _add58i = _mul51i + _mul57i;
        double angle = _add58r; /* +_add58ii */
        double _c59r = 0, _c59i = 0;
        _c59r = 0.0; _c59i = 1.0;
        double _mul60r = 0, _mul60i = 0;
        c_mul(_c59r, _c59i, i_part, 0, &_mul60r, &_mul60i);
        double _add61r = 0, _add61i = 0;
        _add61r = r_part + _mul60r; _add61i = 0 + _mul60i;
        double _mul62r = 0, _mul62i = 0;
        c_mul(magnitude, 0, _add61r, _add61i, &_mul62r, &_mul62i);
        double _cos63r = 0, _cos63i = 0;
        c_cos(angle, 0, &_cos63r, &_cos63i);
        double _c64r = 0, _c64i = 0;
        _c64r = 0.0; _c64i = 1.0;
        double _sin65r = 0, _sin65i = 0;
        c_sin(angle, 0, &_sin65r, &_sin65i);
        double _mul66r = 0, _mul66i = 0;
        c_mul(_c64r, _c64i, _sin65r, _sin65i, &_mul66r, &_mul66i);
        double _add67r = 0, _add67i = 0;
        _add67r = _cos63r + _mul66r; _add67i = _cos63i + _mul66i;
        double _mul68r = 0, _mul68i = 0;
        c_mul(_mul62r, _mul62i, _add67r, _add67i, &_mul68r, &_mul68i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul68r; cIm[_idx] = _mul68i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_446_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_abs1r, _abs1i, j, 0, &_mul2r, &_mul2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x2r, x2i); _abs3i = 0;
        double _div4r = 0, _div4i = 0;
        c_div(_abs3r, _abs3i, j, 0, &_div4r, &_div4i);
        double _add5r = 0, _add5i = 0;
        _add5r = _mul2r + _div4r; _add5i = _mul2i + _div4i;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _add5r + _c6r; _add7i = _add5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 4.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _c9r + _sin13r; _add14i = _c9i + _sin13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log8r, _log8i, _add14r, _add14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1r; _attr16i = 0;
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x2i; _attr17i = 0;
        double _prod18r = 0, _prod18i = 0;
        c_mul(_attr16r, _attr16i, _attr17r, _attr17i, &_prod18r, &_prod18i);
        double _prod19r = 0, _prod19i = 0;
        c_mul(_prod18r, _prod18i, j, 0, &_prod19r, &_prod19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul15r + _prod19r; _add20i = _mul15i + _prod19i;
        double mag = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang21r, _ang21i, j, 0, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 5.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang23r, _ang23i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul22r + _mul28r; _add29i = _mul22i + _mul28i;
        double _sin30r = 0, _sin30i = 0;
        c_sin(j, 0, &_sin30r, &_sin30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(_sin30r, _sin30i, _sin30r, _sin30i, &_pow32r, &_pow32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _add29r + _pow32r; _add33i = _add29i + _pow32i;
        double angle = _add33r; /* +_add33ii */
        double _cos34r = 0, _cos34i = 0;
        c_cos(angle, 0, &_cos34r, &_cos34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(angle, 0, &_sin35r, &_sin35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sin35r, _sin35i, _c36r, _c36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cos34r + _mul37r; _add38i = _cos34i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_447: too complex for auto-transpile, stubbed */
static void poly_447_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_448_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x2r; _attr4i = 0;
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = _attr4r - _attr5r; _sub6i = _attr4i - _attr5i;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_sub6r, _sub6i, j, 0, &_mul7r, &_mul7i);
        double _div8r = 0, _div8i = 0;
        c_div(_mul7r, _mul7i, n, 0, &_div8r, &_div8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _attr3r + _div8r; _add9i = _attr3i + _div8i;
        double rec = _add9r; /* +_add9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2i; _attr11i = 0;
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1i; _attr12i = 0;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _attr11r - _attr12r; _sub13i = _attr11i - _attr12i;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sub13r, _sub13i, j, 0, &_mul14r, &_mul14i);
        double _div15r = 0, _div15i = 0;
        c_div(_mul14r, _mul14i, n, 0, &_div15r, &_div15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _attr10r + _div15r; _add16i = _attr10i + _div15i;
        double imc = _add16r; /* +_add16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _abs18r; _add19i = _abs17i + _abs18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_mul(j, 0, j, 0, &_pow21r, &_pow21i);
        c_mul(_pow21r, _pow21i, j, 0, &_pow21r, &_pow21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _add19r + _pow21r; _add22i = _add19i + _pow21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 5.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _c24r + _sin28r; _add29i = _c24i + _sin28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_log23r, _log23i, _add29r, _add29i, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 4.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _c31r + _cos35r; _add36i = _c31i + _cos35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_mul30r, _mul30i, _add36r, _add36i, &_mul37r, &_mul37i);
        double mag = _mul37r; /* +_mul37ii */
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, M_PI, 0, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 3.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_mul39r, _mul39i, _c40r, _c40i, &_div41r, &_div41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_div41r, _div41i, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang38r, _ang38i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(j, 0, M_PI, 0, &_mul45r, &_mul45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 4.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(_mul45r, _mul45i, _c46r, _c46i, &_div47r, &_div47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_div47r, _div47i, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_ang44r, _ang44i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul43r + _mul49r; _add50i = _mul43i + _mul49i;
        double _mul51r = 0, _mul51i = 0;
        c_mul(j, 0, rec, 0, &_mul51r, &_mul51i);
        double _sin52r = 0, _sin52i = 0;
        c_sin(_mul51r, _mul51i, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(j, 0, imc, 0, &_mul53r, &_mul53i);
        double _cos54r = 0, _cos54i = 0;
        c_cos(_mul53r, _mul53i, &_cos54r, &_cos54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_sin52r, _sin52i, _cos54r, _cos54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _add50r + _mul55r; _add56i = _add50i + _mul55i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul62r; cIm[_idx] = _mul62i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_449: too complex for auto-transpile, stubbed */
static void poly_449_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_450_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x1r, x1i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _mul4r + _cos7r; _add8i = _mul4i + _cos7i;
        double _sin9r = 0, _sin9i = 0;
        c_sin(_add8r, _add8i, &_sin9r, &_sin9i);
        double angle_part = _sin9r; /* +_sin9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _abs11r; _add12i = _abs10i + _abs11i;
        double _add13r = 0, _add13i = 0;
        _add13r = _add12r + j; _add13i = _add12i + 0;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.1; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, j, 0, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _c15r + _mul17r; _add18i = _c15i + _mul17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log14r, _log14i, _add18r, _add18i, &_mul19r, &_mul19i);
        double mag_part = _mul19r; /* +_mul19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(j, 0, n, 0, &_div21r, &_div21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_div21r, _div21i, M_PI, 0, &_mul22r, &_mul22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, n, 0, &_div26r, &_div26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_div26r, _div26i, M_PI, 0, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul24r - _mul29r; _sub30i = _mul24i - _mul29i;
        double phase_shift = _sub30r; /* +_sub30ii */
        double _add31r = 0, _add31i = 0;
        _add31r = angle_part + phase_shift; _add31i = 0 + 0;
        double _cos32r = 0, _cos32i = 0;
        c_cos(_add31r, _add31i, &_cos32r, &_cos32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _sub34r = 0, _sub34i = 0;
        _sub34r = angle_part - phase_shift; _sub34i = 0 - 0;
        double _sin35r = 0, _sin35i = 0;
        c_sin(_sub34r, _sub34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c33r, _c33i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _cos32r + _mul36r; _add37i = _cos32i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(mag_part, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int k = 1; k < 6; k++) {
        double _sub39r = 0, _sub39i = 0;
        _sub39r = n - k; _sub39i = 0 - 0;
        double idx = _sub39r; /* +_sub39ii */
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 0;
        if (idx >= _c40r) {
            double _cf41r = 0, _cf41i = 0;
            { int _idx = (int)(idx); if (_idx >= 0 && _idx < 36) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
            double _conj42r = 0, _conj42i = 0;
            _conj42r = x1r; _conj42i = -(x1i);
            double _c43r = 0, _c43i = 0;
            _c43r = 3.0; _c43i = 0;
            double _mod44r = 0, _mod44i = 0;
            _mod44r = fmod(k, _c43r); _mod44i = 0;
            double _c45r = 0, _c45i = 0;
            _c45r = 1.0; _c45i = 0;
            double _add46r = 0, _add46i = 0;
            _add46r = _mod44r + _c45r; _add46i = _mod44i + _c45i;
            double _pow47r = 0, _pow47i = 0;
            c_powr(_conj42r, _conj42i, _add46r, &_pow47r, &_pow47i);
            double _mul48r = 0, _mul48i = 0;
            c_mul(_cf41r, _cf41i, _pow47r, _pow47i, &_mul48r, &_mul48i);
            double _conj49r = 0, _conj49i = 0;
            _conj49r = x2r; _conj49i = -(x2i);
            double _c50r = 0, _c50i = 0;
            _c50r = 4.0; _c50i = 0;
            double _mod51r = 0, _mod51i = 0;
            _mod51r = fmod(k, _c50r); _mod51i = 0;
            double _pow52r = 0, _pow52i = 0;
            c_powr(_conj49r, _conj49i, _mod51r, &_pow52r, &_pow52i);
            double _add53r = 0, _add53i = 0;
            _add53r = _mul48r + _pow52r; _add53i = _mul48i + _pow52i;
            { int _idx = (int)(idx); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add53r; cIm[_idx] = _add53i; } }
        }
    }
    double cumulative = 1.0;
    double _c54r = 0, _c54i = 0;
    _c54r = 1.0; _c54i = 0;
    double _add55r = 0, _add55i = 0;
    _add55r = n + _c54r; _add55i = 0 + _c54i;
    for (int r = 1; r < (int)(_add55r); r++) {
        double _abs56r = 0, _abs56i = 0;
        _abs56r = c_abs(x1r, x1i); _abs56i = 0;
        double _abs57r = 0, _abs57i = 0;
        _abs57r = c_abs(x2r, x2i); _abs57i = 0;
        double _add58r = 0, _add58i = 0;
        _add58r = _abs56r + _abs57r; _add58i = _abs56i + _abs57i;
        double _add59r = 0, _add59i = 0;
        _add59r = _add58r + r; _add59i = _add58i + 0;
        cumulative *= _add59r;
        double _c60r = 0, _c60i = 0;
        _c60r = 1.0; _c60i = 0;
        double _add61r = 0, _add61i = 0;
        _add61r = r + _c60r; _add61i = 0 + _c60i;
        double _div62r = 0, _div62i = 0;
        c_div(cumulative, 0, _add61r, _add61i, &_div62r, &_div62i);
        cRe[(r - 1)] += _div62r; cIm[(r - 1)] += _div62i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_451_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _sqrt6r = 0, _sqrt6i = 0;
        c_powr(j, 0, 0.5, &_sqrt6r, &_sqrt6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log5r, _log5i, _sqrt6r, _sqrt6i, &_mul7r, &_mul7i);
        double mag_part1 = _mul7r; /* +_mul7ii */
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _c9r + _sin10r; _add11i = _c9i + _sin10i;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_abs8r, _abs8i, _add11r, &_pow12r, &_pow12i);
        double mag_part2 = _pow12r; /* +_pow12ii */
        double _add13r = 0, _add13i = 0;
        _add13r = mag_part1 + mag_part2; _add13i = 0 + 0;
        double magnitude = _add13r; /* +_add13ii */
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1r; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x2i; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _attr17r, _attr17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _sin16r + _cos19r; _add20i = _sin16i + _cos19i;
        double phase_part1 = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang21r, _ang21i, j, 0, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _sub24r = 0, _sub24i = 0;
        _sub24r = n - j; _sub24i = 0 - 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _sub24r, _sub24i, &_mul25r, &_mul25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul22r - _mul25r; _sub26i = _mul22i - _mul25i;
        double phase_part2 = _sub26r; /* +_sub26ii */
        double _mul27r = 0, _mul27i = 0;
        c_mul(phase_part1, 0, phase_part2, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 3.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(j, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin30r, _sin30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul27r + _mul34r; _add35i = _mul27i + _mul34i;
        double phase = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, phase, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_452_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double temp = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        for (int j = 1; j < (int)(_add5r); j++) {
            double _attr6r = 0, _attr6i = 0;
            _attr6r = x1r; _attr6i = 0;
            double _mul7r = 0, _mul7i = 0;
            c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
            double _sin8r = 0, _sin8i = 0;
            c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
            double _attr9r = 0, _attr9i = 0;
            _attr9r = x2i; _attr9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
            double _cos11r = 0, _cos11i = 0;
            c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_sin8r, _sin8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
            double _div13r = 0, _div13i = 0;
            c_div(_mul12r, _mul12i, j, 0, &_div13r, &_div13i);
            temp += _div13r;
        }
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_abs14r, _abs14i, _abs15r, _abs15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul16r + k; _add17i = _mul16i + 0;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(k, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_div21r, _div21i, &_sin22r, &_sin22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 3.0; _c23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sin22r, _sin22i, _c23r, _c23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _c19r + _mul24r; _add25i = _c19i + _mul24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log18r, _log18i, _add25r, _add25i, &_mul26r, &_mul26i);
        double magnitude = _mul26r; /* +_mul26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = k + _c29r; _add30i = 0 + _c29i;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _log31r, _log31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _ang27r + _mul32r; _add33i = _ang27i + _mul32i;
        double _add34r = 0, _add34i = 0;
        _add34r = _add33r + temp; _add34i = _add33i + 0;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_453_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_attr3r, _attr3i, j, 0, &_mul4r, &_mul4i);
        double _div5r = 0, _div5i = 0;
        c_div(_mul4r, _mul4i, n, 0, &_div5r, &_div5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = n - j; _sub7i = 0 - 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _sub7r + _c8r; _add9i = _sub7i + _c8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr6r, _attr6i, _add9r, _add9i, &_mul10r, &_mul10i);
        double _div11r = 0, _div11i = 0;
        c_div(_mul10r, _mul10i, n, 0, &_div11r, &_div11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _div5r + _div11r; _add12i = _div5i + _div11i;
        double r_part = _add12r; /* +_add12ii */
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _div14r = 0, _div14i = 0;
        c_div(_mul13r, _mul13i, n, 0, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1i; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_sin15r, _sin15i, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _div19r = 0, _div19i = 0;
        c_div(_mul18r, _mul18i, n, 0, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2i; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_cos20r, _cos20i, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _mul17r - _mul22r; _sub23i = _mul17i - _mul22i;
        double i_part = _sub23r; /* +_sub23ii */
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs24r + j; _add25i = _abs24i + 0;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 3.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(j, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(_sin30r, _sin30i); _abs31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _c27r + _abs31r; _add32i = _c27i + _abs31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_log26r, _log26i, _add32r, _add32i, &_mul33r, &_mul33i);
        double magnitude = _mul33r; /* +_mul33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _cos35r = 0, _cos35i = 0;
        c_cos(j, 0, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang34r, _ang34i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(j, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang37r, _ang37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul36r + _mul39r; _add40i = _mul36i + _mul39i;
        double angle = _add40r; /* +_add40ii */
        double _cos41r = 0, _cos41i = 0;
        c_cos(angle, 0, &_cos41r, &_cos41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(angle, 0, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c42r, _c42i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos41r + _mul44r; _add45i = _cos41i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_454: too complex for auto-transpile, stubbed */
static void poly_454_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_455_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs1r + _c2r; _add3i = _abs1i + _c2i;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2r; _attr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _attr5r, _attr5i, &_mul6r, &_mul6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log4r, _log4i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1i; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_cos11r, _cos11i, _cos11r, _cos11i, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _pow13r; _add14i = _mul8i + _pow13i;
        double mag = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang15r, _ang15i, j, 0, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _div20r = 0, _div20i = 0;
        c_div(_ang17r, _ang17i, _add19r, _add19i, &_div20r, &_div20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul16r + _div20r; _add21i = _mul16i + _div20i;
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x1r; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _attr22r, _attr22i, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul23r, _mul23i, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add21r + _sin26r; _add27i = _add21i + _sin26i;
        double angle = _add27r; /* +_add27ii */
        double _cos28r = 0, _cos28i = 0;
        c_cos(angle, 0, &_cos28r, &_cos28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(angle, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c29r, _c29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _cos28r + _mul31r; _add32i = _cos28i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(mag, 0, _add32r, _add32i, &_mul33r, &_mul33i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int k = 1; k < 36; k++) {
        double _cf34r = 0, _cf34i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 0.05; _c36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, k, 0, &_mul37r, &_mul37i);
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x1r; _attr38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_mul37r, _mul37i, _attr38r, _attr38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _c35r + _mul39r; _add40i = _c35i + _mul39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_cf34r, _cf34i, _add40r, _add40i, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 0.03; _c43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, k, 0, &_mul44r, &_mul44i);
        double _attr45r = 0, _attr45i = 0;
        _attr45r = x2i; _attr45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_mul44r, _mul44i, _attr45r, _attr45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _c42r + _mul46r; _add47i = _c42i + _mul46i;
        double _div48r = 0, _div48i = 0;
        c_div(_mul41r, _mul41i, _add47r, _add47i, &_div48r, &_div48i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _div48r; cIm[_idx] = _div48i; } }
        double _cf49r = 0, _cf49i = 0;
        { int _idx = (35 - k); if (_idx >= 0 && _idx < 35) { _cf49r = cRe[_idx]; _cf49i = cIm[_idx]; } }
        double _conj50r = 0, _conj50i = 0;
        _conj50r = _cf49r; _conj50i = -(_cf49i);
        double _c51r = 0, _c51i = 0;
        _c51r = 0.1; _c51i = 0;
        double _neg52r = 0, _neg52i = 0;
        _neg52r = -(_c51r); _neg52i = -(_c51i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_neg52r, _neg52i, k, 0, &_mul53r, &_mul53i);
        double _exp54r = 0, _exp54i = 0;
        c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_conj50r, _conj50i, _exp54r, _exp54i, &_mul55r, &_mul55i);
        cRe[(k - 1)] += _mul55r; cIm[(k - 1)] += _mul55i;
    }
    for (int r = 1; r < 36; r++) {
        double _cf56r = 0, _cf56i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 35) { _cf56r = cRe[_idx]; _cf56i = cIm[_idx]; } }
        double _abs57r = 0, _abs57i = 0;
        _abs57r = c_abs(_cf56r, _cf56i); _abs57i = 0;
        double _mul58r = 0, _mul58i = 0;
        c_mul(r, 0, M_PI, 0, &_mul58r, &_mul58i);
        double _c59r = 0, _c59i = 0;
        _c59r = 17.0; _c59i = 0;
        double _div60r = 0, _div60i = 0;
        c_div(_mul58r, _mul58i, _c59r, _c59i, &_div60r, &_div60i);
        double _sin61r = 0, _sin61i = 0;
        c_sin(_div60r, _div60i, &_sin61r, &_sin61i);
        double _pow62r = 0, _pow62i = 0;
        c_powr(_abs57r, _abs57i, _sin61r, &_pow62r, &_pow62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(r, 0, M_PI, 0, &_mul63r, &_mul63i);
        double _c64r = 0, _c64i = 0;
        _c64r = 23.0; _c64i = 0;
        double _div65r = 0, _div65i = 0;
        c_div(_mul63r, _mul63i, _c64r, _c64i, &_div65r, &_div65i);
        double _cos66r = 0, _cos66i = 0;
        c_cos(_div65r, _div65i, &_cos66r, &_cos66i);
        double _c67r = 0, _c67i = 0;
        _c67r = 0.0; _c67i = 1.0;
        double _mul68r = 0, _mul68i = 0;
        c_mul(r, 0, M_PI, 0, &_mul68r, &_mul68i);
        double _c69r = 0, _c69i = 0;
        _c69r = 23.0; _c69i = 0;
        double _div70r = 0, _div70i = 0;
        c_div(_mul68r, _mul68i, _c69r, _c69i, &_div70r, &_div70i);
        double _sin71r = 0, _sin71i = 0;
        c_sin(_div70r, _div70i, &_sin71r, &_sin71i);
        double _mul72r = 0, _mul72i = 0;
        c_mul(_c67r, _c67i, _sin71r, _sin71i, &_mul72r, &_mul72i);
        double _add73r = 0, _add73i = 0;
        _add73r = _cos66r + _mul72r; _add73i = _cos66i + _mul72i;
        double _mul74r = 0, _mul74i = 0;
        c_mul(_pow62r, _pow62i, _add73r, _add73i, &_mul74r, &_mul74i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul74r; cIm[_idx] = _mul74i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_456_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double ang = 0;
        double mag = 0;
        double _c1r = 0, _c1i = 0;
        _c1r = 10.0; _c1i = 0;
        if (j <= _c1r) {
            double _abs2r = 0, _abs2i = 0;
            _abs2r = c_abs(x1r, x1i); _abs2i = 0;
            double _c3r = 0, _c3i = 0;
            _c3r = 2.0; _c3i = 0;
            double _pow4r = 0, _pow4i = 0;
            c_mul(j, 0, j, 0, &_pow4r, &_pow4i);
            double _add5r = 0, _add5i = 0;
            _add5r = _abs2r + _pow4r; _add5i = _abs2i + _pow4i;
            double _log6r = 0, _log6i = 0;
            c_log(_add5r, _add5i, &_log6r, &_log6i);
            double _attr7r = 0, _attr7i = 0;
            _attr7r = x2r; _attr7i = 0;
            double _mul8r = 0, _mul8i = 0;
            c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
            double _sin9r = 0, _sin9i = 0;
            c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
            double _attr10r = 0, _attr10i = 0;
            _attr10r = x1i; _attr10i = 0;
            double _mul11r = 0, _mul11i = 0;
            c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
            double _cos12r = 0, _cos12i = 0;
            c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
            double _add13r = 0, _add13i = 0;
            _add13r = _sin9r + _cos12r; _add13i = _sin9i + _cos12i;
            double _abs14r = 0, _abs14i = 0;
            _abs14r = c_abs(_add13r, _add13i); _abs14i = 0;
            double _mul15r = 0, _mul15i = 0;
            c_mul(_log6r, _log6i, _abs14r, _abs14i, &_mul15r, &_mul15i);
            mag = _mul15r;
            double _ang16r = 0, _ang16i = 0;
            _ang16r = c_arg(x1r, x1i); _ang16i = 0;
            double _pow17r = 0, _pow17i = 0;
            c_powr(_ang16r, _ang16i, j, &_pow17r, &_pow17i);
            double _ang18r = 0, _ang18i = 0;
            _ang18r = c_arg(x2r, x2i); _ang18i = 0;
            double _sin19r = 0, _sin19i = 0;
            c_sin(j, 0, &_sin19r, &_sin19i);
            double _mul20r = 0, _mul20i = 0;
            c_mul(_ang18r, _ang18i, _sin19r, _sin19i, &_mul20r, &_mul20i);
            double _add21r = 0, _add21i = 0;
            _add21r = _pow17r + _mul20r; _add21i = _pow17i + _mul20i;
            ang = _add21r;
        } else {
            double _c22r = 0, _c22i = 0;
            _c22r = 20.0; _c22i = 0;
            if (j <= _c22r) {
                double _abs23r = 0, _abs23i = 0;
                _abs23r = c_abs(x2r, x2i); _abs23i = 0;
                double _c24r = 0, _c24i = 0;
                _c24r = 1.5; _c24i = 0;
                double _pow25r = 0, _pow25i = 0;
                c_powr(j, 0, 1.5, &_pow25r, &_pow25i);
                double _add26r = 0, _add26i = 0;
                _add26r = _abs23r + _pow25r; _add26i = _abs23i + _pow25i;
                double _log27r = 0, _log27i = 0;
                c_log(_add26r, _add26i, &_log27r, &_log27i);
                double _attr28r = 0, _attr28i = 0;
                _attr28r = x1r; _attr28i = 0;
                double _mul29r = 0, _mul29i = 0;
                c_mul(j, 0, _attr28r, _attr28i, &_mul29r, &_mul29i);
                double _cos30r = 0, _cos30i = 0;
                c_cos(_mul29r, _mul29i, &_cos30r, &_cos30i);
                double _attr31r = 0, _attr31i = 0;
                _attr31r = x2i; _attr31i = 0;
                double _mul32r = 0, _mul32i = 0;
                c_mul(j, 0, _attr31r, _attr31i, &_mul32r, &_mul32i);
                double _sin33r = 0, _sin33i = 0;
                c_sin(_mul32r, _mul32i, &_sin33r, &_sin33i);
                double _sub34r = 0, _sub34i = 0;
                _sub34r = _cos30r - _sin33r; _sub34i = _cos30i - _sin33i;
                double _abs35r = 0, _abs35i = 0;
                _abs35r = c_abs(_sub34r, _sub34i); _abs35i = 0;
                double _mul36r = 0, _mul36i = 0;
                c_mul(_log27r, _log27i, _abs35r, _abs35i, &_mul36r, &_mul36i);
                mag = _mul36r;
                double _ang37r = 0, _ang37i = 0;
                _ang37r = c_arg(x2r, x2i); _ang37i = 0;
                double _pow38r = 0, _pow38i = 0;
                c_powr(_ang37r, _ang37i, j, &_pow38r, &_pow38i);
                double _ang39r = 0, _ang39i = 0;
                _ang39r = c_arg(x1r, x1i); _ang39i = 0;
                double _c40r = 0, _c40i = 0;
                _c40r = 1.0; _c40i = 0;
                double _add41r = 0, _add41i = 0;
                _add41r = j + _c40r; _add41i = 0 + _c40i;
                double _log42r = 0, _log42i = 0;
                c_log(_add41r, _add41i, &_log42r, &_log42i);
                double _mul43r = 0, _mul43i = 0;
                c_mul(_ang39r, _ang39i, _log42r, _log42i, &_mul43r, &_mul43i);
                double _sub44r = 0, _sub44i = 0;
                _sub44r = _pow38r - _mul43r; _sub44i = _pow38i - _mul43i;
                ang = _sub44r;
            } else {
                double _mul45r = 0, _mul45i = 0;
                c_mul(x1r, x1i, x2r, x2i, &_mul45r, &_mul45i);
                double _abs46r = 0, _abs46i = 0;
                _abs46r = c_abs(_mul45r, _mul45i); _abs46i = 0;
                double _add47r = 0, _add47i = 0;
                _add47r = _abs46r + j; _add47i = _abs46i + 0;
                double _log48r = 0, _log48i = 0;
                c_log(_add47r, _add47i, &_log48r, &_log48i);
                double _attr49r = 0, _attr49i = 0;
                _attr49r = x1r; _attr49i = 0;
                double _mul50r = 0, _mul50i = 0;
                c_mul(j, 0, _attr49r, _attr49i, &_mul50r, &_mul50i);
                double _attr51r = 0, _attr51i = 0;
                _attr51r = x2r; _attr51i = 0;
                double _mul52r = 0, _mul52i = 0;
                c_mul(j, 0, _attr51r, _attr51i, &_mul52r, &_mul52i);
                double _cos53r = 0, _cos53i = 0;
                c_cos(_mul52r, _mul52i, &_cos53r, &_cos53i);
                double _add54r = 0, _add54i = 0;
                _add54r = _mul50r + _cos53r; _add54i = _mul50i + _cos53i;
                double _sin55r = 0, _sin55i = 0;
                c_sin(_add54r, _add54i, &_sin55r, &_sin55i);
                double _abs56r = 0, _abs56i = 0;
                _abs56r = c_abs(_sin55r, _sin55i); _abs56i = 0;
                double _mul57r = 0, _mul57i = 0;
                c_mul(_log48r, _log48i, _abs56r, _abs56i, &_mul57r, &_mul57i);
                mag = _mul57r;
                double _conj58r = 0, _conj58i = 0;
                _conj58r = x2r; _conj58i = -(x2i);
                double _add59r = 0, _add59i = 0;
                _add59r = x1r + _conj58r; _add59i = x1i + _conj58i;
                double _ang60r = 0, _ang60i = 0;
                _ang60r = c_arg(_add59r, _add59i); _ang60i = 0;
                double _mul61r = 0, _mul61i = 0;
                c_mul(_ang60r, _ang60i, j, 0, &_mul61r, &_mul61i);
                double _sub62r = 0, _sub62i = 0;
                _sub62r = x1r - x2r; _sub62i = x1i - x2i;
                double _abs63r = 0, _abs63i = 0;
                _abs63r = c_abs(_sub62r, _sub62i); _abs63i = 0;
                double _c64r = 0, _c64i = 0;
                _c64r = 1.0; _c64i = 0;
                double _add65r = 0, _add65i = 0;
                _add65r = _abs63r + _c64r; _add65i = _abs63i + _c64i;
                double _log66r = 0, _log66i = 0;
                c_log(_add65r, _add65i, &_log66r, &_log66i);
                double _add67r = 0, _add67i = 0;
                _add67r = _mul61r + _log66r; _add67i = _mul61i + _log66i;
                ang = _add67r;
            }
        }
        double _c68r = 0, _c68i = 0;
        _c68r = 0.0; _c68i = 1.0;
        double _mul69r = 0, _mul69i = 0;
        c_mul(_c68r, _c68i, ang, 0, &_mul69r, &_mul69i);
        double _exp70r = 0, _exp70i = 0;
        c_exp2(_mul69r, _mul69i, &_exp70r, &_exp70i);
        double _mul71r = 0, _mul71i = 0;
        c_mul(mag, 0, _exp70r, _exp70i, &_mul71r, &_mul71i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul71r; cIm[_idx] = _mul71i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_457_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_arr7r, _arr7i); _abs8i = 0;
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
        double mag_part1 = _mul16r; /* +_mul16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _mod19r = 0, _mod19i = 0;
        _mod19r = fmod(j, _c18r); _mod19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _mod19r + _c20r; _add21i = _mod19i + _c20i;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_abs17r, _abs17i, _add21r, &_pow22r, &_pow22i);
        double mag_part2 = _pow22r; /* +_pow22ii */
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag_part2, 0, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + _mul26r; _add27i = 0 + _mul26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double angle_part1 = _mul32r; /* +_mul32ii */
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x2i; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 6.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _sin35r + _cos39r; _add40i = _sin35i + _cos39i;
        double angle_part2 = _add40r; /* +_add40ii */
        double _add41r = 0, _add41i = 0;
        _add41r = angle_part1 + angle_part2; _add41i = 0 + 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_458_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 5.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mod2r + _c3r; _add4i = _mod2i + _c3i;
        double k = _add4r; /* +_add4ii */
        double _sqrt5r = 0, _sqrt5i = 0;
        c_powr(j, 0, 0.5, &_sqrt5r, &_sqrt5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _abs7r; _add8i = _abs6i + _abs7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _add8r + _c9r; _add10i = _add8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sqrt5r, _sqrt5i, _log11r, _log11i, &_mul12r, &_mul12i);
        double r = _mul12r; /* +_mul12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(k, 0, j, 0, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang13r, _ang13i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, j, 0, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang17r, _ang17i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul16r + _mul20r; _add21i = _mul16i + _mul20i;
        double angle = _add21r; /* +_add21ii */
        double _re22r = 0, _re22i = 0;
        _re22r = x1r; _re22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_re22r, _re22i, k, &_pow23r, &_pow23i);
        double _im24r = 0, _im24i = 0;
        _im24r = x2i; _im24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_im24r, _im24i, k, &_pow25r, &_pow25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _pow23r + _pow25r; _add26i = _pow23i + _pow25i;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 7.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _c27r + _cos31r; _add32i = _c27i + _cos31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_add26r, _add26i, _add32r, _add32i, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 5.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _c34r + _sin38r; _add39i = _c34i + _sin38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_mul33r, _mul33i, _add39r, _add39i, &_mul40r, &_mul40i);
        double magnitude = _mul40r; /* +_mul40ii */
        double _cos41r = 0, _cos41i = 0;
        c_cos(angle, 0, &_cos41r, &_cos41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(angle, 0, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c42r, _c42i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos41r + _mul44r; _add45i = _cos41i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_459_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_abs4r, _abs4i, k, 0, &_mul5r, &_mul5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mul5r + _abs6r; _add7i = _mul5i + _abs6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _add7r + _c8r; _add9i = _add7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr3r, _attr3i, _log10r, _log10i, &_mul11r, &_mul11i);
        double temp_real = _mul11r; /* +_mul11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(k, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr12r, _attr12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1r; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(k, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul14r + _cos17r; _add18i = _mul14i + _cos17i;
        double temp_imag = _add18r; /* +_add18ii */
        double _add19r = 0, _add19i = 0;
        _add19r = x1r + x2r; _add19i = x1i + x2i;
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(_add19r, _add19i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = k + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_ang20r, _ang20i, _add22r, _add22i, &_div23r, &_div23i);
        double temp_angle = _div23r; /* +_div23ii */
        double _mul24r = 0, _mul24i = 0;
        c_mul(temp_imag, 0, temp_angle, 0, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = temp_real + _mul24r; _add25i = 0 + _mul24i;
        double magnitude = _add25r; /* +_add25ii */
        double _sin26r = 0, _sin26i = 0;
        c_sin(temp_real, 0, &_sin26r, &_sin26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(temp_imag, 0, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_cos27r, _cos27i, temp_angle, 0, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _sin26r + _mul28r; _add29i = _sin26i + _mul28i;
        double angle = _add29r; /* +_add29ii */
        double _cos30r = 0, _cos30i = 0;
        c_cos(angle, 0, &_cos30r, &_cos30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(angle, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c31r, _c31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _cos30r + _mul33r; _add34i = _cos30i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(magnitude, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_460_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x1r, x1i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_ang7r, _ang7i, j, 0, &_mul8r, &_mul8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _div12r = 0, _div12i = 0;
        c_div(_ang9r, _ang9i, _add11r, _add11i, &_div12r, &_div12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul8r + _div12r; _add13i = _mul8i + _div12i;
        double _arr14r = 0, _arr14i = 0;
        { int _idx = (j - 1); _arr14r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr14i = 0; }
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _arr14r, _arr14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _add13r + _sin16r; _add17i = _add13i + _sin16i;
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr18i = 0; }
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, _arr18r, _arr18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _add17r - _cos20r; _sub21i = _add17i - _cos20i;
        double phase = _sub21r; /* +_sub21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _abs23r; _add24i = _abs22i + _abs23i;
        double _add25r = 0, _add25i = 0;
        _add25r = _add24r + j; _add25i = _add24i + 0;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_mul(j, 0, j, 0, &_pow28r, &_pow28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(j, 0, &_sin29r, &_sin29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(j, 0, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_sin29r, _sin29i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _pow28r + _mul31r; _add32i = _pow28i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_log26r, _log26i, _add32r, _add32i, &_mul33r, &_mul33i);
        double magnitude = _mul33r; /* +_mul33ii */
        double _cos34r = 0, _cos34i = 0;
        c_cos(phase, 0, &_cos34r, &_cos34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _sin36r = 0, _sin36i = 0;
        c_sin(phase, 0, &_sin36r, &_sin36i);
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

static void poly_461_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double rec1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double imc1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc2 = _attr4r; /* +_attr4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
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
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _mod20r = 0, _mod20i = 0;
        _mod20r = fmod(j, _c19r); _mod20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _mod20r + _c21r; _add22i = _mod20i + _c21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_cos18r, _cos18i, _add22r, _add22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul14r + _mul23r; _add24i = _mul14i + _mul23i;
        double magnitude = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 3.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_div27r, _div27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul29r + _mul34r; _add35i = _mul29i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 6.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _add35r + _sin39r; _add40i = _add35i + _sin39i;
        double angle = _add40r; /* +_add40ii */
        double _cos41r = 0, _cos41i = 0;
        c_cos(angle, 0, &_cos41r, &_cos41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(angle, 0, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c42r, _c42i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos41r + _mul44r; _add45i = _cos41i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    double _c47r = 0, _c47i = 0;
    _c47r = 1.0; _c47i = 0;
    double _add48r = 0, _add48i = 0;
    _add48r = n + _c47r; _add48i = 0 + _c47i;
    for (int k = 1; k < (int)(_add48r); k++) {
        double _attr49r = 0, _attr49i = 0;
        _attr49r = x1r; _attr49i = 0;
        double _attr50r = 0, _attr50i = 0;
        _attr50r = x2r; _attr50i = 0;
        double _sub51r = 0, _sub51i = 0;
        _sub51r = _attr49r - _attr50r; _sub51i = _attr49i - _attr50i;
        double _mul52r = 0, _mul52i = 0;
        c_mul(k, 0, M_PI, 0, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 7.0; _c53i = 0;
        double _div54r = 0, _div54i = 0;
        c_div(_mul52r, _mul52i, _c53r, _c53i, &_div54r, &_div54i);
        double _sin55r = 0, _sin55i = 0;
        c_sin(_div54r, _div54i, &_sin55r, &_sin55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_sub51r, _sub51i, _sin55r, _sin55i, &_mul56r, &_mul56i);
        double _attr57r = 0, _attr57i = 0;
        _attr57r = x1i; _attr57i = 0;
        double _attr58r = 0, _attr58i = 0;
        _attr58r = x2i; _attr58i = 0;
        double _add59r = 0, _add59i = 0;
        _add59r = _attr57r + _attr58r; _add59i = _attr57i + _attr58i;
        double _mul60r = 0, _mul60i = 0;
        c_mul(k, 0, M_PI, 0, &_mul60r, &_mul60i);
        double _c61r = 0, _c61i = 0;
        _c61r = 8.0; _c61i = 0;
        double _div62r = 0, _div62i = 0;
        c_div(_mul60r, _mul60i, _c61r, _c61i, &_div62r, &_div62i);
        double _cos63r = 0, _cos63i = 0;
        c_cos(_div62r, _div62i, &_cos63r, &_cos63i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(_add59r, _add59i, _cos63r, _cos63i, &_mul64r, &_mul64i);
        double _add65r = 0, _add65i = 0;
        _add65r = _mul56r + _mul64r; _add65i = _mul56i + _mul64i;
        cRe[(k - 1)] += _add65r; cIm[(k - 1)] += _add65i;
    }
    double _c66r = 0, _c66i = 0;
    _c66r = 1.0; _c66i = 0;
    double _add67r = 0, _add67i = 0;
    _add67r = n + _c66r; _add67i = 0 + _c66i;
    for (int r = 1; r < (int)(_add67r); r++) {
        double _c68r = 0, _c68i = 0;
        _c68r = 0.0; _c68i = 1.0;
        double _sin69r = 0, _sin69i = 0;
        c_sin(r, 0, &_sin69r, &_sin69i);
        double _cos70r = 0, _cos70i = 0;
        c_cos(r, 0, &_cos70r, &_cos70i);
        double _add71r = 0, _add71i = 0;
        _add71r = _sin69r + _cos70r; _add71i = _sin69i + _cos70i;
        double _mul72r = 0, _mul72i = 0;
        c_mul(_c68r, _c68i, _add71r, _add71i, &_mul72r, &_mul72i);
        double _exp73r = 0, _exp73i = 0;
        c_exp2(_mul72r, _mul72i, &_exp73r, &_exp73i);
        { double _tr = cRe[(r - 1)]*_exp73r - cIm[(r - 1)]*_exp73i; cIm[(r - 1)] = cRe[(r - 1)]*_exp73i + cIm[(r - 1)]*_exp73r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_462_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_arr7r, _arr7i, _arr8r, _arr8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _mul9r + _c10r; _add11i = _mul9i + _c10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(j, 0, &_sin17r, &_sin17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 3.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_sin17r, _sin17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _c16r + _mul21r; _add22i = _c16i + _mul21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log15r, _log15i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 4.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang24r, _ang24i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, M_PI, 0, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 5.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_mul31r, _mul31i, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang30r, _ang30i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul29r + _mul35r; _add36i = _mul29i + _mul35i;
        double angle = _add36r; /* +_add36ii */
        double _cos37r = 0, _cos37i = 0;
        c_cos(angle, 0, &_cos37r, &_cos37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(angle, 0, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c38r, _c38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _cos37r + _mul40r; _add41i = _cos37i + _mul40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(mag, 0, _add41r, _add41i, &_mul42r, &_mul42i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_463_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
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
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul9r + _cos12r; _add13i = _mul9i + _cos12i;
        double mag_part = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang14r, _ang14i, j, 0, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = n - j; _sub17i = 0 - 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sub17r, _sub17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _mul18r; _add19i = _mul15i + _mul18i;
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _add19r + _sin22r; _add23i = _add19i + _sin22i;
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1i; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _add23r - _cos26r; _sub27i = _add23i - _cos26i;
        double ang_part = _sub27r; /* +_sub27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c28r, _c28i, ang_part, 0, &_mul29r, &_mul29i);
        double _exp30r = 0, _exp30i = 0;
        c_exp2(_mul29r, _mul29i, &_exp30r, &_exp30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag_part, 0, _exp30r, _exp30i, &_mul31r, &_mul31i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    for (int k = 1; k < 6; k++) {
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = n + _c32r; _add33i = 0 + _c32i;
        for (int r = 1; r < (int)(_add33r); r++) {
            double _attr34r = 0, _attr34i = 0;
            _attr34r = x1r; _attr34i = 0;
            double _mul35r = 0, _mul35i = 0;
            c_mul(k, 0, r, 0, &_mul35r, &_mul35i);
            double _cos36r = 0, _cos36i = 0;
            c_cos(_mul35r, _mul35i, &_cos36r, &_cos36i);
            double _mul37r = 0, _mul37i = 0;
            c_mul(_attr34r, _attr34i, _cos36r, _cos36i, &_mul37r, &_mul37i);
            double _attr38r = 0, _attr38i = 0;
            _attr38r = x2i; _attr38i = 0;
            double _mul39r = 0, _mul39i = 0;
            c_mul(k, 0, r, 0, &_mul39r, &_mul39i);
            double _sin40r = 0, _sin40i = 0;
            c_sin(_mul39r, _mul39i, &_sin40r, &_sin40i);
            double _mul41r = 0, _mul41i = 0;
            c_mul(_attr38r, _attr38i, _sin40r, _sin40i, &_mul41r, &_mul41i);
            double _add42r = 0, _add42i = 0;
            _add42r = _mul37r + _mul41r; _add42i = _mul37i + _mul41i;
            double _c43r = 0, _c43i = 0;
            _c43r = 0.0; _c43i = 1.0;
            double _attr44r = 0, _attr44i = 0;
            _attr44r = x2r; _attr44i = 0;
            double _mul45r = 0, _mul45i = 0;
            c_mul(_attr44r, _attr44i, r, 0, &_mul45r, &_mul45i);
            double _attr46r = 0, _attr46i = 0;
            _attr46r = x1i; _attr46i = 0;
            double _mul47r = 0, _mul47i = 0;
            c_mul(_attr46r, _attr46i, k, 0, &_mul47r, &_mul47i);
            double _sub48r = 0, _sub48i = 0;
            _sub48r = _mul45r - _mul47r; _sub48i = _mul45i - _mul47i;
            double _mul49r = 0, _mul49i = 0;
            c_mul(_c43r, _c43i, _sub48r, _sub48i, &_mul49r, &_mul49i);
            double _exp50r = 0, _exp50i = 0;
            c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
            double _mul51r = 0, _mul51i = 0;
            c_mul(_add42r, _add42i, _exp50r, _exp50i, &_mul51r, &_mul51i);
            cRe[(r - 1)] += _mul51r; cIm[(r - 1)] += _mul51i;
        }
    }
    double _c52r = 0, _c52i = 0;
    _c52r = 1.0; _c52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = n + _c52r; _add53i = 0 + _c52i;
    for (int j = 1; j < (int)(_add53r); j++) {
        double _c54r = 0, _c54i = 0;
        _c54r = 1.0; _c54i = 0;
        double _c55r = 0, _c55i = 0;
        _c55r = 0.1; _c55i = 0;
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c55r, _c55i, j, 0, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _c54r + _mul56r; _add57i = _c54i + _mul56i;
        double _c58r = 0, _c58i = 0;
        _c58r = 1.0; _c58i = 0;
        double _c59r = 0, _c59i = 0;
        _c59r = 1.0; _c59i = 0;
        double _add60r = 0, _add60i = 0;
        _add60r = j + _c59r; _add60i = 0 + _c59i;
        double _log61r = 0, _log61i = 0;
        c_log(_add60r, _add60i, &_log61r, &_log61i);
        double _add62r = 0, _add62i = 0;
        _add62r = _c58r + _log61r; _add62i = _c58i + _log61i;
        double _div63r = 0, _div63i = 0;
        c_div(_add57r, _add57i, _add62r, _add62i, &_div63r, &_div63i);
        { double _tr = cRe[(j - 1)]*_div63r - cIm[(j - 1)]*_div63i; cIm[(j - 1)] = cRe[(j - 1)]*_div63i + cIm[(j - 1)]*_div63r; cRe[(j - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_464_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + k; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(k, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _sqrt19r = 0, _sqrt19i = 0;
        c_powr(k, 0, 0.5, &_sqrt19r, &_sqrt19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_cos18r, _cos18i, _sqrt19r, _sqrt19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul14r + _mul20r; _add21i = _mul14i + _mul20i;
        double mag_part = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(k, 0, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _cos26r = 0, _cos26i = 0;
        c_cos(k, 0, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul24r + _mul27r; _add28i = _mul24i + _mul27i;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1r; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(k, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_mul30r, _mul30i, &_sin31r, &_sin31i);
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2i; _attr32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(k, 0, _attr32r, _attr32i, &_mul33r, &_mul33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_mul33r, _mul33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sin31r, _sin31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add28r + _mul35r; _add36i = _add28i + _mul35i;
        double angle_part = _add36r; /* +_add36ii */
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, angle_part, 0, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(mag_part, 0, _exp39r, _exp39i, &_mul40r, &_mul40i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    double _c41r = 0, _c41i = 0;
    _c41r = 1.0; _c41i = 0;
    double _add42r = 0, _add42i = 0;
    _add42r = n + _c41r; _add42i = 0 + _c41i;
    for (int r = 1; r < (int)(_add42r); r++) {
        double _attr43r = 0, _attr43i = 0;
        _attr43r = x1r; _attr43i = 0;
        double _attr44r = 0, _attr44i = 0;
        _attr44r = x2r; _attr44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_attr43r, _attr43i, _attr44r, _attr44i, &_mul45r, &_mul45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 1.0; _c46i = 0;
        double _add47r = 0, _add47i = 0;
        _add47r = r + _c46r; _add47i = 0 + _c46i;
        double _div48r = 0, _div48i = 0;
        c_div(_mul45r, _mul45i, _add47r, _add47i, &_div48r, &_div48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _attr50r = 0, _attr50i = 0;
        _attr50r = x1i; _attr50i = 0;
        double _attr51r = 0, _attr51i = 0;
        _attr51r = x2i; _attr51i = 0;
        double _sub52r = 0, _sub52i = 0;
        _sub52r = _attr50r - _attr51r; _sub52i = _attr50i - _attr51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c49r, _c49i, _sub52r, _sub52i, &_mul53r, &_mul53i);
        double _sin54r = 0, _sin54i = 0;
        c_sin(r, 0, &_sin54r, &_sin54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_mul53r, _mul53i, _sin54r, _sin54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _div48r + _mul55r; _add56i = _div48i + _mul55i;
        cRe[(r - 1)] += _add56r; cIm[(r - 1)] += _add56i;
    }
    double _c57r = 0, _c57i = 0;
    _c57r = 1.0; _c57i = 0;
    double _add58r = 0, _add58i = 0;
    _add58r = n + _c57r; _add58i = 0 + _c57i;
    for (int j = 1; j < (int)(_add58r); j++) {
        double _c59r = 0, _c59i = 0;
        _c59r = 1.0; _c59i = 0;
        double _c60r = 0, _c60i = 0;
        _c60r = 0.1; _c60i = 0;
        double _mul61r = 0, _mul61i = 0;
        c_mul(_c60r, _c60i, j, 0, &_mul61r, &_mul61i);
        double _add62r = 0, _add62i = 0;
        _add62r = _c59r + _mul61r; _add62i = _c59i + _mul61i;
        double _c63r = 0, _c63i = 0;
        _c63r = 0.0; _c63i = 0.05;
        double _mul64r = 0, _mul64i = 0;
        c_mul(_c63r, _c63i, j, 0, &_mul64r, &_mul64i);
        double _exp65r = 0, _exp65i = 0;
        c_exp2(_mul64r, _mul64i, &_exp65r, &_exp65i);
        double _mul66r = 0, _mul66i = 0;
        c_mul(_add62r, _add62i, _exp65r, _exp65i, &_mul66r, &_mul66i);
        { double _tr = cRe[(j - 1)]*_mul66r - cIm[(j - 1)]*_mul66i; cIm[(j - 1)] = cRe[(j - 1)]*_mul66i + cIm[(j - 1)]*_mul66r; cRe[(j - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_465_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x1r, x1i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x2r, x2i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul9r + _cos12r; _add13i = _mul9i + _cos12i;
        double mag_part = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang14r, _ang14i, j, 0, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = n - j; _sub17i = 0 - 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sub17r, _sub17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _mul18r; _add19i = _mul15i + _mul18i;
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _add19r + _sin22r; _add23i = _add19i + _sin22i;
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _add23r - _cos26r; _sub27i = _add23i - _cos26i;
        double ang_part = _sub27r; /* +_sub27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c28r, _c28i, ang_part, 0, &_mul29r, &_mul29i);
        double _exp30r = 0, _exp30i = 0;
        c_exp2(_mul29r, _mul29i, &_exp30r, &_exp30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag_part, 0, _exp30r, _exp30i, &_mul31r, &_mul31i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    for (int k = 1; k < 6; k++) {
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = n + _c32r; _add33i = 0 + _c32i;
        for (int r = 1; r < (int)(_add33r); r++) {
            double _attr34r = 0, _attr34i = 0;
            _attr34r = x1r; _attr34i = 0;
            double _mul35r = 0, _mul35i = 0;
            c_mul(k, 0, r, 0, &_mul35r, &_mul35i);
            double _cos36r = 0, _cos36i = 0;
            c_cos(_mul35r, _mul35i, &_cos36r, &_cos36i);
            double _mul37r = 0, _mul37i = 0;
            c_mul(_attr34r, _attr34i, _cos36r, _cos36i, &_mul37r, &_mul37i);
            double _attr38r = 0, _attr38i = 0;
            _attr38r = x2i; _attr38i = 0;
            double _mul39r = 0, _mul39i = 0;
            c_mul(k, 0, r, 0, &_mul39r, &_mul39i);
            double _sin40r = 0, _sin40i = 0;
            c_sin(_mul39r, _mul39i, &_sin40r, &_sin40i);
            double _mul41r = 0, _mul41i = 0;
            c_mul(_attr38r, _attr38i, _sin40r, _sin40i, &_mul41r, &_mul41i);
            double _add42r = 0, _add42i = 0;
            _add42r = _mul37r + _mul41r; _add42i = _mul37i + _mul41i;
            double _c43r = 0, _c43i = 0;
            _c43r = 0.0; _c43i = 1.0;
            double _attr44r = 0, _attr44i = 0;
            _attr44r = x2r; _attr44i = 0;
            double _mul45r = 0, _mul45i = 0;
            c_mul(_attr44r, _attr44i, r, 0, &_mul45r, &_mul45i);
            double _attr46r = 0, _attr46i = 0;
            _attr46r = x1i; _attr46i = 0;
            double _mul47r = 0, _mul47i = 0;
            c_mul(_attr46r, _attr46i, k, 0, &_mul47r, &_mul47i);
            double _sub48r = 0, _sub48i = 0;
            _sub48r = _mul45r - _mul47r; _sub48i = _mul45i - _mul47i;
            double _mul49r = 0, _mul49i = 0;
            c_mul(_c43r, _c43i, _sub48r, _sub48i, &_mul49r, &_mul49i);
            double _exp50r = 0, _exp50i = 0;
            c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
            double _mul51r = 0, _mul51i = 0;
            c_mul(_add42r, _add42i, _exp50r, _exp50i, &_mul51r, &_mul51i);
            cRe[(r - 1)] += _mul51r; cIm[(r - 1)] += _mul51i;
        }
    }
    double _c52r = 0, _c52i = 0;
    _c52r = 1.0; _c52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = n + _c52r; _add53i = 0 + _c52i;
    for (int j = 1; j < (int)(_add53r); j++) {
        double _c54r = 0, _c54i = 0;
        _c54r = 1.0; _c54i = 0;
        double _c55r = 0, _c55i = 0;
        _c55r = 0.1; _c55i = 0;
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c55r, _c55i, j, 0, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _c54r + _mul56r; _add57i = _c54i + _mul56i;
        { double _tr = cRe[(j - 1)]*_add57r - cIm[(j - 1)]*_add57i; cIm[(j - 1)] = cRe[(j - 1)]*_add57i + cIm[(j - 1)]*_add57r; cRe[(j - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_466_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _abs3r + _pow5r; _add6i = _abs3i + _pow5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x1r, x1i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_sin10r, _sin10i); _abs11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log7r, _log7i, _abs11r, _abs11i, &_mul12r, &_mul12i);
        double _sqrt13r = 0, _sqrt13i = 0;
        c_powr(j, 0, 0.5, &_sqrt13r, &_sqrt13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x2r, x2i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_sqrt13r, _sqrt13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul12r + _mul17r; _add18i = _mul12i + _mul17i;
        double magnitude = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang19r, _ang19i, _log22r, _log22i, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 0.5; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _div27r = 0, _div27i = 0;
        c_div(_attr24r, _attr24i, _add26r, _add26i, &_div27r, &_div27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul23r - _div27r; _sub28i = _mul23i - _div27i;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1r; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_mul30r, _mul30i, &_sin31r, &_sin31i);
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2i; _attr32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, _attr32r, _attr32i, &_mul33r, &_mul33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_mul33r, _mul33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sin31r, _sin31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _sub28r + _mul35r; _add36i = _sub28i + _mul35i;
        double angle = _add36r; /* +_add36ii */
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, angle, 0, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(magnitude, 0, _exp39r, _exp39i, &_mul40r, &_mul40i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_467_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double mag_sum = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        for (int k = 1; k < (int)(_add5r); k++) {
            double _abs6r = 0, _abs6i = 0;
            _abs6r = c_abs(x1r, x1i); _abs6i = 0;
            double _add7r = 0, _add7i = 0;
            _add7r = _abs6r + k; _add7i = _abs6i + 0;
            double _log8r = 0, _log8i = 0;
            c_log(_add7r, _add7i, &_log8r, &_log8i);
            double _ang9r = 0, _ang9i = 0;
            _ang9r = c_arg(x2r, x2i); _ang9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_log8r, _log8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            mag_sum += _mul12r;
        }
        double _sub13r = 0, _sub13i = 0;
        _sub13r = n - j; _sub13i = 0 - 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _sub13r + _c14r; _add15i = _sub13i + _c14i;
        for (int r = 1; r < (int)(_add15r); r++) {
            double _abs16r = 0, _abs16i = 0;
            _abs16r = c_abs(x2r, x2i); _abs16i = 0;
            double _add17r = 0, _add17i = 0;
            _add17r = _abs16r + r; _add17i = _abs16i + 0;
            double _log18r = 0, _log18i = 0;
            c_log(_add17r, _add17i, &_log18r, &_log18i);
            double _ang19r = 0, _ang19i = 0;
            _ang19r = c_arg(x2r, x2i); _ang19i = 0;
            double _mul20r = 0, _mul20i = 0;
            c_mul(r, 0, _ang19r, _ang19i, &_mul20r, &_mul20i);
            double _cos21r = 0, _cos21i = 0;
            c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
            double _mul22r = 0, _mul22i = 0;
            c_mul(_log18r, _log18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
            mag_sum += _mul22r;
        }
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = mag_sum + _c23r; _add24i = 0 + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double mag = _log25r; /* +_log25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = j + _c26r; _add27i = 0 + _c26i;
        double _div28r = 0, _div28i = 0;
        c_div(mag_sum, 0, _add27r, _add27i, &_div28r, &_div28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = n - j; _sub29i = 0 - 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _sub29r + _c30r; _add31i = _sub29i + _c30i;
        double _div32r = 0, _div32i = 0;
        c_div(mag_sum, 0, _add31r, _add31i, &_div32r, &_div32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _div28r + _div32r; _add33i = _div28i + _div32i;
        double angle = _add33r; /* +_add33ii */
        double _cos34r = 0, _cos34i = 0;
        c_cos(angle, 0, &_cos34r, &_cos34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(angle, 0, &_sin35r, &_sin35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sin35r, _sin35i, _c36r, _c36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cos34r + _mul37r; _add38i = _cos34i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = n + _c40r; _add41i = 0 + _c40i;
    for (int j = 1; j < (int)(_add41r); j++) {
        double _cf42r = 0, _cf42i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 0.05; _c44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 2.0; _c45i = 0;
        double _pow46r = 0, _pow46i = 0;
        c_mul(j, 0, j, 0, &_pow46r, &_pow46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c44r, _c44i, _pow46r, _pow46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _c43r + _mul47r; _add48i = _c43i + _mul47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_cf42r, _cf42i, _add48r, _add48i, &_mul49r, &_mul49i);
        double _cf50r = 0, _cf50i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
        double _conj51r = 0, _conj51i = 0;
        _conj51r = _cf50r; _conj51i = -(_cf50i);
        double _c52r = 0, _c52i = 0;
        _c52r = 0.02; _c52i = 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_conj51r, _conj51i, _c52r, _c52i, &_mul53r, &_mul53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _mul49r + _mul53r; _add54i = _mul49i + _mul53i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add54r; cIm[_idx] = _add54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_467_old_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double mag = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double angle = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
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
            mag += _mul11r;
            double _pow12r = 0, _pow12i = 0;
            c_powr(x2r, x2i, k, &_pow12r, &_pow12i);
            double _ang13r = 0, _ang13i = 0;
            _ang13r = c_arg(_pow12r, _pow12i); _ang13i = 0;
            double _c14r = 0, _c14i = 0;
            _c14r = 1.0; _c14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = j + _c14r; _add15i = 0 + _c14i;
            double _div16r = 0, _div16i = 0;
            c_div(k, 0, _add15r, _add15i, &_div16r, &_div16i);
            double _cos17r = 0, _cos17i = 0;
            c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(_ang13r, _ang13i, _cos17r, _cos17i, &_mul18r, &_mul18i);
            angle += _mul18r;
        }
        double _cos19r = 0, _cos19i = 0;
        c_cos(angle, 0, &_cos19r, &_cos19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(angle, 0, &_sin20r, &_sin20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin20r, _sin20i, _c21r, _c21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _cos19r + _mul22r; _add23i = _cos19i + _mul22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(mag, 0, _add23r, _add23i, &_mul24r, &_mul24i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    }
    for (int j = 0; j < (int)(n); j++) {
        double _cf25r = 0, _cf25i = 0;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.05; _c27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(_add29r, _add29i, _add29r, _add29i, &_pow31r, &_pow31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c27r, _c27i, _pow31r, _pow31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _c26r + _mul32r; _add33i = _c26i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_cf25r, _cf25i, _add33r, _add33i, &_mul34r, &_mul34i);
        double _cf35r = 0, _cf35i = 0;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
        double _conj36r = 0, _conj36i = 0;
        _conj36r = _cf35r; _conj36i = -(_cf35i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.02; _c37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_conj36r, _conj36i, _c37r, _c37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul34r + _mul38r; _add39i = _mul34i + _mul38i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_468_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double ang = 0;
        double mag = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 10.0; _c3i = 0;
        if (j <= _c3r) {
            double _abs4r = 0, _abs4i = 0;
            _abs4r = c_abs(x1r, x1i); _abs4i = 0;
            double _c5r = 0, _c5i = 0;
            _c5r = 2.0; _c5i = 0;
            double _pow6r = 0, _pow6i = 0;
            c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
            double _add7r = 0, _add7i = 0;
            _add7r = _abs4r + _pow6r; _add7i = _abs4i + _pow6i;
            double _log8r = 0, _log8i = 0;
            c_log(_add7r, _add7i, &_log8r, &_log8i);
            double _attr9r = 0, _attr9i = 0;
            _attr9r = x2r; _attr9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _abs12r = 0, _abs12i = 0;
            _abs12r = c_abs(_sin11r, _sin11i); _abs12i = 0;
            double _mul13r = 0, _mul13i = 0;
            c_mul(_log8r, _log8i, _abs12r, _abs12i, &_mul13r, &_mul13i);
            double _attr14r = 0, _attr14i = 0;
            _attr14r = x1i; _attr14i = 0;
            double _mul15r = 0, _mul15i = 0;
            c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
            double _cos16r = 0, _cos16i = 0;
            c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
            double _c17r = 0, _c17i = 0;
            _c17r = 2.0; _c17i = 0;
            double _pow18r = 0, _pow18i = 0;
            c_mul(_cos16r, _cos16i, _cos16r, _cos16i, &_pow18r, &_pow18i);
            double _add19r = 0, _add19i = 0;
            _add19r = _mul13r + _pow18r; _add19i = _mul13i + _pow18i;
            mag = _add19r;
            double _ang20r = 0, _ang20i = 0;
            _ang20r = c_arg(x1r, x1i); _ang20i = 0;
            double _pow21r = 0, _pow21i = 0;
            c_powr(_ang20r, _ang20i, j, &_pow21r, &_pow21i);
            double _ang22r = 0, _ang22i = 0;
            _ang22r = c_arg(x2r, x2i); _ang22i = 0;
            double _sin23r = 0, _sin23i = 0;
            c_sin(j, 0, &_sin23r, &_sin23i);
            double _mul24r = 0, _mul24i = 0;
            c_mul(_ang22r, _ang22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
            double _add25r = 0, _add25i = 0;
            _add25r = _pow21r + _mul24r; _add25i = _pow21i + _mul24i;
            ang = _add25r;
        } else {
            double _c26r = 0, _c26i = 0;
            _c26r = 20.0; _c26i = 0;
            if (j <= _c26r) {
                double _abs27r = 0, _abs27i = 0;
                _abs27r = c_abs(x2r, x2i); _abs27i = 0;
                double _c28r = 0, _c28i = 0;
                _c28r = 1.5; _c28i = 0;
                double _pow29r = 0, _pow29i = 0;
                c_powr(j, 0, 1.5, &_pow29r, &_pow29i);
                double _add30r = 0, _add30i = 0;
                _add30r = _abs27r + _pow29r; _add30i = _abs27i + _pow29i;
                double _log31r = 0, _log31i = 0;
                c_log(_add30r, _add30i, &_log31r, &_log31i);
                double _attr32r = 0, _attr32i = 0;
                _attr32r = x1r; _attr32i = 0;
                double _mul33r = 0, _mul33i = 0;
                c_mul(j, 0, _attr32r, _attr32i, &_mul33r, &_mul33i);
                double _cos34r = 0, _cos34i = 0;
                c_cos(_mul33r, _mul33i, &_cos34r, &_cos34i);
                double _attr35r = 0, _attr35i = 0;
                _attr35r = x2i; _attr35i = 0;
                double _mul36r = 0, _mul36i = 0;
                c_mul(j, 0, _attr35r, _attr35i, &_mul36r, &_mul36i);
                double _sin37r = 0, _sin37i = 0;
                c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
                double _sub38r = 0, _sub38i = 0;
                _sub38r = _cos34r - _sin37r; _sub38i = _cos34i - _sin37i;
                double _abs39r = 0, _abs39i = 0;
                _abs39r = c_abs(_sub38r, _sub38i); _abs39i = 0;
                double _mul40r = 0, _mul40i = 0;
                c_mul(_log31r, _log31i, _abs39r, _abs39i, &_mul40r, &_mul40i);
                mag = _mul40r;
                double _ang41r = 0, _ang41i = 0;
                _ang41r = c_arg(x2r, x2i); _ang41i = 0;
                double _pow42r = 0, _pow42i = 0;
                c_powr(_ang41r, _ang41i, j, &_pow42r, &_pow42i);
                double _ang43r = 0, _ang43i = 0;
                _ang43r = c_arg(x1r, x1i); _ang43i = 0;
                double _c44r = 0, _c44i = 0;
                _c44r = 1.0; _c44i = 0;
                double _add45r = 0, _add45i = 0;
                _add45r = j + _c44r; _add45i = 0 + _c44i;
                double _log46r = 0, _log46i = 0;
                c_log(_add45r, _add45i, &_log46r, &_log46i);
                double _mul47r = 0, _mul47i = 0;
                c_mul(_ang43r, _ang43i, _log46r, _log46i, &_mul47r, &_mul47i);
                double _sub48r = 0, _sub48i = 0;
                _sub48r = _pow42r - _mul47r; _sub48i = _pow42i - _mul47i;
                ang = _sub48r;
            } else {
                double _mul49r = 0, _mul49i = 0;
                c_mul(x1r, x1i, x2r, x2i, &_mul49r, &_mul49i);
                double _abs50r = 0, _abs50i = 0;
                _abs50r = c_abs(_mul49r, _mul49i); _abs50i = 0;
                double _add51r = 0, _add51i = 0;
                _add51r = _abs50r + j; _add51i = _abs50i + 0;
                double _log52r = 0, _log52i = 0;
                c_log(_add51r, _add51i, &_log52r, &_log52i);
                double _attr53r = 0, _attr53i = 0;
                _attr53r = x1r; _attr53i = 0;
                double _mul54r = 0, _mul54i = 0;
                c_mul(j, 0, _attr53r, _attr53i, &_mul54r, &_mul54i);
                double _attr55r = 0, _attr55i = 0;
                _attr55r = x2r; _attr55i = 0;
                double _mul56r = 0, _mul56i = 0;
                c_mul(j, 0, _attr55r, _attr55i, &_mul56r, &_mul56i);
                double _cos57r = 0, _cos57i = 0;
                c_cos(_mul56r, _mul56i, &_cos57r, &_cos57i);
                double _add58r = 0, _add58i = 0;
                _add58r = _mul54r + _cos57r; _add58i = _mul54i + _cos57i;
                double _sin59r = 0, _sin59i = 0;
                c_sin(_add58r, _add58i, &_sin59r, &_sin59i);
                double _abs60r = 0, _abs60i = 0;
                _abs60r = c_abs(_sin59r, _sin59i); _abs60i = 0;
                double _mul61r = 0, _mul61i = 0;
                c_mul(_log52r, _log52i, _abs60r, _abs60i, &_mul61r, &_mul61i);
                mag = _mul61r;
                double _conj62r = 0, _conj62i = 0;
                _conj62r = x2r; _conj62i = -(x2i);
                double _add63r = 0, _add63i = 0;
                _add63r = x1r + _conj62r; _add63i = x1i + _conj62i;
                double _ang64r = 0, _ang64i = 0;
                _ang64r = c_arg(_add63r, _add63i); _ang64i = 0;
                double _mul65r = 0, _mul65i = 0;
                c_mul(_ang64r, _ang64i, j, 0, &_mul65r, &_mul65i);
                double _sub66r = 0, _sub66i = 0;
                _sub66r = x1r - x2r; _sub66i = x1i - x2i;
                double _abs67r = 0, _abs67i = 0;
                _abs67r = c_abs(_sub66r, _sub66i); _abs67i = 0;
                double _c68r = 0, _c68i = 0;
                _c68r = 1.0; _c68i = 0;
                double _add69r = 0, _add69i = 0;
                _add69r = _abs67r + _c68r; _add69i = _abs67i + _c68i;
                double _log70r = 0, _log70i = 0;
                c_log(_add69r, _add69i, &_log70r, &_log70i);
                double _add71r = 0, _add71i = 0;
                _add71r = _mul65r + _log70r; _add71i = _mul65i + _log70i;
                ang = _add71r;
            }
        }
        double _c72r = 0, _c72i = 0;
        _c72r = 0.0; _c72i = 1.0;
        double _mul73r = 0, _mul73i = 0;
        c_mul(_c72r, _c72i, ang, 0, &_mul73r, &_mul73i);
        double _exp74r = 0, _exp74i = 0;
        c_exp2(_mul73r, _mul73i, &_exp74r, &_exp74i);
        double _mul75r = 0, _mul75i = 0;
        c_mul(mag, 0, _exp74r, _exp74i, &_mul75r, &_mul75i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul75r; cIm[_idx] = _mul75i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_469_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _abs4r; _add5i = _abs3i + _abs4i;
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + j; _add6i = _add5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _ang12r, _ang12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin11r, _sin11i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c8r + _mul15r; _add16i = _c8i + _mul15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log7r, _log7i, _add16r, _add16i, &_mul17r, &_mul17i);
        double mag = _mul17r; /* +_mul17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_ang18r, _ang18i, j, &_pow19r, &_pow19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _mod22r = 0, _mod22i = 0;
        _mod22r = fmod(j, _c21r); _mod22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_ang20r, _ang20i, _mod22r, &_pow23r, &_pow23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _pow19r - _pow23r; _sub24i = _pow19i - _pow23i;
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1r; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sub24r + _sin27r; _add28i = _sub24i + _sin27i;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _add28r - _cos31r; _sub32i = _add28i - _cos31i;
        double angle = _sub32r; /* +_sub32ii */
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
        c_mul(mag, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
        for (int k = 1; k < 4; k++) {
            double _c39r = 0, _c39i = 0;
            _c39r = 1.0; _c39i = 0;
            double _c40r = 0, _c40i = 0;
            _c40r = 0.1; _c40i = 0;
            double _attr41r = 0, _attr41i = 0;
            _attr41r = x1r; _attr41i = 0;
            double _mul42r = 0, _mul42i = 0;
            c_mul(k, 0, _attr41r, _attr41i, &_mul42r, &_mul42i);
            double _c43r = 0, _c43i = 0;
            _c43r = 0.0; _c43i = 0;
            double _add44r = 0, _add44i = 0;
            _add44r = _mul42r + _c43r; _add44i = _mul42i + _c43i;
            double _sin45r = 0, _sin45i = 0;
            c_sin(_add44r, _add44i, &_sin45r, &_sin45i);
            double _mul46r = 0, _mul46i = 0;
            c_mul(_c40r, _c40i, _sin45r, _sin45i, &_mul46r, &_mul46i);
            double _add47r = 0, _add47i = 0;
            _add47r = _c39r + _mul46r; _add47i = _c39i + _mul46i;
            double _mul48r = 0, _mul48i = 0;
            c_mul(mag, 0, _add47r, _add47i, &_mul48r, &_mul48i);
            mag = _mul48r;
            double _c49r = 0, _c49i = 0;
            _c49r = 0.5; _c49i = 0;
            double _ang50r = 0, _ang50i = 0;
            _ang50r = c_arg(x1r, x1i); _ang50i = 0;
            double _pow51r = 0, _pow51i = 0;
            c_powr(_ang50r, _ang50i, k, &_pow51r, &_pow51i);
            double _mul52r = 0, _mul52i = 0;
            c_mul(_c49r, _c49i, _pow51r, _pow51i, &_mul52r, &_mul52i);
            double _add53r = 0, _add53i = 0;
            _add53r = angle + _mul52r; _add53i = 0 + _mul52i;
            double _c54r = 0, _c54i = 0;
            _c54r = 0.3; _c54i = 0;
            double _ang55r = 0, _ang55i = 0;
            _ang55r = c_arg(x2r, x2i); _ang55i = 0;
            double _pow56r = 0, _pow56i = 0;
            c_powr(_ang55r, _ang55i, k, &_pow56r, &_pow56i);
            double _mul57r = 0, _mul57i = 0;
            c_mul(_c54r, _c54i, _pow56r, _pow56i, &_mul57r, &_mul57i);
            double _sub58r = 0, _sub58i = 0;
            _sub58r = _add53r - _mul57r; _sub58i = _add53i - _mul57i;
            angle = _sub58r;
            double _cos59r = 0, _cos59i = 0;
            c_cos(angle, 0, &_cos59r, &_cos59i);
            double _c60r = 0, _c60i = 0;
            _c60r = 0.0; _c60i = 1.0;
            double _sin61r = 0, _sin61i = 0;
            c_sin(angle, 0, &_sin61r, &_sin61i);
            double _mul62r = 0, _mul62i = 0;
            c_mul(_c60r, _c60i, _sin61r, _sin61i, &_mul62r, &_mul62i);
            double _add63r = 0, _add63i = 0;
            _add63r = _cos59r + _mul62r; _add63i = _cos59i + _mul62i;
            double _mul64r = 0, _mul64i = 0;
            c_mul(mag, 0, _add63r, _add63i, &_mul64r, &_mul64i);
            cRe[(j - 1)] += _mul64r; cIm[(j - 1)] += _mul64i;
        }
        double _cf65r = 0, _cf65i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf65r = cRe[_idx]; _cf65i = cIm[_idx]; } }
        double _c66r = 0, _c66i = 0;
        _c66r = 1.0; _c66i = 0;
        double _c67r = 0, _c67i = 0;
        _c67r = 0.05; _c67i = 0;
        double _mul68r = 0, _mul68i = 0;
        c_mul(_c67r, _c67i, j, 0, &_mul68r, &_mul68i);
        double _add69r = 0, _add69i = 0;
        _add69r = _c66r + _mul68r; _add69i = _c66i + _mul68i;
        double _mul70r = 0, _mul70i = 0;
        c_mul(_cf65r, _cf65i, _add69r, _add69i, &_mul70r, &_mul70i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul70r; cIm[_idx] = _mul70i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_470_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (k - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double r = _arr7r; /* +_arr7ii */
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (k - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr8i = 0; }
        double im = _arr8r; /* +_arr8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(r, 0); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 0.5; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs13r, _abs13i, 0.5, &_pow15r, &_pow15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log12r, _log12i, _pow15r, _pow15i, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(r, 0, k, 0, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = k + _c19r; _add20i = 0 + _c19i;
        double _div21r = 0, _div21i = 0;
        c_div(im, 0, _add20r, _add20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin18r, _sin18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul16r + _mul23r; _add24i = _mul16i + _mul23i;
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _mod26r = 0, _mod26i = 0;
        _mod26r = fmod(k, _c25r); _mod26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _mod26r + _c27r; _add28i = _mod26i + _c27i;
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x2r, x2i); _abs29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_add28r, _add28i, _abs29r, _abs29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add24r + _mul30r; _add31i = _add24i + _mul30i;
        double mag = _add31r; /* +_add31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(k, 0, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang32r, _ang32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(k, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 4.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang35r, _ang35i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul34r + _mul40r; _add41i = _mul34i + _mul40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(im, 0, k, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 2.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
        double _sin45r = 0, _sin45i = 0;
        c_sin(_div44r, _div44i, &_sin45r, &_sin45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _add41r + _sin45r; _add46i = _add41i + _sin45i;
        double angle = _add46r; /* +_add46ii */
        double _cos47r = 0, _cos47i = 0;
        c_cos(angle, 0, &_cos47r, &_cos47i);
        double _sin48r = 0, _sin48i = 0;
        c_sin(angle, 0, &_sin48r, &_sin48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_sin48r, _sin48i, _c49r, _c49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _cos47r + _mul50r; _add51i = _cos47i + _mul50i;
        double _mul52r = 0, _mul52i = 0;
        c_mul(mag, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_471_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(j, 0, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log5r, _log5i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _cos10r; _add11i = _mul7i + _cos10i;
        double mag_part = _add11r; /* +_add11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _sqrt13r = 0, _sqrt13i = 0;
        c_powr(j, 0, 0.5, &_sqrt13r, &_sqrt13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _sqrt13r, _sqrt13i, &_mul14r, &_mul14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x2i; _attr15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = j + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_attr15r, _attr15i, _add17r, _add17i, &_div18r, &_div18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul14r + _div18r; _add19i = _mul14i + _div18i;
        double angle_part = _add19r; /* +_add19ii */
        double _cos20r = 0, _cos20i = 0;
        c_cos(angle_part, 0, &_cos20r, &_cos20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(angle_part, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c21r, _c21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _cos20r + _mul23r; _add24i = _cos20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag_part, 0, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    double _c26r = 0, _c26i = 0;
    _c26r = 1.0; _c26i = 0;
    double _add27r = 0, _add27i = 0;
    _add27r = n + _c26r; _add27i = 0 + _c26i;
    for (int k = 1; k < (int)(_add27r); k++) {
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x2r; _attr28i = 0;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1i; _attr29i = 0;
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _attr28r - _attr29r; _sub30i = _attr28i - _attr29i;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = k + _c32r; _add33i = 0 + _c32i;
        double _log34r = 0, _log34i = 0;
        c_log(_add33r, _add33i, &_log34r, &_log34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c31r, _c31i, _log34r, _log34i, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sub30r, _sub30i, _exp36r, _exp36i, &_mul37r, &_mul37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(k, 0, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_mul37r, _mul37i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        cRe[(k - 1)] += _mul39r; cIm[(k - 1)] += _mul39i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_472_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 7.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c10r + _sin14r; _add15i = _c10i + _sin14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log9r, _log9i, _add15r, _add15i, &_mul16r, &_mul16i);
        double mag = _mul16r; /* +_mul16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang17r, _ang17i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang23r, _ang23i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul22r + _mul28r; _add29i = _mul22i + _mul28i;
        double ang = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, ang, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(mag, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        double _attr34r = 0, _attr34i = 0;
        _attr34r = x1r; _attr34i = 0;
        double _attr35r = 0, _attr35i = 0;
        _attr35r = x2r; _attr35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _attr34r + _attr35r; _add36i = _attr34i + _attr35i;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = j + _c37r; _add38i = 0 + _c37i;
        double _div39r = 0, _div39i = 0;
        c_div(_add36r, _add36i, _add38r, _add38i, &_div39r, &_div39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul33r + _div39r; _add40i = _mul33i + _div39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    double _c41r = 0, _c41i = 0;
    _c41r = 1.0; _c41i = 0;
    double _add42r = 0, _add42i = 0;
    _add42r = n + _c41r; _add42i = 0 + _c41i;
    for (int k = 1; k < (int)(_add42r); k++) {
        double _conj43r = 0, _conj43i = 0;
        _conj43r = x1r; _conj43i = -(x1i);
        double _pow44r = 0, _pow44i = 0;
        c_powr(_conj43r, _conj43i, k, &_pow44r, &_pow44i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x2r; _conj45i = -(x2i);
        double _sub46r = 0, _sub46i = 0;
        _sub46r = n - k; _sub46i = 0 - 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _add48r = 0, _add48i = 0;
        _add48r = _sub46r + _c47r; _add48i = _sub46i + _c47i;
        double _pow49r = 0, _pow49i = 0;
        c_powr(_conj45r, _conj45i, _add48r, &_pow49r, &_pow49i);
        double _sub50r = 0, _sub50i = 0;
        _sub50r = _pow44r - _pow49r; _sub50i = _pow44i - _pow49i;
        cRe[(k - 1)] += _sub50r; cIm[(k - 1)] += _sub50i;
    }
    double _c51r = 0, _c51i = 0;
    _c51r = 1.0; _c51i = 0;
    double _add52r = 0, _add52i = 0;
    _add52r = n + _c51r; _add52i = 0 + _c51i;
    for (int r = 1; r < (int)(_add52r); r++) {
        double _c53r = 0, _c53i = 0;
        _c53r = 1.0; _c53i = 0;
        double _c54r = 0, _c54i = 0;
        _c54r = 0.1; _c54i = 0;
        double _ang55r = 0, _ang55i = 0;
        _ang55r = c_arg(x1r, x1i); _ang55i = 0;
        double _mul56r = 0, _mul56i = 0;
        c_mul(r, 0, _ang55r, _ang55i, &_mul56r, &_mul56i);
        double _cos57r = 0, _cos57i = 0;
        c_cos(_mul56r, _mul56i, &_cos57r, &_cos57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_c54r, _c54i, _cos57r, _cos57i, &_mul58r, &_mul58i);
        double _ang59r = 0, _ang59i = 0;
        _ang59r = c_arg(x2r, x2i); _ang59i = 0;
        double _mul60r = 0, _mul60i = 0;
        c_mul(r, 0, _ang59r, _ang59i, &_mul60r, &_mul60i);
        double _sin61r = 0, _sin61i = 0;
        c_sin(_mul60r, _mul60i, &_sin61r, &_sin61i);
        double _mul62r = 0, _mul62i = 0;
        c_mul(_mul58r, _mul58i, _sin61r, _sin61i, &_mul62r, &_mul62i);
        double _add63r = 0, _add63i = 0;
        _add63r = _c53r + _mul62r; _add63i = _c53i + _mul62i;
        { double _tr = cRe[(r - 1)]*_add63r - cIm[(r - 1)]*_add63i; cIm[(r - 1)] = cRe[(r - 1)]*_add63i + cIm[(r - 1)]*_add63r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_473_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(_arr7r, _arr7i, _arr8r, _arr8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _mul9r + _c10r; _add11i = _mul9i + _c10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        _pow14r = _abs12r; _pow14i = _abs12i;
        double _log15r = 0, _log15i = 0;
        c_log(_pow14r, _pow14i, &_log15r, &_log15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 6.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin20r, _sin20i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _c16r + _mul25r; _add26i = _c16i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log15r, _log15i, _add26r, _add26i, &_mul27r, &_mul27i);
        double mag = _mul27r; /* +_mul27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 3.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(j, 0, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang33r, _ang33i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul32r + _mul37r; _add38i = _mul32i + _mul37i;
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(j, 0); _abs39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _abs39r + _c40r; _add41i = _abs39i + _c40i;
        double _log42r = 0, _log42i = 0;
        c_log(_add41r, _add41i, &_log42r, &_log42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add38r + _log42r; _add43i = _add38i + _log42i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_474_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(j, 0, j, 0, &_pow4r, &_pow4i);
        double k = _pow4r; /* +_pow4ii */
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs5r + j; _add6i = _abs5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x2r, x2i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log7r, _log7i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double r = _mul11r; /* +_mul11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(k, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _cos16r = 0, _cos16i = 0;
        c_cos(k, 0, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = _mul14r - _mul17r; _sub18i = _mul14i - _mul17i;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _sub18r + _log22r; _add23i = _sub18i + _log22i;
        double angle = _add23r; /* +_add23ii */
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1r; _attr24i = 0;
        double _cos25r = 0, _cos25i = 0;
        c_cos(k, 0, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_attr24r, _attr24i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _attr27r = 0, _attr27i = 0;
        _attr27r = x2i; _attr27i = 0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(k, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_attr27r, _attr27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul26r + _mul29r; _add30i = _mul26i + _mul29i;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x1r, x1i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_mul(_abs31r, _abs31i, _abs31r, _abs31i, &_pow33r, &_pow33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = k + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_pow33r, _pow33i, _add35r, _add35i, &_div36r, &_div36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_add30r, _add30i, _div36r, _div36i, &_mul37r, &_mul37i);
        double magnitude = _mul37r; /* +_mul37ii */
        double _cos38r = 0, _cos38i = 0;
        c_cos(angle, 0, &_cos38r, &_cos38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(angle, 0, &_sin39r, &_sin39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_sin39r, _sin39i, _c40r, _c40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos38r + _mul41r; _add42i = _cos38i + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_475_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 40.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_attr3r, _attr3i, j, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs5r + j; _add6i = _abs5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow4r + _log7r; _add8i = _pow4i + _log7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 3.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_c9r, _c9i, _c10r, _c10i, &_div11r, &_div11i);
        double _pow12r = 0, _pow12i = 0;
        c_powr(0, 0, _div11r, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _add8r + _pow12r; _add13i = _add8i + _pow12i;
        double mag = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 6.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang14r, _ang14i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 4.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang20r, _ang20i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul19r + _mul25r; _add26i = _mul19i + _mul25i;
        double angle = _add26r; /* +_add26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, angle, 0, &_mul28r, &_mul28i);
        double _exp29r = 0, _exp29i = 0;
        c_exp2(_mul28r, _mul28i, &_exp29r, &_exp29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag, 0, _exp29r, _exp29i, &_mul30r, &_mul30i);
        double _conj31r = 0, _conj31i = 0;
        _conj31r = x1r; _conj31i = -(x1i);
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_conj31r, _conj31i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul30r + _mul35r; _add36i = _mul30i + _mul35i;
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x2r; _conj37i = -(x2i);
        double _c38r = 0, _c38i = 0;
        _c38r = 3.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(j, 0, _c38r, _c38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj37r, _conj37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _sub42r = 0, _sub42i = 0;
        _sub42r = _add36r - _mul41r; _sub42i = _add36i - _mul41i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub42r; cIm[_idx] = _sub42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_476_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double mag_part1 = _mul9r; /* +_mul9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 0.5; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_abs10r, _abs10i, 0.5, &_pow12r, &_pow12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 3.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(j, 0, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_pow12r, _pow12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double mag_part2 = _mul16r; /* +_mul16ii */
        double _add17r = 0, _add17i = 0;
        _add17r = mag_part1 + mag_part2; _add17i = 0 + 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_mul(j, 0, j, 0, &_pow19r, &_pow19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add17r + _pow19r; _add20i = _add17i + _pow19i;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 4.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang21r, _ang21i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double angle_part1 = _mul26r; /* +_mul26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 5.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang27r, _ang27i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double angle_part2 = _mul32r; /* +_mul32ii */
        double _add33r = 0, _add33i = 0;
        _add33r = angle_part1 + angle_part2; _add33i = 0 + 0;
        double _sin34r = 0, _sin34i = 0;
        c_sin(j, 0, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add33r + _sin34r; _add35i = _add33i + _sin34i;
        double angle = _add35r; /* +_add35ii */
        double _cos36r = 0, _cos36i = 0;
        c_cos(angle, 0, &_cos36r, &_cos36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(angle, 0, &_sin37r, &_sin37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_sin37r, _sin37i, _c38r, _c38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _cos36r + _mul39r; _add40i = _cos36i + _mul39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude, 0, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_477_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 40.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x2r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr4r, _attr4i, j, 0, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _attr3r + _mul5r; _add6i = _attr3i + _mul5i;
        double r = _add6r; /* +_add6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1i; _attr7i = 0;
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_attr8r, _attr8i, j, 0, &_mul9r, &_mul9i);
        double _sub10r = 0, _sub10i = 0;
        _sub10r = _attr7r - _mul9r; _sub10i = _attr7i - _mul9i;
        double i_part = _sub10r; /* +_sub10ii */
        double _sin11r = 0, _sin11i = 0;
        c_sin(r, 0, &_sin11r, &_sin11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(i_part, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin11r, _sin11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs14r + j; _add15i = _abs14i + 0;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul13r + _log16r; _add17i = _mul13i + _log16i;
        double phase = _add17r; /* +_add17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.5; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_abs18r, _abs18i, 0.5, &_pow20r, &_pow20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x2r, x2i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 0.3; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs21r, _abs21i, 0.3, &_pow23r, &_pow23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_pow20r, _pow20i, _pow23r, _pow23i, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(j, 0, &_sin25r, &_sin25i);
        double _pow26r = 0, _pow26i = 0;
        c_powr(j, 0, _sin25r, &_pow26r, &_pow26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul24r, _mul24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, _ang28r, _ang28i, &_mul29r, &_mul29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_mul29r, _mul29i, &_cos30r, &_cos30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul27r + _cos30r; _add31i = _mul27i + _cos30i;
        double magnitude = _add31r; /* +_add31ii */
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, phase, 0, &_mul33r, &_mul33i);
        double _exp34r = 0, _exp34i = 0;
        c_exp2(_mul33r, _mul33i, &_exp34r, &_exp34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(magnitude, 0, _exp34r, _exp34i, &_mul35r, &_mul35i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_478_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double mag_part1 = _log5r; /* +_log5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1i; _attr9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _attr9r + _c10r; _add11i = _attr9i + _c10i;
        double _div12r = 0, _div12i = 0;
        c_div(j, 0, _add11r, _add11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin8r, _sin8i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double mag_part2 = _mul14r; /* +_mul14ii */
        double _mul15r = 0, _mul15i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 0.5; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(0, 0, 0.5, &_pow17r, &_pow17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul15r + _pow17r; _add18i = _mul15i + _pow17i;
        double magnitude = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _cos23r = 0, _cos23i = 0;
        c_cos(j, 0, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
        double angle_part1 = _add25r; /* +_add25ii */
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x1r; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _sin28r - _cos31r; _sub32i = _sin28i - _cos31i;
        double angle_part2 = _sub32r; /* +_sub32ii */
        double _add33r = 0, _add33i = 0;
        _add33r = angle_part1 + angle_part2; _add33i = 0 + 0;
        double angle = _add33r; /* +_add33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, angle, 0, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(magnitude, 0, _exp36r, _exp36i, &_mul37r, &_mul37i);
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x1r; _conj38i = -(x1i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(j, 0, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_conj38r, _conj38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul37r + _mul40r; _add41i = _mul37i + _mul40i;
        double _conj42r = 0, _conj42i = 0;
        _conj42r = x2r; _conj42i = -(x2i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(j, 0, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_conj42r, _conj42i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _add41r - _mul44r; _sub45i = _add41i - _mul44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_479_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_arr7r, _arr7i); _abs8i = 0;
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
        double mag_part1 = _mul16r; /* +_mul16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _mod19r = 0, _mod19i = 0;
        _mod19r = fmod(j, _c18r); _mod19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _mod19r + _c20r; _add21i = _mod19i + _c20i;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_abs17r, _abs17i, _add21r, &_pow22r, &_pow22i);
        double mag_part2 = _pow22r; /* +_pow22ii */
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag_part2, 0, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + _mul26r; _add27i = 0 + _mul26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double angle_part1 = _mul32r; /* +_mul32ii */
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x2i; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 6.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _sin35r + _cos39r; _add40i = _sin35i + _cos39i;
        double angle_part2 = _add40r; /* +_add40ii */
        double _add41r = 0, _add41i = 0;
        _add41r = angle_part1 + angle_part2; _add41i = 0 + 0;
        double angle = _add41r; /* +_add41ii */
        double _cos42r = 0, _cos42i = 0;
        c_cos(angle, 0, &_cos42r, &_cos42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(angle, 0, &_sin43r, &_sin43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_sin43r, _sin43i, _c44r, _c44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _cos42r + _mul45r; _add46i = _cos42i + _mul45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(mag, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_480_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double mag_part1 = _mul9r; /* +_mul9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs10r + j; _add11i = _abs10i + 0;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log12r, _log12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double mag_part2 = _mul16r; /* +_mul16ii */
        double _add17r = 0, _add17i = 0;
        _add17r = mag_part1 + mag_part2; _add17i = 0 + 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.5; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(j, 0, 0.5, &_pow19r, &_pow19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add17r + _pow19r; _add20i = _add17i + _pow19i;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x1r; _attr22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _attr22r + _c23r; _add24i = _attr22i + _c23i;
        double _div25r = 0, _div25i = 0;
        c_div(j, 0, _add24r, _add24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang21r, _ang21i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double angle_part1 = _mul27r; /* +_mul27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _attr29r + _c30r; _add31i = _attr29i + _c30i;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _add31r, _add31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang28r, _ang28i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double angle_part2 = _mul34r; /* +_mul34ii */
        double _sub35r = 0, _sub35i = 0;
        _sub35r = angle_part1 - angle_part2; _sub35i = 0 - 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 6.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _sub35r + _sin39r; _add40i = _sub35i + _sin39i;
        double angle = _add40r; /* +_add40ii */
        double _cos41r = 0, _cos41i = 0;
        c_cos(angle, 0, &_cos41r, &_cos41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(angle, 0, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c42r, _c42i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos41r + _mul44r; _add45i = _cos41i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_481_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 1.0;
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_c8r, _c8i, M_PI, 0, &_mul9r, &_mul9i);
        double _arr10r = 0, _arr10i = 0;
        { int _idx = (j - 1); _arr10r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr10i = 0; }
        double _mul11r = 0, _mul11i = 0;
        c_mul(_mul9r, _mul9i, _arr10r, _arr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_c7r, _c7i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _exp14r = 0, _exp14i = 0;
        c_exp2(_mul13r, _mul13i, &_exp14r, &_exp14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1r; _attr15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _attr15r + j; _add16i = _attr15i + 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_exp14r, _exp14i, _add16r, _add16i, &_mul17r, &_mul17i);
        double term1 = _mul17r; /* +_mul17ii */
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 1.0;
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c19r, _c19i, M_PI, 0, &_mul20r, &_mul20i);
        double _arr21r = 0, _arr21i = 0;
        { int _idx = (j - 1); _arr21r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr21i = 0; }
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul20r, _mul20i, _arr21r, _arr21i, &_mul22r, &_mul22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c18r, _c18i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x2i; _attr26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_mul(j, 0, j, 0, &_pow28r, &_pow28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _attr26r + _pow28r; _add29i = _attr26i + _pow28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_exp25r, _exp25i, _add29r, _add29i, &_mul30r, &_mul30i);
        double term2 = _mul30r; /* +_mul30ii */
        double _add31r = 0, _add31i = 0;
        _add31r = term1 + term2; _add31i = 0 + 0;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x1r, x1i); _abs32i = 0;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(x2r, x2i); _abs33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_abs32r, _abs32i, _abs33r, _abs33i, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _mul34r + _c35r; _add36i = _mul34i + _c35i;
        double _log37r = 0, _log37i = 0;
        c_log(_add36r, _add36i, &_log37r, &_log37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add31r + _log37r; _add38i = _add31i + _log37i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    }
    double _c39r = 0, _c39i = 0;
    _c39r = 1.0; _c39i = 0;
    double _add40r = 0, _add40i = 0;
    _add40r = n + _c39r; _add40i = 0 + _c39i;
    for (int k = 1; k < (int)(_add40r); k++) {
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 0.05; _c42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, k, 0, &_mul43r, &_mul43i);
        double _cf44r = 0, _cf44i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf44r = cRe[_idx]; _cf44i = cIm[_idx]; } }
        double _attr45r = 0, _attr45i = 0;
        _attr45r = _cf44r; _attr45i = 0;
        double _sin46r = 0, _sin46i = 0;
        c_sin(_attr45r, _attr45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_mul43r, _mul43i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _c41r + _mul47r; _add48i = _c41i + _mul47i;
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 0.05;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c49r, _c49i, k, 0, &_mul50r, &_mul50i);
        double _cf51r = 0, _cf51i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
        double _attr52r = 0, _attr52i = 0;
        _attr52r = _cf51i; _attr52i = 0;
        double _cos53r = 0, _cos53i = 0;
        c_cos(_attr52r, _attr52i, &_cos53r, &_cos53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_mul50r, _mul50i, _cos53r, _cos53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _add48r + _mul54r; _add55i = _add48i + _mul54i;
        { double _tr = cRe[(k - 1)]*_add55r - cIm[(k - 1)]*_add55i; cIm[(k - 1)] = cRe[(k - 1)]*_add55i + cIm[(k - 1)]*_add55r; cRe[(k - 1)] = _tr; }
    }
    double _c56r = 0, _c56i = 0;
    _c56r = 1.0; _c56i = 0;
    double _add57r = 0, _add57i = 0;
    _add57r = n + _c56r; _add57i = 0 + _c56i;
    for (int r = 1; r < (int)(_add57r); r++) {
        double _cf58r = 0, _cf58i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf58r = cRe[_idx]; _cf58i = cIm[_idx]; } }
        double _conj59r = 0, _conj59i = 0;
        _conj59r = _cf58r; _conj59i = -(_cf58i);
        double _attr60r = 0, _attr60i = 0;
        _attr60r = x1r; _attr60i = 0;
        double _mul61r = 0, _mul61i = 0;
        c_mul(r, 0, _attr60r, _attr60i, &_mul61r, &_mul61i);
        double _sin62r = 0, _sin62i = 0;
        c_sin(_mul61r, _mul61i, &_sin62r, &_sin62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(_conj59r, _conj59i, _sin62r, _sin62i, &_mul63r, &_mul63i);
        double _attr64r = 0, _attr64i = 0;
        _attr64r = x2i; _attr64i = 0;
        double _mul65r = 0, _mul65i = 0;
        c_mul(r, 0, _attr64r, _attr64i, &_mul65r, &_mul65i);
        double _cos66r = 0, _cos66i = 0;
        c_cos(_mul65r, _mul65i, &_cos66r, &_cos66i);
        double _mul67r = 0, _mul67i = 0;
        c_mul(_mul63r, _mul63i, _cos66r, _cos66i, &_mul67r, &_mul67i);
        cRe[(r - 1)] += _mul67r; cIm[(r - 1)] += _mul67i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_482_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double mag_part1 = _log5r; /* +_log5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1i; _attr9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _attr9r + _c10r; _add11i = _attr9i + _c10i;
        double _div12r = 0, _div12i = 0;
        c_div(j, 0, _add11r, _add11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin8r, _sin8i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double mag_part2 = _mul14r; /* +_mul14ii */
        double _mul15r = 0, _mul15i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 0.5; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(0, 0, 0.5, &_pow17r, &_pow17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul15r + _pow17r; _add18i = _mul15i + _pow17i;
        double magnitude = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _cos23r = 0, _cos23i = 0;
        c_cos(j, 0, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
        double angle_part1 = _add25r; /* +_add25ii */
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x1r; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _sin28r - _cos31r; _sub32i = _sin28i - _cos31i;
        double angle_part2 = _sub32r; /* +_sub32ii */
        double _add33r = 0, _add33i = 0;
        _add33r = angle_part1 + angle_part2; _add33i = 0 + 0;
        double angle = _add33r; /* +_add33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, angle, 0, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(magnitude, 0, _exp36r, _exp36i, &_mul37r, &_mul37i);
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x1r; _conj38i = -(x1i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(j, 0, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_conj38r, _conj38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul37r + _mul40r; _add41i = _mul37i + _mul40i;
        double _conj42r = 0, _conj42i = 0;
        _conj42r = x2r; _conj42i = -(x2i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(j, 0, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_conj42r, _conj42i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _add41r - _mul44r; _sub45i = _add41i - _mul44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_483_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double r1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double i1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double r2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double i2 = _attr4r; /* +_attr4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _pow7r = 0, _pow7i = 0;
        c_powr(r1, 0, j, &_pow7r, &_pow7i);
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x2r, x2i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_pow7r, _pow7i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double part1 = _mul11r; /* +_mul11ii */
        double _sub12r = 0, _sub12i = 0;
        _sub12r = n - j; _sub12i = 0 - 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(i2, 0, _sub12r, &_pow13r, &_pow13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _abs14r, _abs14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_pow13r, _pow13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double part2 = _mul17r; /* +_mul17ii */
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
        double part3 = _log22r; /* +_log22ii */
        double _add23r = 0, _add23i = 0;
        _add23r = r1 + j; _add23i = 0 + 0;
        double _add24r = 0, _add24i = 0;
        _add24r = i2 + j; _add24i = 0 + 0;
        double _prod25r = 0, _prod25i = 0;
        c_mul(_add23r, _add23i, _add24r, _add24i, &_prod25r, &_prod25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _prod30r = 0, _prod30i = 0;
        c_mul(_prod25r, _prod25i, _log29r, _log29i, &_prod30r, &_prod30i);
        double part4 = _prod30r; /* +_prod30ii */
        double _mul31r = 0, _mul31i = 0;
        c_mul(part1, 0, part2, 0, &_mul31r, &_mul31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(part3, 0, part4, 0, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul31r + _mul32r; _add33i = _mul31i + _mul32i;
        double magnitude = _add33r; /* +_add33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _sin35r = 0, _sin35i = 0;
        c_sin(j, 0, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang34r, _ang34i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _cos38r = 0, _cos38i = 0;
        c_cos(j, 0, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang37r, _ang37i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul36r + _mul39r; _add40i = _mul36i + _mul39i;
        double _abs41r = 0, _abs41i = 0;
        _abs41r = c_abs(x1r, x1i); _abs41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = _abs41r + _c42r; _add43i = _abs41i + _c42i;
        double _log44r = 0, _log44i = 0;
        c_log(_add43r, _add43i, &_log44r, &_log44i);
        double _div45r = 0, _div45i = 0;
        c_div(_log44r, _log44i, j, 0, &_div45r, &_div45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _add40r + _div45r; _add46i = _add40i + _div45i;
        double angle = _add46r; /* +_add46ii */
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c47r, _c47i, angle, 0, &_mul48r, &_mul48i);
        double _exp49r = 0, _exp49i = 0;
        c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(magnitude, 0, _exp49r, _exp49i, &_mul50r, &_mul50i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_484_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _sqrt6r = 0, _sqrt6i = 0;
        c_powr(j, 0, 0.5, &_sqrt6r, &_sqrt6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log5r, _log5i, _sqrt6r, _sqrt6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(_sin10r, _sin10i, _sin10r, _sin10i, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _pow12r; _add13i = _mul7i + _pow12i;
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = j + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_mul15r, _mul15i, _add17r, _add17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add13r + _cos19r; _add20i = _add13i + _cos19i;
        double mag = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang21r, _ang21i, j, 0, &_mul22r, &_mul22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2r; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul24r, _mul24i, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul22r + _sin27r; _add28i = _mul22i + _sin27i;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _add28r - _cos31r; _sub32i = _add28i - _cos31i;
        double angle = _sub32r; /* +_sub32ii */
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
        c_mul(mag, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_484_old_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _sqrt6r = 0, _sqrt6i = 0;
        c_powr(j, 0, 0.5, &_sqrt6r, &_sqrt6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log5r, _log5i, _sqrt6r, _sqrt6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(_sin10r, _sin10i, _sin10r, _sin10i, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _pow12r; _add13i = _mul7i + _pow12i;
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = j + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_mul15r, _mul15i, _add17r, _add17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add13r + _cos19r; _add20i = _add13i + _cos19i;
        double mag = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang21r, _ang21i, j, 0, &_mul22r, &_mul22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2r; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul24r, _mul24i, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul22r + _sin27r; _add28i = _mul22i + _sin27i;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _add28r - _cos31r; _sub32i = _add28i - _cos31i;
        double angle = _sub32r; /* +_sub32ii */
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
        c_mul(mag, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_485_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 4.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin10r, _sin10i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _arr16r = 0, _arr16i = 0;
        { int _idx = (j - 1); _arr16r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr16i = 0; }
        double _arr17r = 0, _arr17i = 0;
        { int _idx = (j - 1); _arr17r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr17i = 0; }
        double _add18r = 0, _add18i = 0;
        _add18r = _arr16r + _arr17r; _add18i = _arr16i + _arr17i;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(_add18r, _add18i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul15r + _log22r; _add23i = _mul15i + _log22i;
        double phase = _add23r; /* +_add23ii */
        double _arr24r = 0, _arr24i = 0;
        { int _idx = (j - 1); _arr24r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr24i = 0; }
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(_arr24r, _arr24i, _arr24r, _arr24i, &_pow26r, &_pow26i);
        double _arr27r = 0, _arr27i = 0;
        { int _idx = (j - 1); _arr27r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr27i = 0; }
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(_arr27r, _arr27i, _arr27r, _arr27i, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _pow26r + _pow29r; _add30i = _pow26i + _pow29i;
        double _sqrt31r = 0, _sqrt31i = 0;
        c_powr(_add30r, _add30i, 0.5, &_sqrt31r, &_sqrt31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 0.1; _c33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c33r, _c33i, j, 0, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _c32r + _mul34r; _add35i = _c32i + _mul34i;
        double _pow36r = 0, _pow36i = 0;
        c_powr(_sqrt31r, _sqrt31i, _add35r, &_pow36r, &_pow36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(j, 0, &_sin37r, &_sin37i);
        double _abs38r = 0, _abs38i = 0;
        _abs38r = c_abs(_sin37r, _sin37i); _abs38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_pow36r, _pow36i, _abs38r, _abs38i, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 2.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(j, 0, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _abs43r = 0, _abs43i = 0;
        _abs43r = c_abs(_cos42r, _cos42i); _abs43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = _mul39r + _abs43r; _add44i = _mul39i + _abs43i;
        double magnitude = _add44r; /* +_add44ii */
        double _cos45r = 0, _cos45i = 0;
        c_cos(phase, 0, &_cos45r, &_cos45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _sin47r = 0, _sin47i = 0;
        c_sin(phase, 0, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c46r, _c46i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _cos45r + _mul48r; _add49i = _cos45i + _mul48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(magnitude, 0, _add49r, _add49i, &_mul50r, &_mul50i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_486_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _abs4r; _add5i = _abs3i + _abs4i;
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + j; _add6i = _add5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2i; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin10r + _cos13r; _add14i = _sin10i + _cos13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(_add14r, _add14i); _abs15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log7r, _log7i, _abs15r, _abs15i, &_mul16r, &_mul16i);
        double mag_real = _mul16r; /* +_mul16ii */
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
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x1i; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _attr22r, _attr22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _sin24r - _cos27r; _sub28i = _sin24i - _cos27i;
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(_sub28r, _sub28i); _abs29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_log21r, _log21i, _abs29r, _abs29i, &_mul30r, &_mul30i);
        double mag_imag = _mul30r; /* +_mul30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, n, 0, &_div32r, &_div32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_div32r, _div32i, M_PI, 0, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, n, 0, &_div37r, &_div37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_div37r, _div37i, M_PI, 0, &_mul38r, &_mul38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_mul38r, _mul38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang36r, _ang36i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul35r + _mul40r; _add41i = _mul35i + _mul40i;
        double angle_real = _add41r; /* +_add41ii */
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x1r, x1i); _ang42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(j, 0, n, 0, &_div43r, &_div43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_div43r, _div43i, M_PI, 0, &_mul44r, &_mul44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_mul44r, _mul44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang42r, _ang42i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x2r, x2i); _ang47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(j, 0, n, 0, &_div48r, &_div48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_div48r, _div48i, M_PI, 0, &_mul49r, &_mul49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(_mul49r, _mul49i, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_ang47r, _ang47i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _sub52r = 0, _sub52i = 0;
        _sub52r = _mul46r - _mul51r; _sub52i = _mul46i - _mul51i;
        double angle_imag = _sub52r; /* +_sub52ii */
        double _cos53r = 0, _cos53i = 0;
        c_cos(angle_real, 0, &_cos53r, &_cos53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(mag_real, 0, _cos53r, _cos53i, &_mul54r, &_mul54i);
        double _sin55r = 0, _sin55i = 0;
        c_sin(angle_imag, 0, &_sin55r, &_sin55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(mag_imag, 0, _sin55r, _sin55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _mul54r + _mul56r; _add57i = _mul54i + _mul56i;
        double _c58r = 0, _c58i = 0;
        _c58r = 0.0; _c58i = 1.0;
        double _sin59r = 0, _sin59i = 0;
        c_sin(angle_real, 0, &_sin59r, &_sin59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(mag_real, 0, _sin59r, _sin59i, &_mul60r, &_mul60i);
        double _cos61r = 0, _cos61i = 0;
        c_cos(angle_imag, 0, &_cos61r, &_cos61i);
        double _mul62r = 0, _mul62i = 0;
        c_mul(mag_imag, 0, _cos61r, _cos61i, &_mul62r, &_mul62i);
        double _sub63r = 0, _sub63i = 0;
        _sub63r = _mul60r - _mul62r; _sub63i = _mul60i - _mul62i;
        double _mul64r = 0, _mul64i = 0;
        c_mul(_c58r, _c58i, _sub63r, _sub63i, &_mul64r, &_mul64i);
        double _add65r = 0, _add65i = 0;
        _add65r = _add57r + _mul64r; _add65i = _add57i + _mul64i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add65r; cIm[_idx] = _add65i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_487_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x2r; _attr4i = 0;
        double rec_seq[35];
        for (int _li = 0; _li < 35; _li++) {
            rec_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
        }
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1i; _attr5i = 0;
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double imc_seq[35];
        for (int _li = 0; _li < 35; _li++) {
            imc_seq[_li] = _attr5r + (_attr6r - _attr5r) * _li / 34.0;
        }
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_arr7r, _arr7i); _abs8i = 0;
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
        double mag_part1 = _mul16r; /* +_mul16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _mod19r = 0, _mod19i = 0;
        _mod19r = fmod(j, _c18r); _mod19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _mod19r + _c20r; _add21i = _mod19i + _c20i;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_abs17r, _abs17i, _add21r, &_pow22r, &_pow22i);
        double mag_part2 = _pow22r; /* +_pow22ii */
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag_part2, 0, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + _mul26r; _add27i = 0 + _mul26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double angle_part1 = _mul32r; /* +_mul32ii */
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x2i; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 6.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _sin35r + _cos39r; _add40i = _sin35i + _cos39i;
        double angle_part2 = _add40r; /* +_add40ii */
        double _add41r = 0, _add41i = 0;
        _add41r = angle_part1 + angle_part2; _add41i = 0 + 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 40) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    double _c48r = 0, _c48i = 0;
    _c48r = 1.0; _c48i = 0;
    double _add49r = 0, _add49i = 0;
    _add49r = n + _c48r; _add49i = 0 + _c48i;
    for (int k = 1; k < (int)(_add49r); k++) {
        double _c50r = 0, _c50i = 0;
        _c50r = 1.0; _c50i = 0;
        if ((k > _c50r) && (k < n)) {
            double _c51r = 0, _c51i = 0;
            _c51r = 0.5; _c51i = 0;
            double _cf52r = 0, _cf52i = 0;
            { int _idx = (k - 2); if (_idx >= 0 && _idx < 40) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
            double _cf53r = 0, _cf53i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 40) { _cf53r = cRe[_idx]; _cf53i = cIm[_idx]; } }
            double _conj54r = 0, _conj54i = 0;
            _conj54r = _cf53r; _conj54i = -(_cf53i);
            double _mul55r = 0, _mul55i = 0;
            c_mul(_cf52r, _cf52i, _conj54r, _conj54i, &_mul55r, &_mul55i);
            double _mul56r = 0, _mul56i = 0;
            c_mul(_c51r, _c51i, _mul55r, _mul55i, &_mul56r, &_mul56i);
            double _mul57r = 0, _mul57i = 0;
            c_mul(k, 0, M_PI, 0, &_mul57r, &_mul57i);
            double _div58r = 0, _div58i = 0;
            c_div(_mul57r, _mul57i, n, 0, &_div58r, &_div58i);
            double _cos59r = 0, _cos59i = 0;
            c_cos(_div58r, _div58i, &_cos59r, &_cos59i);
            double _mul60r = 0, _mul60i = 0;
            c_mul(_mul56r, _mul56i, _cos59r, _cos59i, &_mul60r, &_mul60i);
            cRe[(k - 1)] += _mul60r; cIm[(k - 1)] += _mul60i;
        } else {
            double _c61r = 0, _c61i = 0;
            _c61r = 1.0; _c61i = 0;
            if (k == _c61r) {
                double _c62r = 0, _c62i = 0;
                _c62r = 0.3; _c62i = 0;
                double _cf63r = 0, _cf63i = 0;
                { int _idx = k; if (_idx >= 0 && _idx < 40) { _cf63r = cRe[_idx]; _cf63i = cIm[_idx]; } }
                double _conj64r = 0, _conj64i = 0;
                _conj64r = _cf63r; _conj64i = -(_cf63i);
                double _mul65r = 0, _mul65i = 0;
                c_mul(_c62r, _c62i, _conj64r, _conj64i, &_mul65r, &_mul65i);
                double _mul66r = 0, _mul66i = 0;
                c_mul(k, 0, M_PI, 0, &_mul66r, &_mul66i);
                double _div67r = 0, _div67i = 0;
                c_div(_mul66r, _mul66i, n, 0, &_div67r, &_div67i);
                double _sin68r = 0, _sin68i = 0;
                c_sin(_div67r, _div67i, &_sin68r, &_sin68i);
                double _mul69r = 0, _mul69i = 0;
                c_mul(_mul65r, _mul65i, _sin68r, _sin68i, &_mul69r, &_mul69i);
                cRe[(k - 1)] += _mul69r; cIm[(k - 1)] += _mul69i;
            } else {
                double _c70r = 0, _c70i = 0;
                _c70r = 0.3; _c70i = 0;
                double _cf71r = 0, _cf71i = 0;
                { int _idx = (k - 2); if (_idx >= 0 && _idx < 40) { _cf71r = cRe[_idx]; _cf71i = cIm[_idx]; } }
                double _conj72r = 0, _conj72i = 0;
                _conj72r = _cf71r; _conj72i = -(_cf71i);
                double _mul73r = 0, _mul73i = 0;
                c_mul(_c70r, _c70i, _conj72r, _conj72i, &_mul73r, &_mul73i);
                double _mul74r = 0, _mul74i = 0;
                c_mul(k, 0, M_PI, 0, &_mul74r, &_mul74i);
                double _div75r = 0, _div75i = 0;
                c_div(_mul74r, _mul74i, n, 0, &_div75r, &_div75i);
                double _sin76r = 0, _sin76i = 0;
                c_sin(_div75r, _div75i, &_sin76r, &_sin76i);
                double _mul77r = 0, _mul77i = 0;
                c_mul(_mul73r, _mul73i, _sin76r, _sin76i, &_mul77r, &_mul77i);
                cRe[(k - 1)] += _mul77r; cIm[(k - 1)] += _mul77i;
            }
        }
    }
    for (int _i = 0; _i < 40; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_488_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double r1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double i1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double r2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double i2 = _attr4r; /* +_attr4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_sin12r, _sin12i); _abs13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _abs13r, _abs13i, &_mul14r, &_mul14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs15r + j; _add16i = _abs15i + 0;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x1i; _attr18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, _attr18r, _attr18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_mul19r, _mul19i, _add21r, _add21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_cos23r, _cos23i); _abs24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log17r, _log17i, _abs24r, _abs24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul14r + _mul25r; _add26i = _mul14i + _mul25i;
        double mag = _add26r; /* +_add26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(j, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang27r, _ang27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _cos31r = 0, _cos31i = 0;
        c_cos(j, 0, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang30r, _ang30i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul29r + _mul32r; _add33i = _mul29i + _mul32i;
        double angle_part1 = _add33r; /* +_add33ii */
        double _attr34r = 0, _attr34i = 0;
        _attr34r = x1r; _attr34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, _attr34r, _attr34i, &_mul35r, &_mul35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_mul35r, _mul35i, &_sin36r, &_sin36i);
        double _attr37r = 0, _attr37i = 0;
        _attr37r = x2i; _attr37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, _attr37r, _attr37i, &_mul38r, &_mul38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_mul38r, _mul38i, &_cos39r, &_cos39i);
        double _sub40r = 0, _sub40i = 0;
        _sub40r = _sin36r - _cos39r; _sub40i = _sin36i - _cos39i;
        double angle_part2 = _sub40r; /* +_sub40ii */
        double _add41r = 0, _add41i = 0;
        _add41r = angle_part1 + angle_part2; _add41i = 0 + 0;
        double angle = _add41r; /* +_add41ii */
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, angle, 0, &_mul43r, &_mul43i);
        double _exp44r = 0, _exp44i = 0;
        c_exp2(_mul43r, _mul43i, &_exp44r, &_exp44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag, 0, _exp44r, _exp44i, &_mul45r, &_mul45i);
        double _conj46r = 0, _conj46i = 0;
        _conj46r = x1r; _conj46i = -(x1i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(j, 0, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_conj46r, _conj46i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul45r + _mul48r; _add49i = _mul45i + _mul48i;
        double _conj50r = 0, _conj50i = 0;
        _conj50r = x2r; _conj50i = -(x2i);
        double _cos51r = 0, _cos51i = 0;
        c_cos(j, 0, &_cos51r, &_cos51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_conj50r, _conj50i, _cos51r, _cos51i, &_mul52r, &_mul52i);
        double _sub53r = 0, _sub53i = 0;
        _sub53r = _add49r - _mul52r; _sub53i = _add49i - _mul52i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub53r; cIm[_idx] = _sub53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_489_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double sum_mag = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        for (int k = 1; k < (int)(_add5r); k++) {
            double _attr6r = 0, _attr6i = 0;
            _attr6r = x1r; _attr6i = 0;
            double _pow7r = 0, _pow7i = 0;
            c_powr(_attr6r, _attr6i, k, &_pow7r, &_pow7i);
            double _ang8r = 0, _ang8i = 0;
            _ang8r = c_arg(x1r, x1i); _ang8i = 0;
            double _mul9r = 0, _mul9i = 0;
            c_mul(k, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
            double _sin10r = 0, _sin10i = 0;
            c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
            double _mul11r = 0, _mul11i = 0;
            c_mul(_pow7r, _pow7i, _sin10r, _sin10i, &_mul11r, &_mul11i);
            sum_mag += _mul11r;
        }
        double _sub12r = 0, _sub12i = 0;
        _sub12r = n - j; _sub12i = 0 - 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _sub12r + _c13r; _add14i = _sub12i + _c13i;
        for (int r = 1; r < (int)(_add14r); r++) {
            double _attr15r = 0, _attr15i = 0;
            _attr15r = x2i; _attr15i = 0;
            double _pow16r = 0, _pow16i = 0;
            c_powr(_attr15r, _attr15i, r, &_pow16r, &_pow16i);
            double _ang17r = 0, _ang17i = 0;
            _ang17r = c_arg(x2r, x2i); _ang17i = 0;
            double _mul18r = 0, _mul18i = 0;
            c_mul(r, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
            double _cos19r = 0, _cos19i = 0;
            c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
            double _mul20r = 0, _mul20i = 0;
            c_mul(_pow16r, _pow16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
            sum_mag += _mul20r;
        }
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = sum_mag + _c21r; _add22i = 0 + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double mag = _log23r; /* +_log23ii */
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _div26r = 0, _div26i = 0;
        c_div(sum_mag, 0, _add25r, _add25i, &_div26r, &_div26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = n - j; _sub27i = 0 - 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _sub27r + _c28r; _add29i = _sub27i + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(sum_mag, 0, _add29r, _add29i, &_div30r, &_div30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _div26r + _div30r; _add31i = _div26i + _div30i;
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

static void poly_490_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 5.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(j, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_attr3r, _attr3i, _add7r, &_pow8r, &_pow8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 7.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(j, 0, _c10r, _c10i, &_div11r, &_div11i);
        double _flr12r = 0, _flr12i = 0;
        _flr12r = floor(_div11r); _flr12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _flr12r + _c13r; _add14i = _flr12i + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs9r, _abs9i, _add14r, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _pow8r + _pow15r; _add16i = _pow8i + _pow15i;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log19r, _log19i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _add16r + _mul24r; _add25i = _add16i + _mul24i;
        double mag = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 6.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang26r, _ang26i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 8.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang32r, _ang32i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul31r + _mul37r; _add38i = _mul31i + _mul37i;
        double angle = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(mag, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    double _c43r = 0, _c43i = 0;
    _c43r = 1.0; _c43i = 0;
    double _add44r = 0, _add44i = 0;
    _add44r = n + _c43r; _add44i = 0 + _c43i;
    for (int k = 1; k < (int)(_add44r); k++) {
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x1r; _conj45i = -(x1i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(k, 0, M_PI, 0, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 5.0; _c47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(_mul46r, _mul46i, _c47r, _c47i, &_div48r, &_div48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_div48r, _div48i, &_cos49r, &_cos49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_conj45r, _conj45i, _cos49r, _cos49i, &_mul50r, &_mul50i);
        double _conj51r = 0, _conj51i = 0;
        _conj51r = x2r; _conj51i = -(x2i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(k, 0, M_PI, 0, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 3.0; _c53i = 0;
        double _div54r = 0, _div54i = 0;
        c_div(_mul52r, _mul52i, _c53r, _c53i, &_div54r, &_div54i);
        double _sin55r = 0, _sin55i = 0;
        c_sin(_div54r, _div54i, &_sin55r, &_sin55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_conj51r, _conj51i, _sin55r, _sin55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _mul50r + _mul56r; _add57i = _mul50i + _mul56i;
        cRe[(k - 1)] += _add57r; cIm[(k - 1)] += _add57i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_491_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double r = _arr7r; /* +_arr7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr8r, _attr8i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2i; _attr11i = 0;
        double _cos12r = 0, _cos12i = 0;
        c_cos(j, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr11r, _attr11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul10r + _mul13r; _add14i = _mul10i + _mul13i;
        double i_part = _add14r; /* +_add14ii */
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
        _c20r = 1.5; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(j, 0, 1.5, &_pow21r, &_pow21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _pow21r + 0; _add22i = _pow21i + 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log19r, _log19i, _add22r, _add22i, &_mul23r, &_mul23i);
        double magnitude = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _div26r = 0, _div26i = 0;
        c_div(_mul25r, _mul25i, n, 0, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _div31r = 0, _div31i = 0;
        c_div(_mul30r, _mul30i, n, 0, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul28r + _mul33r; _add34i = _mul28i + _mul33i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_492_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, M_PI, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 7.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul4r, _mul4i, _c5r, _c5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr3r, _attr3i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr9r, _attr9i, _log12r, _log12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _mul13r; _add14i = _mul8i + _mul13i;
        double real_part = _add14r; /* +_add14ii */
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_mul(j, 0, j, 0, &_pow17r, &_pow17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_pow17r, _pow17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_attr15r, _attr15i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x2i; _attr22i = 0;
        double _neg23r = 0, _neg23i = 0;
        _neg23r = -(j); _neg23i = -(0);
        double _c24r = 0, _c24i = 0;
        _c24r = 10.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_neg23r, _neg23i, _c24r, _c24i, &_div25r, &_div25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_div25r, _div25i, &_exp26r, &_exp26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_attr22r, _attr22i, _exp26r, _exp26i, &_mul27r, &_mul27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul21r - _mul27r; _sub28i = _mul21i - _mul27i;
        double imag_part = _sub28r; /* +_sub28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs29r + _abs30r; _add31i = _abs29i + _abs30i;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.5; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_powr(j, 0, 1.5, &_pow33r, &_pow33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = n - j; _sub34i = 0 - 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.2; _c35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_powr(_sub34r, _sub34i, 1.2, &_pow36r, &_pow36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _pow33r + _pow36r; _add37i = _pow33i + _pow36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_add31r, _add31i, _add37r, _add37i, &_mul38r, &_mul38i);
        double magnitude = _mul38r; /* +_mul38ii */
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _sqrt40r = 0, _sqrt40i = 0;
        c_powr(j, 0, 0.5, &_sqrt40r, &_sqrt40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang39r, _ang39i, _sqrt40r, _sqrt40i, &_mul41r, &_mul41i);
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x2r, x2i); _ang42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(j, 0, M_PI, 0, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 3.0; _c44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(_mul43r, _mul43i, _c44r, _c44i, &_div45r, &_div45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_div45r, _div45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_ang42r, _ang42i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul41r + _mul47r; _add48i = _mul41i + _mul47i;
        double angle = _add48r; /* +_add48ii */
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c49r, _c49i, imag_part, 0, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = real_part + _mul50r; _add51i = 0 + _mul50i;
        double _cos52r = 0, _cos52i = 0;
        c_cos(angle, 0, &_cos52r, &_cos52i);
        double _sin53r = 0, _sin53i = 0;
        c_sin(angle, 0, &_sin53r, &_sin53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 1.0;
        double _mul55r = 0, _mul55i = 0;
        c_mul(_sin53r, _sin53i, _c54r, _c54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _cos52r + _mul55r; _add56i = _cos52i + _mul55i;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_add51r, _add51i, _add56r, _add56i, &_mul57r, &_mul57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_mul57r, _mul57i, magnitude, 0, &_mul58r, &_mul58i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_493_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 40.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _abs4r; _add5i = _abs3i + _abs4i;
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + j; _add6i = _add5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _c8r + _sin9r; _add10i = _c8i + _sin9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 3.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(j, 0, _c11r, _c11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add10r + _cos13r; _add14i = _add10i + _cos13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log7r, _log7i, _add14r, _add14i, &_mul15r, &_mul15i);
        double mag = _mul15r; /* +_mul15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang16r, _ang16i, j, 0, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(j, 0, _c19r, _c19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang18r, _ang18i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul17r + _mul22r; _add23i = _mul17i + _mul22i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_494_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_attr3r, _attr3i, j, 0, &_mul4r, &_mul4i);
        double rec = _mul4r; /* +_mul4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2i; _attr5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_attr5r, _attr5i, j, 0, &_div6r, &_div6i);
        double imc = _div6r; /* +_div6ii */
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 7.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c10r + _sin14r; _add15i = _c10i + _sin14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log9r, _log9i, _add15r, _add15i, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _c17r + _cos21r; _add22i = _c17i + _cos21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul16r, _mul16i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
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
        _c30r = 4.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul28r + _mul33r; _add34i = _mul28i + _mul33i;
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, _c35r, _c35i, &_div36r, &_div36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add34r + _sin37r; _add38i = _add34i + _sin37i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_495_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double rec1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double imc1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc2 = _attr4r; /* +_attr4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(rec1, 0, j, 0, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.2; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(j, 0, 1.2, &_pow10r, &_pow10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(imc2, 0, _pow10r, _pow10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _sin8r + _cos12r; _add13i = _sin8i + _cos12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _c14r + j; _add15i = _c14i + 0;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_add13r, _add13i, _log16r, _log16i, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.5; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_abs18r, _abs18i, 0.5, &_pow20r, &_pow20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x2r, x2i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 0.3; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs21r, _abs21i, 0.3, &_pow23r, &_pow23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_pow20r, _pow20i, _pow23r, _pow23i, &_mul24r, &_mul24i);
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
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(j, 0, j, 0, &_pow31r, &_pow31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_pow31r, _pow31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_496_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs3r, _abs3i, j, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_mul8r, _mul8i, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _c11r; _add12i = 0 + _c11i;
        double _div13r = 0, _div13i = 0;
        c_div(_mul10r, _mul10i, _add12r, _add12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log7r, _log7i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = n - j; _sub17i = 0 - 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _sub17r + _c18r; _add19i = _sub17i + _c18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_abs16r, _abs16i, _add19r, _add19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _mul20r + _c21r; _add22i = _mul20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul24r, _mul24i, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_mul26r, _mul26i, _add28r, _add28i, &_div29r, &_div29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log23r, _log23i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul15r + _mul31r; _add32i = _mul15i + _mul31i;
        double mag = _add32r; /* +_add32ii */
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x1r, x1i); _ang33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 2.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(j, 0, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang33r, _ang33i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(j, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang38r, _ang38i, _cos41r, _cos41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul37r + _mul42r; _add43i = _mul37i + _mul42i;
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = j + _c44r; _add45i = 0 + _c44i;
        double _log46r = 0, _log46i = 0;
        c_log(_add45r, _add45i, &_log46r, &_log46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _add43r + _log46r; _add47i = _add43i + _log46i;
        double ang = _add47r; /* +_add47ii */
        double _cos48r = 0, _cos48i = 0;
        c_cos(ang, 0, &_cos48r, &_cos48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _sin50r = 0, _sin50i = 0;
        c_sin(ang, 0, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c49r, _c49i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _cos48r + _mul51r; _add52i = _cos48i + _mul51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(mag, 0, _add52r, _add52i, &_mul53r, &_mul53i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_497_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr8i = 0; }
        double _add9r = 0, _add9i = 0;
        _add9r = _arr7r + _arr8r; _add9i = _arr7i + _arr8i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_add9r, _add9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double mag_part1 = _log13r; /* +_log13ii */
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 6.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 4.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin18r, _sin18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _c14r + _mul23r; _add24i = _c14i + _mul23i;
        double mag_part2 = _add24r; /* +_add24ii */
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _arr27r = 0, _arr27i = 0;
        { int _idx = (j - 1); _arr27r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr27i = 0; }
        double _prod28r = 0, _prod28i = 0;
        c_mul(j, 0, _arr27r, _arr27i, &_prod28r, &_prod28i);
        double _arr29r = 0, _arr29i = 0;
        { int _idx = (j - 1); _arr29r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr29i = 0; }
        double _prod30r = 0, _prod30i = 0;
        c_mul(_prod28r, _prod28i, _arr29r, _arr29i, &_prod30r, &_prod30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 3.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_c31r, _c31i, _c32r, _c32i, &_div33r, &_div33i);
        double _pow34r = 0, _pow34i = 0;
        c_powr(_prod30r, _prod30i, _div33r, &_pow34r, &_pow34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _c26r + _pow34r; _add35i = _c26i + _pow34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_mul25r, _mul25i, _add35r, _add35i, &_mul36r, &_mul36i);
        double magnitude = _mul36r; /* +_mul36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x1r, x1i); _ang37i = 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(j, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang37r, _ang37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(x2r, x2i); _ang40i = 0;
        double _cos41r = 0, _cos41i = 0;
        c_cos(j, 0, &_cos41r, &_cos41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang40r, _ang40i, _cos41r, _cos41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul39r + _mul42r; _add43i = _mul39i + _mul42i;
        double ang_part1 = _add43r; /* +_add43ii */
        double _mul44r = 0, _mul44i = 0;
        c_mul(j, 0, M_PI, 0, &_mul44r, &_mul44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 5.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(_mul44r, _mul44i, _c45r, _c45i, &_div46r, &_div46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(_div46r, _div46i, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(j, 0, M_PI, 0, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 7.0; _c49i = 0;
        double _div50r = 0, _div50i = 0;
        c_div(_mul48r, _mul48i, _c49r, _c49i, &_div50r, &_div50i);
        double _cos51r = 0, _cos51i = 0;
        c_cos(_div50r, _div50i, &_cos51r, &_cos51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_sin47r, _sin47i, _cos51r, _cos51i, &_mul52r, &_mul52i);
        double ang_part2 = _mul52r; /* +_mul52ii */
        double _add53r = 0, _add53i = 0;
        _add53r = ang_part1 + ang_part2; _add53i = 0 + 0;
        double angle = _add53r; /* +_add53ii */
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 1.0;
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c54r, _c54i, angle, 0, &_mul55r, &_mul55i);
        double _exp56r = 0, _exp56i = 0;
        c_exp2(_mul55r, _mul55i, &_exp56r, &_exp56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(magnitude, 0, _exp56r, _exp56i, &_mul57r, &_mul57i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    }
    double _c58r = 0, _c58i = 0;
    _c58r = 1.0; _c58i = 0;
    double _add59r = 0, _add59i = 0;
    _add59r = n + _c58r; _add59i = 0 + _c58i;
    for (int k = 1; k < (int)(_add59r); k++) {
        double _c60r = 0, _c60i = 0;
        _c60r = 1.0; _c60i = 0;
        if ((k > _c60r) && (k < n)) {
            double _c61r = 0, _c61i = 0;
            _c61r = 0.5; _c61i = 0;
            double _cf62r = 0, _cf62i = 0;
            { int _idx = (k - 2); if (_idx >= 0 && _idx < 36) { _cf62r = cRe[_idx]; _cf62i = cIm[_idx]; } }
            double _cf63r = 0, _cf63i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf63r = cRe[_idx]; _cf63i = cIm[_idx]; } }
            double _conj64r = 0, _conj64i = 0;
            _conj64r = _cf63r; _conj64i = -(_cf63i);
            double _mul65r = 0, _mul65i = 0;
            c_mul(_cf62r, _cf62i, _conj64r, _conj64i, &_mul65r, &_mul65i);
            double _mul66r = 0, _mul66i = 0;
            c_mul(_c61r, _c61i, _mul65r, _mul65i, &_mul66r, &_mul66i);
            double _mul67r = 0, _mul67i = 0;
            c_mul(k, 0, M_PI, 0, &_mul67r, &_mul67i);
            double _div68r = 0, _div68i = 0;
            c_div(_mul67r, _mul67i, n, 0, &_div68r, &_div68i);
            double _cos69r = 0, _cos69i = 0;
            c_cos(_div68r, _div68i, &_cos69r, &_cos69i);
            double _mul70r = 0, _mul70i = 0;
            c_mul(_mul66r, _mul66i, _cos69r, _cos69i, &_mul70r, &_mul70i);
            cRe[(k - 1)] += _mul70r; cIm[(k - 1)] += _mul70i;
        } else {
            double _c71r = 0, _c71i = 0;
            _c71r = 1.0; _c71i = 0;
            if (k == _c71r) {
                double _c72r = 0, _c72i = 0;
                _c72r = 0.3; _c72i = 0;
                double _cf73r = 0, _cf73i = 0;
                { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf73r = cRe[_idx]; _cf73i = cIm[_idx]; } }
                double _conj74r = 0, _conj74i = 0;
                _conj74r = _cf73r; _conj74i = -(_cf73i);
                double _mul75r = 0, _mul75i = 0;
                c_mul(_c72r, _c72i, _conj74r, _conj74i, &_mul75r, &_mul75i);
                double _mul76r = 0, _mul76i = 0;
                c_mul(k, 0, M_PI, 0, &_mul76r, &_mul76i);
                double _div77r = 0, _div77i = 0;
                c_div(_mul76r, _mul76i, n, 0, &_div77r, &_div77i);
                double _sin78r = 0, _sin78i = 0;
                c_sin(_div77r, _div77i, &_sin78r, &_sin78i);
                double _mul79r = 0, _mul79i = 0;
                c_mul(_mul75r, _mul75i, _sin78r, _sin78i, &_mul79r, &_mul79i);
                cRe[(k - 1)] += _mul79r; cIm[(k - 1)] += _mul79i;
            } else {
                double _c80r = 0, _c80i = 0;
                _c80r = 0.3; _c80i = 0;
                double _cf81r = 0, _cf81i = 0;
                { int _idx = (k - 2); if (_idx >= 0 && _idx < 36) { _cf81r = cRe[_idx]; _cf81i = cIm[_idx]; } }
                double _conj82r = 0, _conj82i = 0;
                _conj82r = _cf81r; _conj82i = -(_cf81i);
                double _mul83r = 0, _mul83i = 0;
                c_mul(_c80r, _c80i, _conj82r, _conj82i, &_mul83r, &_mul83i);
                double _mul84r = 0, _mul84i = 0;
                c_mul(k, 0, M_PI, 0, &_mul84r, &_mul84i);
                double _div85r = 0, _div85i = 0;
                c_div(_mul84r, _mul84i, n, 0, &_div85r, &_div85i);
                double _sin86r = 0, _sin86i = 0;
                c_sin(_div85r, _div85i, &_sin86r, &_sin86i);
                double _mul87r = 0, _mul87i = 0;
                c_mul(_mul83r, _mul83i, _sin86r, _sin86i, &_mul87r, &_mul87i);
                cRe[(k - 1)] += _mul87r; cIm[(k - 1)] += _mul87i;
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_498_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs3r, _abs3i, j, 0, &_mul4r, &_mul4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _div8r = 0, _div8i = 0;
        c_div(_abs5r, _abs5i, _add7r, _add7i, &_div8r, &_div8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul4r + _div8r; _add9i = _mul4i + _div8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _add9r + _c10r; _add11i = _add9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double magnitude = _log12r; /* +_log12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(j, 0, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang13r, _ang13i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _cos17r = 0, _cos17i = 0;
        c_cos(j, 0, &_cos17r, &_cos17i);
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
        c_mul(magnitude, 0, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_499_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.5; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_powr(j, 0, 1.5, &_pow5r, &_pow5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _abs3r + _pow5r; _add6i = _abs3i + _pow5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 6.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log7r, _log7i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double mag_part1 = _mul12r; /* +_mul12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _div16r = 0, _div16i = 0;
        c_div(_abs13r, _abs13i, _add15r, _add15i, &_div16r, &_div16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _div16r + _cos20r; _add21i = _div16i + _cos20i;
        double mag_part2 = _add21r; /* +_add21ii */
        double _neg22r = 0, _neg22i = 0;
        _neg22r = -(j); _neg22i = -(0);
        double _c23r = 0, _c23i = 0;
        _c23r = 10.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_neg22r, _neg22i, _c23r, _c23i, &_div24r, &_div24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_div24r, _div24i, &_exp25r, &_exp25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag_part2, 0, _exp25r, _exp25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + _mul26r; _add27i = 0 + _mul26i;
        double magnitude = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double angle_part1 = _mul32r; /* +_mul32ii */
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 5.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(j, 0, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang33r, _ang33i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 2.0; _c38i = 0;
        double _pow39r = 0, _pow39i = 0;
        c_mul(j, 0, j, 0, &_pow39r, &_pow39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 7.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_pow39r, _pow39i, _c40r, _c40i, &_div41r, &_div41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_div41r, _div41i, &_sin42r, &_sin42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul37r + _sin42r; _add43i = _mul37i + _sin42i;
        double angle_part2 = _add43r; /* +_add43ii */
        double _add44r = 0, _add44i = 0;
        _add44r = angle_part1 + angle_part2; _add44i = 0 + 0;
        double angle = _add44r; /* +_add44ii */
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
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_500_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x1r, x1i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_sin5r, _sin5i, _cos8r, _cos8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_mul(j, 0, j, 0, &_pow11r, &_pow11i);
        double _add12r = 0, _add12i = 0;
        _add12r = x1r + x2r; _add12i = x1i + x2i;
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(_add12r, _add12i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_pow11r, _pow11i, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul9r + _sin15r; _add16i = _mul9i + _sin15i;
        double angle = _add16r; /* +_add16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs17r, _abs17i, j, &_pow18r, &_pow18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _sub20r = 0, _sub20i = 0;
        _sub20r = n - j; _sub20i = 0 - 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_abs19r, _abs19i, _sub20r, &_pow21r, &_pow21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _pow18r + _pow21r; _add22i = _pow18i + _pow21i;
        double _sub23r = 0, _sub23i = 0;
        _sub23r = x1r - x2r; _sub23i = x1i - x2i;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_sub23r, _sub23i); _abs24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _abs24r; _add25i = 0 + _abs24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_add22r, _add22i, _log26r, _log26i, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 5.0; _c29i = 0;
        double _mod30r = 0, _mod30i = 0;
        _mod30r = fmod(j, _c29r); _mod30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _c28r + _mod30r; _add31i = _c28i + _mod30i;
        double _div32r = 0, _div32i = 0;
        c_div(_mul27r, _mul27i, _add31r, _add31i, &_div32r, &_div32i);
        double magnitude = _div32r; /* +_div32ii */
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
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x1r; _conj39i = -(x1i);
        double _attr40r = 0, _attr40i = 0;
        _attr40r = x2i; _attr40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, _attr40r, _attr40i, &_mul41r, &_mul41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_mul41r, _mul41i, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_conj39r, _conj39i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul38r + _mul43r; _add44i = _mul38i + _mul43i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add44r; cIm[_idx] = _add44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
