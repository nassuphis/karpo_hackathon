/* AUTO-GENERATED from poly500.py — do not edit manually */
/* 102 coefficient functions */

static void poly_401_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(j, 0, _c9r, _c9i, &_div10r, &_div10i);
        double _pow11r = 0, _pow11i = 0;
        c_powr(_abs8r, _abs8i, _div10r, &_pow11r, &_pow11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(j, 0); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_pow11r, _pow11i, _log15r, _log15i, &_mul16r, &_mul16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _sqrt18r = 0, _sqrt18i = 0;
        c_powr(j, 0, 0.5, &_sqrt18r, &_sqrt18i);
        double _pow19r = 0, _pow19i = 0;
        c_powr(_abs17r, _abs17i, _sqrt18r, &_pow19r, &_pow19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_pow19r, _pow19i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul16r + _mul21r; _add22i = _mul16i + _mul21i;
        double magnitude = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _cos24r = 0, _cos24i = 0;
        c_cos(j, 0, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(j, 0); _abs32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _abs32r + _c33r; _add34i = _abs32i + _c33i;
        double _log35r = 0, _log35i = 0;
        c_log(_add34r, _add34i, &_log35r, &_log35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add31r + _log35r; _add36i = _add31i + _log35i;
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
    double _c41r = 0, _c41i = 0;
    _c41r = 1.0; _c41i = 0;
    double _add42r = 0, _add42i = 0;
    _add42r = n + _c41r; _add42i = 0 + _c41i;
    for (int k = 1; k < (int)(_add42r); k++) {
        double _c43r = 0, _c43i = 0;
        _c43r = 0.5; _c43i = 0;
        double _re44r = 0, _re44i = 0;
        _re44r = x1r; _re44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c43r, _c43i, _re44r, _re44i, &_mul45r, &_mul45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(k, 0, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_mul45r, _mul45i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.3; _c48i = 0;
        double _im49r = 0, _im49i = 0;
        _im49r = x2i; _im49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c48r, _c48i, _im49r, _im49i, &_mul50r, &_mul50i);
        double _cos51r = 0, _cos51i = 0;
        c_cos(k, 0, &_cos51r, &_cos51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_mul50r, _mul50i, _cos51r, _cos51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul47r + _mul52r; _add53i = _mul47i + _mul52i;
        cRe[(k - 1)] += _add53r; cIm[(k - 1)] += _add53i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_402_c(double x1r, double x1i, double x2r, double x2i,
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
        double temp_mag = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        double temp_angle = _c5r; /* +_c5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        for (int k = 1; k < (int)(_add7r); k++) {
            double _abs8r = 0, _abs8i = 0;
            _abs8r = c_abs(x1r, x1i); _abs8i = 0;
            double _mul9r = 0, _mul9i = 0;
            c_mul(_abs8r, _abs8i, k, 0, &_mul9r, &_mul9i);
            double _c10r = 0, _c10i = 0;
            _c10r = 1.0; _c10i = 0;
            double _add11r = 0, _add11i = 0;
            _add11r = _mul9r + _c10r; _add11i = _mul9i + _c10i;
            double _log12r = 0, _log12i = 0;
            c_log(_add11r, _add11i, &_log12r, &_log12i);
            double _re13r = 0, _re13i = 0;
            _re13r = x2r; _re13i = 0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(k, 0, _re13r, _re13i, &_mul14r, &_mul14i);
            double _sin15r = 0, _sin15i = 0;
            c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_log12r, _log12i, _sin15r, _sin15i, &_mul16r, &_mul16i);
            temp_mag += _mul16r;
            double _im17r = 0, _im17i = 0;
            _im17r = x1i; _im17i = 0;
            double _mul18r = 0, _mul18i = 0;
            c_mul(k, 0, _im17r, _im17i, &_mul18r, &_mul18i);
            double _cos19r = 0, _cos19i = 0;
            c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
            double _add20r = 0, _add20i = 0;
            _add20r = x2r + k; _add20i = x2i + 0;
            double _ang21r = 0, _ang21i = 0;
            _ang21r = c_arg(_add20r, _add20i); _ang21i = 0;
            double _mul22r = 0, _mul22i = 0;
            c_mul(_cos19r, _cos19i, _ang21r, _ang21i, &_mul22r, &_mul22i);
            temp_angle += _mul22r;
        }
        double _cos23r = 0, _cos23i = 0;
        c_cos(temp_angle, 0, &_cos23r, &_cos23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(temp_angle, 0, &_sin24r, &_sin24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_sin24r, _sin24i, _c25r, _c25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _cos23r + _mul26r; _add27i = _cos23i + _mul26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(temp_mag, 0, _add27r, _add27i, &_mul28r, &_mul28i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    double _c29r = 0, _c29i = 0;
    _c29r = 1.0; _c29i = 0;
    double _add30r = 0, _add30i = 0;
    _add30r = n + _c29r; _add30i = 0 + _c29i;
    for (int r = 1; r < (int)(_add30r); r++) {
        double _cf31r = 0, _cf31i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x2r, x2i); _abs32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(r, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _pow35r = 0, _pow35i = 0;
        c_powr(_abs32r, _abs32i, _div34r, &_pow35r, &_pow35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_cf31r, _cf31i, _pow35r, _pow35i, &_mul36r, &_mul36i);
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x1r; _conj37i = -(x1i);
        double _pow38r = 0, _pow38i = 0;
        c_powr(_conj37r, _conj37i, r, &_pow38r, &_pow38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul36r + _pow38r; _add39i = _mul36i + _pow38i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_403_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec[] = {0}; /* WARNING: could not evaluate */
    static const double imc[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, M_PI, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 7.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul4r, _mul4i, _c5r, _c5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 5.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_div10r, _div10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sin7r, _sin7i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = x1r + x2r; _add13i = x1i + x2i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_add13r, _add13i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul12r + _log17r; _add18i = _mul12i + _log17i;
        double phase = _add18r; /* +_add18ii */
        double _re19r = 0, _re19i = 0;
        _re19r = x1r; _re19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_re19r, _re19i, j, &_pow20r, &_pow20i);
        double _im21r = 0, _im21i = 0;
        _im21r = x2i; _im21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_im21r, _im21i, j, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _pow20r + _pow22r; _add23i = _pow20i + _pow22i;
        double _sin24r = 0, _sin24i = 0;
        c_sin(j, 0, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_add23r, _add23i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul25r + _cos29r; _add30i = _mul25i + _cos29i;
        double magnitude = _add30r; /* +_add30ii */
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, phase, 0, &_mul32r, &_mul32i);
        double _exp33r = 0, _exp33i = 0;
        c_exp2(_mul32r, _mul32i, &_exp33r, &_exp33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(magnitude, 0, _exp33r, _exp33i, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, phase, 0, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(magnitude, 0, _exp37r, _exp37i, &_mul38r, &_mul38i);
        double _conj39r = 0, _conj39i = 0;
        _conj39r = _mul38r; _conj39i = -(_mul38i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul34r + _conj39r; _add40i = _mul34i + _conj39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_404_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 1; j < 36; j++) {
        double _re2r = 0, _re2i = 0;
        _re2r = x1r; _re2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_re2r, _re2i, _log5r, _log5i, &_mul6r, &_mul6i);
        double mag_part1 = _mul6r; /* +_mul6ii */
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _sin8r = 0, _sin8i = 0;
        c_sin(j, 0, &_sin8r, &_sin8i);
        double _pow9r = 0, _pow9i = 0;
        c_powr(_abs7r, _abs7i, _sin8r, &_pow9r, &_pow9i);
        double mag_part2 = _pow9r; /* +_pow9ii */
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 35.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(j, 0, _c11r, _c11i, &_div12r, &_div12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _c10r + _div12r; _add13i = _c10i + _div12i;
        double mag_part3 = _add13r; /* +_add13ii */
        double _mul14r = 0, _mul14i = 0;
        c_mul(mag_part2, 0, mag_part3, 0, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = mag_part1 + _mul14r; _add15i = 0 + _mul14i;
        double magnitude = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang16r, _ang16i, j, 0, &_mul17r, &_mul17i);
        double ang_part1 = _mul17r; /* +_mul17ii */
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 7.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double ang_part2 = _cos21r; /* +_cos21ii */
        double _add22r = 0, _add22i = 0;
        _add22r = ang_part1 + ang_part2; _add22i = 0 + 0;
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
        c_mul(magnitude, 0, _add27r, _add27i, &_mul28r, &_mul28i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_405_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x1r, x1i); _ang4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, _ang4r, _ang4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(k, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _sin6r + _cos9r; _add10i = _sin6i + _cos9i;
        double phase = _add10r; /* +_add10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs11r + k; _add12i = _abs11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _neg14r = 0, _neg14i = 0;
        _neg14r = -(k); _neg14i = -(0);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_neg14r, _neg14i, _add17r, _add17i, &_div18r, &_div18i);
        double _exp19r = 0, _exp19i = 0;
        c_exp2(_div18r, _div18i, &_exp19r, &_exp19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log13r, _log13i, _exp19r, _exp19i, &_mul20r, &_mul20i);
        double _sqrt21r = 0, _sqrt21i = 0;
        c_powr(k, 0, 0.5, &_sqrt21r, &_sqrt21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = x1r - x2r; _sub22i = x1i - x2i;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_sub22r, _sub22i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sqrt21r, _sqrt21i, _abs23r, _abs23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul20r + _mul24r; _add25i = _mul20i + _mul24i;
        double magnitude = _add25r; /* +_add25ii */
        double _cos26r = 0, _cos26i = 0;
        c_cos(phase, 0, &_cos26r, &_cos26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(phase, 0, &_sin27r, &_sin27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin27r, _sin27i, _c28r, _c28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _cos26r + _mul29r; _add30i = _cos26i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(magnitude, 0, _add30r, _add30i, &_mul31r, &_mul31i);
        double _conj32r = 0, _conj32i = 0;
        _conj32r = x1r; _conj32i = -(x1i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(k, 0, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_conj32r, _conj32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(k, 0, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_mul34r, _mul34i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul31r + _mul36r; _add37i = _mul31i + _mul36i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_406_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec[] = {0}; /* WARNING: could not evaluate */
    static const double imc[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int r = 1; r < (int)(_add3r); r++) {
        double _mul4r = 0, _mul4i = 0;
        c_mul(r, 0, M_PI, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 8.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul4r, _mul4i, _c5r, _c5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _re8r = 0, _re8i = 0;
        _re8r = x1r; _re8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_sin7r, _sin7i, _re8r, _re8i, &_mul9r, &_mul9i);
        double _arr10r = 0, _arr10i = 0;
        _arr10r = rec[(r - 1)]; _arr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_mul9r, _mul9i, _arr10r, _arr10i, &_mul11r, &_mul11i);
        double term1 = _mul11r; /* +_mul11ii */
        double _mul12r = 0, _mul12i = 0;
        c_mul(r, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 6.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_cos15r, _cos15i, _im16r, _im16i, &_mul17r, &_mul17i);
        double _arr18r = 0, _arr18i = 0;
        _arr18r = imc[(r - 1)]; _arr18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul17r, _mul17i, _arr18r, _arr18i, &_mul19r, &_mul19i);
        double term2 = _mul19r; /* +_mul19ii */
        double _arr20r = 0, _arr20i = 0;
        _arr20r = rec[(r - 1)]; _arr20i = 0;
        double _arr21r = 0, _arr21i = 0;
        _arr21r = imc[(r - 1)]; _arr21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _arr20r + _arr21r; _add22i = _arr20i + _arr21i;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_add22r, _add22i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _c24r; _add25i = _abs23i + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double term3 = _log26r; /* +_log26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = term1 + term2; _add27i = 0 + 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _add27r + term3; _add28i = _add27i + 0;
        double mag = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(r, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang29r, _ang29i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(r, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 7.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang35r, _ang35i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul34r + _mul40r; _add41i = _mul34i + _mul40i;
        double _arr42r = 0, _arr42i = 0;
        _arr42r = imc[(r - 1)]; _arr42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 3.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_arr42r, _arr42i, _c43r, _c43i, &_div44r, &_div44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _add41r + _div44r; _add45i = _add41i + _div44i;
        double angle = _add45r; /* +_add45ii */
        double _cos46r = 0, _cos46i = 0;
        c_cos(angle, 0, &_cos46r, &_cos46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(angle, 0, &_sin47r, &_sin47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_sin47r, _sin47i, _c48r, _c48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _cos46r + _mul49r; _add50i = _cos46i + _mul49i;
        double _mul51r = 0, _mul51i = 0;
        c_mul(mag, 0, _add50r, _add50i, &_mul51r, &_mul51i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double rec_t1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double imc_t1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2r; _attr4i = 0;
    double rec_t2 = _attr4r; /* +_attr4ii */
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc_t2 = _attr5r; /* +_attr5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_mul8r, _mul8i, rec_t1, 0, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_mul11r, _mul11i, imc_t2, 0, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin10r + _cos13r; _add14i = _sin10i + _cos13i;
        double angle_part = _add14r; /* +_add14ii */
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, rec_t2, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _mul15r + _c16r; _add17i = _mul15i + _c16i;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_add17r, _add17i); _abs18i = 0;
        double _log19r = 0, _log19i = 0;
        c_log(_abs18r, _abs18i, &_log19r, &_log19i);
        double _sqrt20r = 0, _sqrt20i = 0;
        c_powr(j, 0, 0.5, &_sqrt20r, &_sqrt20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log19r, _log19i, _sqrt20r, _sqrt20i, &_mul21r, &_mul21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.5; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_abs22r, _abs22i, 0.5, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul21r + _pow24r; _add25i = _mul21i + _pow24i;
        double magnitude_part = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _ang27r, _ang27i, &_mul28r, &_mul28i);
        double _div29r = 0, _div29i = 0;
        c_div(_mul28r, _mul28i, j, 0, &_div29r, &_div29i);
        double phase_shift = _div29r; /* +_div29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _add31r = 0, _add31i = 0;
        _add31r = angle_part + phase_shift; _add31i = 0 + 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c30r, _c30i, _add31r, _add31i, &_mul32r, &_mul32i);
        double _exp33r = 0, _exp33i = 0;
        c_exp2(_mul32r, _mul32i, &_exp33r, &_exp33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(magnitude_part, 0, _exp33r, _exp33i, &_mul34r, &_mul34i);
        double _conj35r = 0, _conj35i = 0;
        _conj35r = x1r; _conj35i = -(x1i);
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, _ang36r, _ang36i, &_mul37r, &_mul37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_mul37r, _mul37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_conj35r, _conj35i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul34r + _mul39r; _add40i = _mul34i + _mul39i;
        double _abs41r = 0, _abs41i = 0;
        _abs41r = c_abs(x1r, x1i); _abs41i = 0;
        double _cos42r = 0, _cos42i = 0;
        c_cos(_abs41r, _abs41i, &_cos42r, &_cos42i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = _add40r - _cos42r; _sub43i = _add40i - _cos42i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    }
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _fdiv45r = 0, _fdiv45i = 0;
    c_div(n, 0, _c44r, _c44i, &_fdiv45r, &_fdiv45i);
    _fdiv45r = floor(_fdiv45r); _fdiv45i = 0;
    double _c46r = 0, _c46i = 0;
    _c46r = 1.0; _c46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = _fdiv45r + _c46r; _add47i = _fdiv45i + _c46i;
    for (int k = 1; k < (int)(_add47r); k++) {
        double _c48r = 0, _c48i = 0;
        _c48r = 2.0; _c48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(k, 0, _c48r, _c48i, &_mul49r, &_mul49i);
        double idx = _mul49r; /* +_mul49ii */
        if (idx <= n) {
            double _c50r = 0, _c50i = 0;
            _c50r = 0.0; _c50i = 1.0;
            double _sin51r = 0, _sin51i = 0;
            c_sin(k, 0, &_sin51r, &_sin51i);
            double _cos52r = 0, _cos52i = 0;
            c_cos(k, 0, &_cos52r, &_cos52i);
            double _add53r = 0, _add53i = 0;
            _add53r = _sin51r + _cos52r; _add53i = _sin51i + _cos52i;
            double _mul54r = 0, _mul54i = 0;
            c_mul(_c50r, _c50i, _add53r, _add53i, &_mul54r, &_mul54i);
            double _exp55r = 0, _exp55i = 0;
            c_exp2(_mul54r, _mul54i, &_exp55r, &_exp55i);
            double perturbation = _exp55r; /* +_exp55ii */
            double _cf56r = 0, _cf56i = 0;
            { int _idx = (idx - 1); if (_idx >= 0 && _idx < 36) { _cf56r = cRe[_idx]; _cf56i = cIm[_idx]; } }
            double _mul57r = 0, _mul57i = 0;
            c_mul(_cf56r, _cf56i, perturbation, 0, &_mul57r, &_mul57i);
            double _cf58r = 0, _cf58i = 0;
            { int _idx = (idx - 1); if (_idx >= 0 && _idx < 36) { _cf58r = cRe[_idx]; _cf58i = cIm[_idx]; } }
            double _abs59r = 0, _abs59i = 0;
            _abs59r = c_abs(_cf58r, _cf58i); _abs59i = 0;
            double _c60r = 0, _c60i = 0;
            _c60r = 1.0; _c60i = 0;
            double _add61r = 0, _add61i = 0;
            _add61r = _abs59r + _c60r; _add61i = _abs59i + _c60i;
            double _log62r = 0, _log62i = 0;
            c_log(_add61r, _add61i, &_log62r, &_log62i);
            double _add63r = 0, _add63i = 0;
            _add63r = _mul57r + _log62r; _add63i = _mul57i + _log62i;
            { int _idx = (idx - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add63r; cIm[_idx] = _add63i; } }
        }
    }
    double _c64r = 0, _c64i = 0;
    _c64r = 1.0; _c64i = 0;
    double _add65r = 0, _add65i = 0;
    _add65r = n + _c64r; _add65i = 0 + _c64i;
    for (int r = 1; r < (int)(_add65r); r++) {
        double _c66r = 0, _c66i = 0;
        _c66r = 2.0; _c66i = 0;
        double _pow67r = 0, _pow67i = 0;
        c_mul(r, 0, r, 0, &_pow67r, &_pow67i);
        double _sqrt68r = 0, _sqrt68i = 0;
        c_powr(r, 0, 0.5, &_sqrt68r, &_sqrt68i);
        double _add69r = 0, _add69i = 0;
        _add69r = _pow67r + _sqrt68r; _add69i = _pow67i + _sqrt68i;
        double _abs70r = 0, _abs70i = 0;
        _abs70r = c_abs(x1r, x1i); _abs70i = 0;
        double _abs71r = 0, _abs71i = 0;
        _abs71r = c_abs(x2r, x2i); _abs71i = 0;
        double _add72r = 0, _add72i = 0;
        _add72r = _abs70r + _abs71r; _add72i = _abs70i + _abs71i;
        double _c73r = 0, _c73i = 0;
        _c73r = 1.0; _c73i = 0;
        double _add74r = 0, _add74i = 0;
        _add74r = _add72r + _c73r; _add74i = _add72i + _c73i;
        double _div75r = 0, _div75i = 0;
        c_div(_add69r, _add69i, _add74r, _add74i, &_div75r, &_div75i);
        double scaling_factor = _div75r; /* +_div75ii */
        double _cf76r = 0, _cf76i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf76r = cRe[_idx]; _cf76i = cIm[_idx]; } }
        double _mul77r = 0, _mul77i = 0;
        c_mul(_cf76r, _cf76i, scaling_factor, 0, &_mul77r, &_mul77i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul77r; cIm[_idx] = _mul77i; } }
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 7.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(j, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double k = _add7r; /* +_add7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(j, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _flr10r = 0, _flr10i = 0;
        _flr10r = floor(_div9r); _flr10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _flr10r + _c11r; _add12i = _flr10i + _c11i;
        double r = _add12r; /* +_add12ii */
        double _re13r = 0, _re13i = 0;
        _re13r = x1r; _re13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(k, 0, _re13r, _re13i, &_mul14r, &_mul14i);
        double _im15r = 0, _im15i = 0;
        _im15r = x2i; _im15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(r, 0, _im15r, _im15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul14r + _mul16r; _add17i = _mul14i + _mul16i;
        double _sin18r = 0, _sin18i = 0;
        c_sin(_add17r, _add17i, &_sin18r, &_sin18i);
        double _im19r = 0, _im19i = 0;
        _im19r = x1i; _im19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(k, 0, _im19r, _im19i, &_mul20r, &_mul20i);
        double _re21r = 0, _re21i = 0;
        _re21r = x2r; _re21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(r, 0, _re21r, _re21i, &_mul22r, &_mul22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _mul20r - _mul22r; _sub23i = _mul20i - _mul22i;
        double _cos24r = 0, _cos24i = 0;
        c_cos(_sub23r, _sub23i, &_cos24r, &_cos24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _sin18r + _cos24r; _add25i = _sin18i + _cos24i;
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _ang27r, _ang27i, &_mul28r, &_mul28i);
        double _div29r = 0, _div29i = 0;
        c_div(_mul28r, _mul28i, j, 0, &_div29r, &_div29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add25r + _div29r; _add30i = _add25i + _div29i;
        double angle = _add30r; /* +_add30ii */
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x1r, x1i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _abs31r + _c32r; _add33i = _abs31i + _c32i;
        double _log34r = 0, _log34i = 0;
        c_log(_add33r, _add33i, &_log34r, &_log34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.5; _c35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_powr(j, 0, 0.5, &_pow36r, &_pow36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _pow36r + r; _add37i = _pow36i + 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_log34r, _log34i, _add37r, _add37i, &_mul38r, &_mul38i);
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(x2r, x2i); _abs39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.2; _c40i = 0;
        double _pow41r = 0, _pow41i = 0;
        c_powr(r, 0, 1.2, &_pow41r, &_pow41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_abs39r, _abs39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul38r + _mul42r; _add43i = _mul38i + _mul42i;
        double magnitude = _add43r; /* +_add43ii */
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, angle, 0, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(magnitude, 0, _exp46r, _exp46i, &_mul47r, &_mul47i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_413_c(double x1r, double x1i, double x2r, double x2i,
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
        double _sin5r = 0, _sin5i = 0;
        c_sin(j, 0, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_ang4r, _ang4i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 3.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(j, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_div9r, _div9i, &_cos10r, &_cos10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_ang7r, _ang7i, _cos10r, _cos10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul6r + _mul11r; _add12i = _mul6i + _mul11i;
        double angle = _add12r; /* +_add12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs13r, _abs13i, j, &_pow14r, &_pow14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _sub19r = 0, _sub19i = 0;
        _sub19r = n - j; _sub19i = 0 - 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_log18r, _log18i, _sub19r, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_pow14r, _pow14i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 5.0; _c22i = 0;
        double _mod23r = 0, _mod23i = 0;
        _mod23r = fmod(j, _c22r); _mod23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _mod23r + _c24r; _add25i = _mod23i + _c24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul21r, _mul21i, _add25r, _add25i, &_mul26r, &_mul26i);
        double magnitude = _mul26r; /* +_mul26ii */
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
        c_mul(magnitude, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _div34r = 0, _div34i = 0;
    c_div(n, 0, _c33r, _c33i, &_div34r, &_div34i);
    double _int35r = 0, _int35i = 0;
    _int35r = (int)(_div34r); _int35i = 0;
    double _c36r = 0, _c36i = 0;
    _c36r = 1.0; _c36i = 0;
    double _add37r = 0, _add37i = 0;
    _add37r = _int35r + _c36r; _add37i = _int35i + _c36i;
    for (int k = 1; k < (int)(_add37r); k++) {
        double _c38r = 0, _c38i = 0;
        _c38r = 2.0; _c38i = 0;
        double _pow39r = 0, _pow39i = 0;
        c_mul(k, 0, k, 0, &_pow39r, &_pow39i);
        double _sqrt40r = 0, _sqrt40i = 0;
        c_powr(k, 0, 0.5, &_sqrt40r, &_sqrt40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _pow39r + _sqrt40r; _add41i = _pow39i + _sqrt40i;
        double r = _add41r; /* +_add41ii */
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, r, 0, &_mul43r, &_mul43i);
        double _exp44r = 0, _exp44i = 0;
        c_exp2(_mul43r, _mul43i, &_exp44r, &_exp44i);
        { double _tr = cRe[(k - 1)]*_exp44r - cIm[(k - 1)]*_exp44i; cIm[(k - 1)] = cRe[(k - 1)]*_exp44i + cIm[(k - 1)]*_exp44r; cRe[(k - 1)] = _tr; }
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, r, 0, &_mul46r, &_mul46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
        double _conj48r = 0, _conj48i = 0;
        _conj48r = _exp47r; _conj48i = -(_exp47i);
        { double _tr = cRe[(-k)]*_conj48r - cIm[(-k)]*_conj48i; cIm[(-k)] = cRe[(-k)]*_conj48i + cIm[(-k)]*_conj48r; cRe[(-k)] = _tr; }
    }
    double _c49r = 0, _c49i = 0;
    _c49r = 1.0; _c49i = 0;
    double _add50r = 0, _add50i = 0;
    _add50r = n + _c49r; _add50i = 0 + _c49i;
    for (int r = 1; r < (int)(_add50r); r++) {
        double _c51r = 0, _c51i = 0;
        _c51r = 0.1; _c51i = 0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c51r, _c51i, r, 0, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 0.0; _c53i = 1.0;
        double _neg54r = 0, _neg54i = 0;
        _neg54r = -(_c53r); _neg54i = -(_c53i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_neg54r, _neg54i, r, 0, &_mul55r, &_mul55i);
        double _div56r = 0, _div56i = 0;
        c_div(_mul55r, _mul55i, n, 0, &_div56r, &_div56i);
        double _exp57r = 0, _exp57i = 0;
        c_exp2(_div56r, _div56i, &_exp57r, &_exp57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_mul52r, _mul52i, _exp57r, _exp57i, &_mul58r, &_mul58i);
        cRe[(r - 1)] += _mul58r; cIm[(r - 1)] += _mul58i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_414_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c4r = 0, _c4i = 0;
        _c4r = 5.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(k, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double j = _add7r; /* +_add7ii */
        double _re8r = 0, _re8i = 0;
        _re8r = x1r; _re8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(k, 0, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_re8r, _re8i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _re11r = 0, _re11i = 0;
        _re11r = x2r; _re11i = 0;
        double _cos12r = 0, _cos12i = 0;
        c_cos(j, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_re11r, _re11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul10r + _mul13r; _add14i = _mul10i + _mul13i;
        double _add15r = 0, _add15i = 0;
        _add15r = j + k; _add15i = 0 + 0;
        double _div16r = 0, _div16i = 0;
        c_div(_add14r, _add14i, _add15r, _add15i, &_div16r, &_div16i);
        double r = _div16r; /* +_div16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _abs18r; _add19i = _abs17i + _abs18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.5; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(k, 0, 1.5, &_pow21r, &_pow21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _add19r + _pow21r; _add22i = _add19i + _pow21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(r, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _div25r = 0, _div25i = 0;
        c_div(_mul24r, _mul24i, j, 0, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log23r, _log23i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(r, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = k + _c29r; _add30i = 0 + _c29i;
        double _div31r = 0, _div31i = 0;
        c_div(_mul28r, _mul28i, _add30r, _add30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _pow34r = 0, _pow34i = 0;
        c_mul(_cos32r, _cos32i, _cos32r, _cos32i, &_pow34r, &_pow34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul27r + _pow34r; _add35i = _mul27i + _pow34i;
        double mag = _add35r; /* +_add35ii */
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x1r, x1i); _ang36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = k + _c37r; _add38i = 0 + _c37i;
        double _div39r = 0, _div39i = 0;
        c_div(j, 0, _add38r, _add38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang36r, _ang36i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x2r, x2i); _ang42i = 0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(r, 0, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_ang42r, _ang42i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul41r + _mul44r; _add45i = _mul41i + _mul44i;
        double angle = _add45r; /* +_add45ii */
        double _cos46r = 0, _cos46i = 0;
        c_cos(angle, 0, &_cos46r, &_cos46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _sin48r = 0, _sin48i = 0;
        c_sin(angle, 0, &_sin48r, &_sin48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c47r, _c47i, _sin48r, _sin48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _cos46r + _mul49r; _add50i = _cos46i + _mul49i;
        double _mul51r = 0, _mul51i = 0;
        c_mul(mag, 0, _add50r, _add50i, &_mul51r, &_mul51i);
        double _c52r = 0, _c52i = 0;
        _c52r = 1.0; _c52i = 0;
        double _c53r = 0, _c53i = 0;
        _c53r = 0.05; _c53i = 0;
        double _c54r = 0, _c54i = 0;
        _c54r = 2.0; _c54i = 0;
        double _pow55r = 0, _pow55i = 0;
        c_mul(k, 0, k, 0, &_pow55r, &_pow55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c53r, _c53i, _pow55r, _pow55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _c52r + _mul56r; _add57i = _c52i + _mul56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(_mul51r, _mul51i, _add57r, _add57i, &_mul58r, &_mul58i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_415_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs4r + _abs5r; _add6i = _abs4i + _abs5i;
        double _add7r = 0, _add7i = 0;
        _add7r = _add6r + j; _add7i = _add6i + 0;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _c9r + _sin13r; _add14i = _c9i + _sin13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log8r, _log8i, _add14r, _add14i, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _c16r + _cos20r; _add21i = _c16i + _cos20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul15r, _mul15i, _add21r, _add21i, &_mul22r, &_mul22i);
        double mag = _mul22r; /* +_mul22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(j, 0, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang23r, _ang23i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 4.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul27r + _mul32r; _add33i = _mul27i + _mul32i;
        double ang = _add33r; /* +_add33ii */
        double _cos34r = 0, _cos34i = 0;
        c_cos(ang, 0, &_cos34r, &_cos34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _sin36r = 0, _sin36i = 0;
        c_sin(ang, 0, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c35r, _c35i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cos34r + _mul37r; _add38i = _cos34i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x1r; _conj40i = -(x1i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(j, 0, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj40r, _conj40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul39r + _mul42r; _add43i = _mul39i + _mul42i;
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x2r; _conj44i = -(x2i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(j, 0, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_conj44r, _conj44i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _sub47r = 0, _sub47i = 0;
        _sub47r = _add43r - _mul46r; _sub47i = _add43i - _mul46i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub47r; cIm[_idx] = _sub47i; } }
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double r1 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double i1 = _im3r; /* +_im3ii */
    double _re4r = 0, _re4i = 0;
    _re4r = x2r; _re4i = 0;
    double r2 = _re4r; /* +_re4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double i2 = _im5r; /* +_im5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _add8r = 0, _add8i = 0;
        _add8r = r1 + j; _add8i = 0 + 0;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.5; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(j, 0, 1.5, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, r2, 0, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _pow14r + _sin16r; _add17i = _pow14i + _sin16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log12r, _log12i, _add17r, _add17i, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, i1, 0, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_cos21r, _cos21i); _abs22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _c19r + _abs22r; _add23i = _c19i + _abs22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul18r, _mul18i, _add23r, _add23i, &_mul24r, &_mul24i);
        double mag = _mul24r; /* +_mul24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, r2, 0, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang25r, _ang25i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, i1, 0, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang29r, _ang29i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul28r + _mul32r; _add33i = _mul28i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, i2, 0, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add33r + _sin35r; _add36i = _add33i + _sin35i;
        double ang = _add36r; /* +_add36ii */
        double _cos37r = 0, _cos37i = 0;
        c_cos(ang, 0, &_cos37r, &_cos37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(ang, 0, &_sin39r, &_sin39i);
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

static void poly_418_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double sum_re = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        double sum_im = _c5r; /* +_c5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        for (int k = 1; k < (int)(_add7r); k++) {
            double _arr8r = 0, _arr8i = 0;
            _arr8r = rec_seq[(k - 1)]; _arr8i = 0;
            double _pow9r = 0, _pow9i = 0;
            c_powr(_arr8r, _arr8i, k, &_pow9r, &_pow9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, M_PI, 0, &_mul10r, &_mul10i);
            double _c11r = 0, _c11i = 0;
            _c11r = 1.0; _c11i = 0;
            double _add12r = 0, _add12i = 0;
            _add12r = j + _c11r; _add12i = 0 + _c11i;
            double _div13r = 0, _div13i = 0;
            c_div(_mul10r, _mul10i, _add12r, _add12i, &_div13r, &_div13i);
            double _cos14r = 0, _cos14i = 0;
            c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
            double _mul15r = 0, _mul15i = 0;
            c_mul(_pow9r, _pow9i, _cos14r, _cos14i, &_mul15r, &_mul15i);
            sum_re += _mul15r;
            double _arr16r = 0, _arr16i = 0;
            _arr16r = imc_seq[(k - 1)]; _arr16i = 0;
            double _pow17r = 0, _pow17i = 0;
            c_powr(_arr16r, _arr16i, k, &_pow17r, &_pow17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(k, 0, M_PI, 0, &_mul18r, &_mul18i);
            double _c19r = 0, _c19i = 0;
            _c19r = 1.0; _c19i = 0;
            double _add20r = 0, _add20i = 0;
            _add20r = j + _c19r; _add20i = 0 + _c19i;
            double _div21r = 0, _div21i = 0;
            c_div(_mul18r, _mul18i, _add20r, _add20i, &_div21r, &_div21i);
            double _sin22r = 0, _sin22i = 0;
            c_sin(_div21r, _div21i, &_sin22r, &_sin22i);
            double _mul23r = 0, _mul23i = 0;
            c_mul(_pow17r, _pow17i, _sin22r, _sin22i, &_mul23r, &_mul23i);
            sum_im += _mul23r;
        }
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(sum_re, 0, sum_re, 0, &_pow26r, &_pow26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _c24r + _pow26r; _add27i = _c24i + _pow26i;
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(sum_im, 0, sum_im, 0, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add27r + _pow29r; _add30i = _add27i + _pow29i;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 5.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_log31r, _log31i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(sum_re, 0, sum_im, 0, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _c37r + _mul38r; _add39i = _c37i + _mul38i;
        double _log40r = 0, _log40i = 0;
        c_log(_add39r, _add39i, &_log40r, &_log40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, M_PI, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 7.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(_mul41r, _mul41i, _c42r, _c42i, &_div43r, &_div43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_div43r, _div43i, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_log40r, _log40i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul36r + _mul45r; _add46i = _mul36i + _mul45i;
        double magnitude = _add46r; /* +_add46ii */
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x1r, x1i); _ang47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang47r, _ang47i, sum_re, 0, &_mul48r, &_mul48i);
        double _ang49r = 0, _ang49i = 0;
        _ang49r = c_arg(x2r, x2i); _ang49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_ang49r, _ang49i, sum_im, 0, &_mul50r, &_mul50i);
        double _sub51r = 0, _sub51i = 0;
        _sub51r = _mul48r - _mul50r; _sub51i = _mul48i - _mul50i;
        double _mul52r = 0, _mul52i = 0;
        c_mul(j, 0, M_PI, 0, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 3.0; _c53i = 0;
        double _div54r = 0, _div54i = 0;
        c_div(_mul52r, _mul52i, _c53r, _c53i, &_div54r, &_div54i);
        double _sin55r = 0, _sin55i = 0;
        c_sin(_div54r, _div54i, &_sin55r, &_sin55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _sub51r + _sin55r; _add56i = _sub51i + _sin55i;
        double _mul57r = 0, _mul57i = 0;
        c_mul(j, 0, M_PI, 0, &_mul57r, &_mul57i);
        double _c58r = 0, _c58i = 0;
        _c58r = 7.0; _c58i = 0;
        double _div59r = 0, _div59i = 0;
        c_div(_mul57r, _mul57i, _c58r, _c58i, &_div59r, &_div59i);
        double _cos60r = 0, _cos60i = 0;
        c_cos(_div59r, _div59i, &_cos60r, &_cos60i);
        double _sub61r = 0, _sub61i = 0;
        _sub61r = _add56r - _cos60r; _sub61i = _add56i - _cos60i;
        double angle = _sub61r; /* +_sub61ii */
        double _c62r = 0, _c62i = 0;
        _c62r = 0.0; _c62i = 1.0;
        double _mul63r = 0, _mul63i = 0;
        c_mul(_c62r, _c62i, angle, 0, &_mul63r, &_mul63i);
        double _exp64r = 0, _exp64i = 0;
        c_exp2(_mul63r, _mul63i, &_exp64r, &_exp64i);
        double _mul65r = 0, _mul65i = 0;
        c_mul(magnitude, 0, _exp64r, _exp64i, &_mul65r, &_mul65i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul65r; cIm[_idx] = _mul65i; } }
    }
    double _c66r = 0, _c66i = 0;
    _c66r = 1.0; _c66i = 0;
    double _add67r = 0, _add67i = 0;
    _add67r = n + _c66r; _add67i = 0 + _c66i;
    for (int k = 1; k < (int)(_add67r); k++) {
        double _re68r = 0, _re68i = 0;
        _re68r = x1r; _re68i = 0;
        double _re69r = 0, _re69i = 0;
        _re69r = x2r; _re69i = 0;
        double _mul70r = 0, _mul70i = 0;
        c_mul(_re68r, _re68i, _re69r, _re69i, &_mul70r, &_mul70i);
        double _c71r = 0, _c71i = 0;
        _c71r = 1.0; _c71i = 0;
        double _add72r = 0, _add72i = 0;
        _add72r = k + _c71r; _add72i = 0 + _c71i;
        double _div73r = 0, _div73i = 0;
        c_div(_mul70r, _mul70i, _add72r, _add72i, &_div73r, &_div73i);
        double _mul74r = 0, _mul74i = 0;
        c_mul(k, 0, M_PI, 0, &_mul74r, &_mul74i);
        double _c75r = 0, _c75i = 0;
        _c75r = 6.0; _c75i = 0;
        double _div76r = 0, _div76i = 0;
        c_div(_mul74r, _mul74i, _c75r, _c75i, &_div76r, &_div76i);
        double _sin77r = 0, _sin77i = 0;
        c_sin(_div76r, _div76i, &_sin77r, &_sin77i);
        double _mul78r = 0, _mul78i = 0;
        c_mul(_div73r, _div73i, _sin77r, _sin77i, &_mul78r, &_mul78i);
        double _im79r = 0, _im79i = 0;
        _im79r = x1i; _im79i = 0;
        double _im80r = 0, _im80i = 0;
        _im80r = x2i; _im80i = 0;
        double _add81r = 0, _add81i = 0;
        _add81r = _im79r + _im80r; _add81i = _im79i + _im80i;
        double _mul82r = 0, _mul82i = 0;
        c_mul(k, 0, M_PI, 0, &_mul82r, &_mul82i);
        double _c83r = 0, _c83i = 0;
        _c83r = 8.0; _c83i = 0;
        double _div84r = 0, _div84i = 0;
        c_div(_mul82r, _mul82i, _c83r, _c83i, &_div84r, &_div84i);
        double _cos85r = 0, _cos85i = 0;
        c_cos(_div84r, _div84i, &_cos85r, &_cos85i);
        double _mul86r = 0, _mul86i = 0;
        c_mul(_add81r, _add81i, _cos85r, _cos85i, &_mul86r, &_mul86i);
        double _add87r = 0, _add87i = 0;
        _add87r = _mul78r + _mul86r; _add87i = _mul78i + _mul86i;
        cRe[(k - 1)] += _add87r; cIm[(k - 1)] += _add87i;
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double r1 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double i1 = _im3r; /* +_im3ii */
    double _re4r = 0, _re4i = 0;
    _re4r = x2r; _re4i = 0;
    double r2 = _re4r; /* +_re4ii */
    double _im5r = 0, _im5i = 0;
    _im5r = x2i; _im5i = 0;
    double i2 = _im5r; /* +_im5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x1r, x1i); _ang8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_ang8r, _ang8i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x2r, x2i); _ang11i = 0;
        double _cos12r = 0, _cos12i = 0;
        c_cos(j, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_ang11r, _ang11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul10r + _mul13r; _add14i = _mul10i + _mul13i;
        double angle_component = _add14r; /* +_add14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _mod17r = 0, _mod17i = 0;
        _mod17r = fmod(j, _c16r); _mod17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _mod17r + _c18r; _add19i = _mod17i + _c18i;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_abs15r, _abs15i, _add19r, &_pow20r, &_pow20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x2r, x2i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log24r, _log24i, j, 0, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _pow20r + _mul25r; _add26i = _pow20i + _mul25i;
        double magnitude_component = _add26r; /* +_add26ii */
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 7.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, M_PI, 0, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 5.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_mul31r, _mul31i, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _sin30r + _cos34r; _add35i = _sin30i + _cos34i;
        double phase_shift = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _add37r = 0, _add37i = 0;
        _add37r = angle_component + phase_shift; _add37i = 0 + 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c36r, _c36i, _add37r, _add37i, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(magnitude_component, 0, _exp39r, _exp39i, &_mul40r, &_mul40i);
        double _conj41r = 0, _conj41i = 0;
        _conj41r = x1r; _conj41i = -(x1i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj41r, _conj41i, r2, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = j + _c43r; _add44i = 0 + _c43i;
        double _div45r = 0, _div45i = 0;
        c_div(_mul42r, _mul42i, _add44r, _add44i, &_div45r, &_div45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul40r + _div45r; _add46i = _mul40i + _div45i;
        double _add47r = 0, _add47i = 0;
        _add47r = i1 + i2; _add47i = 0 + 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 3.0; _c48i = 0;
        double _mod49r = 0, _mod49i = 0;
        _mod49r = fmod(j, _c48r); _mod49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_add47r, _add47i, _mod49r, _mod49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _add46r + _mul50r; _add51i = _add46i + _mul50i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    double _c52r = 0, _c52i = 0;
    _c52r = 1.0; _c52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = n + _c52r; _add53i = 0 + _c52i;
    for (int k = 1; k < (int)(_add53r); k++) {
        double _c54r = 0, _c54i = 0;
        _c54r = 3.0; _c54i = 0;
        double _div55r = 0, _div55i = 0;
        c_div(n, 0, _c54r, _c54i, &_div55r, &_div55i);
        if (k <= _div55r) {
            double _c56r = 0, _c56i = 0;
            _c56r = 1.0; _c56i = 0;
            double _c57r = 0, _c57i = 0;
            _c57r = 0.5; _c57i = 0;
            double _sin58r = 0, _sin58i = 0;
            c_sin(k, 0, &_sin58r, &_sin58i);
            double _mul59r = 0, _mul59i = 0;
            c_mul(_c57r, _c57i, _sin58r, _sin58i, &_mul59r, &_mul59i);
            double _add60r = 0, _add60i = 0;
            _add60r = _c56r + _mul59r; _add60i = _c56i + _mul59i;
            { double _tr = cRe[(k - 1)]*_add60r - cIm[(k - 1)]*_add60i; cIm[(k - 1)] = cRe[(k - 1)]*_add60i + cIm[(k - 1)]*_add60r; cRe[(k - 1)] = _tr; }
        } else {
            double _c61r = 0, _c61i = 0;
            _c61r = 2.0; _c61i = 0;
            double _mul62r = 0, _mul62i = 0;
            c_mul(_c61r, _c61i, n, 0, &_mul62r, &_mul62i);
            double _c63r = 0, _c63i = 0;
            _c63r = 3.0; _c63i = 0;
            double _div64r = 0, _div64i = 0;
            c_div(_mul62r, _mul62i, _c63r, _c63i, &_div64r, &_div64i);
            if (k <= _div64r) {
                double _c65r = 0, _c65i = 0;
                _c65r = 1.0; _c65i = 0;
                double _c66r = 0, _c66i = 0;
                _c66r = 0.3; _c66i = 0;
                double _c67r = 0, _c67i = 0;
                _c67r = 2.0; _c67i = 0;
                double _mul68r = 0, _mul68i = 0;
                c_mul(k, 0, _c67r, _c67i, &_mul68r, &_mul68i);
                double _cos69r = 0, _cos69i = 0;
                c_cos(_mul68r, _mul68i, &_cos69r, &_cos69i);
                double _mul70r = 0, _mul70i = 0;
                c_mul(_c66r, _c66i, _cos69r, _cos69i, &_mul70r, &_mul70i);
                double _add71r = 0, _add71i = 0;
                _add71r = _c65r + _mul70r; _add71i = _c65i + _mul70i;
                { double _tr = cRe[(k - 1)]*_add71r - cIm[(k - 1)]*_add71i; cIm[(k - 1)] = cRe[(k - 1)]*_add71i + cIm[(k - 1)]*_add71r; cRe[(k - 1)] = _tr; }
            } else {
                double _c72r = 0, _c72i = 0;
                _c72r = 1.0; _c72i = 0;
                double _c73r = 0, _c73i = 0;
                _c73r = 0.2; _c73i = 0;
                double _c74r = 0, _c74i = 0;
                _c74r = 3.0; _c74i = 0;
                double _mul75r = 0, _mul75i = 0;
                c_mul(k, 0, _c74r, _c74i, &_mul75r, &_mul75i);
                double _sin76r = 0, _sin76i = 0;
                c_sin(_mul75r, _mul75i, &_sin76r, &_sin76i);
                double _mul77r = 0, _mul77i = 0;
                c_mul(_c73r, _c73i, _sin76r, _sin76i, &_mul77r, &_mul77i);
                double _cos78r = 0, _cos78i = 0;
                c_cos(k, 0, &_cos78r, &_cos78i);
                double _mul79r = 0, _mul79i = 0;
                c_mul(_mul77r, _mul77i, _cos78r, _cos78i, &_mul79r, &_mul79i);
                double _add80r = 0, _add80i = 0;
                _add80r = _c72r + _mul79r; _add80i = _c72i + _mul79i;
                { double _tr = cRe[(k - 1)]*_add80r - cIm[(k - 1)]*_add80i; cIm[(k - 1)] = cRe[(k - 1)]*_add80i + cIm[(k - 1)]*_add80r; cRe[(k - 1)] = _tr; }
            }
        }
    }
    for (int r = 1; r < (int)(n); r++) {
        double _c81r = 0, _c81i = 0;
        _c81r = 0.1; _c81i = 0;
        double _cf82r = 0, _cf82i = 0;
        { int _idx = r; if (_idx >= 0 && _idx < 36) { _cf82r = cRe[_idx]; _cf82i = cIm[_idx]; } }
        double _mul83r = 0, _mul83i = 0;
        c_mul(_c81r, _c81i, _cf82r, _cf82i, &_mul83r, &_mul83i);
        double _c84r = 0, _c84i = 0;
        _c84r = 0.0; _c84i = 1.0;
        double _cf85r = 0, _cf85i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf85r = cRe[_idx]; _cf85i = cIm[_idx]; } }
        double _ang86r = 0, _ang86i = 0;
        _ang86r = c_arg(_cf85r, _cf85i); _ang86i = 0;
        double _mul87r = 0, _mul87i = 0;
        c_mul(_c84r, _c84i, _ang86r, _ang86i, &_mul87r, &_mul87i);
        double _exp88r = 0, _exp88i = 0;
        c_exp2(_mul87r, _mul87i, &_exp88r, &_exp88i);
        double _mul89r = 0, _mul89i = 0;
        c_mul(_mul83r, _mul83i, _exp88r, _exp88i, &_mul89r, &_mul89i);
        cRe[(r - 1)] += _mul89r; cIm[(r - 1)] += _mul89i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_421: too complex for auto-transpile, stubbed */
static void poly_421_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
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

/* poly_423: too complex for auto-transpile, stubbed */
static void poly_423_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_424_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_re4r, _re4i, _pow6r, _pow6i, &_mul7r, &_mul7i);
        double mag_part1 = _mul7r; /* +_mul7ii */
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs8r + j; _add9i = _abs8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log10r, _log10i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double mag_part2 = _mul14r; /* +_mul14ii */
        double _re15r = 0, _re15i = 0;
        _re15r = x2r; _re15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _re15r, _re15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _sqrt18r = 0, _sqrt18i = 0;
        c_powr(j, 0, 0.5, &_sqrt18r, &_sqrt18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_cos17r, _cos17i, _sqrt18r, _sqrt18i, &_mul19r, &_mul19i);
        double mag_part3 = _mul19r; /* +_mul19ii */
        double _add20r = 0, _add20i = 0;
        _add20r = mag_part1 + mag_part2; _add20i = 0 + 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _add20r + mag_part3; _add21i = _add20i + 0;
        double magnitude = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _re23r = 0, _re23i = 0;
        _re23r = x1r; _re23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _re23r, _re23i, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_mul24r, _mul24i, &_sin25r, &_sin25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _ang22r + _sin25r; _add26i = _ang22i + _sin25i;
        double angle_part1 = _add26r; /* +_add26ii */
        double _im27r = 0, _im27i = 0;
        _im27r = x2i; _im27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, _im27r, _im27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_ang30r, _ang30i, j, 0, &_div31r, &_div31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _cos29r - _div31r; _sub32i = _cos29i - _div31i;
        double angle_part2 = _sub32r; /* +_sub32ii */
        double _add33r = 0, _add33i = 0;
        _add33r = angle_part1 + angle_part2; _add33i = 0 + 0;
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
        c_mul(magnitude, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_425_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, M_PI, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 6.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul4r, _mul4i, _c5r, _c5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 8.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_div10r, _div10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sin7r, _sin7i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang13r, _ang13i, _log16r, _log16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul12r + _mul17r; _add18i = _mul12i + _mul17i;
        double angle_part = _add18r; /* +_add18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_mul(j, 0, j, 0, &_pow21r, &_pow21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _abs19r + _pow21r; _add22i = _abs19i + _pow21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(j, 0, &_cos24r, &_cos24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_cos24r, _cos24i); _abs25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log23r, _log23i, _abs25r, _abs25i, &_mul26r, &_mul26i);
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x2r, x2i); _abs27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs27r + j; _add28i = _abs27i + 0;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_sin32r, _sin32i); _abs33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_log29r, _log29i, _abs33r, _abs33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul26r + _mul34r; _add35i = _mul26i + _mul34i;
        double magnitude_part = _add35r; /* +_add35ii */
        double _re36r = 0, _re36i = 0;
        _re36r = x1r; _re36i = 0;
        double _re37r = 0, _re37i = 0;
        _re37r = x2r; _re37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_re36r, _re36i, _re37r, _re37i, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = j + _c39r; _add40i = 0 + _c39i;
        double _div41r = 0, _div41i = 0;
        c_div(_mul38r, _mul38i, _add40r, _add40i, &_div41r, &_div41i);
        double _add42r = 0, _add42i = 0;
        _add42r = magnitude_part + _div41r; _add42i = 0 + _div41i;
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, angle_part, 0, &_mul44r, &_mul44i);
        double _exp45r = 0, _exp45i = 0;
        c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_add42r, _add42i, _exp45r, _exp45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
        double _c47r = 0, _c47i = 0;
        _c47r = 5.0; _c47i = 0;
        double _mod48r = 0, _mod48i = 0;
        _mod48r = fmod(j, _c47r); _mod48i = 0;
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 0;
        if (_mod48r == _c49r) {
            double _cf50r = 0, _cf50i = 0;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
            double _conj51r = 0, _conj51i = 0;
            _conj51r = _cf50r; _conj51i = -(_cf50i);
            cRe[(j - 1)] += _conj51r; cIm[(j - 1)] += _conj51i;
        }
        double _c52r = 0, _c52i = 0;
        _c52r = 1.0; _c52i = 0;
        double _c53r = 0, _c53i = 0;
        _c53r = 0.1; _c53i = 0;
        double _sin54r = 0, _sin54i = 0;
        c_sin(j, 0, &_sin54r, &_sin54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c53r, _c53i, _sin54r, _sin54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _c52r + _mul55r; _add56i = _c52i + _mul55i;
        { double _tr = cRe[(j - 1)]*_add56r - cIm[(j - 1)]*_add56i; cIm[(j - 1)] = cRe[(j - 1)]*_add56i + cIm[(j - 1)]*_add56r; cRe[(j - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_426_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int k = 1; k < (int)(_add3r); k++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 36.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - k; _sub5i = _c4i - 0;
        double j = _sub5r; /* +_sub5ii */
        double _re6r = 0, _re6i = 0;
        _re6r = x1r; _re6i = 0;
        double _sin7r = 0, _sin7i = 0;
        c_sin(k, 0, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re6r, _re6i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(j, 0, &_cos9r, &_cos9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_mul8r, _mul8i, _cos9r, _cos9i, &_mul10r, &_mul10i);
        double term1 = _mul10r; /* +_mul10ii */
        double _im11r = 0, _im11i = 0;
        _im11r = x2i; _im11i = 0;
        double _cos12r = 0, _cos12i = 0;
        c_cos(k, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_im11r, _im11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(j, 0, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_mul13r, _mul13i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double term2 = _mul15r; /* +_mul15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs16r + k; _add17i = _abs16i + 0;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double term3 = _log18r; /* +_log18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs19r + j; _add20i = _abs19i + 0;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double term4 = _log21r; /* +_log21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(k, 0, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang22r, _ang22i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 3.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(j, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang27r, _ang27i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul26r + _mul31r; _add32i = _mul26i + _mul31i;
        double angle = _add32r; /* +_add32ii */
        double _add33r = 0, _add33i = 0;
        _add33r = term1 + term2; _add33i = 0 + 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _add33r + term3; _add34i = _add33i + 0;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _add34r - term4; _sub35i = _add34i - 0;
        double magnitude = _sub35r; /* +_sub35ii */
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
        c_mul(magnitude, 0, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_427_c(double x1r, double x1i, double x2r, double x2i,
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
        double _re5r = 0, _re5i = 0;
        _re5r = x2r; _re5i = 0;
        double _re6r = 0, _re6i = 0;
        _re6r = x1r; _re6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _re5r - _re6r; _sub7i = _re5i - _re6i;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_sub7r, _sub7i, j, 0, &_mul8r, &_mul8i);
        double _div9r = 0, _div9i = 0;
        c_div(_mul8r, _mul8i, n, 0, &_div9r, &_div9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _re4r + _div9r; _add10i = _re4i + _div9i;
        double rec = _add10r; /* +_add10ii */
        double _im11r = 0, _im11i = 0;
        _im11r = x1i; _im11i = 0;
        double _im12r = 0, _im12i = 0;
        _im12r = x2i; _im12i = 0;
        double _im13r = 0, _im13i = 0;
        _im13r = x1i; _im13i = 0;
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _im12r - _im13r; _sub14i = _im12i - _im13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sub14r, _sub14i, j, 0, &_mul15r, &_mul15i);
        double _div16r = 0, _div16i = 0;
        c_div(_mul15r, _mul15i, n, 0, &_div16r, &_div16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _im11r + _div16r; _add17i = _im11i + _div16i;
        double imc = _add17r; /* +_add17ii */
        double _conj18r = 0, _conj18i = 0;
        _conj18r = x1r; _conj18i = -(x1i);
        double conj_t1 = _conj18r; /* +_conj18ii */
        double _conj19r = 0, _conj19i = 0;
        _conj19r = x2r; _conj19i = -(x2i);
        double conj_t2 = _conj19r; /* +_conj19ii */
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(rec, 0); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log23r, _log23i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double mag_part1 = _mul28r; /* +_mul28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(imc, 0); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs29r + _c30r; _add31i = _abs29i + _c30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 4.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_log32r, _log32i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double mag_part2 = _mul37r; /* +_mul37ii */
        double _add38r = 0, _add38i = 0;
        _add38r = mag_part1 + mag_part2; _add38i = 0 + 0;
        double magnitude = _add38r; /* +_add38ii */
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang39r, _ang39i, j, 0, &_mul40r, &_mul40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = j + _c42r; _add43i = 0 + _c42i;
        double _div44r = 0, _div44i = 0;
        c_div(_ang41r, _ang41i, _add43r, _add43i, &_div44r, &_div44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul40r + _div44r; _add45i = _mul40i + _div44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(j, 0, imc, 0, &_mul46r, &_mul46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(_mul46r, _mul46i, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(j, 0, rec, 0, &_mul48r, &_mul48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_mul48r, _mul48i, &_cos49r, &_cos49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_sin47r, _sin47i, _cos49r, _cos49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _add45r + _mul50r; _add51i = _add45i + _mul50i;
        double angle = _add51r; /* +_add51ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_428_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec[] = {0}; /* WARNING: could not evaluate */
    static const double imc[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec[(j - 1)]; _arr4i = 0;
        double _arr5r = 0, _arr5i = 0;
        _arr5r = imc[(j - 1)]; _arr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_arr4r, _arr4i, _arr5r, _arr5i, &_mul6r, &_mul6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_mul6r, _mul6i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 5.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c11r + _sin15r; _add16i = _c11i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log10r, _log10i, _add16r, _add16i, &_mul17r, &_mul17i);
        double mag_part = _mul17r; /* +_mul17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(j, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul20r + _mul25r; _add26i = _mul20i + _mul25i;
        double angle_part = _add26r; /* +_add26ii */
        double _cos27r = 0, _cos27i = 0;
        c_cos(angle_part, 0, &_cos27r, &_cos27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(angle_part, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c28r, _c28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _cos27r + _mul30r; _add31i = _cos27i + _mul30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag_part, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
        double _unk33r = 0, _unk33i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='j', ctx=Load())) */
        double _arr34r = 0, _arr34i = 0;
        _arr34r = rec[(int)(_unk33r)]; _arr34i = 0;
        double _unk35r = 0, _unk35i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='j', ctx=Load())) */
        double _arr36r = 0, _arr36i = 0;
        _arr36r = imc[(int)(_unk35r)]; _arr36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _arr34r + _arr36r; _add37i = _arr34i + _arr36i;
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_c38r, _c38i, j, 0, &_div39r, &_div39i);
        double _pow40r = 0, _pow40i = 0;
        c_powr(_add37r, _add37i, _div39r, &_pow40r, &_pow40i);
        cRe[(j - 1)] += _pow40r; cIm[(j - 1)] += _pow40i;
        double _c41r = 0, _c41i = 0;
        _c41r = 5.0; _c41i = 0;
        double _mod42r = 0, _mod42i = 0;
        _mod42r = fmod(j, _c41r); _mod42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 0;
        if (_mod42r == _c43r) {
            double _c44r = 0, _c44i = 0;
            _c44r = 0.0; _c44i = 1.0;
            double _cf45r = 0, _cf45i = 0;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf45r = cRe[_idx]; _cf45i = cIm[_idx]; } }
            double _ang46r = 0, _ang46i = 0;
            _ang46r = c_arg(_cf45r, _cf45i); _ang46i = 0;
            double _mul47r = 0, _mul47i = 0;
            c_mul(_c44r, _c44i, _ang46r, _ang46i, &_mul47r, &_mul47i);
            double _exp48r = 0, _exp48i = 0;
            c_exp2(_mul47r, _mul47i, &_exp48r, &_exp48i);
            { double _tr = cRe[(j - 1)]*_exp48r - cIm[(j - 1)]*_exp48i; cIm[(j - 1)] = cRe[(j - 1)]*_exp48i + cIm[(j - 1)]*_exp48r; cRe[(j - 1)] = _tr; }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec[] = {0}; /* WARNING: could not evaluate */
    static const double imc[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec[(j - 1)]; _arr4i = 0;
        double _arr5r = 0, _arr5i = 0;
        _arr5r = imc[(j - 1)]; _arr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_arr4r, _arr4i, _arr5r, _arr5i, &_mul6r, &_mul6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_mul6r, _mul6i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 4.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c11r + _sin15r; _add16i = _c11i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log10r, _log10i, _add16r, _add16i, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _c18r + _cos22r; _add23i = _c18i + _cos22i;
        double _add24r = 0, _add24i = 0;
        _add24r = _mul17r + _add23r; _add24i = _mul17i + _add23i;
        double mag = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _ang25r + _ang26r; _add27i = _ang25i + _ang26i;
        double _sin28r = 0, _sin28i = 0;
        c_sin(j, 0, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add27r + _sin28r; _add29i = _add27i + _sin28i;
        double _cos30r = 0, _cos30i = 0;
        c_cos(j, 0, &_cos30r, &_cos30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _add29r - _cos30r; _sub31i = _add29i - _cos30i;
        double ang = _sub31r; /* +_sub31ii */
        double _cos32r = 0, _cos32i = 0;
        c_cos(ang, 0, &_cos32r, &_cos32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _sin34r = 0, _sin34i = 0;
        c_sin(ang, 0, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c33r, _c33i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _cos32r + _mul35r; _add36i = _cos32i + _mul35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(mag, 0, _add36r, _add36i, &_mul37r, &_mul37i);
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x1r; _conj38i = -(x1i);
        double _re39r = 0, _re39i = 0;
        _re39r = x2r; _re39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_conj38r, _conj38i, _re39r, _re39i, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 2.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(j, 0, _c41r, _c41i, &_div42r, &_div42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_div42r, _div42i, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_mul40r, _mul40i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul37r + _mul44r; _add45i = _mul37i + _mul44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_431_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_arr4r, _arr4i, j, 0, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _arr7r = 0, _arr7i = 0;
        _arr7r = imc_seq[(j - 1)]; _arr7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.5; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_powr(j, 0, 1.5, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_arr7r, _arr7i, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _sin6r + _cos11r; _add12i = _sin6i + _cos11i;
        double term_real = _add12r; /* +_add12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs13r + j; _add14i = _abs13i + 0;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _arr16r = 0, _arr16i = 0;
        _arr16r = imc_seq[(j - 1)]; _arr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_arr16r, _arr16i, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _div20r = 0, _div20i = 0;
        c_div(_mul17r, _mul17i, _add19r, _add19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _log15r - _sin21r; _sub22i = _log15i - _sin21i;
        double term_imag = _sub22r; /* +_sub22ii */
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(term_real, 0, term_real, 0, &_pow24r, &_pow24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(term_imag, 0, term_imag, 0, &_pow26r, &_pow26i);
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
        double mag = _mul33r; /* +_mul33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = j + _c35r; _add36i = 0 + _c35i;
        double _log37r = 0, _log37i = 0;
        c_log(_add36r, _add36i, &_log37r, &_log37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang34r, _ang34i, _log37r, _log37i, &_mul38r, &_mul38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, M_PI, 0, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 7.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_mul39r, _mul39i, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul38r + _cos42r; _add43i = _mul38i + _cos42i;
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

static void poly_432_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_part1 = _mul10r; /* +_mul10ii */
        double _im11r = 0, _im11i = 0;
        _im11r = x1i; _im11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _im11r, _im11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 10.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(j, 0, _c16r, _c16i, &_div17r, &_div17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _c15r + _div17r; _add18i = _c15i + _div17i;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_abs14r, _abs14i, _add18r, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_cos13r, _cos13i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double mag_part2 = _mul20r; /* +_mul20ii */
        double _add21r = 0, _add21i = 0;
        _add21r = mag_part1 + mag_part2; _add21i = 0 + 0;
        double magnitude = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 5.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(j, 0, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang22r, _ang22i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double angle_part1 = _mul26r; /* +_mul26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 3.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(j, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang27r, _ang27i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double angle_part2 = _mul31r; /* +_mul31ii */
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
    double _c39r = 0, _c39i = 0;
    _c39r = 1.0; _c39i = 0;
    double _add40r = 0, _add40i = 0;
    _add40r = n + _c39r; _add40i = 0 + _c39i;
    for (int k = 1; k < (int)(_add40r); k++) {
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _re42r = 0, _re42i = 0;
        _re42r = x1r; _re42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_re42r, _re42i, k, 0, &_mul43r, &_mul43i);
        double _div44r = 0, _div44i = 0;
        c_div(_mul43r, _mul43i, n, 0, &_div44r, &_div44i);
        double _im45r = 0, _im45i = 0;
        _im45r = x2i; _im45i = 0;
        double _sub46r = 0, _sub46i = 0;
        _sub46r = n - k; _sub46i = 0 - 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_im45r, _im45i, _sub46r, _sub46i, &_mul47r, &_mul47i);
        double _div48r = 0, _div48i = 0;
        c_div(_mul47r, _mul47i, n, 0, &_div48r, &_div48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _div44r + _div48r; _add49i = _div44i + _div48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c41r, _c41i, _add49r, _add49i, &_mul50r, &_mul50i);
        double _exp51r = 0, _exp51i = 0;
        c_exp2(_mul50r, _mul50i, &_exp51r, &_exp51i);
        { double _tr = cRe[(k - 1)]*_exp51r - cIm[(k - 1)]*_exp51i; cIm[(k - 1)] = cRe[(k - 1)]*_exp51i + cIm[(k - 1)]*_exp51r; cRe[(k - 1)] = _tr; }
    }
    double _c52r = 0, _c52i = 0;
    _c52r = 1.0; _c52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = n + _c52r; _add53i = 0 + _c52i;
    for (int r = 1; r < (int)(_add53r); r++) {
        double _conj54r = 0, _conj54i = 0;
        _conj54r = x1r; _conj54i = -(x1i);
        double _conj55r = 0, _conj55i = 0;
        _conj55r = x2r; _conj55i = -(x2i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_conj54r, _conj54i, _conj55r, _conj55i, &_mul56r, &_mul56i);
        double _c57r = 0, _c57i = 0;
        _c57r = 1.0; _c57i = 0;
        double _add58r = 0, _add58i = 0;
        _add58r = r + _c57r; _add58i = 0 + _c57i;
        double _div59r = 0, _div59i = 0;
        c_div(_mul56r, _mul56i, _add58r, _add58i, &_div59r, &_div59i);
        cRe[(r - 1)] += _div59r; cIm[(r - 1)] += _div59i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_433_c(double x1r, double x1i, double x2r, double x2i,
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
        double _pow5r = 0, _pow5i = 0;
        c_powr(_re4r, _re4i, j, &_pow5r, &_pow5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_pow5r, _pow5i, _cos8r, _cos8i, &_mul9r, &_mul9i);
        double _re10r = 0, _re10i = 0;
        _re10r = x1r; _re10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _re10r, _re10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs13r + j; _add14i = _abs13i + 0;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sin12r, _sin12i, _log15r, _log15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul9r + _mul16r; _add17i = _mul9i + _mul16i;
        double real_component = _add17r; /* +_add17ii */
        double _im18r = 0, _im18i = 0;
        _im18r = x1i; _im18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_im18r, _im18i, _log21r, _log21i, &_mul22r, &_mul22i);
        double _im23r = 0, _im23i = 0;
        _im23r = x2i; _im23i = 0;
        double _cos24r = 0, _cos24i = 0;
        c_cos(_im23r, _im23i, &_cos24r, &_cos24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.5; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_abs25r, _abs25i, 0.5, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_cos24r, _cos24i, _pow27r, _pow27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul22r + _mul28r; _add29i = _mul22i + _mul28i;
        double imag_component = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, imag_component, 0, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = real_component + _mul31r; _add32i = 0 + _mul31i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_434_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(_arr4r, _arr4i); _abs5i = 0;
        double _arr6r = 0, _arr6i = 0;
        _arr6r = imc_seq[(j - 1)]; _arr6i = 0;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_arr6r, _arr6i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs5r + _abs7r; _add8i = _abs5i + _abs7i;
        double _add9r = 0, _add9i = 0;
        _add9r = _add8r + j; _add9i = _add8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _abs12r; _add13i = _abs11i + _abs12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 10.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(j, 0, _c15r, _c15i, &_div16r, &_div16i);
        double _pow17r = 0, _pow17i = 0;
        c_powr(_log14r, _log14i, _div16r, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log10r, _log10i, _pow17r, _pow17i, &_mul18r, &_mul18i);
        double magnitude = _mul18r; /* +_mul18ii */
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(j, 0, _c19r, _c19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin21r, _sin21i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
        double angle_part = _sin26r; /* +_sin26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _ang27r + _ang28r; _add29i = _ang27i + _ang28i;
        double _add30r = 0, _add30i = 0;
        _add30r = _add29r + angle_part; _add30i = _add29i + 0;
        double angle = _add30r; /* +_add30ii */
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
        double _sin36r = 0, _sin36i = 0;
        c_sin(j, 0, &_sin36r, &_sin36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 2.0; _c37i = 0;
        double _pow38r = 0, _pow38i = 0;
        c_mul(_sin36r, _sin36i, _sin36r, _sin36i, &_pow38r, &_pow38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_conj35r, _conj35i, _pow38r, _pow38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul34r + _mul39r; _add40i = _mul34i + _mul39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_435_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul5r = 0, _mul5i = 0;
        c_mul(_abs4r, _abs4i, j, 0, &_mul5r, &_mul5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = n - j; _sub7i = 0 - 0;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_abs6r, _abs6i, _sub7r, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _pow8r; _add9i = _mul5i + _pow8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _add9r + _c10r; _add11i = _add9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double mag_part = _log12r; /* +_log12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 7.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang13r, _ang13i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 5.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang19r, _ang19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul18r + _mul24r; _add25i = _mul18i + _mul24i;
        double angle_part = _add25r; /* +_add25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        double real_sum = _c26r; /* +_c26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 0;
        double imag_sum = _c27r; /* +_c27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        for (int k = 1; k < (int)(_add29r); k++) {
            double _re30r = 0, _re30i = 0;
            _re30r = x1r; _re30i = 0;
            double _pow31r = 0, _pow31i = 0;
            c_powr(_re30r, _re30i, k, &_pow31r, &_pow31i);
            double _re32r = 0, _re32i = 0;
            _re32r = x2r; _re32i = 0;
            double _sub33r = 0, _sub33i = 0;
            _sub33r = j - k; _sub33i = 0 - 0;
            double _pow34r = 0, _pow34i = 0;
            c_powr(_re32r, _re32i, _sub33r, &_pow34r, &_pow34i);
            double _mul35r = 0, _mul35i = 0;
            c_mul(_pow31r, _pow31i, _pow34r, _pow34i, &_mul35r, &_mul35i);
            real_sum += _mul35r;
            double _im36r = 0, _im36i = 0;
            _im36r = x1i; _im36i = 0;
            double _pow37r = 0, _pow37i = 0;
            c_powr(_im36r, _im36i, k, &_pow37r, &_pow37i);
            double _im38r = 0, _im38i = 0;
            _im38r = x2i; _im38i = 0;
            double _sub39r = 0, _sub39i = 0;
            _sub39r = j - k; _sub39i = 0 - 0;
            double _pow40r = 0, _pow40i = 0;
            c_powr(_im38r, _im38i, _sub39r, &_pow40r, &_pow40i);
            double _mul41r = 0, _mul41i = 0;
            c_mul(_pow37r, _pow37i, _pow40r, _pow40i, &_mul41r, &_mul41i);
            imag_sum += _mul41r;
        }
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = j + _c43r; _add44i = 0 + _c43i;
        double _div45r = 0, _div45i = 0;
        c_div(real_sum, 0, _add44r, _add44i, &_div45r, &_div45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_div45r, _div45i, &_sin46r, &_sin46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _c42r + _sin46r; _add47i = _c42i + _sin46i;
        double _mul48r = 0, _mul48i = 0;
        c_mul(mag_part, 0, _add47r, _add47i, &_mul48r, &_mul48i);
        double intricate_mag = _mul48r; /* +_mul48ii */
        double _c49r = 0, _c49i = 0;
        _c49r = 1.0; _c49i = 0;
        double _add50r = 0, _add50i = 0;
        _add50r = j + _c49r; _add50i = 0 + _c49i;
        double _div51r = 0, _div51i = 0;
        c_div(imag_sum, 0, _add50r, _add50i, &_div51r, &_div51i);
        double _cos52r = 0, _cos52i = 0;
        c_cos(_div51r, _div51i, &_cos52r, &_cos52i);
        double _add53r = 0, _add53i = 0;
        _add53r = angle_part + _cos52r; _add53i = 0 + _cos52i;
        double intricate_angle = _add53r; /* +_add53ii */
        double _cos54r = 0, _cos54i = 0;
        c_cos(intricate_angle, 0, &_cos54r, &_cos54i);
        double _c55r = 0, _c55i = 0;
        _c55r = 0.0; _c55i = 1.0;
        double _sin56r = 0, _sin56i = 0;
        c_sin(intricate_angle, 0, &_sin56r, &_sin56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_c55r, _c55i, _sin56r, _sin56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _cos54r + _mul57r; _add58i = _cos54i + _mul57i;
        double _mul59r = 0, _mul59i = 0;
        c_mul(intricate_mag, 0, _add58r, _add58i, &_mul59r, &_mul59i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec[] = {0}; /* WARNING: could not evaluate */
    static const double imc[] = {0}; /* WARNING: could not evaluate */
    for (int j = 0; j < (int)(n); j++) {
        double _arr2r = 0, _arr2i = 0;
        _arr2r = rec[j]; _arr2i = 0;
        double r = _arr2r; /* +_arr2ii */
        double _arr3r = 0, _arr3i = 0;
        _arr3r = imc[j]; _arr3i = 0;
        double im = _arr3r; /* +_arr3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 1.0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(im, 0, _c4r, _c4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = r + _mul5r; _add6i = 0 + _mul5i;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_add6r, _add6i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_add13r, _add13i, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 4.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _c11r + _sin17r; _add18i = _c11i + _sin17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log10r, _log10i, _add18r, _add18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_add22r, _add22i, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _c20r + _cos26r; _add27i = _c20i + _cos26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul19r, _mul19i, _add27r, _add27i, &_mul28r, &_mul28i);
        double mag = _mul28r; /* +_mul28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(im, 0, _c29r, _c29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = r + _mul30r; _add31i = 0 + _mul30i;
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(_add31r, _add31i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_add34r, _add34i, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 5.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _ang32r + _sin38r; _add39i = _ang32i + _sin38i;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = j + _c40r; _add41i = 0 + _c40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_add41r, _add41i, M_PI, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 6.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _sub46r = 0, _sub46i = 0;
        _sub46r = _add39r - _cos45r; _sub46i = _add39i - _cos45i;
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
        c_mul(mag, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_440_c(double x1r, double x1i, double x2r, double x2i,
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
        double _pow5r = 0, _pow5i = 0;
        c_powr(_re4r, _re4i, j, &_pow5r, &_pow5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(j, 0); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_pow5r, _pow5i, _log9r, _log9i, &_mul10r, &_mul10i);
        double _re11r = 0, _re11i = 0;
        _re11r = x2r; _re11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _re11r, _re11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(j, 0, j, 0, &_pow15r, &_pow15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_pow15r, _pow15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_sin13r, _sin13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul10r + _mul17r; _add18i = _mul10i + _mul17i;
        double real_part = _add18r; /* +_add18ii */
        double _im19r = 0, _im19i = 0;
        _im19r = x1i; _im19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 0.5; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(j, 0, 0.5, &_pow21r, &_pow21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_im19r, _im19i, _pow21r, _pow21i, &_mul22r, &_mul22i);
        double _im23r = 0, _im23i = 0;
        _im23r = x2i; _im23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _im23r, _im23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
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
        c_mul(_cos25r, _cos25i, _log30r, _log30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul22r + _mul31r; _add32i = _mul22i + _mul31i;
        double imag_part = _add32r; /* +_add32ii */
        double _call33r = 0, _call33i = 0;
        /* WARNING: unhandled call Name(id='complex', ctx=Load()) */
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 0.1; _c35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, j, 0, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _c34r + _mul36r; _add37i = _c34i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_call33r, _call33i, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_441_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    for (int j = 0; j < (int)(n); j++) {
        double _arr2r = 0, _arr2i = 0;
        _arr2r = rec_seq[j]; _arr2i = 0;
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(_arr2r, _arr2i); _abs3i = 0;
        double _arr4r = 0, _arr4i = 0;
        _arr4r = imc_seq[j]; _arr4i = 0;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(_arr4r, _arr4i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs3r + _abs5r; _add6i = _abs3i + _abs5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _add9r = 0, _add9i = 0;
        _add9r = _add6r + _add8r; _add9i = _add6i + _add8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _abs12r; _add13i = _abs11i + _abs12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _c16r = 0, _c16i = 0;
        _c16r = 10.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_add15r, _add15i, _c16r, _c16i, &_div17r, &_div17i);
        double _pow18r = 0, _pow18i = 0;
        c_powr(_add13r, _add13i, _div17r, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log10r, _log10i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double mag_part = _mul19r; /* +_mul19ii */
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_add21r, _add21i, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_add26r, _add26i, _ang27r, _ang27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_sin24r, _sin24i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x1r, x1i); _abs31i = 0;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x2r, x2i); _abs32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _abs31r + _abs32r; _add33i = _abs31i + _abs32i;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _add36r = 0, _add36i = 0;
        _add36r = _add33r + _add35r; _add36i = _add33i + _add35i;
        double _log37r = 0, _log37i = 0;
        c_log(_add36r, _add36i, &_log37r, &_log37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul30r + _log37r; _add38i = _mul30i + _log37i;
        double angle_part = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle_part, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(mag_part, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _neg44r = 0, _neg44i = 0;
        _neg44r = -(_c43r); _neg44i = -(_c43i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_neg44r, _neg44i, angle_part, 0, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(mag_part, 0, _exp46r, _exp46i, &_mul47r, &_mul47i);
        double _conj48r = 0, _conj48i = 0;
        _conj48r = _mul47r; _conj48i = -(_mul47i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul42r + _conj48r; _add49i = _mul42i + _conj48i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_442_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _abs4r + _pow6r; _add7i = _abs4i + _pow6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 4.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log8r, _log8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 6.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul13r + _cos17r; _add18i = _mul13i + _cos17i;
        double mag_part = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sin20r, _sin20i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _ang19r + _mul24r; _add25i = _ang19i + _mul24i;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add25r + _log29r; _add30i = _add25i + _log29i;
        double angle_part = _add30r; /* +_add30ii */
        double _cos31r = 0, _cos31i = 0;
        c_cos(angle_part, 0, &_cos31r, &_cos31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(angle_part, 0, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c32r, _c32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag_part, 0, _add35r, _add35i, &_mul36r, &_mul36i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    double _c37r = 0, _c37i = 0;
    _c37r = 1.0; _c37i = 0;
    double _add38r = 0, _add38i = 0;
    _add38r = n + _c37r; _add38i = 0 + _c37i;
    for (int k = 1; k < (int)(_add38r); k++) {
        double _c39r = 0, _c39i = 0;
        _c39r = 2.0; _c39i = 0;
        double _mod40r = 0, _mod40i = 0;
        _mod40r = fmod(k, _c39r); _mod40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 0;
        if (_mod40r == _c41r) {
            double _cf42r = 0, _cf42i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
            double _c43r = 0, _c43i = 0;
            _c43r = 0.0; _c43i = 1.0;
            double _ang44r = 0, _ang44i = 0;
            _ang44r = c_arg(x1r, x1i); _ang44i = 0;
            double _mul45r = 0, _mul45i = 0;
            c_mul(_c43r, _c43i, _ang44r, _ang44i, &_mul45r, &_mul45i);
            double _mul46r = 0, _mul46i = 0;
            c_mul(_mul45r, _mul45i, k, 0, &_mul46r, &_mul46i);
            double _c47r = 0, _c47i = 0;
            _c47r = 10.0; _c47i = 0;
            double _div48r = 0, _div48i = 0;
            c_div(_mul46r, _mul46i, _c47r, _c47i, &_div48r, &_div48i);
            double _exp49r = 0, _exp49i = 0;
            c_exp2(_div48r, _div48i, &_exp49r, &_exp49i);
            double _mul50r = 0, _mul50i = 0;
            c_mul(_cf42r, _cf42i, _exp49r, _exp49i, &_mul50r, &_mul50i);
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
        } else {
            double _cf51r = 0, _cf51i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
            double _c52r = 0, _c52i = 0;
            _c52r = 0.0; _c52i = 1.0;
            double _neg53r = 0, _neg53i = 0;
            _neg53r = -(_c52r); _neg53i = -(_c52i);
            double _ang54r = 0, _ang54i = 0;
            _ang54r = c_arg(x2r, x2i); _ang54i = 0;
            double _mul55r = 0, _mul55i = 0;
            c_mul(_neg53r, _neg53i, _ang54r, _ang54i, &_mul55r, &_mul55i);
            double _mul56r = 0, _mul56i = 0;
            c_mul(_mul55r, _mul55i, k, 0, &_mul56r, &_mul56i);
            double _c57r = 0, _c57i = 0;
            _c57r = 15.0; _c57i = 0;
            double _div58r = 0, _div58i = 0;
            c_div(_mul56r, _mul56i, _c57r, _c57i, &_div58r, &_div58i);
            double _exp59r = 0, _exp59i = 0;
            c_exp2(_div58r, _div58i, &_exp59r, &_exp59i);
            double _mul60r = 0, _mul60i = 0;
            c_mul(_cf51r, _cf51i, _exp59r, _exp59i, &_mul60r, &_mul60i);
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul60r; cIm[_idx] = _mul60i; } }
        }
    }
    double _c61r = 0, _c61i = 0;
    _c61r = 1.0; _c61i = 0;
    double _add62r = 0, _add62i = 0;
    _add62r = n + _c61r; _add62i = 0 + _c61i;
    for (int r = 1; r < (int)(_add62r); r++) {
        double _cf63r = 0, _cf63i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf63r = cRe[_idx]; _cf63i = cIm[_idx]; } }
        double _cf64r = 0, _cf64i = 0;
        { int _idx = (n - r); if (_idx >= 0 && _idx < 36) { _cf64r = cRe[_idx]; _cf64i = cIm[_idx]; } }
        double _conj65r = 0, _conj65i = 0;
        _conj65r = _cf64r; _conj65i = -(_cf64i);
        double _abs66r = 0, _abs66i = 0;
        _abs66r = c_abs(x1r, x1i); _abs66i = 0;
        double _c67r = 0, _c67i = 0;
        _c67r = 1.0; _c67i = 0;
        double _add68r = 0, _add68i = 0;
        _add68r = r + _c67r; _add68i = 0 + _c67i;
        double _div69r = 0, _div69i = 0;
        c_div(_abs66r, _abs66i, _add68r, _add68i, &_div69r, &_div69i);
        double _mul70r = 0, _mul70i = 0;
        c_mul(_conj65r, _conj65i, _div69r, _div69i, &_mul70r, &_mul70i);
        double _add71r = 0, _add71i = 0;
        _add71r = _cf63r + _mul70r; _add71i = _cf63i + _mul70i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add71r; cIm[_idx] = _add71i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_443_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double real_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imag_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = real_seq[(j - 1)]; _arr4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _arr4r + x2r; _add5i = _arr4i + x2i;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(_add5r, _add5i); _abs6i = 0;
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
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_cos18r, _cos18i, _abs19r, _abs19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul14r + _mul20r; _add21i = _mul14i + _mul20i;
        double mag = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, j, 0, &_mul23r, &_mul23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 6.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul23r + _sin27r; _add28i = _mul23i + _sin27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 7.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _add28r - _cos32r; _sub33i = _add28i - _cos32i;
        double angle = _sub33r; /* +_sub33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, angle, 0, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(mag, 0, _exp36r, _exp36i, &_mul37r, &_mul37i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = j + _c38r; _add39i = 0 + _c38i;
        for (int k = 1; k < (int)(_add39r); k++) {
            double _attr40r = 0, _attr40i = 0;
            _attr40r = x1r; _attr40i = 0;
            double _attr41r = 0, _attr41i = 0;
            _attr41r = x2r; _attr41i = 0;
            double _mul42r = 0, _mul42i = 0;
            c_mul(_attr40r, _attr40i, _attr41r, _attr41i, &_mul42r, &_mul42i);
            double _c43r = 0, _c43i = 0;
            _c43r = 1.0; _c43i = 0;
            double _add44r = 0, _add44i = 0;
            _add44r = k + _c43r; _add44i = 0 + _c43i;
            double _div45r = 0, _div45i = 0;
            c_div(_mul42r, _mul42i, _add44r, _add44i, &_div45r, &_div45i);
            double _c46r = 0, _c46i = 0;
            _c46r = 0.0; _c46i = 1.0;
            double _sin47r = 0, _sin47i = 0;
            c_sin(k, 0, &_sin47r, &_sin47i);
            double _cos48r = 0, _cos48i = 0;
            c_cos(k, 0, &_cos48r, &_cos48i);
            double _sub49r = 0, _sub49i = 0;
            _sub49r = _sin47r - _cos48r; _sub49i = _sin47i - _cos48i;
            double _mul50r = 0, _mul50i = 0;
            c_mul(_c46r, _c46i, _sub49r, _sub49i, &_mul50r, &_mul50i);
            double _exp51r = 0, _exp51i = 0;
            c_exp2(_mul50r, _mul50i, &_exp51r, &_exp51i);
            double _mul52r = 0, _mul52i = 0;
            c_mul(_div45r, _div45i, _exp51r, _exp51i, &_mul52r, &_mul52i);
            cRe[(j - 1)] += _mul52r; cIm[(j - 1)] += _mul52i;
        }
    }
    double _c53r = 0, _c53i = 0;
    _c53r = 1.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = n + _c53r; _add54i = 0 + _c53i;
    for (int r = 1; r < (int)(_add54r); r++) {
        double _cf55r = 0, _cf55i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
        double _c56r = 0, _c56i = 0;
        _c56r = 1.0; _c56i = 0;
        double _c57r = 0, _c57i = 0;
        _c57r = 0.05; _c57i = 0;
        double _c58r = 0, _c58i = 0;
        _c58r = 2.0; _c58i = 0;
        double _pow59r = 0, _pow59i = 0;
        c_mul(r, 0, r, 0, &_pow59r, &_pow59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_c57r, _c57i, _pow59r, _pow59i, &_mul60r, &_mul60i);
        double _add61r = 0, _add61i = 0;
        _add61r = _c56r + _mul60r; _add61i = _c56i + _mul60i;
        double _mul62r = 0, _mul62i = 0;
        c_mul(_cf55r, _cf55i, _add61r, _add61i, &_mul62r, &_mul62i);
        double _conj63r = 0, _conj63i = 0;
        _conj63r = x2r; _conj63i = -(x2i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(r, 0, M_PI, 0, &_mul64r, &_mul64i);
        double _c65r = 0, _c65i = 0;
        _c65r = 8.0; _c65i = 0;
        double _div66r = 0, _div66i = 0;
        c_div(_mul64r, _mul64i, _c65r, _c65i, &_div66r, &_div66i);
        double _sin67r = 0, _sin67i = 0;
        c_sin(_div66r, _div66i, &_sin67r, &_sin67i);
        double _mul68r = 0, _mul68i = 0;
        c_mul(_conj63r, _conj63i, _sin67r, _sin67i, &_mul68r, &_mul68i);
        double _add69r = 0, _add69i = 0;
        _add69r = _mul62r + _mul68r; _add69i = _mul62i + _mul68i;
        double _attr70r = 0, _attr70i = 0;
        _attr70r = x1r; _attr70i = 0;
        double _mul71r = 0, _mul71i = 0;
        c_mul(r, 0, M_PI, 0, &_mul71r, &_mul71i);
        double _c72r = 0, _c72i = 0;
        _c72r = 9.0; _c72i = 0;
        double _div73r = 0, _div73i = 0;
        c_div(_mul71r, _mul71i, _c72r, _c72i, &_div73r, &_div73i);
        double _cos74r = 0, _cos74i = 0;
        c_cos(_div73r, _div73i, &_cos74r, &_cos74i);
        double _mul75r = 0, _mul75i = 0;
        c_mul(_attr70r, _attr70i, _cos74r, _cos74i, &_mul75r, &_mul75i);
        double _sub76r = 0, _sub76i = 0;
        _sub76r = _add69r - _mul75r; _sub76i = _add69i - _mul75i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub76r; cIm[_idx] = _sub76i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_444_c(double x1r, double x1i, double x2r, double x2i,
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
        double _pow5r = 0, _pow5i = 0;
        c_powr(_abs4r, _abs4i, j, &_pow5r, &_pow5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = n - j; _sub7i = 0 - 0;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_abs6r, _abs6i, _sub7r, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow5r + _pow8r; _add9i = _pow5i + _pow8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x2i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_sin13r, _sin13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _log10r + _mul17r; _add18i = _log10i + _mul17i;
        double mag = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang19r, _ang19i, j, 0, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _sub22r = 0, _sub22i = 0;
        _sub22r = n - j; _sub22i = 0 - 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang21r, _ang21i, _sub22r, _sub22i, &_mul23r, &_mul23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _mul20r - _mul23r; _sub24i = _mul20i - _mul23i;
        double _sin25r = 0, _sin25i = 0;
        c_sin(j, 0, &_sin25r, &_sin25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _sub24r + _sin25r; _add26i = _sub24i + _sin25i;
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _add26r - _cos27r; _sub28i = _add26i - _cos27i;
        double angle = _sub28r; /* +_sub28ii */
        double _cos29r = 0, _cos29i = 0;
        c_cos(angle, 0, &_cos29r, &_cos29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(angle, 0, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c30r, _c30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _cos29r + _mul32r; _add33i = _cos29i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(mag, 0, _add33r, _add33i, &_mul34r, &_mul34i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_445_c(double x1r, double x1i, double x2r, double x2i,
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
        _c4r = 5.0; _c4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _c4r, _c4i, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mul5r + _c6r; _add7i = _mul5i + _c6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 12.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(_add7r, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _mod9r + _c10r; _add11i = _mod9i + _c10i;
        double k = _add11r; /* +_add11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1r; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _div14r = 0, _div14i = 0;
        c_div(_mul13r, _mul13i, k, 0, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_attr12r, _attr12i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x2r; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = k + _c19r; _add20i = 0 + _c19i;
        double _div21r = 0, _div21i = 0;
        c_div(_mul18r, _mul18i, _add20r, _add20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr17r, _attr17i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul16r + _mul23r; _add24i = _mul16i + _mul23i;
        double r_part = _add24r; /* +_add24ii */
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _div27r = 0, _div27i = 0;
        c_div(_mul26r, _mul26i, k, 0, &_div27r, &_div27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_div27r, _div27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_attr25r, _attr25i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, M_PI, 0, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = k + _c32r; _add33i = 0 + _c32i;
        double _div34r = 0, _div34i = 0;
        c_div(_mul31r, _mul31i, _add33r, _add33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_attr30r, _attr30i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _mul29r - _mul36r; _sub37i = _mul29i - _mul36i;
        double i_part = _sub37r; /* +_sub37ii */
        double _abs38r = 0, _abs38i = 0;
        _abs38r = c_abs(x1r, x1i); _abs38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _abs38r + j; _add39i = _abs38i + 0;
        double _log40r = 0, _log40i = 0;
        c_log(_add39r, _add39i, &_log40r, &_log40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, M_PI, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 10.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(_mul41r, _mul41i, _c42r, _c42i, &_div43r, &_div43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_div43r, _div43i, &_sin44r, &_sin44i);
        double _abs45r = 0, _abs45i = 0;
        _abs45r = c_abs(_sin44r, _sin44i); _abs45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_log40r, _log40i, _abs45r, _abs45i, &_mul46r, &_mul46i);
        double magnitude = _mul46r; /* +_mul46ii */
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x1r, x1i); _ang47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(j, 0, M_PI, 0, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 8.0; _c49i = 0;
        double _div50r = 0, _div50i = 0;
        c_div(_mul48r, _mul48i, _c49r, _c49i, &_div50r, &_div50i);
        double _cos51r = 0, _cos51i = 0;
        c_cos(_div50r, _div50i, &_cos51r, &_cos51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_ang47r, _ang47i, _cos51r, _cos51i, &_mul52r, &_mul52i);
        double _ang53r = 0, _ang53i = 0;
        _ang53r = c_arg(x2r, x2i); _ang53i = 0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(j, 0, M_PI, 0, &_mul54r, &_mul54i);
        double _c55r = 0, _c55i = 0;
        _c55r = 9.0; _c55i = 0;
        double _div56r = 0, _div56i = 0;
        c_div(_mul54r, _mul54i, _c55r, _c55i, &_div56r, &_div56i);
        double _sin57r = 0, _sin57i = 0;
        c_sin(_div56r, _div56i, &_sin57r, &_sin57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_ang53r, _ang53i, _sin57r, _sin57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _mul52r + _mul58r; _add59i = _mul52i + _mul58i;
        double angle = _add59r; /* +_add59ii */
        double _c60r = 0, _c60i = 0;
        _c60r = 0.0; _c60i = 1.0;
        double _mul61r = 0, _mul61i = 0;
        c_mul(_c60r, _c60i, i_part, 0, &_mul61r, &_mul61i);
        double _add62r = 0, _add62i = 0;
        _add62r = r_part + _mul61r; _add62i = 0 + _mul61i;
        double _mul63r = 0, _mul63i = 0;
        c_mul(magnitude, 0, _add62r, _add62i, &_mul63r, &_mul63i);
        double _cos64r = 0, _cos64i = 0;
        c_cos(angle, 0, &_cos64r, &_cos64i);
        double _c65r = 0, _c65i = 0;
        _c65r = 0.0; _c65i = 1.0;
        double _sin66r = 0, _sin66i = 0;
        c_sin(angle, 0, &_sin66r, &_sin66i);
        double _mul67r = 0, _mul67i = 0;
        c_mul(_c65r, _c65i, _sin66r, _sin66i, &_mul67r, &_mul67i);
        double _add68r = 0, _add68i = 0;
        _add68r = _cos64r + _mul67r; _add68i = _cos64i + _mul67i;
        double _mul69r = 0, _mul69i = 0;
        c_mul(_mul63r, _mul63i, _add68r, _add68i, &_mul69r, &_mul69i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul69r; cIm[_idx] = _mul69i; } }
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
        double _unk16r = 0, _unk16i = 0;
        /* WARNING: unhandled node List(elts=[Attribute(value=Name(id='t1', ctx=Load()), attr='real', ctx=Load()), Attribute(value=Name(id='t2', ctx=Load()), attr='imag', ctx=Load()), Name(id='j', ctx=Load())], ctx=Load()) */
        double _add17r = 0, _add17i = 0;
        _add17r = _mul15r + _unk16r; _add17i = _mul15i + _unk16i;
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang18r, _ang18i, j, 0, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 5.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang20r, _ang20i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul19r + _mul25r; _add26i = _mul19i + _mul25i;
        double _sin27r = 0, _sin27i = 0;
        c_sin(j, 0, &_sin27r, &_sin27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(_sin27r, _sin27i, _sin27r, _sin27i, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add26r + _pow29r; _add30i = _add26i + _pow29i;
        double angle = _add30r; /* +_add30ii */
        double _cos31r = 0, _cos31i = 0;
        c_cos(angle, 0, &_cos31r, &_cos31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(angle, 0, &_sin32r, &_sin32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin32r, _sin32i, _c33r, _c33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag, 0, _add35r, _add35i, &_mul36r, &_mul36i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2r; _attr5i = 0;
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _attr5r - _attr6r; _sub7i = _attr5i - _attr6i;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_sub7r, _sub7i, j, 0, &_mul8r, &_mul8i);
        double _div9r = 0, _div9i = 0;
        c_div(_mul8r, _mul8i, n, 0, &_div9r, &_div9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _attr4r + _div9r; _add10i = _attr4i + _div9i;
        double rec = _add10r; /* +_add10ii */
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1i; _attr11i = 0;
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _attr12r - _attr13r; _sub14i = _attr12i - _attr13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sub14r, _sub14i, j, 0, &_mul15r, &_mul15i);
        double _div16r = 0, _div16i = 0;
        c_div(_mul15r, _mul15i, n, 0, &_div16r, &_div16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _attr11r + _div16r; _add17i = _attr11i + _div16i;
        double imc = _add17r; /* +_add17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _abs19r; _add20i = _abs18i + _abs19i;
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_mul(j, 0, j, 0, &_pow22r, &_pow22i);
        c_mul(_pow22r, _pow22i, j, 0, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _add20r + _pow22r; _add23i = _add20i + _pow22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 5.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _c25r + _sin29r; _add30i = _c25i + _sin29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log24r, _log24i, _add30r, _add30i, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 4.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _c32r + _cos36r; _add37i = _c32i + _cos36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_mul31r, _mul31i, _add37r, _add37i, &_mul38r, &_mul38i);
        double mag = _mul38r; /* +_mul38ii */
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, M_PI, 0, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 3.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(_mul40r, _mul40i, _c41r, _c41i, &_div42r, &_div42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_div42r, _div42i, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_ang39r, _ang39i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x2r, x2i); _ang45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(j, 0, M_PI, 0, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 4.0; _c47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(_mul46r, _mul46i, _c47r, _c47i, &_div48r, &_div48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_div48r, _div48i, &_cos49r, &_cos49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_ang45r, _ang45i, _cos49r, _cos49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul44r + _mul50r; _add51i = _mul44i + _mul50i;
        double _mul52r = 0, _mul52i = 0;
        c_mul(j, 0, rec, 0, &_mul52r, &_mul52i);
        double _sin53r = 0, _sin53i = 0;
        c_sin(_mul52r, _mul52i, &_sin53r, &_sin53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(j, 0, imc, 0, &_mul54r, &_mul54i);
        double _cos55r = 0, _cos55i = 0;
        c_cos(_mul54r, _mul54i, &_cos55r, &_cos55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_sin53r, _sin53i, _cos55r, _cos55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _add51r + _mul56r; _add57i = _add51i + _mul56i;
        double angle = _add57r; /* +_add57ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul63r; cIm[_idx] = _mul63i; } }
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
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _ang4r, _ang4i, &_mul5r, &_mul5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _cos8r; _add9i = _mul5i + _cos8i;
        double _sin10r = 0, _sin10i = 0;
        c_sin(_add9r, _add9i, &_sin10r, &_sin10i);
        double angle_part = _sin10r; /* +_sin10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _abs12r; _add13i = _abs11i + _abs12i;
        double _add14r = 0, _add14i = 0;
        _add14r = _add13r + j; _add14i = _add13i + 0;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 0.1; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c17r, _c17i, j, 0, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _c16r + _mul18r; _add19i = _c16i + _mul18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log15r, _log15i, _add19r, _add19i, &_mul20r, &_mul20i);
        double mag_part = _mul20r; /* +_mul20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, n, 0, &_div22r, &_div22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_div22r, _div22i, M_PI, 0, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, n, 0, &_div27r, &_div27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_div27r, _div27i, M_PI, 0, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _mul25r - _mul30r; _sub31i = _mul25i - _mul30i;
        double phase_shift = _sub31r; /* +_sub31ii */
        double _add32r = 0, _add32i = 0;
        _add32r = angle_part + phase_shift; _add32i = 0 + 0;
        double _cos33r = 0, _cos33i = 0;
        c_cos(_add32r, _add32i, &_cos33r, &_cos33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = angle_part - phase_shift; _sub35i = 0 - 0;
        double _sin36r = 0, _sin36i = 0;
        c_sin(_sub35r, _sub35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c34r, _c34i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cos33r + _mul37r; _add38i = _cos33i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag_part, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int k = 1; k < 6; k++) {
        double _sub40r = 0, _sub40i = 0;
        _sub40r = n - k; _sub40i = 0 - 0;
        double idx = _sub40r; /* +_sub40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 0;
        if (idx >= _c41r) {
            double _cf42r = 0, _cf42i = 0;
            { int _idx = idx; if (_idx >= 0 && _idx < 36) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
            double _conj43r = 0, _conj43i = 0;
            _conj43r = x1r; _conj43i = -(x1i);
            double _c44r = 0, _c44i = 0;
            _c44r = 3.0; _c44i = 0;
            double _mod45r = 0, _mod45i = 0;
            _mod45r = fmod(k, _c44r); _mod45i = 0;
            double _c46r = 0, _c46i = 0;
            _c46r = 1.0; _c46i = 0;
            double _add47r = 0, _add47i = 0;
            _add47r = _mod45r + _c46r; _add47i = _mod45i + _c46i;
            double _pow48r = 0, _pow48i = 0;
            c_powr(_conj43r, _conj43i, _add47r, &_pow48r, &_pow48i);
            double _mul49r = 0, _mul49i = 0;
            c_mul(_cf42r, _cf42i, _pow48r, _pow48i, &_mul49r, &_mul49i);
            double _conj50r = 0, _conj50i = 0;
            _conj50r = x2r; _conj50i = -(x2i);
            double _c51r = 0, _c51i = 0;
            _c51r = 4.0; _c51i = 0;
            double _mod52r = 0, _mod52i = 0;
            _mod52r = fmod(k, _c51r); _mod52i = 0;
            double _pow53r = 0, _pow53i = 0;
            c_powr(_conj50r, _conj50i, _mod52r, &_pow53r, &_pow53i);
            double _add54r = 0, _add54i = 0;
            _add54r = _mul49r + _pow53r; _add54i = _mul49i + _pow53i;
            { int _idx = idx; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add54r; cIm[_idx] = _add54i; } }
        }
    }
    double _c55r = 0, _c55i = 0;
    _c55r = 1.0; _c55i = 0;
    double cumulative = _c55r; /* +_c55ii */
    double _c56r = 0, _c56i = 0;
    _c56r = 1.0; _c56i = 0;
    double _add57r = 0, _add57i = 0;
    _add57r = n + _c56r; _add57i = 0 + _c56i;
    for (int r = 1; r < (int)(_add57r); r++) {
        double _abs58r = 0, _abs58i = 0;
        _abs58r = c_abs(x1r, x1i); _abs58i = 0;
        double _abs59r = 0, _abs59i = 0;
        _abs59r = c_abs(x2r, x2i); _abs59i = 0;
        double _add60r = 0, _add60i = 0;
        _add60r = _abs58r + _abs59r; _add60i = _abs58i + _abs59i;
        double _add61r = 0, _add61i = 0;
        _add61r = _add60r + r; _add61i = _add60i + 0;
        cumulative *= _add61r;
        double _c62r = 0, _c62i = 0;
        _c62r = 1.0; _c62i = 0;
        double _add63r = 0, _add63i = 0;
        _add63r = r + _c62r; _add63i = 0 + _c62i;
        double _div64r = 0, _div64i = 0;
        c_div(cumulative, 0, _add63r, _add63i, &_div64r, &_div64i);
        cRe[(r - 1)] += _div64r; cIm[(r - 1)] += _div64i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_451_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_part1 = _mul8r; /* +_mul8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _c10r + _sin11r; _add12i = _c10i + _sin11i;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs9r, _abs9i, _add12r, &_pow13r, &_pow13i);
        double mag_part2 = _pow13r; /* +_pow13ii */
        double _add14r = 0, _add14i = 0;
        _add14r = mag_part1 + mag_part2; _add14i = 0 + 0;
        double magnitude = _add14r; /* +_add14ii */
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1r; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x2i; _attr18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, _attr18r, _attr18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _sin17r + _cos20r; _add21i = _sin17i + _cos20i;
        double phase_part1 = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, j, 0, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _sub25r = 0, _sub25i = 0;
        _sub25r = n - j; _sub25i = 0 - 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _sub25r, _sub25i, &_mul26r, &_mul26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _mul23r - _mul26r; _sub27i = _mul23i - _mul26i;
        double phase_part2 = _sub27r; /* +_sub27ii */
        double _mul28r = 0, _mul28i = 0;
        c_mul(phase_part1, 0, phase_part2, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 5.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sin31r, _sin31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul28r + _mul35r; _add36i = _mul28i + _mul35i;
        double phase = _add36r; /* +_add36ii */
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, phase, 0, &_mul38r, &_mul38i);
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

static void poly_452_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double temp = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = k + _c5r; _add6i = 0 + _c5i;
        for (int j = 1; j < (int)(_add6r); j++) {
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
            double _div14r = 0, _div14i = 0;
            c_div(_mul13r, _mul13i, j, 0, &_div14r, &_div14i);
            temp += _div14r;
        }
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_abs15r, _abs15i, _abs16r, _abs16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul17r + k; _add18i = _mul17i + 0;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(k, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin23r, _sin23i, _c24r, _c24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _c20r + _mul25r; _add26i = _c20i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log19r, _log19i, _add26r, _add26i, &_mul27r, &_mul27i);
        double magnitude = _mul27r; /* +_mul27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = k + _c30r; _add31i = 0 + _c30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _log32r, _log32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _ang28r + _mul33r; _add34i = _ang28i + _mul33i;
        double _add35r = 0, _add35i = 0;
        _add35r = _add34r + temp; _add35i = _add34i + 0;
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
        c_mul(magnitude, 0, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_453_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr4r, _attr4i, j, 0, &_mul5r, &_mul5i);
        double _div6r = 0, _div6i = 0;
        c_div(_mul5r, _mul5i, n, 0, &_div6r, &_div6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2r; _attr7i = 0;
        double _sub8r = 0, _sub8i = 0;
        _sub8r = n - j; _sub8i = 0 - 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _sub8r + _c9r; _add10i = _sub8i + _c9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr7r, _attr7i, _add10r, _add10i, &_mul11r, &_mul11i);
        double _div12r = 0, _div12i = 0;
        c_div(_mul11r, _mul11i, n, 0, &_div12r, &_div12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _div6r + _div12r; _add13i = _div6i + _div12i;
        double r_part = _add13r; /* +_add13ii */
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _div15r = 0, _div15i = 0;
        c_div(_mul14r, _mul14i, n, 0, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x1i; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sin16r, _sin16i, _attr17r, _attr17i, &_mul18r, &_mul18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _div20r = 0, _div20i = 0;
        c_div(_mul19r, _mul19i, n, 0, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x2i; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_cos21r, _cos21i, _attr22r, _attr22i, &_mul23r, &_mul23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _mul18r - _mul23r; _sub24i = _mul18i - _mul23i;
        double i_part = _sub24r; /* +_sub24ii */
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs25r + j; _add26i = _abs25i + 0;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(_sin31r, _sin31i); _abs32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _c28r + _abs32r; _add33i = _c28i + _abs32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_log27r, _log27i, _add33r, _add33i, &_mul34r, &_mul34i);
        double magnitude = _mul34r; /* +_mul34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x1r, x1i); _ang35i = 0;
        double _cos36r = 0, _cos36i = 0;
        c_cos(j, 0, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang35r, _ang35i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(j, 0, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang38r, _ang38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul37r + _mul40r; _add41i = _mul37i + _mul40i;
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
        c_mul(magnitude, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_454_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _arr5r = 0, _arr5i = 0;
        _arr5r = imc_seq[(j - 1)]; _arr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_arr4r, _arr4i, _arr5r, _arr5i, &_mul6r, &_mul6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_mul6r, _mul6i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 4.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c11r + _sin15r; _add16i = _c11i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log10r, _log10i, _add16r, _add16i, &_mul17r, &_mul17i);
        double _prod18r = 0, _prod18i = 0;
        /* WARNING: unhandled np.prod */
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_c19r, _c19i, j, 0, &_div20r, &_div20i);
        double _pow21r = 0, _pow21i = 0;
        c_powr(_prod18r, _prod18i, _div20r, &_pow21r, &_pow21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul17r + _pow21r; _add22i = _mul17i + _pow21i;
        double mag = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang23r, _ang23i, j, 0, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _ang25r, _ang25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul24r + _sin27r; _add28i = _mul24i + _sin27i;
        double _arr29r = 0, _arr29i = 0;
        _arr29r = imc_seq[(j - 1)]; _arr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _arr29r, _arr29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _add28r + _cos31r; _add32i = _add28i + _cos31i;
        double angle = _add32r; /* +_add32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c33r, _c33i, angle, 0, &_mul34r, &_mul34i);
        double _exp35r = 0, _exp35i = 0;
        c_exp2(_mul34r, _mul34i, &_exp35r, &_exp35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag, 0, _exp35r, _exp35i, &_mul36r, &_mul36i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
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

/* poly_456: too complex for auto-transpile, stubbed */
static void poly_456_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_457_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(_arr4r, _arr4i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 4.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log8r, _log8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double mag_part1 = _mul13r; /* +_mul13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
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
        double mag_part2 = _pow19r; /* +_pow19ii */
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(mag_part2, 0, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = mag_part1 + _mul23r; _add24i = 0 + _mul23i;
        double mag = _add24r; /* +_add24ii */
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
        double angle_part1 = _mul29r; /* +_mul29ii */
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_mul31r, _mul31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sin32r + _cos36r; _add37i = _sin32i + _cos36i;
        double angle_part2 = _add37r; /* +_add37ii */
        double _add38r = 0, _add38i = 0;
        _add38r = angle_part1 + angle_part2; _add38i = 0 + 0;
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int k = 1; k < (int)(_add3r); k++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_abs5r, _abs5i, k, 0, &_mul6r, &_mul6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mul6r + _abs7r; _add8i = _mul6i + _abs7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _add8r + _c9r; _add10i = _add8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr4r, _attr4i, _log11r, _log11i, &_mul12r, &_mul12i);
        double temp_real = _mul12r; /* +_mul12ii */
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2i; _attr13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(k, 0, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr13r, _attr13i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(k, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _cos18r; _add19i = _mul15i + _cos18i;
        double temp_imag = _add19r; /* +_add19ii */
        double _add20r = 0, _add20i = 0;
        _add20r = x1r + x2r; _add20i = x1i + x2i;
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(_add20r, _add20i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = k + _c22r; _add23i = 0 + _c22i;
        double _div24r = 0, _div24i = 0;
        c_div(_ang21r, _ang21i, _add23r, _add23i, &_div24r, &_div24i);
        double temp_angle = _div24r; /* +_div24ii */
        double _mul25r = 0, _mul25i = 0;
        c_mul(temp_imag, 0, temp_angle, 0, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = temp_real + _mul25r; _add26i = 0 + _mul25i;
        double magnitude = _add26r; /* +_add26ii */
        double _sin27r = 0, _sin27i = 0;
        c_sin(temp_real, 0, &_sin27r, &_sin27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(temp_imag, 0, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_cos28r, _cos28i, temp_angle, 0, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _sin27r + _mul29r; _add30i = _sin27i + _mul29i;
        double angle = _add30r; /* +_add30ii */
        double _cos31r = 0, _cos31i = 0;
        c_cos(angle, 0, &_cos31r, &_cos31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(angle, 0, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c32r, _c32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(magnitude, 0, _add35r, _add35i, &_mul36r, &_mul36i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_460_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x1r, x1i); _ang4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_ang4r, _ang4i, j, 0, &_mul5r, &_mul5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _div9r = 0, _div9i = 0;
        c_div(_ang6r, _ang6i, _add8r, _add8i, &_div9r, &_div9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul5r + _div9r; _add10i = _mul5i + _div9i;
        double _arr11r = 0, _arr11i = 0;
        _arr11r = rec_seq[(j - 1)]; _arr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _arr11r, _arr11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add10r + _sin13r; _add14i = _add10i + _sin13i;
        double _arr15r = 0, _arr15i = 0;
        _arr15r = imc_seq[(j - 1)]; _arr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _arr15r, _arr15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = _add14r - _cos17r; _sub18i = _add14i - _cos17i;
        double phase = _sub18r; /* +_sub18ii */
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
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_mul(j, 0, j, 0, &_pow25r, &_pow25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(j, 0, &_sin26r, &_sin26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin26r, _sin26i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _pow25r + _mul28r; _add29i = _pow25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_log23r, _log23i, _add29r, _add29i, &_mul30r, &_mul30i);
        double magnitude = _mul30r; /* +_mul30ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_461_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double rec1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double imc1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2r; _attr4i = 0;
    double rec2 = _attr4r; /* +_attr4ii */
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc2 = _attr5r; /* +_attr5ii */
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
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 4.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log10r, _log10i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _mod21r = 0, _mod21i = 0;
        _mod21r = fmod(j, _c20r); _mod21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _mod21r + _c22r; _add23i = _mod21i + _c22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_cos19r, _cos19i, _add23r, _add23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul15r + _mul24r; _add25i = _mul15i + _mul24i;
        double magnitude = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 5.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul30r + _mul35r; _add36i = _mul30i + _mul35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, M_PI, 0, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 6.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _add36r + _sin40r; _add41i = _add36i + _sin40i;
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
        c_mul(magnitude, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    double _c48r = 0, _c48i = 0;
    _c48r = 1.0; _c48i = 0;
    double _add49r = 0, _add49i = 0;
    _add49r = n + _c48r; _add49i = 0 + _c48i;
    for (int k = 1; k < (int)(_add49r); k++) {
        double _attr50r = 0, _attr50i = 0;
        _attr50r = x1r; _attr50i = 0;
        double _attr51r = 0, _attr51i = 0;
        _attr51r = x2r; _attr51i = 0;
        double _sub52r = 0, _sub52i = 0;
        _sub52r = _attr50r - _attr51r; _sub52i = _attr50i - _attr51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(k, 0, M_PI, 0, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 7.0; _c54i = 0;
        double _div55r = 0, _div55i = 0;
        c_div(_mul53r, _mul53i, _c54r, _c54i, &_div55r, &_div55i);
        double _sin56r = 0, _sin56i = 0;
        c_sin(_div55r, _div55i, &_sin56r, &_sin56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_sub52r, _sub52i, _sin56r, _sin56i, &_mul57r, &_mul57i);
        double _attr58r = 0, _attr58i = 0;
        _attr58r = x1i; _attr58i = 0;
        double _attr59r = 0, _attr59i = 0;
        _attr59r = x2i; _attr59i = 0;
        double _add60r = 0, _add60i = 0;
        _add60r = _attr58r + _attr59r; _add60i = _attr58i + _attr59i;
        double _mul61r = 0, _mul61i = 0;
        c_mul(k, 0, M_PI, 0, &_mul61r, &_mul61i);
        double _c62r = 0, _c62i = 0;
        _c62r = 8.0; _c62i = 0;
        double _div63r = 0, _div63i = 0;
        c_div(_mul61r, _mul61i, _c62r, _c62i, &_div63r, &_div63i);
        double _cos64r = 0, _cos64i = 0;
        c_cos(_div63r, _div63i, &_cos64r, &_cos64i);
        double _mul65r = 0, _mul65i = 0;
        c_mul(_add60r, _add60i, _cos64r, _cos64i, &_mul65r, &_mul65i);
        double _add66r = 0, _add66i = 0;
        _add66r = _mul57r + _mul65r; _add66i = _mul57i + _mul65i;
        cRe[(k - 1)] += _add66r; cIm[(k - 1)] += _add66i;
    }
    double _c67r = 0, _c67i = 0;
    _c67r = 1.0; _c67i = 0;
    double _add68r = 0, _add68i = 0;
    _add68r = n + _c67r; _add68i = 0 + _c67i;
    for (int r = 1; r < (int)(_add68r); r++) {
        double _c69r = 0, _c69i = 0;
        _c69r = 0.0; _c69i = 1.0;
        double _sin70r = 0, _sin70i = 0;
        c_sin(r, 0, &_sin70r, &_sin70i);
        double _cos71r = 0, _cos71i = 0;
        c_cos(r, 0, &_cos71r, &_cos71i);
        double _add72r = 0, _add72i = 0;
        _add72r = _sin70r + _cos71r; _add72i = _sin70i + _cos71i;
        double _mul73r = 0, _mul73i = 0;
        c_mul(_c69r, _c69i, _add72r, _add72i, &_mul73r, &_mul73i);
        double _exp74r = 0, _exp74i = 0;
        c_exp2(_mul73r, _mul73i, &_exp74r, &_exp74i);
        { double _tr = cRe[(r - 1)]*_exp74r - cIm[(r - 1)]*_exp74i; cIm[(r - 1)] = cRe[(r - 1)]*_exp74i + cIm[(r - 1)]*_exp74r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_462_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _arr5r = 0, _arr5i = 0;
        _arr5r = imc_seq[(j - 1)]; _arr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_arr4r, _arr4i, _arr5r, _arr5i, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mul6r + _c7r; _add8i = _mul6i + _c7i;
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
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(j, 0, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sin14r, _sin14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _c13r + _mul18r; _add19i = _c13i + _mul18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log12r, _log12i, _add19r, _add19i, &_mul20r, &_mul20i);
        double mag = _mul20r; /* +_mul20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 4.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang21r, _ang21i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 5.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang27r, _ang27i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul26r + _mul32r; _add33i = _mul26i + _mul32i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_463_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log6r, _log6i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul10r + _cos13r; _add14i = _mul10i + _cos13i;
        double mag_part = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang15r, _ang15i, j, 0, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _sub18r = 0, _sub18i = 0;
        _sub18r = n - j; _sub18i = 0 - 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang17r, _ang17i, _sub18r, _sub18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul16r + _mul19r; _add20i = _mul16i + _mul19i;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2r; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _add20r + _sin23r; _add24i = _add20i + _sin23i;
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _add24r - _cos27r; _sub28i = _add24i - _cos27i;
        double ang_part = _sub28r; /* +_sub28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, ang_part, 0, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag_part, 0, _exp31r, _exp31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int k = 1; k < 6; k++) {
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = n + _c33r; _add34i = 0 + _c33i;
        for (int r = 1; r < (int)(_add34r); r++) {
            double _attr35r = 0, _attr35i = 0;
            _attr35r = x1r; _attr35i = 0;
            double _mul36r = 0, _mul36i = 0;
            c_mul(k, 0, r, 0, &_mul36r, &_mul36i);
            double _cos37r = 0, _cos37i = 0;
            c_cos(_mul36r, _mul36i, &_cos37r, &_cos37i);
            double _mul38r = 0, _mul38i = 0;
            c_mul(_attr35r, _attr35i, _cos37r, _cos37i, &_mul38r, &_mul38i);
            double _attr39r = 0, _attr39i = 0;
            _attr39r = x2i; _attr39i = 0;
            double _mul40r = 0, _mul40i = 0;
            c_mul(k, 0, r, 0, &_mul40r, &_mul40i);
            double _sin41r = 0, _sin41i = 0;
            c_sin(_mul40r, _mul40i, &_sin41r, &_sin41i);
            double _mul42r = 0, _mul42i = 0;
            c_mul(_attr39r, _attr39i, _sin41r, _sin41i, &_mul42r, &_mul42i);
            double _add43r = 0, _add43i = 0;
            _add43r = _mul38r + _mul42r; _add43i = _mul38i + _mul42i;
            double _c44r = 0, _c44i = 0;
            _c44r = 0.0; _c44i = 1.0;
            double _attr45r = 0, _attr45i = 0;
            _attr45r = x2r; _attr45i = 0;
            double _mul46r = 0, _mul46i = 0;
            c_mul(_attr45r, _attr45i, r, 0, &_mul46r, &_mul46i);
            double _attr47r = 0, _attr47i = 0;
            _attr47r = x1i; _attr47i = 0;
            double _mul48r = 0, _mul48i = 0;
            c_mul(_attr47r, _attr47i, k, 0, &_mul48r, &_mul48i);
            double _sub49r = 0, _sub49i = 0;
            _sub49r = _mul46r - _mul48r; _sub49i = _mul46i - _mul48i;
            double _mul50r = 0, _mul50i = 0;
            c_mul(_c44r, _c44i, _sub49r, _sub49i, &_mul50r, &_mul50i);
            double _exp51r = 0, _exp51i = 0;
            c_exp2(_mul50r, _mul50i, &_exp51r, &_exp51i);
            double _mul52r = 0, _mul52i = 0;
            c_mul(_add43r, _add43i, _exp51r, _exp51i, &_mul52r, &_mul52i);
            cRe[(r - 1)] += _mul52r; cIm[(r - 1)] += _mul52i;
        }
    }
    double _c53r = 0, _c53i = 0;
    _c53r = 1.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = n + _c53r; _add54i = 0 + _c53i;
    for (int j = 1; j < (int)(_add54r); j++) {
        double _c55r = 0, _c55i = 0;
        _c55r = 1.0; _c55i = 0;
        double _c56r = 0, _c56i = 0;
        _c56r = 0.1; _c56i = 0;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_c56r, _c56i, j, 0, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _c55r + _mul57r; _add58i = _c55i + _mul57i;
        double _c59r = 0, _c59i = 0;
        _c59r = 1.0; _c59i = 0;
        double _c60r = 0, _c60i = 0;
        _c60r = 1.0; _c60i = 0;
        double _add61r = 0, _add61i = 0;
        _add61r = j + _c60r; _add61i = 0 + _c60i;
        double _log62r = 0, _log62i = 0;
        c_log(_add61r, _add61i, &_log62r, &_log62i);
        double _add63r = 0, _add63i = 0;
        _add63r = _c59r + _log62r; _add63i = _c59i + _log62i;
        double _div64r = 0, _div64i = 0;
        c_div(_add58r, _add58i, _add63r, _add63i, &_div64r, &_div64i);
        { double _tr = cRe[(j - 1)]*_div64r - cIm[(j - 1)]*_div64i; cIm[(j - 1)] = cRe[(j - 1)]*_div64i + cIm[(j - 1)]*_div64r; cRe[(j - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_464_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int k = 1; k < (int)(_add3r); k++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + k; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log6r, _log6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 5.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _sqrt16r = 0, _sqrt16i = 0;
        c_powr(k, 0, 0.5, &_sqrt16r, &_sqrt16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_cos15r, _cos15i, _sqrt16r, _sqrt16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul11r + _mul17r; _add18i = _mul11i + _mul17i;
        double mag_part = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _sin20r = 0, _sin20i = 0;
        c_sin(k, 0, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _cos23r = 0, _cos23i = 0;
        c_cos(k, 0, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x1r; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(k, 0, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2i; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(k, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_sin28r, _sin28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _add25r + _mul32r; _add33i = _add25i + _mul32i;
        double angle_part = _add33r; /* +_add33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, angle_part, 0, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(mag_part, 0, _exp36r, _exp36i, &_mul37r, &_mul37i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    double _c38r = 0, _c38i = 0;
    _c38r = 1.0; _c38i = 0;
    double _add39r = 0, _add39i = 0;
    _add39r = n + _c38r; _add39i = 0 + _c38i;
    for (int r = 1; r < (int)(_add39r); r++) {
        double _attr40r = 0, _attr40i = 0;
        _attr40r = x1r; _attr40i = 0;
        double _attr41r = 0, _attr41i = 0;
        _attr41r = x2r; _attr41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_attr40r, _attr40i, _attr41r, _attr41i, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = r + _c43r; _add44i = 0 + _c43i;
        double _div45r = 0, _div45i = 0;
        c_div(_mul42r, _mul42i, _add44r, _add44i, &_div45r, &_div45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _attr47r = 0, _attr47i = 0;
        _attr47r = x1i; _attr47i = 0;
        double _attr48r = 0, _attr48i = 0;
        _attr48r = x2i; _attr48i = 0;
        double _sub49r = 0, _sub49i = 0;
        _sub49r = _attr47r - _attr48r; _sub49i = _attr47i - _attr48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c46r, _c46i, _sub49r, _sub49i, &_mul50r, &_mul50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(r, 0, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_mul50r, _mul50i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _div45r + _mul52r; _add53i = _div45i + _mul52i;
        cRe[(r - 1)] += _add53r; cIm[(r - 1)] += _add53i;
    }
    double _c54r = 0, _c54i = 0;
    _c54r = 1.0; _c54i = 0;
    double _add55r = 0, _add55i = 0;
    _add55r = n + _c54r; _add55i = 0 + _c54i;
    for (int j = 1; j < (int)(_add55r); j++) {
        double _c56r = 0, _c56i = 0;
        _c56r = 1.0; _c56i = 0;
        double _c57r = 0, _c57i = 0;
        _c57r = 0.1; _c57i = 0;
        double _mul58r = 0, _mul58i = 0;
        c_mul(_c57r, _c57i, j, 0, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _c56r + _mul58r; _add59i = _c56i + _mul58i;
        double _c60r = 0, _c60i = 0;
        _c60r = 0.0; _c60i = 0.05;
        double _mul61r = 0, _mul61i = 0;
        c_mul(_c60r, _c60i, j, 0, &_mul61r, &_mul61i);
        double _exp62r = 0, _exp62i = 0;
        c_exp2(_mul61r, _mul61i, &_exp62r, &_exp62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(_add59r, _add59i, _exp62r, _exp62i, &_mul63r, &_mul63i);
        { double _tr = cRe[(j - 1)]*_mul63r - cIm[(j - 1)]*_mul63i; cIm[(j - 1)] = cRe[(j - 1)]*_mul63i + cIm[(j - 1)]*_mul63r; cRe[(j - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_465_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x1r, x1i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log6r, _log6i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x2r, x2i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul10r + _cos13r; _add14i = _mul10i + _cos13i;
        double mag_part = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang15r, _ang15i, j, 0, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _sub18r = 0, _sub18i = 0;
        _sub18r = n - j; _sub18i = 0 - 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang17r, _ang17i, _sub18r, _sub18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul16r + _mul19r; _add20i = _mul16i + _mul19i;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1r; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _add20r + _sin23r; _add24i = _add20i + _sin23i;
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _add24r - _cos27r; _sub28i = _add24i - _cos27i;
        double ang_part = _sub28r; /* +_sub28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, ang_part, 0, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag_part, 0, _exp31r, _exp31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int k = 1; k < 6; k++) {
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = n + _c33r; _add34i = 0 + _c33i;
        for (int r = 1; r < (int)(_add34r); r++) {
            double _attr35r = 0, _attr35i = 0;
            _attr35r = x1r; _attr35i = 0;
            double _mul36r = 0, _mul36i = 0;
            c_mul(k, 0, r, 0, &_mul36r, &_mul36i);
            double _cos37r = 0, _cos37i = 0;
            c_cos(_mul36r, _mul36i, &_cos37r, &_cos37i);
            double _mul38r = 0, _mul38i = 0;
            c_mul(_attr35r, _attr35i, _cos37r, _cos37i, &_mul38r, &_mul38i);
            double _attr39r = 0, _attr39i = 0;
            _attr39r = x2i; _attr39i = 0;
            double _mul40r = 0, _mul40i = 0;
            c_mul(k, 0, r, 0, &_mul40r, &_mul40i);
            double _sin41r = 0, _sin41i = 0;
            c_sin(_mul40r, _mul40i, &_sin41r, &_sin41i);
            double _mul42r = 0, _mul42i = 0;
            c_mul(_attr39r, _attr39i, _sin41r, _sin41i, &_mul42r, &_mul42i);
            double _add43r = 0, _add43i = 0;
            _add43r = _mul38r + _mul42r; _add43i = _mul38i + _mul42i;
            double _c44r = 0, _c44i = 0;
            _c44r = 0.0; _c44i = 1.0;
            double _attr45r = 0, _attr45i = 0;
            _attr45r = x2r; _attr45i = 0;
            double _mul46r = 0, _mul46i = 0;
            c_mul(_attr45r, _attr45i, r, 0, &_mul46r, &_mul46i);
            double _attr47r = 0, _attr47i = 0;
            _attr47r = x1i; _attr47i = 0;
            double _mul48r = 0, _mul48i = 0;
            c_mul(_attr47r, _attr47i, k, 0, &_mul48r, &_mul48i);
            double _sub49r = 0, _sub49i = 0;
            _sub49r = _mul46r - _mul48r; _sub49i = _mul46i - _mul48i;
            double _mul50r = 0, _mul50i = 0;
            c_mul(_c44r, _c44i, _sub49r, _sub49i, &_mul50r, &_mul50i);
            double _exp51r = 0, _exp51i = 0;
            c_exp2(_mul50r, _mul50i, &_exp51r, &_exp51i);
            double _mul52r = 0, _mul52i = 0;
            c_mul(_add43r, _add43i, _exp51r, _exp51i, &_mul52r, &_mul52i);
            cRe[(r - 1)] += _mul52r; cIm[(r - 1)] += _mul52i;
        }
    }
    double _c53r = 0, _c53i = 0;
    _c53r = 1.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = n + _c53r; _add54i = 0 + _c53i;
    for (int j = 1; j < (int)(_add54r); j++) {
        double _c55r = 0, _c55i = 0;
        _c55r = 1.0; _c55i = 0;
        double _c56r = 0, _c56i = 0;
        _c56r = 0.1; _c56i = 0;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_c56r, _c56i, j, 0, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _c55r + _mul57r; _add58i = _c55i + _mul57i;
        { double _tr = cRe[(j - 1)]*_add58r - cIm[(j - 1)]*_add58i; cIm[(j - 1)] = cRe[(j - 1)]*_add58i + cIm[(j - 1)]*_add58r; cRe[(j - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_466_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _abs4r + _pow6r; _add7i = _abs4i + _pow6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_sin11r, _sin11i); _abs12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log8r, _log8i, _abs12r, _abs12i, &_mul13r, &_mul13i);
        double _sqrt14r = 0, _sqrt14i = 0;
        c_powr(j, 0, 0.5, &_sqrt14r, &_sqrt14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sqrt14r, _sqrt14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul13r + _mul18r; _add19i = _mul13i + _mul18i;
        double magnitude = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _log23r, _log23i, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.5; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = j + _c26r; _add27i = 0 + _c26i;
        double _div28r = 0, _div28i = 0;
        c_div(_attr25r, _attr25i, _add27r, _add27i, &_div28r, &_div28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul24r - _div28r; _sub29i = _mul24i - _div28i;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x1r; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_mul31r, _mul31i, &_sin32r, &_sin32i);
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x2i; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_mul34r, _mul34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_sin32r, _sin32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sub29r + _mul36r; _add37i = _sub29i + _mul36i;
        double angle = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, angle, 0, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_467_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_sum = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        for (int k = 1; k < (int)(_add6r); k++) {
            double _abs7r = 0, _abs7i = 0;
            _abs7r = c_abs(x1r, x1i); _abs7i = 0;
            double _add8r = 0, _add8i = 0;
            _add8r = _abs7r + k; _add8i = _abs7i + 0;
            double _log9r = 0, _log9i = 0;
            c_log(_add8r, _add8i, &_log9r, &_log9i);
            double _ang10r = 0, _ang10i = 0;
            _ang10r = c_arg(x2r, x2i); _ang10i = 0;
            double _mul11r = 0, _mul11i = 0;
            c_mul(k, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
            double _sin12r = 0, _sin12i = 0;
            c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_log9r, _log9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
            mag_sum += _mul13r;
        }
        double _sub14r = 0, _sub14i = 0;
        _sub14r = n - j; _sub14i = 0 - 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _sub14r + _c15r; _add16i = _sub14i + _c15i;
        for (int r = 1; r < (int)(_add16r); r++) {
            double _abs17r = 0, _abs17i = 0;
            _abs17r = c_abs(x2r, x2i); _abs17i = 0;
            double _add18r = 0, _add18i = 0;
            _add18r = _abs17r + r; _add18i = _abs17i + 0;
            double _log19r = 0, _log19i = 0;
            c_log(_add18r, _add18i, &_log19r, &_log19i);
            double _ang20r = 0, _ang20i = 0;
            _ang20r = c_arg(x2r, x2i); _ang20i = 0;
            double _mul21r = 0, _mul21i = 0;
            c_mul(r, 0, _ang20r, _ang20i, &_mul21r, &_mul21i);
            double _cos22r = 0, _cos22i = 0;
            c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
            double _mul23r = 0, _mul23i = 0;
            c_mul(_log19r, _log19i, _cos22r, _cos22i, &_mul23r, &_mul23i);
            mag_sum += _mul23r;
        }
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = mag_sum + _c24r; _add25i = 0 + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double mag = _log26r; /* +_log26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(mag_sum, 0, _add28r, _add28i, &_div29r, &_div29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = n - j; _sub30i = 0 - 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _sub30r + _c31r; _add32i = _sub30i + _c31i;
        double _div33r = 0, _div33i = 0;
        c_div(mag_sum, 0, _add32r, _add32i, &_div33r, &_div33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _div29r + _div33r; _add34i = _div29i + _div33i;
        double angle = _add34r; /* +_add34ii */
        double _cos35r = 0, _cos35i = 0;
        c_cos(angle, 0, &_cos35r, &_cos35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(angle, 0, &_sin36r, &_sin36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_sin36r, _sin36i, _c37r, _c37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _cos35r + _mul38r; _add39i = _cos35i + _mul38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(mag, 0, _add39r, _add39i, &_mul40r, &_mul40i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    double _c41r = 0, _c41i = 0;
    _c41r = 1.0; _c41i = 0;
    double _add42r = 0, _add42i = 0;
    _add42r = n + _c41r; _add42i = 0 + _c41i;
    for (int j = 1; j < (int)(_add42r); j++) {
        double _cf43r = 0, _cf43i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 0.05; _c45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 2.0; _c46i = 0;
        double _pow47r = 0, _pow47i = 0;
        c_mul(j, 0, j, 0, &_pow47r, &_pow47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c45r, _c45i, _pow47r, _pow47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _c44r + _mul48r; _add49i = _c44i + _mul48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_cf43r, _cf43i, _add49r, _add49i, &_mul50r, &_mul50i);
        double _cf51r = 0, _cf51i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
        double _conj52r = 0, _conj52i = 0;
        _conj52r = _cf51r; _conj52i = -(_cf51i);
        double _c53r = 0, _c53i = 0;
        _c53r = 0.02; _c53i = 0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_conj52r, _conj52i, _c53r, _c53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _mul50r + _mul54r; _add55i = _mul50i + _mul54i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_467_old_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double mag = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double angle = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        for (int k = 1; k < (int)(_add5r); k++) {
            double _abs6r = 0, _abs6i = 0;
            _abs6r = c_abs(x1r, x1i); _abs6i = 0;
            double _add7r = 0, _add7i = 0;
            _add7r = _abs6r + k; _add7i = _abs6i + 0;
            double _log8r = 0, _log8i = 0;
            c_log(_add7r, _add7i, &_log8r, &_log8i);
            double _re9r = 0, _re9i = 0;
            _re9r = x2r; _re9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, _re9r, _re9i, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_log8r, _log8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            mag += _mul12r;
            double _pow13r = 0, _pow13i = 0;
            c_powr(x2r, x2i, k, &_pow13r, &_pow13i);
            double _ang14r = 0, _ang14i = 0;
            _ang14r = c_arg(_pow13r, _pow13i); _ang14i = 0;
            double _c15r = 0, _c15i = 0;
            _c15r = 1.0; _c15i = 0;
            double _add16r = 0, _add16i = 0;
            _add16r = j + _c15r; _add16i = 0 + _c15i;
            double _div17r = 0, _div17i = 0;
            c_div(k, 0, _add16r, _add16i, &_div17r, &_div17i);
            double _cos18r = 0, _cos18i = 0;
            c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(_ang14r, _ang14i, _cos18r, _cos18i, &_mul19r, &_mul19i);
            angle += _mul19r;
        }
        double _cos20r = 0, _cos20i = 0;
        c_cos(angle, 0, &_cos20r, &_cos20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(angle, 0, &_sin21r, &_sin21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin21r, _sin21i, _c22r, _c22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _cos20r + _mul23r; _add24i = _cos20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag, 0, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int j = 0; j < (int)(n); j++) {
        double _cf26r = 0, _cf26i = 0;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 0.05; _c28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = j + _c29r; _add30i = 0 + _c29i;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(_add30r, _add30i, _add30r, _add30i, &_pow32r, &_pow32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c28r, _c28i, _pow32r, _pow32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _c27r + _mul33r; _add34i = _c27i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_cf26r, _cf26i, _add34r, _add34i, &_mul35r, &_mul35i);
        double _cf36r = 0, _cf36i = 0;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { _cf36r = cRe[_idx]; _cf36i = cIm[_idx]; } }
        double _conj37r = 0, _conj37i = 0;
        _conj37r = _cf36r; _conj37i = -(_cf36i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.02; _c38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_conj37r, _conj37i, _c38r, _c38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul35r + _mul39r; _add40i = _mul35i + _mul39i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_468: too complex for auto-transpile, stubbed */
static void poly_468_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_469_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs4r + _abs5r; _add6i = _abs4i + _abs5i;
        double _add7r = 0, _add7i = 0;
        _add7r = _add6r + j; _add7i = _add6i + 0;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sin12r, _sin12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _c9r + _mul16r; _add17i = _c9i + _mul16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log8r, _log8i, _add17r, _add17i, &_mul18r, &_mul18i);
        double mag = _mul18r; /* +_mul18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_ang19r, _ang19i, j, &_pow20r, &_pow20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 4.0; _c22i = 0;
        double _mod23r = 0, _mod23i = 0;
        _mod23r = fmod(j, _c22r); _mod23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_ang21r, _ang21i, _mod23r, &_pow24r, &_pow24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _pow20r - _pow24r; _sub25i = _pow20i - _pow24i;
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x1r; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _sub25r + _sin28r; _add29i = _sub25i + _sin28i;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _add29r - _cos32r; _sub33i = _add29i - _cos32i;
        double angle = _sub33r; /* +_sub33ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
        for (int k = 1; k < 4; k++) {
            double _c40r = 0, _c40i = 0;
            _c40r = 1.0; _c40i = 0;
            double _c41r = 0, _c41i = 0;
            _c41r = 0.1; _c41i = 0;
            double _attr42r = 0, _attr42i = 0;
            _attr42r = x1r; _attr42i = 0;
            double _mul43r = 0, _mul43i = 0;
            c_mul(k, 0, _attr42r, _attr42i, &_mul43r, &_mul43i);
            double _c44r = 0, _c44i = 0;
            _c44r = 0.0; _c44i = 0;
            double _add45r = 0, _add45i = 0;
            _add45r = _mul43r + _c44r; _add45i = _mul43i + _c44i;
            double _sin46r = 0, _sin46i = 0;
            c_sin(_add45r, _add45i, &_sin46r, &_sin46i);
            double _mul47r = 0, _mul47i = 0;
            c_mul(_c41r, _c41i, _sin46r, _sin46i, &_mul47r, &_mul47i);
            double _add48r = 0, _add48i = 0;
            _add48r = _c40r + _mul47r; _add48i = _c40i + _mul47i;
            double _mul49r = 0, _mul49i = 0;
            c_mul(mag, 0, _add48r, _add48i, &_mul49r, &_mul49i);
            mag = _mul49r;
            double _c50r = 0, _c50i = 0;
            _c50r = 0.5; _c50i = 0;
            double _ang51r = 0, _ang51i = 0;
            _ang51r = c_arg(x1r, x1i); _ang51i = 0;
            double _pow52r = 0, _pow52i = 0;
            c_powr(_ang51r, _ang51i, k, &_pow52r, &_pow52i);
            double _mul53r = 0, _mul53i = 0;
            c_mul(_c50r, _c50i, _pow52r, _pow52i, &_mul53r, &_mul53i);
            double _add54r = 0, _add54i = 0;
            _add54r = angle + _mul53r; _add54i = 0 + _mul53i;
            double _c55r = 0, _c55i = 0;
            _c55r = 0.3; _c55i = 0;
            double _ang56r = 0, _ang56i = 0;
            _ang56r = c_arg(x2r, x2i); _ang56i = 0;
            double _pow57r = 0, _pow57i = 0;
            c_powr(_ang56r, _ang56i, k, &_pow57r, &_pow57i);
            double _mul58r = 0, _mul58i = 0;
            c_mul(_c55r, _c55i, _pow57r, _pow57i, &_mul58r, &_mul58i);
            double _sub59r = 0, _sub59i = 0;
            _sub59r = _add54r - _mul58r; _sub59i = _add54i - _mul58i;
            angle = _sub59r;
            double _cos60r = 0, _cos60i = 0;
            c_cos(angle, 0, &_cos60r, &_cos60i);
            double _c61r = 0, _c61i = 0;
            _c61r = 0.0; _c61i = 1.0;
            double _sin62r = 0, _sin62i = 0;
            c_sin(angle, 0, &_sin62r, &_sin62i);
            double _mul63r = 0, _mul63i = 0;
            c_mul(_c61r, _c61i, _sin62r, _sin62i, &_mul63r, &_mul63i);
            double _add64r = 0, _add64i = 0;
            _add64r = _cos60r + _mul63r; _add64i = _cos60i + _mul63i;
            double _mul65r = 0, _mul65i = 0;
            c_mul(mag, 0, _add64r, _add64i, &_mul65r, &_mul65i);
            cRe[(j - 1)] += _mul65r; cIm[(j - 1)] += _mul65i;
        }
        double _cf66r = 0, _cf66i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf66r = cRe[_idx]; _cf66i = cIm[_idx]; } }
        double _c67r = 0, _c67i = 0;
        _c67r = 1.0; _c67i = 0;
        double _c68r = 0, _c68i = 0;
        _c68r = 0.05; _c68i = 0;
        double _mul69r = 0, _mul69i = 0;
        c_mul(_c68r, _c68i, j, 0, &_mul69r, &_mul69i);
        double _add70r = 0, _add70i = 0;
        _add70r = _c67r + _mul69r; _add70i = _c67i + _mul69i;
        double _mul71r = 0, _mul71i = 0;
        c_mul(_cf66r, _cf66i, _add70r, _add70i, &_mul71r, &_mul71i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul71r; cIm[_idx] = _mul71i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_470_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int k = 1; k < (int)(_add3r); k++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(k - 1)]; _arr4i = 0;
        double r = _arr4r; /* +_arr4ii */
        double _arr5r = 0, _arr5i = 0;
        _arr5r = imc_seq[(k - 1)]; _arr5i = 0;
        double im = _arr5r; /* +_arr5ii */
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(r, 0); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 0.5; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_abs10r, _abs10i, 0.5, &_pow12r, &_pow12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log9r, _log9i, _pow12r, _pow12i, &_mul13r, &_mul13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(r, 0, k, 0, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = k + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(im, 0, _add17r, _add17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sin15r, _sin15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul13r + _mul20r; _add21i = _mul13i + _mul20i;
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _mod23r = 0, _mod23i = 0;
        _mod23r = fmod(k, _c22r); _mod23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _mod23r + _c24r; _add25i = _mod23i + _c24i;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x2r, x2i); _abs26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_add25r, _add25i, _abs26r, _abs26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _add21r + _mul27r; _add28i = _add21i + _mul27i;
        double mag = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(k, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(k, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 4.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang32r, _ang32i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul31r + _mul37r; _add38i = _mul31i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(im, 0, k, 0, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 2.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_mul39r, _mul39i, _c40r, _c40i, &_div41r, &_div41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_div41r, _div41i, &_sin42r, &_sin42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add38r + _sin42r; _add43i = _add38i + _sin42i;
        double angle = _add43r; /* +_add43ii */
        double _cos44r = 0, _cos44i = 0;
        c_cos(angle, 0, &_cos44r, &_cos44i);
        double _sin45r = 0, _sin45i = 0;
        c_sin(angle, 0, &_sin45r, &_sin45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_sin45r, _sin45i, _c46r, _c46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _cos44r + _mul47r; _add48i = _cos44i + _mul47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(mag, 0, _add48r, _add48i, &_mul49r, &_mul49i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_471_c(double x1r, double x1i, double x2r, double x2i,
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
        double _sin7r = 0, _sin7i = 0;
        c_sin(j, 0, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log6r, _log6i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _cos11r; _add12i = _mul8i + _cos11i;
        double mag_part = _add12r; /* +_add12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _sqrt14r = 0, _sqrt14i = 0;
        c_powr(j, 0, 0.5, &_sqrt14r, &_sqrt14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang13r, _ang13i, _sqrt14r, _sqrt14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2i; _attr16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _div19r = 0, _div19i = 0;
        c_div(_attr16r, _attr16i, _add18r, _add18i, &_div19r, &_div19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul15r + _div19r; _add20i = _mul15i + _div19i;
        double angle_part = _add20r; /* +_add20ii */
        double _cos21r = 0, _cos21i = 0;
        c_cos(angle_part, 0, &_cos21r, &_cos21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(angle_part, 0, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c22r, _c22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _cos21r + _mul24r; _add25i = _cos21i + _mul24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag_part, 0, _add25r, _add25i, &_mul26r, &_mul26i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    double _c27r = 0, _c27i = 0;
    _c27r = 1.0; _c27i = 0;
    double _add28r = 0, _add28i = 0;
    _add28r = n + _c27r; _add28i = 0 + _c27i;
    for (int k = 1; k < (int)(_add28r); k++) {
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x2r; _attr29i = 0;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x1i; _attr30i = 0;
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _attr29r - _attr30r; _sub31i = _attr29i - _attr30i;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = k + _c33r; _add34i = 0 + _c33i;
        double _log35r = 0, _log35i = 0;
        c_log(_add34r, _add34i, &_log35r, &_log35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c32r, _c32i, _log35r, _log35i, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_sub31r, _sub31i, _exp37r, _exp37i, &_mul38r, &_mul38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(k, 0, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_mul38r, _mul38i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        cRe[(k - 1)] += _mul40r; cIm[(k - 1)] += _mul40i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_472_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
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
        _c9r = 7.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _c7r + _sin11r; _add12i = _c7i + _sin11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log6r, _log6i, _add12r, _add12i, &_mul13r, &_mul13i);
        double mag = _mul13r; /* +_mul13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang14r, _ang14i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang20r, _ang20i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul19r + _mul25r; _add26i = _mul19i + _mul25i;
        double ang = _add26r; /* +_add26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, ang, 0, &_mul28r, &_mul28i);
        double _exp29r = 0, _exp29i = 0;
        c_exp2(_mul28r, _mul28i, &_exp29r, &_exp29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag, 0, _exp29r, _exp29i, &_mul30r, &_mul30i);
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x1r; _attr31i = 0;
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2r; _attr32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _attr31r + _attr32r; _add33i = _attr31i + _attr32i;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_add33r, _add33i, _add35r, _add35i, &_div36r, &_div36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul30r + _div36r; _add37i = _mul30i + _div36i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    }
    double _c38r = 0, _c38i = 0;
    _c38r = 1.0; _c38i = 0;
    double _add39r = 0, _add39i = 0;
    _add39r = n + _c38r; _add39i = 0 + _c38i;
    for (int k = 1; k < (int)(_add39r); k++) {
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x1r; _conj40i = -(x1i);
        double _pow41r = 0, _pow41i = 0;
        c_powr(_conj40r, _conj40i, k, &_pow41r, &_pow41i);
        double _conj42r = 0, _conj42i = 0;
        _conj42r = x2r; _conj42i = -(x2i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = n - k; _sub43i = 0 - 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = _sub43r + _c44r; _add45i = _sub43i + _c44i;
        double _pow46r = 0, _pow46i = 0;
        c_powr(_conj42r, _conj42i, _add45r, &_pow46r, &_pow46i);
        double _sub47r = 0, _sub47i = 0;
        _sub47r = _pow41r - _pow46r; _sub47i = _pow41i - _pow46i;
        cRe[(k - 1)] += _sub47r; cIm[(k - 1)] += _sub47i;
    }
    double _c48r = 0, _c48i = 0;
    _c48r = 1.0; _c48i = 0;
    double _add49r = 0, _add49i = 0;
    _add49r = n + _c48r; _add49i = 0 + _c48i;
    for (int r = 1; r < (int)(_add49r); r++) {
        double _c50r = 0, _c50i = 0;
        _c50r = 1.0; _c50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 0.1; _c51i = 0;
        double _ang52r = 0, _ang52i = 0;
        _ang52r = c_arg(x1r, x1i); _ang52i = 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(r, 0, _ang52r, _ang52i, &_mul53r, &_mul53i);
        double _cos54r = 0, _cos54i = 0;
        c_cos(_mul53r, _mul53i, &_cos54r, &_cos54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c51r, _c51i, _cos54r, _cos54i, &_mul55r, &_mul55i);
        double _ang56r = 0, _ang56i = 0;
        _ang56r = c_arg(x2r, x2i); _ang56i = 0;
        double _mul57r = 0, _mul57i = 0;
        c_mul(r, 0, _ang56r, _ang56i, &_mul57r, &_mul57i);
        double _sin58r = 0, _sin58i = 0;
        c_sin(_mul57r, _mul57i, &_sin58r, &_sin58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(_mul55r, _mul55i, _sin58r, _sin58i, &_mul59r, &_mul59i);
        double _add60r = 0, _add60i = 0;
        _add60r = _c50r + _mul59r; _add60i = _c50i + _mul59i;
        { double _tr = cRe[(r - 1)]*_add60r - cIm[(r - 1)]*_add60i; cIm[(r - 1)] = cRe[(r - 1)]*_add60i + cIm[(r - 1)]*_add60r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_473_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _arr5r = 0, _arr5i = 0;
        _arr5r = imc_seq[(j - 1)]; _arr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_arr4r, _arr4i, _arr5r, _arr5i, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mul6r + _c7r; _add8i = _mul6i + _c7i;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        _pow11r = _abs9r; _pow11i = _abs9i;
        double _log12r = 0, _log12i = 0;
        c_log(_pow11r, _pow11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 4.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 6.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin17r, _sin17i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _c13r + _mul22r; _add23i = _c13i + _mul22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log12r, _log12i, _add23r, _add23i, &_mul24r, &_mul24i);
        double mag = _mul24r; /* +_mul24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 3.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul29r + _mul34r; _add35i = _mul29i + _mul34i;
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(j, 0); _abs36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
        double _log39r = 0, _log39i = 0;
        c_log(_add38r, _add38i, &_log39r, &_log39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _add35r + _log39r; _add40i = _add35i + _log39i;
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
        c_mul(mag, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_474_c(double x1r, double x1i, double x2r, double x2i,
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
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double k = _pow5r; /* +_pow5ii */
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs6r + j; _add7i = _abs6i + 0;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log8r, _log8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double r = _mul12r; /* +_mul12ii */
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
        double _sub19r = 0, _sub19i = 0;
        _sub19r = _mul15r - _mul18r; _sub19i = _mul15i - _mul18i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x2r, x2i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _sub19r + _log23r; _add24i = _sub19i + _log23i;
        double angle = _add24r; /* +_add24ii */
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1r; _attr25i = 0;
        double _cos26r = 0, _cos26i = 0;
        c_cos(k, 0, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_attr25r, _attr25i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x2i; _attr28i = 0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(k, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_attr28r, _attr28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul27r + _mul30r; _add31i = _mul27i + _mul30i;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x1r, x1i); _abs32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _pow34r = 0, _pow34i = 0;
        c_mul(_abs32r, _abs32i, _abs32r, _abs32i, &_pow34r, &_pow34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = k + _c35r; _add36i = 0 + _c35i;
        double _div37r = 0, _div37i = 0;
        c_div(_pow34r, _pow34i, _add36r, _add36i, &_div37r, &_div37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_add31r, _add31i, _div37r, _div37i, &_mul38r, &_mul38i);
        double magnitude = _mul38r; /* +_mul38ii */
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
        c_mul(magnitude, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_475_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 40.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_powr(_attr4r, _attr4i, j, &_pow5r, &_pow5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs6r + j; _add7i = _abs6i + 0;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow5r + _log8r; _add9i = _pow5i + _log8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 3.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_c10r, _c10i, _c11r, _c11i, &_div12r, &_div12i);
        double _pow13r = 0, _pow13i = 0;
        c_powr(0, 0, _div12r, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add9r + _pow13r; _add14i = _add9i + _pow13i;
        double mag = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 6.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang15r, _ang15i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
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
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(j, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_conj32r, _conj32i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul31r + _mul36r; _add37i = _mul31i + _mul36i;
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x2r; _conj38i = -(x2i);
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(j, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj38r, _conj38i, _cos41r, _cos41i, &_mul42r, &_mul42i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = _add37r - _mul42r; _sub43i = _add37i - _mul42i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_476_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1r; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log6r, _log6i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double mag_part1 = _mul10r; /* +_mul10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 0.5; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs11r, _abs11i, 0.5, &_pow13r, &_pow13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 3.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(j, 0, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_pow13r, _pow13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double mag_part2 = _mul17r; /* +_mul17ii */
        double _add18r = 0, _add18i = 0;
        _add18r = mag_part1 + mag_part2; _add18i = 0 + 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_mul(j, 0, j, 0, &_pow20r, &_pow20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add18r + _pow20r; _add21i = _add18i + _pow20i;
        double magnitude = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 4.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang22r, _ang22i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double angle_part1 = _mul27r; /* +_mul27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang28r, _ang28i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double angle_part2 = _mul33r; /* +_mul33ii */
        double _add34r = 0, _add34i = 0;
        _add34r = angle_part1 + angle_part2; _add34i = 0 + 0;
        double _sin35r = 0, _sin35i = 0;
        c_sin(j, 0, &_sin35r, &_sin35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add34r + _sin35r; _add36i = _add34i + _sin35i;
        double angle = _add36r; /* +_add36ii */
        double _cos37r = 0, _cos37i = 0;
        c_cos(angle, 0, &_cos37r, &_cos37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(angle, 0, &_sin38r, &_sin38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_sin38r, _sin38i, _c39r, _c39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _cos37r + _mul40r; _add41i = _cos37i + _mul40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(magnitude, 0, _add41r, _add41i, &_mul42r, &_mul42i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_477_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 40.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2r; _attr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_attr5r, _attr5i, j, 0, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _attr4r + _mul6r; _add7i = _attr4i + _mul6i;
        double r = _add7r; /* +_add7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr9r, _attr9i, j, 0, &_mul10r, &_mul10i);
        double _sub11r = 0, _sub11i = 0;
        _sub11r = _attr8r - _mul10r; _sub11i = _attr8i - _mul10i;
        double i_part = _sub11r; /* +_sub11ii */
        double _sin12r = 0, _sin12i = 0;
        c_sin(r, 0, &_sin12r, &_sin12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(i_part, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin12r, _sin12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs15r + j; _add16i = _abs15i + 0;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul14r + _log17r; _add18i = _mul14i + _log17i;
        double phase = _add18r; /* +_add18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 0.5; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_abs19r, _abs19i, 0.5, &_pow21r, &_pow21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.3; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_abs22r, _abs22i, 0.3, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_pow21r, _pow21i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(j, 0, &_sin26r, &_sin26i);
        double _pow27r = 0, _pow27i = 0;
        c_powr(j, 0, _sin26r, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul25r, _mul25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _ang29r, _ang29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul28r + _cos31r; _add32i = _mul28i + _cos31i;
        double magnitude = _add32r; /* +_add32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c33r, _c33i, phase, 0, &_mul34r, &_mul34i);
        double _exp35r = 0, _exp35i = 0;
        c_exp2(_mul34r, _mul34i, &_exp35r, &_exp35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(magnitude, 0, _exp35r, _exp35i, &_mul36r, &_mul36i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_478_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_part1 = _log6r; /* +_log6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1r; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _attr10r + _c11r; _add12i = _attr10i + _c11i;
        double _div13r = 0, _div13i = 0;
        c_div(j, 0, _add12r, _add12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin9r, _sin9i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double mag_part2 = _mul15r; /* +_mul15ii */
        double _mul16r = 0, _mul16i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 0.5; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(0, 0, 0.5, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul16r + _pow18r; _add19i = _mul16i + _pow18i;
        double magnitude = _add19r; /* +_add19ii */
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
        double angle_part1 = _add26r; /* +_add26ii */
        double _attr27r = 0, _attr27i = 0;
        _attr27r = x1r; _attr27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, _attr27r, _attr27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _sin29r - _cos32r; _sub33i = _sin29i - _cos32i;
        double angle_part2 = _sub33r; /* +_sub33ii */
        double _add34r = 0, _add34i = 0;
        _add34r = angle_part1 + angle_part2; _add34i = 0 + 0;
        double angle = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, angle, 0, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(magnitude, 0, _exp37r, _exp37i, &_mul38r, &_mul38i);
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

static void poly_479_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(_arr4r, _arr4i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 4.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log8r, _log8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double mag_part1 = _mul13r; /* +_mul13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
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
        double mag_part2 = _pow19r; /* +_pow19ii */
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(mag_part2, 0, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = mag_part1 + _mul23r; _add24i = 0 + _mul23i;
        double mag = _add24r; /* +_add24ii */
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
        double angle_part1 = _mul29r; /* +_mul29ii */
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_mul31r, _mul31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sin32r + _cos36r; _add37i = _sin32i + _cos36i;
        double angle_part2 = _add37r; /* +_add37ii */
        double _add38r = 0, _add38i = 0;
        _add38r = angle_part1 + angle_part2; _add38i = 0 + 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_480_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1r; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log6r, _log6i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double mag_part1 = _mul10r; /* +_mul10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs11r + j; _add12i = _abs11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log13r, _log13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double mag_part2 = _mul17r; /* +_mul17ii */
        double _add18r = 0, _add18i = 0;
        _add18r = mag_part1 + mag_part2; _add18i = 0 + 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.5; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(j, 0, 0.5, &_pow20r, &_pow20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add18r + _pow20r; _add21i = _add18i + _pow20i;
        double magnitude = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _attr23r + _c24r; _add25i = _attr23i + _c24i;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _add25r, _add25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang22r, _ang22i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double angle_part1 = _mul28r; /* +_mul28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _attr30r + _c31r; _add32i = _attr30i + _c31i;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _add32r, _add32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang29r, _ang29i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double angle_part2 = _mul35r; /* +_mul35ii */
        double _sub36r = 0, _sub36i = 0;
        _sub36r = angle_part1 - angle_part2; _sub36i = 0 - 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, M_PI, 0, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 6.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _sub36r + _sin40r; _add41i = _sub36i + _sin40i;
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
        c_mul(magnitude, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_481_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 1.0;
        double _c5r = 0, _c5i = 0;
        _c5r = 5.0; _c5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_c5r, _c5i, M_PI, 0, &_mul6r, &_mul6i);
        double _arr7r = 0, _arr7i = 0;
        _arr7r = rec_seq[(j - 1)]; _arr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_mul6r, _mul6i, _arr7r, _arr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_c4r, _c4i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _exp11r = 0, _exp11i = 0;
        c_exp2(_mul10r, _mul10i, &_exp11r, &_exp11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1r; _attr12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _attr12r + j; _add13i = _attr12i + 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_exp11r, _exp11i, _add13r, _add13i, &_mul14r, &_mul14i);
        double term1 = _mul14r; /* +_mul14ii */
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 1.0;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, M_PI, 0, &_mul17r, &_mul17i);
        double _arr18r = 0, _arr18i = 0;
        _arr18r = imc_seq[(j - 1)]; _arr18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul17r, _mul17i, _arr18r, _arr18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c15r, _c15i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _exp22r = 0, _exp22i = 0;
        c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x2i; _attr23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_mul(j, 0, j, 0, &_pow25r, &_pow25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _attr23r + _pow25r; _add26i = _attr23i + _pow25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_exp22r, _exp22i, _add26r, _add26i, &_mul27r, &_mul27i);
        double term2 = _mul27r; /* +_mul27ii */
        double _add28r = 0, _add28i = 0;
        _add28r = term1 + term2; _add28i = 0 + 0;
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
        double _add35r = 0, _add35i = 0;
        _add35r = _add28r + _log34r; _add35i = _add28i + _log34i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    }
    double _c36r = 0, _c36i = 0;
    _c36r = 1.0; _c36i = 0;
    double _add37r = 0, _add37i = 0;
    _add37r = n + _c36r; _add37i = 0 + _c36i;
    for (int k = 1; k < (int)(_add37r); k++) {
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 0.05; _c39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, k, 0, &_mul40r, &_mul40i);
        double _cf41r = 0, _cf41i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
        double _attr42r = 0, _attr42i = 0;
        _attr42r = _cf41r; _attr42i = 0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(_attr42r, _attr42i, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_mul40r, _mul40i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _c38r + _mul44r; _add45i = _c38i + _mul44i;
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 0.05;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c46r, _c46i, k, 0, &_mul47r, &_mul47i);
        double _cf48r = 0, _cf48i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
        double _attr49r = 0, _attr49i = 0;
        _attr49r = _cf48i; _attr49i = 0;
        double _cos50r = 0, _cos50i = 0;
        c_cos(_attr49r, _attr49i, &_cos50r, &_cos50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_mul47r, _mul47i, _cos50r, _cos50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _add45r + _mul51r; _add52i = _add45i + _mul51i;
        { double _tr = cRe[(k - 1)]*_add52r - cIm[(k - 1)]*_add52i; cIm[(k - 1)] = cRe[(k - 1)]*_add52i + cIm[(k - 1)]*_add52r; cRe[(k - 1)] = _tr; }
    }
    double _c53r = 0, _c53i = 0;
    _c53r = 1.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = n + _c53r; _add54i = 0 + _c53i;
    for (int r = 1; r < (int)(_add54r); r++) {
        double _cf55r = 0, _cf55i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
        double _conj56r = 0, _conj56i = 0;
        _conj56r = _cf55r; _conj56i = -(_cf55i);
        double _attr57r = 0, _attr57i = 0;
        _attr57r = x1r; _attr57i = 0;
        double _mul58r = 0, _mul58i = 0;
        c_mul(r, 0, _attr57r, _attr57i, &_mul58r, &_mul58i);
        double _sin59r = 0, _sin59i = 0;
        c_sin(_mul58r, _mul58i, &_sin59r, &_sin59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_conj56r, _conj56i, _sin59r, _sin59i, &_mul60r, &_mul60i);
        double _attr61r = 0, _attr61i = 0;
        _attr61r = x2i; _attr61i = 0;
        double _mul62r = 0, _mul62i = 0;
        c_mul(r, 0, _attr61r, _attr61i, &_mul62r, &_mul62i);
        double _cos63r = 0, _cos63i = 0;
        c_cos(_mul62r, _mul62i, &_cos63r, &_cos63i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(_mul60r, _mul60i, _cos63r, _cos63i, &_mul64r, &_mul64i);
        cRe[(r - 1)] += _mul64r; cIm[(r - 1)] += _mul64i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_482_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_part1 = _log6r; /* +_log6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2r; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _attr10r + _c11r; _add12i = _attr10i + _c11i;
        double _div13r = 0, _div13i = 0;
        c_div(j, 0, _add12r, _add12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin9r, _sin9i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double mag_part2 = _mul15r; /* +_mul15ii */
        double _mul16r = 0, _mul16i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 0.5; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(0, 0, 0.5, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul16r + _pow18r; _add19i = _mul16i + _pow18i;
        double magnitude = _add19r; /* +_add19ii */
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
        double angle_part1 = _add26r; /* +_add26ii */
        double _attr27r = 0, _attr27i = 0;
        _attr27r = x1r; _attr27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, _attr27r, _attr27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _sin29r - _cos32r; _sub33i = _sin29i - _cos32i;
        double angle_part2 = _sub33r; /* +_sub33ii */
        double _add34r = 0, _add34i = 0;
        _add34r = angle_part1 + angle_part2; _add34i = 0 + 0;
        double angle = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, angle, 0, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(magnitude, 0, _exp37r, _exp37i, &_mul38r, &_mul38i);
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

static void poly_483_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double r1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double i1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2r; _attr4i = 0;
    double r2 = _attr4r; /* +_attr4ii */
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double i2 = _attr5r; /* +_attr5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _pow8r = 0, _pow8i = 0;
        c_powr(r1, 0, j, &_pow8r, &_pow8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_pow8r, _pow8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double part1 = _mul12r; /* +_mul12ii */
        double _sub13r = 0, _sub13i = 0;
        _sub13r = n - j; _sub13i = 0 - 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(i2, 0, _sub13r, &_pow14r, &_pow14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _abs15r, _abs15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_pow14r, _pow14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double part2 = _mul18r; /* +_mul18ii */
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
        double part3 = _log23r; /* +_log23ii */
        double _unk24r = 0, _unk24i = 0;
        /* WARNING: unhandled node List(elts=[BinOp(left=Name(id='r1', ctx=Load()), op=Add(), right=Name(id='j', ctx=Load())), BinOp(left=Name(id='i2', ctx=Load()), op=Add(), right=Name(id='j', ctx=Load())), Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='log', ctx=Load()), args=[BinOp(left=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='abs', ctx=Load()), args=[Name(id='t1', ctx=Load())]), op=Add(), right=Constant(value=1))])], ctx=Load()) */
        double part4 = _unk24r; /* +_unk24ii */
        double _mul25r = 0, _mul25i = 0;
        c_mul(part1, 0, part2, 0, &_mul25r, &_mul25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(part3, 0, part4, 0, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul25r + _mul26r; _add27i = _mul25i + _mul26i;
        double magnitude = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(j, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _cos32r = 0, _cos32i = 0;
        c_cos(j, 0, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul30r + _mul33r; _add34i = _mul30i + _mul33i;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(x1r, x1i); _abs35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _abs35r + _c36r; _add37i = _abs35i + _c36i;
        double _log38r = 0, _log38i = 0;
        c_log(_add37r, _add37i, &_log38r, &_log38i);
        double _div39r = 0, _div39i = 0;
        c_div(_log38r, _log38i, j, 0, &_div39r, &_div39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _add34r + _div39r; _add40i = _add34i + _div39i;
        double angle = _add40r; /* +_add40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(magnitude, 0, _exp43r, _exp43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_484_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_sin11r, _sin11i, _sin11r, _sin11i, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _pow13r; _add14i = _mul8i + _pow13i;
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _div19r = 0, _div19i = 0;
        c_div(_mul16r, _mul16i, _add18r, _add18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add14r + _cos20r; _add21i = _add14i + _cos20i;
        double mag = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, j, 0, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1r; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x2r; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul25r, _mul25i, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul23r + _sin28r; _add29i = _mul23i + _sin28i;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _add29r - _cos32r; _sub33i = _add29i - _cos32i;
        double angle = _sub33r; /* +_sub33ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_484_old_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_sin11r, _sin11i, _sin11r, _sin11i, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _pow13r; _add14i = _mul8i + _pow13i;
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _div19r = 0, _div19i = 0;
        c_div(_mul16r, _mul16i, _add18r, _add18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add14r + _cos20r; _add21i = _add14i + _cos20i;
        double mag = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, j, 0, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1r; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x2r; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul25r, _mul25i, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul23r + _sin28r; _add29i = _mul23i + _sin28i;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _add29r - _cos32r; _sub33i = _add29i - _cos32i;
        double angle = _sub33r; /* +_sub33ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_485_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, M_PI, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 4.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul4r, _mul4i, _c5r, _c5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 3.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_div10r, _div10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sin7r, _sin7i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _arr13r = 0, _arr13i = 0;
        _arr13r = rec_seq[(j - 1)]; _arr13i = 0;
        double _arr14r = 0, _arr14i = 0;
        _arr14r = imc_seq[(j - 1)]; _arr14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _arr13r + _arr14r; _add15i = _arr13i + _arr14i;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_add15r, _add15i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul12r + _log19r; _add20i = _mul12i + _log19i;
        double phase = _add20r; /* +_add20ii */
        double _arr21r = 0, _arr21i = 0;
        _arr21r = rec_seq[(j - 1)]; _arr21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(_arr21r, _arr21i, _arr21r, _arr21i, &_pow23r, &_pow23i);
        double _arr24r = 0, _arr24i = 0;
        _arr24r = imc_seq[(j - 1)]; _arr24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(_arr24r, _arr24i, _arr24r, _arr24i, &_pow26r, &_pow26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _pow23r + _pow26r; _add27i = _pow23i + _pow26i;
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
        double _pow33r = 0, _pow33i = 0;
        c_powr(_sqrt28r, _sqrt28i, _add32r, &_pow33r, &_pow33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(j, 0, &_sin34r, &_sin34i);
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(_sin34r, _sin34i); _abs35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_pow33r, _pow33i, _abs35r, _abs35i, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 2.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(j, 0, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _abs40r = 0, _abs40i = 0;
        _abs40r = c_abs(_cos39r, _cos39i); _abs40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _mul36r + _abs40r; _add41i = _mul36i + _abs40i;
        double magnitude = _add41r; /* +_add41ii */
        double _cos42r = 0, _cos42i = 0;
        c_cos(phase, 0, &_cos42r, &_cos42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _sin44r = 0, _sin44i = 0;
        c_sin(phase, 0, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c43r, _c43i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _cos42r + _mul45r; _add46i = _cos42i + _mul45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(magnitude, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_486_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs4r + _abs5r; _add6i = _abs4i + _abs5i;
        double _add7r = 0, _add7i = 0;
        _add7r = _add6r + j; _add7i = _add6i + 0;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _sin11r + _cos14r; _add15i = _sin11i + _cos14i;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_add15r, _add15i); _abs16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log8r, _log8i, _abs16r, _abs16i, &_mul17r, &_mul17i);
        double mag_real = _mul17r; /* +_mul17ii */
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
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1i; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_mul24r, _mul24i, &_sin25r, &_sin25i);
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x2i; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _sin25r - _cos28r; _sub29i = _sin25i - _cos28i;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(_sub29r, _sub29i); _abs30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log22r, _log22i, _abs30r, _abs30i, &_mul31r, &_mul31i);
        double mag_imag = _mul31r; /* +_mul31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, n, 0, &_div33r, &_div33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_div33r, _div33i, M_PI, 0, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(j, 0, n, 0, &_div38r, &_div38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_div38r, _div38i, M_PI, 0, &_mul39r, &_mul39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_mul39r, _mul39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang37r, _ang37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul36r + _mul41r; _add42i = _mul36i + _mul41i;
        double angle_real = _add42r; /* +_add42ii */
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x1r, x1i); _ang43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(j, 0, n, 0, &_div44r, &_div44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_div44r, _div44i, M_PI, 0, &_mul45r, &_mul45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(_mul45r, _mul45i, &_cos46r, &_cos46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_ang43r, _ang43i, _cos46r, _cos46i, &_mul47r, &_mul47i);
        double _ang48r = 0, _ang48i = 0;
        _ang48r = c_arg(x2r, x2i); _ang48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(j, 0, n, 0, &_div49r, &_div49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_div49r, _div49i, M_PI, 0, &_mul50r, &_mul50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(_mul50r, _mul50i, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_ang48r, _ang48i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _sub53r = 0, _sub53i = 0;
        _sub53r = _mul47r - _mul52r; _sub53i = _mul47i - _mul52i;
        double angle_imag = _sub53r; /* +_sub53ii */
        double _cos54r = 0, _cos54i = 0;
        c_cos(angle_real, 0, &_cos54r, &_cos54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(mag_real, 0, _cos54r, _cos54i, &_mul55r, &_mul55i);
        double _sin56r = 0, _sin56i = 0;
        c_sin(angle_imag, 0, &_sin56r, &_sin56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(mag_imag, 0, _sin56r, _sin56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _mul55r + _mul57r; _add58i = _mul55i + _mul57i;
        double _c59r = 0, _c59i = 0;
        _c59r = 0.0; _c59i = 1.0;
        double _sin60r = 0, _sin60i = 0;
        c_sin(angle_real, 0, &_sin60r, &_sin60i);
        double _mul61r = 0, _mul61i = 0;
        c_mul(mag_real, 0, _sin60r, _sin60i, &_mul61r, &_mul61i);
        double _cos62r = 0, _cos62i = 0;
        c_cos(angle_imag, 0, &_cos62r, &_cos62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(mag_imag, 0, _cos62r, _cos62i, &_mul63r, &_mul63i);
        double _sub64r = 0, _sub64i = 0;
        _sub64r = _mul61r - _mul63r; _sub64i = _mul61i - _mul63i;
        double _mul65r = 0, _mul65i = 0;
        c_mul(_c59r, _c59i, _sub64r, _sub64i, &_mul65r, &_mul65i);
        double _add66r = 0, _add66i = 0;
        _add66r = _add58r + _mul65r; _add66i = _add58i + _mul65i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add66r; cIm[_idx] = _add66i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_487_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
        static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(_arr4r, _arr4i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 4.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log8r, _log8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double mag_part1 = _mul13r; /* +_mul13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
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
        double mag_part2 = _pow19r; /* +_pow19ii */
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(mag_part2, 0, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = mag_part1 + _mul23r; _add24i = 0 + _mul23i;
        double mag = _add24r; /* +_add24ii */
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
        double angle_part1 = _mul29r; /* +_mul29ii */
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _attr30r, _attr30i, &_mul31r, &_mul31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_mul31r, _mul31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sin32r + _cos36r; _add37i = _sin32i + _cos36i;
        double angle_part2 = _add37r; /* +_add37ii */
        double _add38r = 0, _add38i = 0;
        _add38r = angle_part1 + angle_part2; _add38i = 0 + 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 40) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    double _c45r = 0, _c45i = 0;
    _c45r = 1.0; _c45i = 0;
    double _add46r = 0, _add46i = 0;
    _add46r = n + _c45r; _add46i = 0 + _c45i;
    for (int k = 1; k < (int)(_add46r); k++) {
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        if ((k > _c47r) && (k < n)) {
            double _c48r = 0, _c48i = 0;
            _c48r = 0.5; _c48i = 0;
            double _cf49r = 0, _cf49i = 0;
            { int _idx = (k - 2); if (_idx >= 0 && _idx < 40) { _cf49r = cRe[_idx]; _cf49i = cIm[_idx]; } }
            double _cf50r = 0, _cf50i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 40) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
            double _conj51r = 0, _conj51i = 0;
            _conj51r = _cf50r; _conj51i = -(_cf50i);
            double _mul52r = 0, _mul52i = 0;
            c_mul(_cf49r, _cf49i, _conj51r, _conj51i, &_mul52r, &_mul52i);
            double _mul53r = 0, _mul53i = 0;
            c_mul(_c48r, _c48i, _mul52r, _mul52i, &_mul53r, &_mul53i);
            double _mul54r = 0, _mul54i = 0;
            c_mul(k, 0, M_PI, 0, &_mul54r, &_mul54i);
            double _div55r = 0, _div55i = 0;
            c_div(_mul54r, _mul54i, n, 0, &_div55r, &_div55i);
            double _cos56r = 0, _cos56i = 0;
            c_cos(_div55r, _div55i, &_cos56r, &_cos56i);
            double _mul57r = 0, _mul57i = 0;
            c_mul(_mul53r, _mul53i, _cos56r, _cos56i, &_mul57r, &_mul57i);
            cRe[(k - 1)] += _mul57r; cIm[(k - 1)] += _mul57i;
        } else {
            double _c58r = 0, _c58i = 0;
            _c58r = 1.0; _c58i = 0;
            if (k == _c58r) {
                double _c59r = 0, _c59i = 0;
                _c59r = 0.3; _c59i = 0;
                double _cf60r = 0, _cf60i = 0;
                { int _idx = k; if (_idx >= 0 && _idx < 40) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
                double _conj61r = 0, _conj61i = 0;
                _conj61r = _cf60r; _conj61i = -(_cf60i);
                double _mul62r = 0, _mul62i = 0;
                c_mul(_c59r, _c59i, _conj61r, _conj61i, &_mul62r, &_mul62i);
                double _mul63r = 0, _mul63i = 0;
                c_mul(k, 0, M_PI, 0, &_mul63r, &_mul63i);
                double _div64r = 0, _div64i = 0;
                c_div(_mul63r, _mul63i, n, 0, &_div64r, &_div64i);
                double _sin65r = 0, _sin65i = 0;
                c_sin(_div64r, _div64i, &_sin65r, &_sin65i);
                double _mul66r = 0, _mul66i = 0;
                c_mul(_mul62r, _mul62i, _sin65r, _sin65i, &_mul66r, &_mul66i);
                cRe[(k - 1)] += _mul66r; cIm[(k - 1)] += _mul66i;
            } else {
                double _c67r = 0, _c67i = 0;
                _c67r = 0.3; _c67i = 0;
                double _cf68r = 0, _cf68i = 0;
                { int _idx = (k - 2); if (_idx >= 0 && _idx < 40) { _cf68r = cRe[_idx]; _cf68i = cIm[_idx]; } }
                double _conj69r = 0, _conj69i = 0;
                _conj69r = _cf68r; _conj69i = -(_cf68i);
                double _mul70r = 0, _mul70i = 0;
                c_mul(_c67r, _c67i, _conj69r, _conj69i, &_mul70r, &_mul70i);
                double _mul71r = 0, _mul71i = 0;
                c_mul(k, 0, M_PI, 0, &_mul71r, &_mul71i);
                double _div72r = 0, _div72i = 0;
                c_div(_mul71r, _mul71i, n, 0, &_div72r, &_div72i);
                double _sin73r = 0, _sin73i = 0;
                c_sin(_div72r, _div72i, &_sin73r, &_sin73i);
                double _mul74r = 0, _mul74i = 0;
                c_mul(_mul70r, _mul70i, _sin73r, _sin73i, &_mul74r, &_mul74i);
                cRe[(k - 1)] += _mul74r; cIm[(k - 1)] += _mul74i;
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
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double r1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double i1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2r; _attr4i = 0;
    double r2 = _attr4r; /* +_attr4ii */
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double i2 = _attr5r; /* +_attr5ii */
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
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_sin13r, _sin13i); _abs14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log10r, _log10i, _abs14r, _abs14i, &_mul15r, &_mul15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs16r + j; _add17i = _abs16i + 0;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x1i; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _attr19r, _attr19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_mul20r, _mul20i, _add22r, _add22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_cos24r, _cos24i); _abs25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log18r, _log18i, _abs25r, _abs25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul15r + _mul26r; _add27i = _mul15i + _mul26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(j, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _cos32r = 0, _cos32i = 0;
        c_cos(j, 0, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul30r + _mul33r; _add34i = _mul30i + _mul33i;
        double angle_part1 = _add34r; /* +_add34ii */
        double _attr35r = 0, _attr35i = 0;
        _attr35r = x1r; _attr35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, _attr35r, _attr35i, &_mul36r, &_mul36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x2i; _attr38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, _attr38r, _attr38i, &_mul39r, &_mul39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_mul39r, _mul39i, &_cos40r, &_cos40i);
        double _sub41r = 0, _sub41i = 0;
        _sub41r = _sin37r - _cos40r; _sub41i = _sin37i - _cos40i;
        double angle_part2 = _sub41r; /* +_sub41ii */
        double _add42r = 0, _add42i = 0;
        _add42r = angle_part1 + angle_part2; _add42i = 0 + 0;
        double angle = _add42r; /* +_add42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, angle, 0, &_mul44r, &_mul44i);
        double _exp45r = 0, _exp45i = 0;
        c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(mag, 0, _exp45r, _exp45i, &_mul46r, &_mul46i);
        double _conj47r = 0, _conj47i = 0;
        _conj47r = x1r; _conj47i = -(x1i);
        double _sin48r = 0, _sin48i = 0;
        c_sin(j, 0, &_sin48r, &_sin48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_conj47r, _conj47i, _sin48r, _sin48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul46r + _mul49r; _add50i = _mul46i + _mul49i;
        double _conj51r = 0, _conj51i = 0;
        _conj51r = x2r; _conj51i = -(x2i);
        double _cos52r = 0, _cos52i = 0;
        c_cos(j, 0, &_cos52r, &_cos52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_conj51r, _conj51i, _cos52r, _cos52i, &_mul53r, &_mul53i);
        double _sub54r = 0, _sub54i = 0;
        _sub54r = _add50r - _mul53r; _sub54i = _add50i - _mul53i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub54r; cIm[_idx] = _sub54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_489_c(double x1r, double x1i, double x2r, double x2i,
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
        double sum_mag = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        for (int k = 1; k < (int)(_add6r); k++) {
            double _attr7r = 0, _attr7i = 0;
            _attr7r = x1r; _attr7i = 0;
            double _pow8r = 0, _pow8i = 0;
            c_powr(_attr7r, _attr7i, k, &_pow8r, &_pow8i);
            double _ang9r = 0, _ang9i = 0;
            _ang9r = c_arg(x1r, x1i); _ang9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_pow8r, _pow8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            sum_mag += _mul12r;
        }
        double _sub13r = 0, _sub13i = 0;
        _sub13r = n - j; _sub13i = 0 - 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _sub13r + _c14r; _add15i = _sub13i + _c14i;
        for (int r = 1; r < (int)(_add15r); r++) {
            double _attr16r = 0, _attr16i = 0;
            _attr16r = x2i; _attr16i = 0;
            double _pow17r = 0, _pow17i = 0;
            c_powr(_attr16r, _attr16i, r, &_pow17r, &_pow17i);
            double _ang18r = 0, _ang18i = 0;
            _ang18r = c_arg(x2r, x2i); _ang18i = 0;
            double _mul19r = 0, _mul19i = 0;
            c_mul(r, 0, _ang18r, _ang18i, &_mul19r, &_mul19i);
            double _cos20r = 0, _cos20i = 0;
            c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(_pow17r, _pow17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
            sum_mag += _mul21r;
        }
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = sum_mag + _c22r; _add23i = 0 + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double mag = _log24r; /* +_log24ii */
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _div27r = 0, _div27i = 0;
        c_div(sum_mag, 0, _add26r, _add26i, &_div27r, &_div27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = n - j; _sub28i = 0 - 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _sub28r + _c29r; _add30i = _sub28i + _c29i;
        double _div31r = 0, _div31i = 0;
        c_div(sum_mag, 0, _add30r, _add30i, &_div31r, &_div31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _div27r + _div31r; _add32i = _div27i + _div31i;
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
        c_mul(mag, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_490_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 5.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(j, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double _pow9r = 0, _pow9i = 0;
        c_powr(_attr4r, _attr4i, _add8r, &_pow9r, &_pow9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(j, 0, _c11r, _c11i, &_div12r, &_div12i);
        double _flr13r = 0, _flr13i = 0;
        _flr13r = floor(_div12r); _flr13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _flr13r + _c14r; _add15i = _flr13i + _c14i;
        double _pow16r = 0, _pow16i = 0;
        c_powr(_abs10r, _abs10i, _add15r, &_pow16r, &_pow16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _pow9r + _pow16r; _add17i = _pow9i + _pow16i;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 4.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log20r, _log20i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _add17r + _mul25r; _add26i = _add17i + _mul25i;
        double mag = _add26r; /* +_add26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 6.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang27r, _ang27i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 8.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang33r, _ang33i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul32r + _mul38r; _add39i = _mul32i + _mul38i;
        double angle = _add39r; /* +_add39ii */
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, angle, 0, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(mag, 0, _exp42r, _exp42i, &_mul43r, &_mul43i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    double _c44r = 0, _c44i = 0;
    _c44r = 1.0; _c44i = 0;
    double _add45r = 0, _add45i = 0;
    _add45r = n + _c44r; _add45i = 0 + _c44i;
    for (int k = 1; k < (int)(_add45r); k++) {
        double _conj46r = 0, _conj46i = 0;
        _conj46r = x1r; _conj46i = -(x1i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(k, 0, M_PI, 0, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 5.0; _c48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(_mul47r, _mul47i, _c48r, _c48i, &_div49r, &_div49i);
        double _cos50r = 0, _cos50i = 0;
        c_cos(_div49r, _div49i, &_cos50r, &_cos50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_conj46r, _conj46i, _cos50r, _cos50i, &_mul51r, &_mul51i);
        double _conj52r = 0, _conj52i = 0;
        _conj52r = x2r; _conj52i = -(x2i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(k, 0, M_PI, 0, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 3.0; _c54i = 0;
        double _div55r = 0, _div55i = 0;
        c_div(_mul53r, _mul53i, _c54r, _c54i, &_div55r, &_div55i);
        double _sin56r = 0, _sin56i = 0;
        c_sin(_div55r, _div55i, &_sin56r, &_sin56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_conj52r, _conj52i, _sin56r, _sin56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _mul51r + _mul57r; _add58i = _mul51i + _mul57i;
        cRe[(k - 1)] += _add58r; cIm[(k - 1)] += _add58i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_491_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double r = _arr4r; /* +_arr4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1i; _attr5i = 0;
        double _sin6r = 0, _sin6i = 0;
        c_sin(j, 0, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr5r, _attr5i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _cos9r = 0, _cos9i = 0;
        c_cos(j, 0, &_cos9r, &_cos9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr8r, _attr8i, _cos9r, _cos9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _mul10r; _add11i = _mul7i + _mul10i;
        double i_part = _add11r; /* +_add11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _abs13r; _add14i = _abs12i + _abs13i;
        double _add15r = 0, _add15i = 0;
        _add15r = _add14r + j; _add15i = _add14i + 0;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.5; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(j, 0, 1.5, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _pow18r + 0; _add19i = _pow18i + 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log16r, _log16i, _add19r, _add19i, &_mul20r, &_mul20i);
        double magnitude = _mul20r; /* +_mul20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _div23r = 0, _div23i = 0;
        c_div(_mul22r, _mul22i, n, 0, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _div28r = 0, _div28i = 0;
        c_div(_mul27r, _mul27i, n, 0, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_492_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 7.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_attr4r, _attr4i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2r; _attr10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _c11r; _add12i = 0 + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr10r, _attr10i, _log13r, _log13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul9r + _mul14r; _add15i = _mul9i + _mul14i;
        double real_part = _add15r; /* +_add15ii */
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1i; _attr16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_mul(j, 0, j, 0, &_pow18r, &_pow18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_pow18r, _pow18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_attr16r, _attr16i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x2i; _attr23i = 0;
        double _neg24r = 0, _neg24i = 0;
        _neg24r = -(j); _neg24i = -(0);
        double _c25r = 0, _c25i = 0;
        _c25r = 10.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_neg24r, _neg24i, _c25r, _c25i, &_div26r, &_div26i);
        double _exp27r = 0, _exp27i = 0;
        c_exp2(_div26r, _div26i, &_exp27r, &_exp27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_attr23r, _attr23i, _exp27r, _exp27i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul22r - _mul28r; _sub29i = _mul22i - _mul28i;
        double imag_part = _sub29r; /* +_sub29ii */
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x2r, x2i); _abs31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _abs30r + _abs31r; _add32i = _abs30i + _abs31i;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.5; _c33i = 0;
        double _pow34r = 0, _pow34i = 0;
        c_powr(j, 0, 1.5, &_pow34r, &_pow34i);
        double _sub35r = 0, _sub35i = 0;
        _sub35r = n - j; _sub35i = 0 - 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.2; _c36i = 0;
        double _pow37r = 0, _pow37i = 0;
        c_powr(_sub35r, _sub35i, 1.2, &_pow37r, &_pow37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _pow34r + _pow37r; _add38i = _pow34i + _pow37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_add32r, _add32i, _add38r, _add38i, &_mul39r, &_mul39i);
        double magnitude = _mul39r; /* +_mul39ii */
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(x1r, x1i); _ang40i = 0;
        double _sqrt41r = 0, _sqrt41i = 0;
        c_powr(j, 0, 0.5, &_sqrt41r, &_sqrt41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang40r, _ang40i, _sqrt41r, _sqrt41i, &_mul42r, &_mul42i);
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x2r, x2i); _ang43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(j, 0, M_PI, 0, &_mul44r, &_mul44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 3.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(_mul44r, _mul44i, _c45r, _c45i, &_div46r, &_div46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(_div46r, _div46i, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang43r, _ang43i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul42r + _mul48r; _add49i = _mul42i + _mul48i;
        double angle = _add49r; /* +_add49ii */
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c50r, _c50i, imag_part, 0, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = real_part + _mul51r; _add52i = 0 + _mul51i;
        double _cos53r = 0, _cos53i = 0;
        c_cos(angle, 0, &_cos53r, &_cos53i);
        double _sin54r = 0, _sin54i = 0;
        c_sin(angle, 0, &_sin54r, &_sin54i);
        double _c55r = 0, _c55i = 0;
        _c55r = 0.0; _c55i = 1.0;
        double _mul56r = 0, _mul56i = 0;
        c_mul(_sin54r, _sin54i, _c55r, _c55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _cos53r + _mul56r; _add57i = _cos53i + _mul56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(_add52r, _add52i, _add57r, _add57i, &_mul58r, &_mul58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(_mul58r, _mul58i, magnitude, 0, &_mul59r, &_mul59i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_493_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 40.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs4r + _abs5r; _add6i = _abs4i + _abs5i;
        double _add7r = 0, _add7i = 0;
        _add7r = _add6r + j; _add7i = _add6i + 0;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _c9r + _sin10r; _add11i = _c9i + _sin10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(j, 0, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _add11r + _cos14r; _add15i = _add11i + _cos14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log8r, _log8i, _add15r, _add15i, &_mul16r, &_mul16i);
        double mag = _mul16r; /* +_mul16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang17r, _ang17i, j, 0, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(j, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_div21r, _div21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang19r, _ang19i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul18r + _mul23r; _add24i = _mul18i + _mul23i;
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
        c_mul(mag, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_494_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr4r, _attr4i, j, 0, &_mul5r, &_mul5i);
        double rec = _mul5r; /* +_mul5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_attr6r, _attr6i, j, 0, &_div7r, &_div7i);
        double imc = _div7r; /* +_div7ii */
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs8r + j; _add9i = _abs8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 7.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c11r + _sin15r; _add16i = _c11i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log10r, _log10i, _add16r, _add16i, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _c18r + _cos22r; _add23i = _c18i + _cos22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul17r, _mul17i, _add23r, _add23i, &_mul24r, &_mul24i);
        double mag = _mul24r; /* +_mul24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 3.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 4.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul29r + _mul34r; _add35i = _mul29i + _mul34i;
        double _c36r = 0, _c36i = 0;
        _c36r = 2.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add35r + _sin38r; _add39i = _add35i + _sin38i;
        double angle = _add39r; /* +_add39ii */
        double _cos40r = 0, _cos40i = 0;
        c_cos(angle, 0, &_cos40r, &_cos40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _sin42r = 0, _sin42i = 0;
        c_sin(angle, 0, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c41r, _c41i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _cos40r + _mul43r; _add44i = _cos40i + _mul43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag, 0, _add44r, _add44i, &_mul45r, &_mul45i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_495_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double rec1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double imc1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2r; _attr4i = 0;
    double rec2 = _attr4r; /* +_attr4ii */
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc2 = _attr5r; /* +_attr5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _mul8r = 0, _mul8i = 0;
        c_mul(rec1, 0, j, 0, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.2; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_powr(j, 0, 1.2, &_pow11r, &_pow11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(imc2, 0, _pow11r, _pow11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin9r + _cos13r; _add14i = _sin9i + _cos13i;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _c15r + j; _add16i = _c15i + 0;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_add14r, _add14i, _log17r, _log17i, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 0.5; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_abs19r, _abs19i, 0.5, &_pow21r, &_pow21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.3; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_abs22r, _abs22i, 0.3, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_pow21r, _pow21i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul18r + _mul25r; _add26i = _mul18i + _mul25i;
        double mag = _add26r; /* +_add26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(j, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang27r, _ang27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(j, 0, j, 0, &_pow32r, &_pow32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_pow32r, _pow32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul29r + _mul34r; _add35i = _mul29i + _mul34i;
        double ang = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, ang, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_496_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul5r = 0, _mul5i = 0;
        c_mul(_abs4r, _abs4i, j, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mul5r + _c6r; _add7i = _mul5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_mul9r, _mul9i, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _div14r = 0, _div14i = 0;
        c_div(_mul11r, _mul11i, _add13r, _add13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log8r, _log8i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _sub18r = 0, _sub18i = 0;
        _sub18r = n - j; _sub18i = 0 - 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _sub18r + _c19r; _add20i = _sub18i + _c19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_abs17r, _abs17i, _add20r, _add20i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _mul21r + _c22r; _add23i = _mul21i + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x1i; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul25r, _mul25i, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_mul27r, _mul27i, _add29r, _add29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_log24r, _log24i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul16r + _mul32r; _add33i = _mul16i + _mul32i;
        double mag = _add33r; /* +_add33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, _c35r, _c35i, &_div36r, &_div36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang34r, _ang34i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x2r, x2i); _ang39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 3.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(j, 0, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang39r, _ang39i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul38r + _mul43r; _add44i = _mul38i + _mul43i;
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
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _sin51r = 0, _sin51i = 0;
        c_sin(ang, 0, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c50r, _c50i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _cos49r + _mul52r; _add53i = _cos49i + _mul52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(mag, 0, _add53r, _add53i, &_mul54r, &_mul54i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_497_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    static const double rec_seq[] = {0}; /* WARNING: could not evaluate */
    static const double imc_seq[] = {0}; /* WARNING: could not evaluate */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _arr4r = 0, _arr4i = 0;
        _arr4r = rec_seq[(j - 1)]; _arr4i = 0;
        double _arr5r = 0, _arr5i = 0;
        _arr5r = imc_seq[(j - 1)]; _arr5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _arr4r + _arr5r; _add6i = _arr4i + _arr5i;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_add6r, _add6i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double mag_part1 = _log10r; /* +_log10ii */
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 6.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 4.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sin15r, _sin15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _c11r + _mul20r; _add21i = _c11i + _mul20i;
        double mag_part2 = _add21r; /* +_add21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _unk24r = 0, _unk24i = 0;
        /* WARNING: unhandled node List(elts=[Name(id='j', ctx=Load()), Subscript(value=Name(id='rec_seq', ctx=Load()), slice=BinOp(left=Name(id='j', ctx=Load()), op=Sub(), right=Constant(value=1)), ctx=Load()), Subscript(value=Name(id='imc_seq', ctx=Load()), slice=BinOp(left=Name(id='j', ctx=Load()), op=Sub(), right=Constant(value=1)), ctx=Load())], ctx=Load()) */
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 3.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_c25r, _c25i, _c26r, _c26i, &_div27r, &_div27i);
        double _pow28r = 0, _pow28i = 0;
        c_powr(_unk24r, _unk24i, _div27r, &_pow28r, &_pow28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _c23r + _pow28r; _add29i = _c23i + _pow28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul22r, _mul22i, _add29r, _add29i, &_mul30r, &_mul30i);
        double magnitude = _mul30r; /* +_mul30ii */
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
        double ang_part1 = _add37r; /* +_add37ii */
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 5.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, M_PI, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 7.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_sin41r, _sin41i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double ang_part2 = _mul46r; /* +_mul46ii */
        double _add47r = 0, _add47i = 0;
        _add47r = ang_part1 + ang_part2; _add47i = 0 + 0;
        double angle = _add47r; /* +_add47ii */
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c48r, _c48i, angle, 0, &_mul49r, &_mul49i);
        double _exp50r = 0, _exp50i = 0;
        c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(magnitude, 0, _exp50r, _exp50i, &_mul51r, &_mul51i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
    }
    double _c52r = 0, _c52i = 0;
    _c52r = 1.0; _c52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = n + _c52r; _add53i = 0 + _c52i;
    for (int k = 1; k < (int)(_add53r); k++) {
        double _c54r = 0, _c54i = 0;
        _c54r = 1.0; _c54i = 0;
        if ((k > _c54r) && (k < n)) {
            double _c55r = 0, _c55i = 0;
            _c55r = 0.5; _c55i = 0;
            double _cf56r = 0, _cf56i = 0;
            { int _idx = (k - 2); if (_idx >= 0 && _idx < 36) { _cf56r = cRe[_idx]; _cf56i = cIm[_idx]; } }
            double _cf57r = 0, _cf57i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
            double _conj58r = 0, _conj58i = 0;
            _conj58r = _cf57r; _conj58i = -(_cf57i);
            double _mul59r = 0, _mul59i = 0;
            c_mul(_cf56r, _cf56i, _conj58r, _conj58i, &_mul59r, &_mul59i);
            double _mul60r = 0, _mul60i = 0;
            c_mul(_c55r, _c55i, _mul59r, _mul59i, &_mul60r, &_mul60i);
            double _mul61r = 0, _mul61i = 0;
            c_mul(k, 0, M_PI, 0, &_mul61r, &_mul61i);
            double _div62r = 0, _div62i = 0;
            c_div(_mul61r, _mul61i, n, 0, &_div62r, &_div62i);
            double _cos63r = 0, _cos63i = 0;
            c_cos(_div62r, _div62i, &_cos63r, &_cos63i);
            double _mul64r = 0, _mul64i = 0;
            c_mul(_mul60r, _mul60i, _cos63r, _cos63i, &_mul64r, &_mul64i);
            cRe[(k - 1)] += _mul64r; cIm[(k - 1)] += _mul64i;
        } else {
            double _c65r = 0, _c65i = 0;
            _c65r = 1.0; _c65i = 0;
            if (k == _c65r) {
                double _c66r = 0, _c66i = 0;
                _c66r = 0.3; _c66i = 0;
                double _cf67r = 0, _cf67i = 0;
                { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
                double _conj68r = 0, _conj68i = 0;
                _conj68r = _cf67r; _conj68i = -(_cf67i);
                double _mul69r = 0, _mul69i = 0;
                c_mul(_c66r, _c66i, _conj68r, _conj68i, &_mul69r, &_mul69i);
                double _mul70r = 0, _mul70i = 0;
                c_mul(k, 0, M_PI, 0, &_mul70r, &_mul70i);
                double _div71r = 0, _div71i = 0;
                c_div(_mul70r, _mul70i, n, 0, &_div71r, &_div71i);
                double _sin72r = 0, _sin72i = 0;
                c_sin(_div71r, _div71i, &_sin72r, &_sin72i);
                double _mul73r = 0, _mul73i = 0;
                c_mul(_mul69r, _mul69i, _sin72r, _sin72i, &_mul73r, &_mul73i);
                cRe[(k - 1)] += _mul73r; cIm[(k - 1)] += _mul73i;
            } else {
                double _c74r = 0, _c74i = 0;
                _c74r = 0.3; _c74i = 0;
                double _cf75r = 0, _cf75i = 0;
                { int _idx = (k - 2); if (_idx >= 0 && _idx < 36) { _cf75r = cRe[_idx]; _cf75i = cIm[_idx]; } }
                double _conj76r = 0, _conj76i = 0;
                _conj76r = _cf75r; _conj76i = -(_cf75i);
                double _mul77r = 0, _mul77i = 0;
                c_mul(_c74r, _c74i, _conj76r, _conj76i, &_mul77r, &_mul77i);
                double _mul78r = 0, _mul78i = 0;
                c_mul(k, 0, M_PI, 0, &_mul78r, &_mul78i);
                double _div79r = 0, _div79i = 0;
                c_div(_mul78r, _mul78i, n, 0, &_div79r, &_div79i);
                double _sin80r = 0, _sin80i = 0;
                c_sin(_div79r, _div79i, &_sin80r, &_sin80i);
                double _mul81r = 0, _mul81i = 0;
                c_mul(_mul77r, _mul77i, _sin80r, _sin80i, &_mul81r, &_mul81i);
                cRe[(k - 1)] += _mul81r; cIm[(k - 1)] += _mul81i;
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
        double _mul5r = 0, _mul5i = 0;
        c_mul(_abs4r, _abs4i, j, 0, &_mul5r, &_mul5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _div9r = 0, _div9i = 0;
        c_div(_abs6r, _abs6i, _add8r, _add8i, &_div9r, &_div9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul5r + _div9r; _add10i = _mul5i + _div9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _add10r + _c11r; _add12i = _add10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double magnitude = _log13r; /* +_log13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _sin15r = 0, _sin15i = 0;
        c_sin(j, 0, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang14r, _ang14i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _cos18r = 0, _cos18i = 0;
        c_cos(j, 0, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang17r, _ang17i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul16r + _mul19r; _add20i = _mul16i + _mul19i;
        double angle = _add20r; /* +_add20ii */
        double _cos21r = 0, _cos21i = 0;
        c_cos(angle, 0, &_cos21r, &_cos21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(angle, 0, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c22r, _c22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _cos21r + _mul24r; _add25i = _cos21i + _mul24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(magnitude, 0, _add25r, _add25i, &_mul26r, &_mul26i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_499_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 1.5; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(j, 0, 1.5, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _abs4r + _pow6r; _add7i = _abs4i + _pow6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 6.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log8r, _log8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double mag_part1 = _mul13r; /* +_mul13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = j + _c15r; _add16i = 0 + _c15i;
        double _div17r = 0, _div17i = 0;
        c_div(_abs14r, _abs14i, _add16r, _add16i, &_div17r, &_div17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 4.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _div17r + _cos21r; _add22i = _div17i + _cos21i;
        double mag_part2 = _add22r; /* +_add22ii */
        double _neg23r = 0, _neg23i = 0;
        _neg23r = -(j); _neg23i = -(0);
        double _c24r = 0, _c24i = 0;
        _c24r = 10.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_neg23r, _neg23i, _c24r, _c24i, &_div25r, &_div25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_div25r, _div25i, &_exp26r, &_exp26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag_part2, 0, _exp26r, _exp26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = mag_part1 + _mul27r; _add28i = 0 + _mul27i;
        double magnitude = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 3.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double angle_part1 = _mul33r; /* +_mul33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 5.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, _c35r, _c35i, &_div36r, &_div36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang34r, _ang34i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 2.0; _c39i = 0;
        double _pow40r = 0, _pow40i = 0;
        c_mul(j, 0, j, 0, &_pow40r, &_pow40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 7.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(_pow40r, _pow40i, _c41r, _c41i, &_div42r, &_div42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_div42r, _div42i, &_sin43r, &_sin43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul38r + _sin43r; _add44i = _mul38i + _sin43i;
        double angle_part2 = _add44r; /* +_add44ii */
        double _add45r = 0, _add45i = 0;
        _add45r = angle_part1 + angle_part2; _add45i = 0 + 0;
        double angle = _add45r; /* +_add45ii */
        double _cos46r = 0, _cos46i = 0;
        c_cos(angle, 0, &_cos46r, &_cos46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _sin48r = 0, _sin48i = 0;
        c_sin(angle, 0, &_sin48r, &_sin48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c47r, _c47i, _sin48r, _sin48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _cos46r + _mul49r; _add50i = _cos46i + _mul49i;
        double _mul51r = 0, _mul51i = 0;
        c_mul(magnitude, 0, _add50r, _add50i, &_mul51r, &_mul51i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_500_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul10r = 0, _mul10i = 0;
        c_mul(_sin6r, _sin6i, _cos9r, _cos9i, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(j, 0, j, 0, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = x1r + x2r; _add13i = x1i + x2i;
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(_add13r, _add13i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_pow12r, _pow12i, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul10r + _sin16r; _add17i = _mul10i + _sin16i;
        double angle = _add17r; /* +_add17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_abs18r, _abs18i, j, &_pow19r, &_pow19i);
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x2r, x2i); _abs20i = 0;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = n - j; _sub21i = 0 - 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_abs20r, _abs20i, _sub21r, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _pow19r + _pow22r; _add23i = _pow19i + _pow22i;
        double _sub24r = 0, _sub24i = 0;
        _sub24r = x1r - x2r; _sub24i = x1i - x2i;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_sub24r, _sub24i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _abs25r; _add26i = 0 + _abs25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_add23r, _add23i, _log27r, _log27i, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _mod31r = 0, _mod31i = 0;
        _mod31r = fmod(j, _c30r); _mod31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _c29r + _mod31r; _add32i = _c29i + _mod31i;
        double _div33r = 0, _div33i = 0;
        c_div(_mul28r, _mul28i, _add32r, _add32i, &_div33r, &_div33i);
        double magnitude = _div33r; /* +_div33ii */
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
        double _attr41r = 0, _attr41i = 0;
        _attr41r = x2i; _attr41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, _attr41r, _attr41i, &_mul42r, &_mul42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_mul42r, _mul42i, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_conj40r, _conj40i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul39r + _mul44r; _add45i = _mul39i + _mul44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
