/* AUTO-GENERATED from poly700.py — do not edit manually */
/* 100 coefficient functions */

static void poly_601_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_abs4r, _abs4i, j, 0, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _c3r + _mul5r; _add6i = _c3i + _mul5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 6.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _c8r + _sin12r; _add13i = _c8i + _sin12i;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log7r, _log7i, _add13r, _add13i, &_mul14r, &_mul14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 7.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul14r + _cos18r; _add19i = _mul14i + _cos18i;
        double mag = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 4.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul24r + _mul29r; _add30i = _mul24i + _mul29i;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = j + _c31r; _add32i = 0 + _c31i;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _add30r + _log33r; _add34i = _add30i + _log33i;
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
        double _conj41r = 0, _conj41i = 0;
        _conj41r = x2r; _conj41i = -(x2i);
        double _c42r = 0, _c42i = 0;
        _c42r = 5.0; _c42i = 0;
        double _mod43r = 0, _mod43i = 0;
        _mod43r = fmod(j, _c42r); _mod43i = 0;
        double _pow44r = 0, _pow44i = 0;
        c_powr(_conj41r, _conj41i, _mod43r, &_pow44r, &_pow44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul40r + _pow44r; _add45i = _mul40i + _pow44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_602_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[26];
    for (int _li = 0; _li < 26; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 25.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[26];
    for (int _li = 0; _li < 26; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 25.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 0;
        double mag_sum = _c7r; /* +_c7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        double angle_sum = _c8r; /* +_c8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        for (int k = 1; k < (int)(_add10r); k++) {
            double _arr11r = 0, _arr11i = 0;
            { int _idx = (k - 1); _arr11r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr11i = 0; }
            double _arr12r = 0, _arr12i = 0;
            { int _idx = (k - 1); _arr12r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr12i = 0; }
            double _add13r = 0, _add13i = 0;
            _add13r = _arr11r + _arr12r; _add13i = _arr11i + _arr12i;
            double _abs14r = 0, _abs14i = 0;
            _abs14r = c_abs(_add13r, _add13i); _abs14i = 0;
            double _c15r = 0, _c15i = 0;
            _c15r = 1.0; _c15i = 0;
            double _add16r = 0, _add16i = 0;
            _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
            double _log17r = 0, _log17i = 0;
            c_log(_add16r, _add16i, &_log17r, &_log17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(k, 0, M_PI, 0, &_mul18r, &_mul18i);
            double _c19r = 0, _c19i = 0;
            _c19r = 4.0; _c19i = 0;
            double _div20r = 0, _div20i = 0;
            c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
            double _sin21r = 0, _sin21i = 0;
            c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
            double _mul22r = 0, _mul22i = 0;
            c_mul(_log17r, _log17i, _sin21r, _sin21i, &_mul22r, &_mul22i);
            mag_sum += _mul22r;
            double _arr23r = 0, _arr23i = 0;
            { int _idx = (k - 1); _arr23r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr23i = 0; }
            double _arr24r = 0, _arr24i = 0;
            { int _idx = (k - 1); _arr24r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr24i = 0; }
            double _c25r = 0, _c25i = 0;
            _c25r = 0.0; _c25i = 1.0;
            double _mul26r = 0, _mul26i = 0;
            c_mul(_arr24r, _arr24i, _c25r, _c25i, &_mul26r, &_mul26i);
            double _add27r = 0, _add27i = 0;
            _add27r = _arr23r + _mul26r; _add27i = _arr23i + _mul26i;
            double _ang28r = 0, _ang28i = 0;
            _ang28r = c_arg(_add27r, _add27i); _ang28i = 0;
            double _mul29r = 0, _mul29i = 0;
            c_mul(k, 0, M_PI, 0, &_mul29r, &_mul29i);
            double _c30r = 0, _c30i = 0;
            _c30r = 3.0; _c30i = 0;
            double _div31r = 0, _div31i = 0;
            c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
            double _cos32r = 0, _cos32i = 0;
            c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
            double _mul33r = 0, _mul33i = 0;
            c_mul(_ang28r, _ang28i, _cos32r, _cos32i, &_mul33r, &_mul33i);
            angle_sum += _mul33r;
        }
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 5.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(mag_sum, 0, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(x1r, x1i); _abs39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = j + _c40r; _add41i = 0 + _c40i;
        double _div42r = 0, _div42i = 0;
        c_div(_abs39r, _abs39i, _add41r, _add41i, &_div42r, &_div42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul38r + _div42r; _add43i = _mul38i + _div42i;
        double magnitude = _add43r; /* +_add43ii */
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(j, 0, _ang44r, _ang44i, &_mul45r, &_mul45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_mul45r, _mul45i, &_sin46r, &_sin46i);
        double _add47r = 0, _add47i = 0;
        _add47r = angle_sum + _sin46r; _add47i = 0 + _sin46i;
        double angle = _add47r; /* +_add47ii */
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c48r, _c48i, angle, 0, &_mul49r, &_mul49i);
        double _exp50r = 0, _exp50i = 0;
        c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(magnitude, 0, _exp50r, _exp50i, &_mul51r, &_mul51i);
        double _conj52r = 0, _conj52i = 0;
        _conj52r = x2r; _conj52i = -(x2i);
        double _cos53r = 0, _cos53i = 0;
        c_cos(j, 0, &_cos53r, &_cos53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_conj52r, _conj52i, _cos53r, _cos53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _mul51r + _mul54r; _add55i = _mul51i + _mul54i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_603_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 27; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _attr1r, _attr1i, &_mul2r, &_mul2i);
        double _sin3r = 0, _sin3i = 0;
        c_sin(_mul2r, _mul2i, &_sin3r, &_sin3i);
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x2i; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_mul5r, _mul5i, &_cos6r, &_cos6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _sin3r + _cos6r; _add7i = _sin3i + _cos6i;
        double phase = _add7r; /* +_add7ii */
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
        double _c13r = 0, _c13i = 0;
        _c13r = 1.5; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(j, 0, 1.5, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log12r, _log12i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _mod17r = 0, _mod17i = 0;
        _mod17r = fmod(j, _c16r); _mod17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _mod17r + _c18r; _add19i = _mod17i + _c18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_mul15r, _mul15i, _add19r, _add19i, &_mul20r, &_mul20i);
        double mag = _mul20r; /* +_mul20ii */
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1r; _attr21i = 0;
        double _cos22r = 0, _cos22i = 0;
        c_cos(phase, 0, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr21r, _attr21i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(phase, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_attr24r, _attr24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _mul23r - _mul26r; _sub27i = _mul23i - _mul26i;
        double real_part = _sub27r; /* +_sub27ii */
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x2r; _attr28i = 0;
        double _sin29r = 0, _sin29i = 0;
        c_sin(phase, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_attr28r, _attr28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x1i; _attr31i = 0;
        double _cos32r = 0, _cos32i = 0;
        c_cos(phase, 0, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_attr31r, _attr31i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul30r + _mul33r; _add34i = _mul30i + _mul33i;
        double imag_part = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, imag_part, 0, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = real_part + _mul36r; _add37i = 0 + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_add37r, _add37i, mag, 0, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 26) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_604_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[25];
    for (int _li = 0; _li < 25; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 24.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[25];
    for (int _li = 0; _li < 25; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 24.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 25) ? rec[_idx] : 0.0; _arr7i = 0; }
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_arr7r, _arr7i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 5.0; _c13i = 0;
        double _mod14r = 0, _mod14i = 0;
        _mod14r = fmod(j, _c13r); _mod14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _mod14r + _c15r; _add16i = _mod14i + _c15i;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_abs12r, _abs12i, _add16r, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log11r, _log11i, _pow17r, _pow17i, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _sub20r = 0, _sub20i = 0;
        _sub20r = n - j; _sub20i = 0 - 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 7.0; _c21i = 0;
        double _mod22r = 0, _mod22i = 0;
        _mod22r = fmod(_sub20r, _c21r); _mod22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _mod22r + _c23r; _add24i = _mod22i + _c23i;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_abs19r, _abs19i, _add24r, &_pow25r, &_pow25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul18r + _pow25r; _add26i = _mul18i + _pow25i;
        double magnitude = _add26r; /* +_add26ii */
        double _arr27r = 0, _arr27i = 0;
        { int _idx = (j - 1); _arr27r = (_idx >= 0 && _idx < 25) ? rec[_idx] : 0.0; _arr27i = 0; }
        double _mul28r = 0, _mul28i = 0;
        c_mul(_arr27r, _arr27i, M_PI, 0, &_mul28r, &_mul28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_mul28r, _mul28i, j, 0, &_mul29r, &_mul29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_mul29r, _mul29i, &_sin30r, &_sin30i);
        double _arr31r = 0, _arr31i = 0;
        { int _idx = (j - 1); _arr31r = (_idx >= 0 && _idx < 25) ? imc[_idx] : 0.0; _arr31i = 0; }
        double _mul32r = 0, _mul32i = 0;
        c_mul(_arr31r, _arr31i, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _div35r = 0, _div35i = 0;
        c_div(_mul32r, _mul32i, _add34r, _add34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sin30r + _cos36r; _add37i = _sin30i + _cos36i;
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 2.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = j + _c39r; _add40i = 0 + _c39i;
        double _log41r = 0, _log41i = 0;
        c_log(_add40r, _add40i, &_log41r, &_log41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang38r, _ang38i, _log41r, _log41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add37r + _mul42r; _add43i = _add37i + _mul42i;
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _sqrt45r = 0, _sqrt45i = 0;
        c_powr(j, 0, 0.5, &_sqrt45r, &_sqrt45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang44r, _ang44i, _sqrt45r, _sqrt45i, &_mul46r, &_mul46i);
        double _sub47r = 0, _sub47i = 0;
        _sub47r = _add43r - _mul46r; _sub47i = _add43i - _mul46i;
        double angle = _sub47r; /* +_sub47ii */
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
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_605_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c6r, _c6i, M_PI, 0, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_mul7r, _mul7i, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _div12r = 0, _div12i = 0;
        c_div(_mul9r, _mul9i, _add11r, _add11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log5r, _log5i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double mag_part1 = _mul14r; /* +_mul14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs15r + j; _add16i = _abs15i + 0;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_c18r, _c18i, M_PI, 0, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_mul19r, _mul19i, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = j + _c22r; _add23i = 0 + _c22i;
        double _div24r = 0, _div24i = 0;
        c_div(_mul21r, _mul21i, _add23r, _add23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log17r, _log17i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double mag_part2 = _mul26r; /* +_mul26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + mag_part2; _add27i = 0 + 0;
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x1r; _attr28i = 0;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1i; _attr29i = 0;
        double _prod30r = 0, _prod30i = 0;
        c_mul(_attr28r, _attr28i, _attr29r, _attr29i, &_prod30r, &_prod30i);
        double _prod31r = 0, _prod31i = 0;
        c_mul(_prod30r, _prod30i, j, 0, &_prod31r, &_prod31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _add27r + _prod31r; _add32i = _add27i + _prod31i;
        double magnitude = _add32r; /* +_add32ii */
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x1r, x1i); _ang33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang33r, _ang33i, j, 0, &_mul34r, &_mul34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = degree + _c36r; _add37i = 0 + _c36i;
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _add37r - j; _sub38i = _add37i - 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang35r, _ang35i, _sub38r, _sub38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul34r + _mul39r; _add40i = _mul34i + _mul39i;
        double _sin41r = 0, _sin41i = 0;
        c_sin(j, 0, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _add40r + _sin41r; _add42i = _add40i + _sin41i;
        double _cos43r = 0, _cos43i = 0;
        c_cos(j, 0, &_cos43r, &_cos43i);
        double _sub44r = 0, _sub44i = 0;
        _sub44r = _add42r - _cos43r; _sub44i = _add42i - _cos43i;
        double angle = _sub44r; /* +_sub44ii */
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, angle, 0, &_mul46r, &_mul46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(magnitude, 0, _exp47r, _exp47i, &_mul48r, &_mul48i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_606_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
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
    _c5r = 2.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
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
        c_mul(r1, 0, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(j, 0); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(r2, 0, _log15r, _log15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul11r + _mul16r; _add17i = _mul11i + _mul16i;
        double r_part = _add17r; /* +_add17ii */
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(i1, 0, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(i2, 0, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul22r - _mul27r; _sub28i = _mul22i - _mul27i;
        double i_part = _sub28r; /* +_sub28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(j, 0, j, 0, &_pow31r, &_pow31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _abs29r + _pow31r; _add32i = _abs29i + _pow31i;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 4.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _c34r + _sin38r; _add39i = _c34i + _sin38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_log33r, _log33i, _add39r, _add39i, &_mul40r, &_mul40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, M_PI, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 6.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(_mul41r, _mul41i, _c42r, _c42i, &_div43r, &_div43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_div43r, _div43i, &_cos44r, &_cos44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul40r + _cos44r; _add45i = _mul40i + _cos44i;
        double magnitude = _add45r; /* +_add45ii */
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x1r, x1i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_ang46r, _ang46i, j, 0, &_mul47r, &_mul47i);
        double _ang48r = 0, _ang48i = 0;
        _ang48r = c_arg(x2r, x2i); _ang48i = 0;
        double _c49r = 0, _c49i = 0;
        _c49r = 1.0; _c49i = 0;
        double _add50r = 0, _add50i = 0;
        _add50r = degree + _c49r; _add50i = 0 + _c49i;
        double _sub51r = 0, _sub51i = 0;
        _sub51r = _add50r - j; _sub51i = _add50i - 0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(_ang48r, _ang48i, _sub51r, _sub51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul47r + _mul52r; _add53i = _mul47i + _mul52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(j, 0, M_PI, 0, &_mul54r, &_mul54i);
        double _c55r = 0, _c55i = 0;
        _c55r = 8.0; _c55i = 0;
        double _div56r = 0, _div56i = 0;
        c_div(_mul54r, _mul54i, _c55r, _c55i, &_div56r, &_div56i);
        double _sin57r = 0, _sin57i = 0;
        c_sin(_div56r, _div56i, &_sin57r, &_sin57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _add53r + _sin57r; _add58i = _add53i + _sin57i;
        double angle = _add58r; /* +_add58ii */
        double _cos59r = 0, _cos59i = 0;
        c_cos(angle, 0, &_cos59r, &_cos59i);
        double _sin60r = 0, _sin60i = 0;
        c_sin(angle, 0, &_sin60r, &_sin60i);
        double _c61r = 0, _c61i = 0;
        _c61r = 0.0; _c61i = 1.0;
        double _mul62r = 0, _mul62i = 0;
        c_mul(_sin60r, _sin60i, _c61r, _c61i, &_mul62r, &_mul62i);
        double _add63r = 0, _add63i = 0;
        _add63r = _cos59r + _mul62r; _add63i = _cos59i + _mul62i;
        double _mul64r = 0, _mul64i = 0;
        c_mul(magnitude, 0, _add63r, _add63i, &_mul64r, &_mul64i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul64r; cIm[_idx] = _mul64i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_607_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
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
        double _mul8r = 0, _mul8i = 0;
        c_mul(_pow4r, _pow4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = x1r + x2r; _add9i = x1i + x2i;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(_add9r, _add9i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul8r + _cos12r; _add13i = _mul8i + _cos12i;
        double real_part = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs17r, _abs17i, j, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_sin16r, _sin16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2r; _attr21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _attr20r + _attr21r; _add22i = _attr20i + _attr21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = j + _c23r; _add24i = 0 + _c23i;
        double _div25r = 0, _div25i = 0;
        c_div(_add22r, _add22i, _add24r, _add24i, &_div25r, &_div25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul19r + _div25r; _add26i = _mul19i + _div25i;
        double imag_part = _add26r; /* +_add26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, imag_part, 0, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = real_part + _mul28r; _add29i = 0 + _mul28i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_608_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[26];
    for (int _li = 0; _li < 26; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 25.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[26];
    for (int _li = 0; _li < 26; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 25.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr7i = 0; }
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_arr7r, _arr7i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs8r + j; _add9i = _abs8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_c11r, _c11i, M_PI, 0, &_mul12r, &_mul12i);
        double _arr13r = 0, _arr13i = 0;
        { int _idx = (j - 1); _arr13r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr13i = 0; }
        double _mul14r = 0, _mul14i = 0;
        c_mul(_mul12r, _mul12i, _arr13r, _arr13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log10r, _log10i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c17r, _c17i, M_PI, 0, &_mul18r, &_mul18i);
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr19i = 0; }
        double _mul20r = 0, _mul20i = 0;
        c_mul(_mul18r, _mul18i, _arr19r, _arr19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul16r + _cos21r; _add22i = _mul16i + _cos21i;
        double mag = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang23r, _ang23i, j, 0, &_mul24r, &_mul24i);
        double _arr25r = 0, _arr25i = 0;
        { int _idx = (j - 1); _arr25r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr25i = 0; }
        double _mul26r = 0, _mul26i = 0;
        c_mul(M_PI, 0, _arr25r, _arr25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul24r + _sin27r; _add28i = _mul24i + _sin27i;
        double _arr29r = 0, _arr29i = 0;
        { int _idx = (j - 1); _arr29r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr29i = 0; }
        double _mul30r = 0, _mul30i = 0;
        c_mul(M_PI, 0, _arr29r, _arr29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _add28r - _cos31r; _sub32i = _add28i - _cos31i;
        double ang = _sub32r; /* +_sub32ii */
        double _cos33r = 0, _cos33i = 0;
        c_cos(ang, 0, &_cos33r, &_cos33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(ang, 0, &_sin34r, &_sin34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_sin34r, _sin34i, _c35r, _c35i, &_mul36r, &_mul36i);
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

static void poly_609_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x2r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x1r, x1i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _cos10r; _add11i = _mul7i + _cos10i;
        double rec_part = _add11r; /* +_add11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_attr12r, _attr12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul16r + _sin19r; _add20i = _mul16i + _sin19i;
        double imc_part = _add20r; /* +_add20ii */
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs21r + j; _add22i = _abs21i + 0;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log23r, _log23i, rec_part, 0, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.5; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_abs25r, _abs25i, 0.5, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_pow27r, _pow27i, imc_part, 0, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul24r + _mul28r; _add29i = _mul24i + _mul28i;
        double mag = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 3.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 4.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang35r, _ang35i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul34r + _mul39r; _add40i = _mul34i + _mul39i;
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

static void poly_610_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 26.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_mul(j, 0, j, 0, &_pow8r, &_pow8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log6r, _log6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _sub14r = 0, _sub14i = 0;
        _sub14r = n - j; _sub14i = 0 - 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _sub14r + _c15r; _add16i = _sub14i + _c15i;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.5; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_add16r, _add16i, 1.5, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log13r, _log13i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul9r + _mul19r; _add20i = _mul9i + _mul19i;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, n, 0, &_div22r, &_div22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_div22r, _div22i, M_PI, 0, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, n, 0, &_div27r, &_div27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_div27r, _div27i, M_PI, 0, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
        double _sin32r = 0, _sin32i = 0;
        c_sin(j, 0, &_sin32r, &_sin32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.5; _c33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin32r, _sin32i, _c33r, _c33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add31r + _mul34r; _add35i = _add31i + _mul34i;
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
        double _conj42r = 0, _conj42i = 0;
        _conj42r = x1r; _conj42i = -(x1i);
        double _pow43r = 0, _pow43i = 0;
        c_powr(x2r, x2i, j, &_pow43r, &_pow43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_conj42r, _conj42i, _pow43r, _pow43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul41r + _mul44r; _add45i = _mul41i + _mul44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    }
    double _c46r = 0, _c46i = 0;
    _c46r = 1.0; _c46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = n + _c46r; _add47i = 0 + _c46i;
    for (int k = 1; k < (int)(_add47r); k++) {
        double _cf48r = 0, _cf48i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _attr50r = 0, _attr50i = 0;
        _attr50r = x1r; _attr50i = 0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(k, 0, _attr50r, _attr50i, &_mul51r, &_mul51i);
        double _sin52r = 0, _sin52i = 0;
        c_sin(_mul51r, _mul51i, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c49r, _c49i, _sin52r, _sin52i, &_mul53r, &_mul53i);
        double _exp54r = 0, _exp54i = 0;
        c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_cf48r, _cf48i, _exp54r, _exp54i, &_mul55r, &_mul55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 0.0; _c56i = 1.0;
        double _attr57r = 0, _attr57i = 0;
        _attr57r = x2i; _attr57i = 0;
        double _mul58r = 0, _mul58i = 0;
        c_mul(k, 0, _attr57r, _attr57i, &_mul58r, &_mul58i);
        double _cos59r = 0, _cos59i = 0;
        c_cos(_mul58r, _mul58i, &_cos59r, &_cos59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_c56r, _c56i, _cos59r, _cos59i, &_mul60r, &_mul60i);
        double _exp61r = 0, _exp61i = 0;
        c_exp2(_mul60r, _mul60i, &_exp61r, &_exp61i);
        double _add62r = 0, _add62i = 0;
        _add62r = _mul55r + _exp61r; _add62i = _mul55i + _exp61i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add62r; cIm[_idx] = _add62i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_611_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 27; j++) {
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
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _cos10r; _add11i = _mul7i + _cos10i;
        double mag_part = _add11r; /* +_add11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_mul(j, 0, j, 0, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang12r, _ang12i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang16r, _ang16i, _log19r, _log19i, &_mul20r, &_mul20i);
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _mul15r - _mul20r; _sub21i = _mul15i - _mul20i;
        double _sin22r = 0, _sin22i = 0;
        c_sin(j, 0, &_sin22r, &_sin22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(j, 0, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sin22r, _sin22i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _sub21r + _mul24r; _add25i = _sub21i + _mul24i;
        double angle_part = _add25r; /* +_add25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        double intricate_sum = _c26r; /* +_c26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        for (int k = 1; k < (int)(_add28r); k++) {
            double _attr29r = 0, _attr29i = 0;
            _attr29r = x1r; _attr29i = 0;
            double _pow30r = 0, _pow30i = 0;
            c_powr(_attr29r, _attr29i, k, &_pow30r, &_pow30i);
            double _cos31r = 0, _cos31i = 0;
            c_cos(k, 0, &_cos31r, &_cos31i);
            double _mul32r = 0, _mul32i = 0;
            c_mul(_pow30r, _pow30i, _cos31r, _cos31i, &_mul32r, &_mul32i);
            double _c33r = 0, _c33i = 0;
            _c33r = 1.0; _c33i = 0;
            double _add34r = 0, _add34i = 0;
            _add34r = k + _c33r; _add34i = 0 + _c33i;
            double _div35r = 0, _div35i = 0;
            c_div(_mul32r, _mul32i, _add34r, _add34i, &_div35r, &_div35i);
            intricate_sum += _div35r;
        }
        double _c36r = 0, _c36i = 0;
        _c36r = 2.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _flr38r = 0, _flr38i = 0;
        _flr38r = floor(_div37r); _flr38i = 0;
        double _int39r = 0, _int39i = 0;
        _int39r = (int)(_flr38r); _int39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _int39r + _c40r; _add41i = _int39i + _c40i;
        for (int r = 1; r < (int)(_add41r); r++) {
            double _attr42r = 0, _attr42i = 0;
            _attr42r = x2i; _attr42i = 0;
            double _pow43r = 0, _pow43i = 0;
            c_powr(_attr42r, _attr42i, r, &_pow43r, &_pow43i);
            double _sin44r = 0, _sin44i = 0;
            c_sin(r, 0, &_sin44r, &_sin44i);
            double _mul45r = 0, _mul45i = 0;
            c_mul(_pow43r, _pow43i, _sin44r, _sin44i, &_mul45r, &_mul45i);
            double _c46r = 0, _c46i = 0;
            _c46r = 1.0; _c46i = 0;
            double _add47r = 0, _add47i = 0;
            _add47r = r + _c46r; _add47i = 0 + _c46i;
            double _div48r = 0, _div48i = 0;
            c_div(_mul45r, _mul45i, _add47r, _add47i, &_div48r, &_div48i);
            intricate_sum += _div48r;
        }
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(intricate_sum, 0, _c49r, _c49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = mag_part + _mul50r; _add51i = 0 + _mul50i;
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c52r, _c52i, angle_part, 0, &_mul53r, &_mul53i);
        double _exp54r = 0, _exp54i = 0;
        c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_add51r, _add51i, _exp54r, _exp54i, &_mul55r, &_mul55i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 26) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_612_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, M_PI, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 6.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul4r, _mul4i, _c5r, _c5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr3r, _attr3i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr9r, _attr9i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul8r + _mul14r; _add15i = _mul8i + _mul14i;
        double r_part = _add15r; /* +_add15ii */
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1i; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_attr16r, _attr16i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x2i; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 8.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_attr22r, _attr22i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul21r - _mul27r; _sub28i = _mul21i - _mul27i;
        double i_part = _sub28r; /* +_sub28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs29r + _abs30r; _add31i = _abs29i + _abs30i;
        double _add32r = 0, _add32i = 0;
        _add32r = _add31r + j; _add32i = _add31i + 0;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 4.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _c34r + _sin38r; _add39i = _c34i + _sin38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_log33r, _log33i, _add39r, _add39i, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, M_PI, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 9.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _c41r + _cos45r; _add46i = _c41i + _cos45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_mul40r, _mul40i, _add46r, _add46i, &_mul47r, &_mul47i);
        double magnitude = _mul47r; /* +_mul47ii */
        double _ang48r = 0, _ang48i = 0;
        _ang48r = c_arg(x1r, x1i); _ang48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(j, 0, M_PI, 0, &_mul49r, &_mul49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 10.0; _c50i = 0;
        double _div51r = 0, _div51i = 0;
        c_div(_mul49r, _mul49i, _c50r, _c50i, &_div51r, &_div51i);
        double _sin52r = 0, _sin52i = 0;
        c_sin(_div51r, _div51i, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_ang48r, _ang48i, _sin52r, _sin52i, &_mul53r, &_mul53i);
        double _ang54r = 0, _ang54i = 0;
        _ang54r = c_arg(x2r, x2i); _ang54i = 0;
        double _mul55r = 0, _mul55i = 0;
        c_mul(j, 0, M_PI, 0, &_mul55r, &_mul55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 11.0; _c56i = 0;
        double _div57r = 0, _div57i = 0;
        c_div(_mul55r, _mul55i, _c56r, _c56i, &_div57r, &_div57i);
        double _cos58r = 0, _cos58i = 0;
        c_cos(_div57r, _div57i, &_cos58r, &_cos58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(_ang54r, _ang54i, _cos58r, _cos58i, &_mul59r, &_mul59i);
        double _add60r = 0, _add60i = 0;
        _add60r = _mul53r + _mul59r; _add60i = _mul53i + _mul59i;
        double angle = _add60r; /* +_add60ii */
        double _cos61r = 0, _cos61i = 0;
        c_cos(angle, 0, &_cos61r, &_cos61i);
        double _c62r = 0, _c62i = 0;
        _c62r = 0.0; _c62i = 1.0;
        double _sin63r = 0, _sin63i = 0;
        c_sin(angle, 0, &_sin63r, &_sin63i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(_c62r, _c62i, _sin63r, _sin63i, &_mul64r, &_mul64i);
        double _add65r = 0, _add65i = 0;
        _add65r = _cos61r + _mul64r; _add65i = _cos61i + _mul64i;
        double _mul66r = 0, _mul66i = 0;
        c_mul(magnitude, 0, _add65r, _add65i, &_mul66r, &_mul66i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul66r; cIm[_idx] = _mul66i; } }
    }
    double _c67r = 0, _c67i = 0;
    _c67r = 2.0; _c67i = 0;
    double _add68r = 0, _add68i = 0;
    _add68r = degree + _c67r; _add68i = 0 + _c67i;
    for (int j = 1; j < (int)(_add68r); j++) {
        double _attr69r = 0, _attr69i = 0;
        _attr69r = x1r; _attr69i = 0;
        double _attr70r = 0, _attr70i = 0;
        _attr70r = x2i; _attr70i = 0;
        double _sub71r = 0, _sub71i = 0;
        _sub71r = _attr69r - _attr70r; _sub71i = _attr69i - _attr70i;
        double _mul72r = 0, _mul72i = 0;
        c_mul(j, 0, M_PI, 0, &_mul72r, &_mul72i);
        double _c73r = 0, _c73i = 0;
        _c73r = 3.0; _c73i = 0;
        double _div74r = 0, _div74i = 0;
        c_div(_mul72r, _mul72i, _c73r, _c73i, &_div74r, &_div74i);
        double _sin75r = 0, _sin75i = 0;
        c_sin(_div74r, _div74i, &_sin75r, &_sin75i);
        double _mul76r = 0, _mul76i = 0;
        c_mul(_sub71r, _sub71i, _sin75r, _sin75i, &_mul76r, &_mul76i);
        double _mul77r = 0, _mul77i = 0;
        c_mul(j, 0, M_PI, 0, &_mul77r, &_mul77i);
        double _c78r = 0, _c78i = 0;
        _c78r = 5.0; _c78i = 0;
        double _div79r = 0, _div79i = 0;
        c_div(_mul77r, _mul77i, _c78r, _c78i, &_div79r, &_div79i);
        double _cos80r = 0, _cos80i = 0;
        c_cos(_div79r, _div79i, &_cos80r, &_cos80i);
        double _mul81r = 0, _mul81i = 0;
        c_mul(_mul76r, _mul76i, _cos80r, _cos80i, &_mul81r, &_mul81i);
        cRe[(j - 1)] += _mul81r; cIm[(j - 1)] += _mul81i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_613_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[25];
    for (int _li = 0; _li < 25; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 24.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[25];
    for (int _li = 0; _li < 25; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 24.0;
    }
    for (int j = 1; j < 26; j++) {
        double _arr5r = 0, _arr5i = 0;
        { int _idx = (j - 1); _arr5r = (_idx >= 0 && _idx < 25) ? rec[_idx] : 0.0; _arr5i = 0; }
        double _arr6r = 0, _arr6i = 0;
        { int _idx = (j - 1); _arr6r = (_idx >= 0 && _idx < 25) ? imc[_idx] : 0.0; _arr6i = 0; }
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
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(j, 0, j, 0, &_pow13r, &_pow13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(j, 0, &_sin14r, &_sin14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _pow13r + _sin14r; _add15i = _pow13i + _sin14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log11r, _log11i, _add15r, _add15i, &_mul16r, &_mul16i);
        double mag = _mul16r; /* +_mul16ii */
        double _arr17r = 0, _arr17i = 0;
        { int _idx = (j - 1); _arr17r = (_idx >= 0 && _idx < 25) ? rec[_idx] : 0.0; _arr17i = 0; }
        double _mul18r = 0, _mul18i = 0;
        c_mul(_arr17r, _arr17i, j, 0, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _arr20r = 0, _arr20i = 0;
        { int _idx = (j - 1); _arr20r = (_idx >= 0 && _idx < 25) ? imc[_idx] : 0.0; _arr20i = 0; }
        double _mul21r = 0, _mul21i = 0;
        c_mul(_arr20r, _arr20i, j, 0, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _sin19r + _cos22r; _add23i = _sin19i + _cos22i;
        double ang = _add23r; /* +_add23ii */
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, ang, 0, &_mul25r, &_mul25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag, 0, _exp26r, _exp26i, &_mul27r, &_mul27i);
        double _conj28r = 0, _conj28i = 0;
        _conj28r = x1r; _conj28i = -(x1i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(x2r, x2i, j, &_pow29r, &_pow29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_conj28r, _conj28i, _pow29r, _pow29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul27r + _mul30r; _add31i = _mul27i + _mul30i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_614_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_part1 = _mul6r; /* +_mul6ii */
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _add9r + _c10r; _add11i = _add9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log12r, _log12i, _add14r, _add14i, &_mul15r, &_mul15i);
        double mag_part2 = _mul15r; /* +_mul15ii */
        double _add16r = 0, _add16i = 0;
        _add16r = mag_part1 + mag_part2; _add16i = 0 + 0;
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x2i; _attr17i = 0;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_attr17r, _attr17i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _mod20r = 0, _mod20i = 0;
        _mod20r = fmod(j, _c19r); _mod20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _mod20r + _c21r; _add22i = _mod20i + _c21i;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs18r, _abs18i, _add22r, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _add16r + _pow23r; _add24i = _add16i + _pow23i;
        double mag = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 4.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang25r, _ang25i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double angle_part1 = _mul30r; /* +_mul30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 3.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang31r, _ang31i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double angle_part2 = _mul36r; /* +_mul36ii */
        double _add37r = 0, _add37i = 0;
        _add37r = angle_part1 + angle_part2; _add37i = 0 + 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(j, 0, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add37r + _sin38r; _add39i = _add37i + _sin38i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_615_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
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
        double _add17r = 0, _add17i = 0;
        _add17r = _mul9r + _mul16r; _add17i = _mul9i + _mul16i;
        double mag_part = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang18r, _ang18i, j, 0, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_ang20r, _ang20i, _add22r, _add22i, &_div23r, &_div23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul19r + _div23r; _add24i = _mul19i + _div23i;
        double angle_part = _add24r; /* +_add24ii */
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c25r, _c25i, angle_part, 0, &_mul26r, &_mul26i);
        double _exp27r = 0, _exp27i = 0;
        c_exp2(_mul26r, _mul26i, &_exp27r, &_exp27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_part, 0, _exp27r, _exp27i, &_mul28r, &_mul28i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _add30r = 0, _add30i = 0;
    _add30r = degree + _c29r; _add30i = 0 + _c29i;
    for (int k = 1; k < (int)(_add30r); k++) {
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x1r; _attr31i = 0;
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2i; _attr32i = 0;
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _attr31r - _attr32r; _sub33i = _attr31i - _attr32i;
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(k, 0, _ang34r, _ang34i, &_mul35r, &_mul35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_mul35r, _mul35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sub33r, _sub33i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x2r; _attr38i = 0;
        double _attr39r = 0, _attr39i = 0;
        _attr39r = x1i; _attr39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = _attr38r + _attr39r; _add40i = _attr38i + _attr39i;
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(k, 0, _ang41r, _ang41i, &_mul42r, &_mul42i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(_mul42r, _mul42i, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_add40r, _add40i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul37r + _mul44r; _add45i = _mul37i + _mul44i;
        cRe[(k - 1)] += _add45r; cIm[(k - 1)] += _add45i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_616_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = k + _c3r; _add4i = 0 + _c3i;
        double j = _add4r; /* +_add4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_attr5r, _attr5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2r; _attr10i = 0;
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr10r, _attr10i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul9r + _mul14r; _add15i = _mul9i + _mul14i;
        double r_part = _add15r; /* +_add15ii */
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1i; _attr16i = 0;
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x2i; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, _attr17r, _attr17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_attr16r, _attr16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2i; _attr21i = 0;
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x1i; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(k, 0, _attr22r, _attr22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_attr21r, _attr21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul20r - _mul25r; _sub26i = _mul20i - _mul25i;
        double im_part = _sub26r; /* +_sub26ii */
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
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = k + _c32r; _add33i = 0 + _c32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_log31r, _log31i, _add33r, _add33i, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _c35r + k; _add36i = _c35i + 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul34r, _mul34i, _add36r, _add36i, &_div37r, &_div37i);
        double magnitude = _div37r; /* +_div37ii */
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(k, 0, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang38r, _ang38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _cos42r = 0, _cos42i = 0;
        c_cos(k, 0, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang41r, _ang41i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul40r + _mul43r; _add44i = _mul40i + _mul43i;
        double angle = _add44r; /* +_add44ii */
        double _cos45r = 0, _cos45i = 0;
        c_cos(angle, 0, &_cos45r, &_cos45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(angle, 0, &_sin46r, &_sin46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_sin46r, _sin46i, _c47r, _c47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _cos45r + _mul48r; _add49i = _cos45i + _mul48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(magnitude, 0, _add49r, _add49i, &_mul50r, &_mul50i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_617_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double real_sum = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double imag_sum = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        for (int k = 1; k < (int)(_add4r); k++) {
            double _c5r = 0, _c5i = 0;
            _c5r = 1.0; _c5i = 0;
            double _add6r = 0, _add6i = 0;
            _add6r = k + _c5r; _add6i = 0 + _c5i;
            for (int r = 1; r < (int)(_add6r); r++) {
                double _attr7r = 0, _attr7i = 0;
                _attr7r = x1r; _attr7i = 0;
                double _pow8r = 0, _pow8i = 0;
                c_powr(_attr7r, _attr7i, k, &_pow8r, &_pow8i);
                double _ang9r = 0, _ang9i = 0;
                _ang9r = c_arg(x2r, x2i); _ang9i = 0;
                double _mul10r = 0, _mul10i = 0;
                c_mul(r, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
                double _cos11r = 0, _cos11i = 0;
                c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
                double _mul12r = 0, _mul12i = 0;
                c_mul(_pow8r, _pow8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
                real_sum += _mul12r;
                double _attr13r = 0, _attr13i = 0;
                _attr13r = x2i; _attr13i = 0;
                double _pow14r = 0, _pow14i = 0;
                c_powr(_attr13r, _attr13i, r, &_pow14r, &_pow14i);
                double _ang15r = 0, _ang15i = 0;
                _ang15r = c_arg(x1r, x1i); _ang15i = 0;
                double _mul16r = 0, _mul16i = 0;
                c_mul(k, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
                double _sin17r = 0, _sin17i = 0;
                c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
                double _mul18r = 0, _mul18i = 0;
                c_mul(_pow14r, _pow14i, _sin17r, _sin17i, &_mul18r, &_mul18i);
                imag_sum += _mul18r;
            }
        }
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = real_sum + _c19r; _add20i = 0 + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = imag_sum + _c22r; _add23i = 0 + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _cplx25r = 0, _cplx25i = 0;
        _cplx25r = _log21r; _cplx25i = _log24r;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _cplx25r; cIm[_idx] = _cplx25i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_618_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_mul(r, 0, r, 0, &_pow7r, &_pow7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _abs5r + _pow7r; _add8i = _abs5i + _pow7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2i; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(r, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log9r, _log9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1r; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(r, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul13r + _cos16r; _add17i = _mul13i + _cos16i;
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang18r, _ang18i, r, 0, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2r; _attr20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_attr20r, _attr20i, _add22r, _add22i, &_div23r, &_div23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul19r + _div23r; _add24i = _mul19i + _div23i;
        double ang = _add24r; /* +_add24ii */
        double _cos25r = 0, _cos25i = 0;
        c_cos(ang, 0, &_cos25r, &_cos25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(ang, 0, &_sin26r, &_sin26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin26r, _sin26i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos25r + _mul28r; _add29i = _cos25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_619_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    for (int j = 1; j < 10; j++) {
        double _add5r = 0, _add5i = 0;
        _add5r = x1r + j; _add5i = x1i + 0;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(_add5r, _add5i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_mul(_abs6r, _abs6i, _abs6r, _abs6i, &_pow8r, &_pow8i);
        double _sub9r = 0, _sub9i = 0;
        _sub9r = x2r - j; _sub9i = x2i - 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_sub9r, _sub9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(_abs10r, _abs10i, _abs10r, _abs10i, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _pow8r + _pow12r; _add13i = _pow8i + _pow12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _ang16r, _ang16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _ang19r, _ang19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin18r, _sin18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _c15r + _mul22r; _add23i = _c15i + _mul22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log14r, _log14i, _add23r, _add23i, &_mul24r, &_mul24i);
        double mag = _mul24r; /* +_mul24ii */
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
        double _c30r = 0, _c30i = 0;
        _c30r = 9.0; _c30i = 0;
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _c30r - j; _sub31i = _c30i - 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.5; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_powr(_sub31r, _sub31i, 0.5, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang29r, _ang29i, _pow33r, _pow33i, &_mul34r, &_mul34i);
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _mul28r - _mul34r; _sub35i = _mul28i - _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 5.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _sub35r + _sin39r; _add40i = _sub35i + _sin39i;
        double angle = _add40r; /* +_add40ii */
        double _cos41r = 0, _cos41i = 0;
        c_cos(angle, 0, &_cos41r, &_cos41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(angle, 0, &_sin42r, &_sin42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_sin42r, _sin42i, _c43r, _c43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos41r + _mul44r; _add45i = _cos41i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(mag, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_620_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    for (int j = 1; j < 10; j++) {
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
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
        double mag_real = _mul12r; /* +_mul12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log16r, _log16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double mag_imag = _mul20r; /* +_mul20ii */
        double _add21r = 0, _add21i = 0;
        _add21r = mag_real + mag_imag; _add21i = 0 + 0;
        double magnitude = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_ang22r, _ang22i, j, &_pow23r, &_pow23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_ang24r, _ang24i, j, &_pow25r, &_pow25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _pow23r - _pow25r; _sub26i = _pow23i - _pow25i;
        double angle = _sub26r; /* +_sub26ii */
        double _cos27r = 0, _cos27i = 0;
        c_cos(angle, 0, &_cos27r, &_cos27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(angle, 0, &_sin28r, &_sin28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_sin28r, _sin28i, _c29r, _c29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _cos27r + _mul30r; _add31i = _cos27i + _mul30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(magnitude, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_621_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2i; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _div5r = 0, _div5i = 0;
        c_div(j, 0, _c4r, _c4i, &_div5r, &_div5i);
        double _pow6r = 0, _pow6i = 0;
        c_powr(_attr3r, _attr3i, _div5r, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs8r + j; _add9i = _abs8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_add7r, _add7i, _log10r, _log10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2r; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sin14r, _sin14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul11r + _mul18r; _add19i = _mul11i + _mul18i;
        double mag = _add19r; /* +_add19ii */
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
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul22r - _mul25r; _sub26i = _mul22i - _mul25i;
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
        double _mul33r = 0, _mul33i = 0;
        c_mul(_sin29r, _sin29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _sub26r + _mul33r; _add34i = _sub26i + _mul33i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_622_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _pow2r + _c3r; _add4i = _pow2i + _c3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _attr6r + j; _add7i = _attr6i + 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(_add7r, _add7i, _add7r, _add7i, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log5r, _log5i, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double mag_part = _mul10r; /* +_mul10ii */
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _ang12r, _ang12i, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang11r, _ang11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _cos18r; _add19i = _mul15i + _cos18i;
        double angle_part = _add19r; /* +_add19ii */
        double _cos20r = 0, _cos20i = 0;
        c_cos(angle_part, 0, &_cos20r, &_cos20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(angle_part, 0, &_sin21r, &_sin21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin21r, _sin21i, _c22r, _c22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _cos20r + _mul23r; _add24i = _cos20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag_part, 0, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_623_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 10; k++) {
        double j = k; /* +0i */
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
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1i; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _sin6r + _cos9r; _add10i = _sin6i + _cos9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log3r, _log3i, _add10r, _add10i, &_mul11r, &_mul11i);
        double magnitude = _mul11r; /* +_mul11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1r; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang12r, _ang12i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x1i; _attr18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, _attr18r, _attr18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang17r, _ang17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _mul16r - _mul21r; _sub22i = _mul16i - _mul21i;
        double angle = _sub22r; /* +_sub22ii */
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, angle, 0, &_mul24r, &_mul24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(magnitude, 0, _exp25r, _exp25i, &_mul26r, &_mul26i);
        double _conj27r = 0, _conj27i = 0;
        _conj27r = x1r; _conj27i = -(x1i);
        double _pow28r = 0, _pow28i = 0;
        c_powr(x2r, x2i, j, &_pow28r, &_pow28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_conj27r, _conj27i, _pow28r, _pow28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul26r + _mul29r; _add30i = _mul26i + _mul29i;
        { int _idx = ((int)(j) - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_624_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
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
        _c15r = 2.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(j, 0, _c15r, _c15i, &_div16r, &_div16i);
        double _pow17r = 0, _pow17i = 0;
        c_powr(_ang14r, _ang14i, _div16r, &_pow17r, &_pow17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _pow13r + _pow17r; _add18i = _pow13i + _pow17i;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _add18r + _log21r; _add22i = _add18i + _log21i;
        double angle_part = _add22r; /* +_add22ii */
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(mag_part, 0); _abs23i = 0;
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1r; _attr24i = 0;
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _prod26r = 0, _prod26i = 0;
        c_mul(_attr24r, _attr24i, _attr25r, _attr25i, &_prod26r, &_prod26i);
        double _prod27r = 0, _prod27i = 0;
        c_mul(_prod26r, _prod26i, j, 0, &_prod27r, &_prod27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _abs23r + _prod27r; _add28i = _abs23i + _prod27i;
        double mag = _add28r; /* +_add28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _sum31r = 0, _sum31i = 0;
        _sum31r = _abs29r + _abs30r; _sum31i = _abs29i + _abs30i;
        double _sum32r = 0, _sum32i = 0;
        _sum32r = _sum31r + j; _sum32i = _sum31i + 0;
        double _add33r = 0, _add33i = 0;
        _add33r = angle_part + _sum32r; _add33i = 0 + _sum32i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_625_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x1r, x1i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _c4r + _sin7r; _add8i = _c4i + _sin7i;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _add8r + _cos11r; _add12i = _add8i + _cos11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log3r, _log3i, _add12r, _add12i, &_mul13r, &_mul13i);
        double mag = _mul13r; /* +_mul13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_ang14r, _ang14i, j, &_pow15r, &_pow15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(j, 0, _c17r, _c17i, &_div18r, &_div18i);
        double _pow19r = 0, _pow19i = 0;
        c_powr(_ang16r, _ang16i, _div18r, &_pow19r, &_pow19i);
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _pow15r - _pow19r; _sub20i = _pow15i - _pow19i;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1r; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_sin23r, _sin23i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sub20r + _mul27r; _add28i = _sub20i + _mul27i;
        double angle = _add28r; /* +_add28ii */
        double _cos29r = 0, _cos29i = 0;
        c_cos(angle, 0, &_cos29r, &_cos29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(angle, 0, &_sin30r, &_sin30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_sin30r, _sin30i, _c31r, _c31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _cos29r + _mul32r; _add33i = _cos29i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(mag, 0, _add33r, _add33i, &_mul34r, &_mul34i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_626_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(j, 0, M_PI, 0, &_mul1r, &_mul1i);
        double _attr2r = 0, _attr2i = 0;
        _attr2r = x1r; _attr2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(_mul1r, _mul1i, _attr2r, _attr2i, &_mul3r, &_mul3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _c4r + j; _add5i = _c4i + 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul3r, _mul3i, _add5r, _add5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_mul8r, _mul8i, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _c11r + j; _add12i = _c11i + 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul10r, _mul10i, _add12r, _add12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _sin7r + _cos14r; _add15i = _sin7i + _cos14i;
        double mag_part1 = _add15r; /* +_add15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x2r, x2i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log19r, _log19i, _log23r, _log23i, &_mul24r, &_mul24i);
        double mag_part2 = _mul24r; /* +_mul24ii */
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_mul(j, 0, j, 0, &_pow27r, &_pow27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul25r + _pow27r; _add28i = _mul25i + _pow27i;
        double magnitude = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang30r, _ang30i, j, 0, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _ang29r + _mul31r; _add32i = _ang29i + _mul31i;
        double angle_part1 = _add32r; /* +_add32ii */
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x2r; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_mul34r, _mul34i, &_cos35r, &_cos35i);
        double _attr36r = 0, _attr36i = 0;
        _attr36r = x1i; _attr36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, _attr36r, _attr36i, &_mul37r, &_mul37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_mul37r, _mul37i, &_sin38r, &_sin38i);
        double _sub39r = 0, _sub39i = 0;
        _sub39r = _cos35r - _sin38r; _sub39i = _cos35i - _sin38i;
        double angle_part2 = _sub39r; /* +_sub39ii */
        double _add40r = 0, _add40i = 0;
        _add40r = angle_part1 + angle_part2; _add40i = 0 + 0;
        double angle = _add40r; /* +_add40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(magnitude, 0, _exp43r, _exp43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_627_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 10; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_abs1r, _abs1i, k, &_pow2r, &_pow2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _pow2r + _c3r; _add4i = _pow2i + _c3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _attr6r + k; _add7i = _attr6i + 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(_add7r, _add7i, _add7r, _add7i, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log5r, _log5i, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double mag_part1 = _mul10r; /* +_mul10ii */
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(k, 0, _ang12r, _ang12i, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang11r, _ang11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(k, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _cos18r; _add19i = _mul15i + _cos18i;
        double angle_part = _add19r; /* +_add19ii */
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_sin22r, _sin22i); _abs23i = 0;
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2r; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(k, 0, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_cos26r, _cos26i); _abs27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs23r + _abs27r; _add28i = _abs23i + _abs27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(mag_part1, 0, _add28r, _add28i, &_mul29r, &_mul29i);
        double mag_variation = _mul29r; /* +_mul29ii */
        double angle = angle_part; /* +0i */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, angle, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(mag_variation, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_628_c(double x1r, double x1i, double x2r, double x2i,
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
        for (int k = 1; k < 5; k++) {
            double _attr3r = 0, _attr3i = 0;
            _attr3r = x1r; _attr3i = 0;
            double _pow4r = 0, _pow4i = 0;
            c_powr(_attr3r, _attr3i, k, &_pow4r, &_pow4i);
            double _mul5r = 0, _mul5i = 0;
            c_mul(k, 0, j, 0, &_mul5r, &_mul5i);
            double _sin6r = 0, _sin6i = 0;
            c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
            double _mul7r = 0, _mul7i = 0;
            c_mul(_pow4r, _pow4i, _sin6r, _sin6i, &_mul7r, &_mul7i);
            double _attr8r = 0, _attr8i = 0;
            _attr8r = x2i; _attr8i = 0;
            double _pow9r = 0, _pow9i = 0;
            c_powr(_attr8r, _attr8i, k, &_pow9r, &_pow9i);
            double _add10r = 0, _add10i = 0;
            _add10r = k + j; _add10i = 0 + 0;
            double _cos11r = 0, _cos11i = 0;
            c_cos(_add10r, _add10i, &_cos11r, &_cos11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_pow9r, _pow9i, _cos11r, _cos11i, &_mul12r, &_mul12i);
            double _add13r = 0, _add13i = 0;
            _add13r = _mul7r + _mul12r; _add13i = _mul7i + _mul12i;
            mag += _add13r;
            double _ang14r = 0, _ang14i = 0;
            _ang14r = c_arg(x1r, x1i); _ang14i = 0;
            double _ang15r = 0, _ang15i = 0;
            _ang15r = c_arg(x2r, x2i); _ang15i = 0;
            double _add16r = 0, _add16i = 0;
            _add16r = _ang14r + _ang15r; _add16i = _ang14i + _ang15i;
            double _add17r = 0, _add17i = 0;
            _add17r = k + j; _add17i = 0 + 0;
            double _div18r = 0, _div18i = 0;
            c_div(_add16r, _add16i, _add17r, _add17i, &_div18r, &_div18i);
            angle += _div18r;
        }
        double _c19r = 0, _c19i = 0;
        _c19r = 0.0; _c19i = 1.0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c19r, _c19i, angle, 0, &_mul20r, &_mul20i);
        double _exp21r = 0, _exp21i = 0;
        c_exp2(_mul20r, _mul20i, &_exp21r, &_exp21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(mag, 0, _exp21r, _exp21i, &_mul22r, &_mul22i);
        double _conj23r = 0, _conj23i = 0;
        _conj23r = x1r; _conj23i = -(x1i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_conj23r, _conj23i, _log27r, _log27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul22r + _mul28r; _add29i = _mul22i + _mul28i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_629_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_attr1r, _attr1i, j, 0, &_mul2r, &_mul2i);
        double r = _mul2r; /* +_mul2ii */
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2i; _attr3i = 0;
        double _div4r = 0, _div4i = 0;
        c_div(_attr3r, _attr3i, j, 0, &_div4r, &_div4i);
        double im = _div4r; /* +_div4ii */
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs5r + j; _add6i = _abs5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(r, 0, &_sin8r, &_sin8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(im, 0, &_cos9r, &_cos9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _sin8r + _cos9r; _add10i = _sin8i + _cos9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log7r, _log7i, _add10r, _add10i, &_mul11r, &_mul11i);
        double mag = _mul11r; /* +_mul11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(j, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _cos16r = 0, _cos16i = 0;
        c_cos(j, 0, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul14r + _mul17r; _add18i = _mul14i + _mul17i;
        double angle = _add18r; /* +_add18ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_630_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x2r, x2i); _abs2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs2r + j; _add3i = _abs2i + 0;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr1r, _attr1i, _log4r, _log4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = x1r + x2r; _add6i = x1i + x2i;
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(_add6r, _add6i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul5r + _cos9r; _add10i = _mul5i + _cos9i;
        double r_part = _add10r; /* +_add10ii */
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs14r, _abs14i, j, &_pow15r, &_pow15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sin13r, _sin13i, _pow15r, _pow15i, &_mul16r, &_mul16i);
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x1r; _attr17i = 0;
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x2r; _attr18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _attr17r + _attr18r; _add19i = _attr17i + _attr18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_add19r, _add19i, _add21r, _add21i, &_div22r, &_div22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul16r + _div22r; _add23i = _mul16i + _div22i;
        double im_part = _add23r; /* +_add23ii */
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_mul(r_part, 0, r_part, 0, &_pow25r, &_pow25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_mul(im_part, 0, im_part, 0, &_pow27r, &_pow27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _pow25r + _pow27r; _add28i = _pow25i + _pow27i;
        double _sqrt29r = 0, _sqrt29i = 0;
        c_powr(_add28r, _add28i, 0.5, &_sqrt29r, &_sqrt29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.5; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(j, 0, 1.5, &_pow31r, &_pow31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_sqrt29r, _sqrt29i, _pow31r, _pow31i, &_mul32r, &_mul32i);
        double magnitude = _mul32r; /* +_mul32ii */
        double _at233r = 0, _at233i = 0;
        _at233r = atan2(im_part, r_part); _at233i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 3.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _at233r + _sin37r; _add38i = _at233i + _sin37i;
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
        c_mul(magnitude, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_631_c(double x1r, double x1i, double x2r, double x2i,
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
        double _pow4r = 0, _pow4i = 0;
        c_powr(_attr3r, _attr3i, j, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs5r + j; _add6i = _abs5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_pow4r, _pow4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = x1r + x2r; _add9i = x1i + x2i;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(_add9r, _add9i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul8r + _cos12r; _add13i = _mul8i + _cos12i;
        double real_part = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs17r, _abs17i, j, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_sin16r, _sin16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2r; _attr21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _attr20r + _attr21r; _add22i = _attr20i + _attr21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = j + _c23r; _add24i = 0 + _c23i;
        double _div25r = 0, _div25i = 0;
        c_div(_add22r, _add22i, _add24r, _add24i, &_div25r, &_div25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul19r + _div25r; _add26i = _mul19i + _div25i;
        double imag_part = _add26r; /* +_add26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, imag_part, 0, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = real_part + _mul28r; _add29i = 0 + _mul28i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_632_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _sub4r = 0, _sub4i = 0;
        _sub4r = _attr2r - _attr3r; _sub4i = _attr2i - _attr3i;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_sub4r, _sub4i, j, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 8.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _attr1r + _div7r; _add8i = _attr1i + _div7i;
        double rec = _add8r; /* +_add8ii */
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1i; _attr9i = 0;
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2i; _attr10i = 0;
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1i; _attr11i = 0;
        double _sub12r = 0, _sub12i = 0;
        _sub12r = _attr10r - _attr11r; _sub12i = _attr10i - _attr11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sub12r, _sub12i, j, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 8.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _attr9r + _div15r; _add16i = _attr9i + _div15i;
        double imc = _add16r; /* +_add16ii */
        double _mul17r = 0, _mul17i = 0;
        c_mul(rec, 0, imc, 0, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(_mul17r, _mul17i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(j, 0, j, 0, &_pow23r, &_pow23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(j, 0, &_sin24r, &_sin24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _pow23r + _sin24r; _add25i = _pow23i + _sin24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log21r, _log21i, _add25r, _add25i, &_mul26r, &_mul26i);
        double mag = _mul26r; /* +_mul26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang27r, _ang27i, j, 0, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 9.0; _c30i = 0;
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _c30r - j; _sub31i = _c30i - 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang29r, _ang29i, _sub31r, _sub31i, &_mul32r, &_mul32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _mul28r - _mul32r; _sub33i = _mul28i - _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 4.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sub33r + _cos37r; _add38i = _sub33i + _cos37i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_633_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_mul(r, 0, r, 0, &_pow7r, &_pow7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _abs5r + _pow7r; _add8i = _abs5i + _pow7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2i; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(r, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log9r, _log9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1r; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(r, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul13r + _cos16r; _add17i = _mul13i + _cos16i;
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang18r, _ang18i, r, 0, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2r; _attr20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_attr20r, _attr20i, _add22r, _add22i, &_div23r, &_div23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul19r + _div23r; _add24i = _mul19i + _div23i;
        double ang = _add24r; /* +_add24ii */
        double _cos25r = 0, _cos25i = 0;
        c_cos(ang, 0, &_cos25r, &_cos25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(ang, 0, &_sin26r, &_sin26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin26r, _sin26i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos25r + _mul28r; _add29i = _cos25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_634_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double r1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double im1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double r2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double im2 = _attr4r; /* +_attr4ii */
    for (int j = 1; j < 10; j++) {
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_mul5r, _mul5i, r1, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _c7r + j; _add8i = _c7i + 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul6r, _mul6i, _add8r, _add8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_mul11r, _mul11i, r2, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _c13r + j; _add14i = _c13i + 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul12r, _mul12i, _add14r, _add14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _sin10r + _cos16r; _add17i = _sin10i + _cos16i;
        double mag_part1 = _add17r; /* +_add17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log21r, _log21i, _log25r, _log25i, &_mul26r, &_mul26i);
        double mag_part2 = _mul26r; /* +_mul26ii */
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(j, 0, j, 0, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul27r + _pow29r; _add30i = _mul27i + _pow29i;
        double magnitude = _add30r; /* +_add30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang32r, _ang32i, j, 0, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _ang31r + _mul33r; _add34i = _ang31i + _mul33i;
        double angle_part1 = _add34r; /* +_add34ii */
        double _attr35r = 0, _attr35i = 0;
        _attr35r = x2r; _attr35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, _attr35r, _attr35i, &_mul36r, &_mul36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_mul36r, _mul36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, im1, 0, &_mul38r, &_mul38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_mul38r, _mul38i, &_sin39r, &_sin39i);
        double _sub40r = 0, _sub40i = 0;
        _sub40r = _cos37r - _sin39r; _sub40i = _cos37i - _sin39i;
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
        c_mul(magnitude, 0, _exp44r, _exp44i, &_mul45r, &_mul45i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_635_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 9; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _attr1r + j; _add2i = _attr1i + 0;
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(_add2r, _add2i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double mag_part1 = _log6r; /* +_log6ii */
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 3.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(j, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _pow10r = 0, _pow10i = 0;
        c_powr(_abs7r, _abs7i, _div9r, &_pow10r, &_pow10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 5.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _pow10r + _sin14r; _add15i = _pow10i + _sin14i;
        double mag_part2 = _add15r; /* +_add15ii */
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _attr19r, _attr19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _sin18r + _cos21r; _add22i = _sin18i + _cos21i;
        double angle_part1 = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _cos24r = 0, _cos24i = 0;
        c_cos(j, 0, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(j, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul25r - _mul28r; _sub29i = _mul25i - _mul28i;
        double angle_part2 = _sub29r; /* +_sub29ii */
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul30r, &_mul30i);
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x1r; _attr31i = 0;
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2i; _attr32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _attr32r + j; _add33i = _attr32i + 0;
        double _prod34r = 0, _prod34i = 0;
        c_mul(_attr31r, _attr31i, _add33r, _add33i, &_prod34r, &_prod34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul30r + _prod34r; _add35i = _mul30i + _prod34i;
        double magnitude = _add35r; /* +_add35ii */
        double _add36r = 0, _add36i = 0;
        _add36r = angle_part1 + angle_part2; _add36i = 0 + 0;
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
        c_mul(magnitude, 0, _add41r, _add41i, &_mul42r, &_mul42i);
        { int _idx = j; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_636_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    for (int j = 1; j < 10; j++) {
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
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log7r, _log7i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul13r + _cos18r; _add19i = _mul13i + _cos18i;
        double mag_part = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _cos21r = 0, _cos21i = 0;
        c_cos(j, 0, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang20r, _ang20i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _sin24r = 0, _sin24i = 0;
        c_sin(_ang23r, _ang23i, &_sin24r, &_sin24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul22r + _sin24r; _add25i = _mul22i + _sin24i;
        double angle_part = _add25r; /* +_add25ii */
        double _cos26r = 0, _cos26i = 0;
        c_cos(angle_part, 0, &_cos26r, &_cos26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(angle_part, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c27r, _c27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _cos26r + _mul29r; _add30i = _cos26i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag_part, 0, _add30r, _add30i, &_mul31r, &_mul31i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_637_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(_mul1r, _mul1i); _abs2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs2r + _c3r; _add4i = _abs2i + _c3i;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_attr6r, _attr6i, j, &_pow7r, &_pow7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(j, 0, _c9r, _c9i, &_div10r, &_div10i);
        double _pow11r = 0, _pow11i = 0;
        c_powr(_attr8r, _attr8i, _div10r, &_pow11r, &_pow11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _pow7r + _pow11r; _add12i = _pow7i + _pow11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log5r, _log5i, _add12r, _add12i, &_mul13r, &_mul13i);
        double mag = _mul13r; /* +_mul13ii */
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
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _mul16r - _mul19r; _sub20i = _mul16i - _mul19i;
        double angle = _sub20r; /* +_sub20ii */
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
        c_mul(mag, 0, _add25r, _add25i, &_mul26r, &_mul26i);
        double _conj27r = 0, _conj27i = 0;
        _conj27r = x1r; _conj27i = -(x1i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(j, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_conj27r, _conj27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul26r + _mul29r; _add30i = _mul26i + _mul29i;
        double _conj31r = 0, _conj31i = 0;
        _conj31r = x2r; _conj31i = -(x2i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(j, 0, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_conj31r, _conj31i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _add30r - _mul33r; _sub34i = _add30i - _mul33i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _sub34r; cIm[_idx] = _sub34i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_638_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 9.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_attr3r, _attr3i, _sub5r, &_pow6r, &_pow6i);
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _pow2r - _pow6r; _sub7i = _pow2i - _pow6i;
        double rec_part = _sub7r; /* +_sub7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr8r, _attr8i, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _sin15r; _add16i = _mul12i + _sin15i;
        double _cos17r = 0, _cos17i = 0;
        c_cos(_add16r, _add16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul10r + _cos17r; _add18i = _mul10i + _cos17i;
        double im_part = _add18r; /* +_add18ii */
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
        _c24r = 1.0; _c24i = 0;
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1r; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x2i; _attr28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, _attr28r, _attr28i, &_mul29r, &_mul29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_mul29r, _mul29i, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_sin27r, _sin27i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _c24r + _mul31r; _add32i = _c24i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_log23r, _log23i, _add32r, _add32i, &_mul33r, &_mul33i);
        double mag = _mul33r; /* +_mul33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang34r, _ang34i, j, 0, &_mul35r, &_mul35i);
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = j + _c37r; _add38i = 0 + _c37i;
        double _div39r = 0, _div39i = 0;
        c_div(_ang36r, _ang36i, _add38r, _add38i, &_div39r, &_div39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul35r + _div39r; _add40i = _mul35i + _div39i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_639_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_mul(j, 0, j, 0, &_pow10r, &_pow10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _abs8r + _pow10r; _add11i = _abs8i + _pow10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log12r, _log12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul7r + _mul16r; _add17i = _mul7i + _mul16i;
        double magnitude = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(j, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(j, 0, &_sin22r, &_sin22i);
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
        c_mul(magnitude, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_640_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _c1r, _c1i, &_mul2r, &_mul2i);
        double k = _mul2r; /* +_mul2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 9.0; _c3i = 0;
        double _sub4r = 0, _sub4i = 0;
        _sub4r = _c3r - j; _sub4i = _c3i - 0;
        double r = _sub4r; /* +_sub4ii */
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs5r + j; _add6i = _abs5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _c8r + _sin9r; _add10i = _c8i + _sin9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log7r, _log7i, _add10r, _add10i, &_mul11r, &_mul11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 0.5; _c13i = 0;
        double _cos14r = 0, _cos14i = 0;
        c_cos(j, 0, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c13r + _cos14r; _add15i = _c13i + _cos14i;
        double _pow16r = 0, _pow16i = 0;
        c_powr(_abs12r, _abs12i, _add15r, &_pow16r, &_pow16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul11r + _pow16r; _add17i = _mul11i + _pow16i;
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang18r, _ang18i, j, 0, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang20r, _ang20i, k, 0, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul19r + _mul21r; _add22i = _mul19i + _mul21i;
        double _sin23r = 0, _sin23i = 0;
        c_sin(j, 0, &_sin23r, &_sin23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(r, 0, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin23r, _sin23i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _add22r + _mul25r; _add26i = _add22i + _mul25i;
        double angle = _add26r; /* +_add26ii */
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
        c_mul(mag, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_641_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 9.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_attr3r, _attr3i, _sub5r, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double r_part = _add7r; /* +_add7ii */
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
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _mul10r - _mul13r; _sub14i = _mul10i - _mul13i;
        double im_part = _sub14r; /* +_sub14ii */
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
        double _sin21r = 0, _sin21i = 0;
        c_sin(j, 0, &_sin21r, &_sin21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(j, 0, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin21r, _sin21i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _c20r + _mul23r; _add24i = _c20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log19r, _log19i, _add24r, _add24i, &_mul25r, &_mul25i);
        double magnitude = _mul25r; /* +_mul25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang26r, _ang26i, j, 0, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = j + _c29r; _add30i = 0 + _c29i;
        double _div31r = 0, _div31i = 0;
        c_div(_ang28r, _ang28i, _add30r, _add30i, &_div31r, &_div31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _mul27r - _div31r; _sub32i = _mul27i - _div31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 4.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sub32r + _sin36r; _add37i = _sub32i + _sin36i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_642_c(double x1r, double x1i, double x2r, double x2i,
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
        _c3r = 3.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(j, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double k = _add6r; /* +_add6ii */
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _fdiv8r = 0, _fdiv8i = 0;
        c_div(j, 0, _c7r, _c7i, &_fdiv8r, &_fdiv8i);
        _fdiv8r = floor(_fdiv8r); _fdiv8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _fdiv8r + _c9r; _add10i = _fdiv8i + _c9i;
        double r = _add10r; /* +_add10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 4.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log14r, _log14i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double mag_part1 = _mul19r; /* +_mul19ii */
        double _mul20r = 0, _mul20i = 0;
        c_mul(k, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_abs24r, _abs24i, r, &_pow25r, &_pow25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_cos23r, _cos23i, _pow25r, _pow25i, &_mul26r, &_mul26i);
        double mag_part2 = _mul26r; /* +_mul26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + mag_part2; _add27i = 0 + 0;
        double magnitude = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang28r, _ang28i, j, 0, &_mul29r, &_mul29i);
        double angle_part1 = _mul29r; /* +_mul29ii */
        double _mul30r = 0, _mul30i = 0;
        c_mul(r, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang34r, _ang34i, k, 0, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _sin33r + _mul35r; _add36i = _sin33i + _mul35i;
        double angle_part2 = _add36r; /* +_add36ii */
        double _add37r = 0, _add37i = 0;
        _add37r = angle_part1 + angle_part2; _add37i = 0 + 0;
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

static void poly_643_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 10; k++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_attr1r, _attr1i, k, &_pow2r, &_pow2i);
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2i; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 9.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - k; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_attr3r, _attr3i, _sub5r, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _abs9r; _add10i = _abs8i + _abs9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _add10r + _c11r; _add12i = _add10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(k, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul15r, _mul15i, _ang16r, _ang16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log13r, _log13i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add7r + _mul19r; _add20i = _add7i + _mul19i;
        double mag = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x2r; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(k, 0, _attr22r, _attr22i, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _attr27r = 0, _attr27i = 0;
        _attr27r = x1i; _attr27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(k, 0, _attr27r, _attr27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _mul25r - _mul30r; _sub31i = _mul25i - _mul30i;
        double angle = _sub31r; /* +_sub31ii */
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, angle, 0, &_mul33r, &_mul33i);
        double _exp34r = 0, _exp34i = 0;
        c_exp2(_mul33r, _mul33i, &_exp34r, &_exp34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(mag, 0, _exp34r, _exp34i, &_mul35r, &_mul35i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_644_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2i; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 4.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(j, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_attr3r, _attr3i, _add7r, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow2r + _pow8r; _add9i = _pow2i + _pow8i;
        double part1 = _add9r; /* +_add9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul11r + _cos14r; _add15i = _mul11i + _cos14i;
        double _sin16r = 0, _sin16i = 0;
        c_sin(_add15r, _add15i, &_sin16r, &_sin16i);
        double part2 = _sin16r; /* +_sin16ii */
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
        double part3 = _log21r; /* +_log21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(part1, 0, part2, 0, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul22r + part3; _add23i = _mul22i + 0;
        double magnitude = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_ang24r, _ang24i, j, &_pow25r, &_pow25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(j, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _pow25r + _mul28r; _add29i = _pow25i + _mul28i;
        double _conj30r = 0, _conj30i = 0;
        _conj30r = x1r; _conj30i = -(x1i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(_conj30r, _conj30i); _ang31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _add29r + _ang31r; _add32i = _add29i + _ang31i;
        double _conj33r = 0, _conj33i = 0;
        _conj33r = x2r; _conj33i = -(x2i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(_conj33r, _conj33i); _ang34i = 0;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _add32r - _ang34r; _sub35i = _add32i - _ang34i;
        double angle = _sub35r; /* +_sub35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, angle, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_645_c(double x1r, double x1i, double x2r, double x2i,
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
    double real_seq[9];
    for (int _li = 0; _li < 9; _li++) {
        real_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double im_seq[9];
    for (int _li = 0; _li < 9; _li++) {
        im_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
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
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_cos18r, _cos18i, _abs19r, _abs19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul14r + _mul20r; _add21i = _mul14i + _mul20i;
        double mag_component = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, j, 0, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = degree + _c25r; _add26i = 0 + _c25i;
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _add26r - j; _sub27i = _add26i - 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _sub27r, _sub27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul23r + _mul28r; _add29i = _mul23i + _mul28i;
        double angle_component = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _arr31r = 0, _arr31i = 0;
        { int _idx = (j - 1); _arr31r = (_idx >= 0 && _idx < 9) ? real_seq[_idx] : 0.0; _arr31i = 0; }
        double _sin32r = 0, _sin32i = 0;
        c_sin(_arr31r, _arr31i, &_sin32r, &_sin32i);
        double _arr33r = 0, _arr33i = 0;
        { int _idx = (j - 1); _arr33r = (_idx >= 0 && _idx < 9) ? im_seq[_idx] : 0.0; _arr33i = 0; }
        double _cos34r = 0, _cos34i = 0;
        c_cos(_arr33r, _arr33i, &_cos34r, &_cos34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _sin32r + _cos34r; _add35i = _sin32i + _cos34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c30r, _c30i, _add35r, _add35i, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double intricate_part = _exp37r; /* +_exp37ii */
        double _mul38r = 0, _mul38i = 0;
        c_mul(mag_component, 0, intricate_part, 0, &_mul38r, &_mul38i);
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x2r; _conj39i = -(x2i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_mul38r, _mul38i, _conj39r, _conj39i, &_mul40r, &_mul40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(j, 0, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(0, 0, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul40r + _mul42r; _add43i = _mul40i + _mul42i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_646_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_abs5r, _abs5i, j, &_pow6r, &_pow6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 9.0; _c8i = 0;
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _c8r - j; _sub9i = _c8i - 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_abs7r, _abs7i, _sub9r, &_pow10r, &_pow10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _pow6r + _pow10r; _add11i = _pow6i + _pow10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _add11r + _c12r; _add13i = _add11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(j, 0, &_sin15r, &_sin15i);
        double _pow16r = 0, _pow16i = 0;
        c_powr(j, 0, _sin15r, &_pow16r, &_pow16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(j, 0, &_cos17r, &_cos17i);
        double _pow18r = 0, _pow18i = 0;
        c_powr(j, 0, _cos17r, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _pow16r + _pow18r; _add19i = _pow16i + _pow18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log14r, _log14i, _add19r, _add19i, &_mul20r, &_mul20i);
        double magnitude = _mul20r; /* +_mul20ii */
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
        _c29r = 3.0; _c29i = 0;
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
        c_mul(magnitude, 0, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int k = 1; k < 10; k++) {
        double _cf40r = 0, _cf40i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 0.1; _c42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, k, 0, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _c41r + _mul43r; _add44i = _c41i + _mul43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_cf40r, _cf40i, _add44r, _add44i, &_mul45r, &_mul45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 1.0; _c46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 0.05; _c47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 2.0; _c48i = 0;
        double _pow49r = 0, _pow49i = 0;
        c_mul(k, 0, k, 0, &_pow49r, &_pow49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c47r, _c47i, _pow49r, _pow49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _c46r + _mul50r; _add51i = _c46i + _mul50i;
        double _div52r = 0, _div52i = 0;
        c_div(_mul45r, _mul45i, _add51r, _add51i, &_div52r, &_div52i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _div52r; cIm[_idx] = _div52i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_647_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 9; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double j = _add2r; /* +_add2ii */
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x2r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr4r, _attr4i, k, 0, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _attr3r + _mul5r; _add6i = _attr3i + _mul5i;
        double r = _add6r; /* +_add6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1i; _attr7i = 0;
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_attr8r, _attr8i, k, 0, &_mul9r, &_mul9i);
        double _sub10r = 0, _sub10i = 0;
        _sub10r = _attr7r - _mul9r; _sub10i = _attr7i - _mul9i;
        double im = _sub10r; /* +_sub10ii */
        double _sin11r = 0, _sin11i = 0;
        c_sin(r, 0, &_sin11r, &_sin11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(im, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin11r, _sin11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul14r, &_mul14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(_mul14r, _mul14i); _ang15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = k + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_ang15r, _ang15i, _add17r, _add17i, &_div18r, &_div18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul13r + _div18r; _add19i = _mul13i + _div18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(r, 0, im, 0, &_mul20r, &_mul20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(_mul20r, _mul20i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _add19r - _log24r; _sub25i = _add19i - _log24i;
        double angle = _sub25r; /* +_sub25ii */
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x2r, x2i); _abs27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_abs26r, _abs26i, _abs27r, _abs27i, &_mul28r, &_mul28i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(_mul28r, _mul28i, k, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = r + im; _add30i = 0 + 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _add30r + k; _add31i = _add30i + 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _pow29r + _add31r; _add32i = _pow29i + _add31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(r, 0, im, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = k + _c34r; _add35i = 0 + _c34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_mul33r, _mul33i, _add35r, _add35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add32r + _mul36r; _add37i = _add32i + _mul36i;
        double mag = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, angle, 0, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(mag, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
        { int _idx = k; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_648_c(double x1r, double x1i, double x2r, double x2i,
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
        _c3r = 0.0; _c3i = 0;
        double sum_real = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double sum_imag = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
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
            double _ang12r = 0, _ang12i = 0;
            _ang12r = c_arg(x2r, x2i); _ang12i = 0;
            double _mul13r = 0, _mul13i = 0;
            c_mul(k, 0, _ang12r, _ang12i, &_mul13r, &_mul13i);
            double _sin14r = 0, _sin14i = 0;
            c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
            double _mul15r = 0, _mul15i = 0;
            c_mul(_log11r, _log11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
            double term_real = _mul15r; /* +_mul15ii */
            double _abs16r = 0, _abs16i = 0;
            _abs16r = c_abs(x2r, x2i); _abs16i = 0;
            double _sub17r = 0, _sub17i = 0;
            _sub17r = degree - j; _sub17i = 0 - 0;
            double _add18r = 0, _add18i = 0;
            _add18r = _sub17r + k; _add18i = _sub17i + 0;
            double _mul19r = 0, _mul19i = 0;
            c_mul(_abs16r, _abs16i, _add18r, _add18i, &_mul19r, &_mul19i);
            double _c20r = 0, _c20i = 0;
            _c20r = 1.0; _c20i = 0;
            double _add21r = 0, _add21i = 0;
            _add21r = _mul19r + _c20r; _add21i = _mul19i + _c20i;
            double _log22r = 0, _log22i = 0;
            c_log(_add21r, _add21i, &_log22r, &_log22i);
            double _ang23r = 0, _ang23i = 0;
            _ang23r = c_arg(x1r, x1i); _ang23i = 0;
            double _mul24r = 0, _mul24i = 0;
            c_mul(k, 0, _ang23r, _ang23i, &_mul24r, &_mul24i);
            double _cos25r = 0, _cos25i = 0;
            c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
            double _mul26r = 0, _mul26i = 0;
            c_mul(_log22r, _log22i, _cos25r, _cos25i, &_mul26r, &_mul26i);
            double term_imag = _mul26r; /* +_mul26ii */
            sum_real += term_real;
            sum_imag += term_imag;
        }
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_mul(sum_real, 0, sum_real, 0, &_pow28r, &_pow28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_mul(sum_imag, 0, sum_imag, 0, &_pow30r, &_pow30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _pow28r + _pow30r; _add31i = _pow28i + _pow30i;
        double magnitude = _add31r; /* +_add31ii */
        double _c32r = 0, _c32i = 0;
        _c32r = 1e-08; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = sum_imag + _c32r; _add33i = 0 + _c32i;
        double _div34r = 0, _div34i = 0;
        c_div(sum_real, 0, _add33r, _add33i, &_div34r, &_div34i);
        double angle = _div34r; /* +_div34ii */
        double _sqrt35r = 0, _sqrt35i = 0;
        c_powr(magnitude, 0, 0.5, &_sqrt35r, &_sqrt35i);
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
        c_mul(_sqrt35r, _sqrt35i, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_649_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.5; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_add10r, _add10i, 1.5, &_pow12r, &_pow12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log8r, _log8i, _pow12r, _pow12i, &_mul13r, &_mul13i);
        double mag_part1 = _mul13r; /* +_mul13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = degree - j; _sub18i = 0 - 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _sub18r + _c19r; _add20i = _sub18i + _c19i;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.2; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_add20r, _add20i, 1.2, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log17r, _log17i, _pow22r, _pow22i, &_mul23r, &_mul23i);
        double mag_part2 = _mul23r; /* +_mul23ii */
        double _add24r = 0, _add24i = 0;
        _add24r = mag_part1 + mag_part2; _add24i = 0 + 0;
        double magnitude = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _ang25r, _ang25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double angle_part1 = _sin27r; /* +_sin27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, _ang28r, _ang28i, &_mul29r, &_mul29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_mul29r, _mul29i, &_cos30r, &_cos30i);
        double angle_part2 = _cos30r; /* +_cos30ii */
        double _add31r = 0, _add31i = 0;
        _add31r = angle_part1 + angle_part2; _add31i = 0 + 0;
        double angle = _add31r; /* +_add31ii */
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, angle, 0, &_mul33r, &_mul33i);
        double _exp34r = 0, _exp34i = 0;
        c_exp2(_mul33r, _mul33i, &_exp34r, &_exp34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(magnitude, 0, _exp34r, _exp34i, &_mul35r, &_mul35i);
        double _conj36r = 0, _conj36i = 0;
        _conj36r = x1r; _conj36i = -(x1i);
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x2r; _conj37i = -(x2i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_conj36r, _conj36i, _conj37r, _conj37i, &_mul38r, &_mul38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_mul38r, _mul38i, j, 0, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul35r + _mul39r; _add40i = _mul35i + _mul39i;
        { int _idx = ((int)(k) - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_650_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul9r + _cos12r; _add13i = _mul9i + _cos12i;
        double mag_factor = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _sqrt15r = 0, _sqrt15i = 0;
        c_powr(j, 0, 0.5, &_sqrt15r, &_sqrt15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang14r, _ang14i, _sqrt15r, _sqrt15i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _div20r = 0, _div20i = 0;
        c_div(_ang17r, _ang17i, _add19r, _add19i, &_div20r, &_div20i);
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _mul16r - _div20r; _sub21i = _mul16i - _div20i;
        double _sin22r = 0, _sin22i = 0;
        c_sin(j, 0, &_sin22r, &_sin22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _sub21r + _sin22r; _add23i = _sub21i + _sin22i;
        double angle_factor = _add23r; /* +_add23ii */
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, angle_factor, 0, &_mul25r, &_mul25i);
        double _exp26r = 0, _exp26i = 0;
        c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag_factor, 0, _exp26r, _exp26i, &_mul27r, &_mul27i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_651_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(_arr5r, _arr5i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(j, 0, &_cos10r, &_cos10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log9r, _log9i, _cos10r, _cos10i, &_mul11r, &_mul11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_abs12r, _abs12i, j, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul11r + _pow13r; _add14i = _mul11i + _pow13i;
        double mag = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _arr17r = 0, _arr17i = 0;
        { int _idx = (j - 1); _arr17r = (_idx >= 0 && _idx < 9) ? imc[_idx] : 0.0; _arr17i = 0; }
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul16r, _mul16i, _arr17r, _arr17i, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang15r, _ang15i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _arr23r = 0, _arr23i = 0;
        { int _idx = (j - 1); _arr23r = (_idx >= 0 && _idx < 9) ? rec[_idx] : 0.0; _arr23i = 0; }
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul22r, _mul22i, _arr23r, _arr23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang21r, _ang21i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul20r + _mul26r; _add27i = _mul20i + _mul26i;
        double ang = _add27r; /* +_add27ii */
        double _cos28r = 0, _cos28i = 0;
        c_cos(ang, 0, &_cos28r, &_cos28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(ang, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c29r, _c29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _cos28r + _mul31r; _add32i = _cos28i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(mag, 0, _add32r, _add32i, &_mul33r, &_mul33i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_652_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _sin2r = 0, _sin2i = 0;
        c_sin(j, 0, &_sin2r, &_sin2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(_ang1r, _ang1i, _sin2r, _sin2i, &_mul3r, &_mul3i);
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x2r, x2i); _ang4i = 0;
        double _cos5r = 0, _cos5i = 0;
        c_cos(j, 0, &_cos5r, &_cos5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_ang4r, _ang4i, _cos5r, _cos5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _mul3r + _mul6r; _add7i = _mul3i + _mul6i;
        double angle_part = _add7r; /* +_add7ii */
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_powr(_abs8r, _abs8i, j, &_pow9r, &_pow9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log13r, _log13i, j, 0, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _pow9r + _mul14r; _add15i = _pow9i + _mul14i;
        double magnitude_part = _add15r; /* +_add15ii */
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _attr19r, _attr19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _sin18r + _cos21r; _add22i = _sin18i + _cos21i;
        double phase_variation = _add22r; /* +_add22ii */
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_attr23r, _attr23i, magnitude_part, 0, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(angle_part, 0, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul24r, _mul24i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul26r + phase_variation; _add27i = _mul26i + 0;
        double real_component = _add27r; /* +_add27ii */
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x2i; _attr28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_attr28r, _attr28i, magnitude_part, 0, &_mul29r, &_mul29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(angle_part, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_mul29r, _mul29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul31r + phase_variation; _add32i = _mul31i + 0;
        double imag_component = _add32r; /* +_add32ii */
        double _cplx33r = 0, _cplx33i = 0;
        _cplx33r = real_component; _cplx33i = imag_component;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _cplx33r; cIm[_idx] = _cplx33i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_653_c(double x1r, double x1i, double x2r, double x2i,
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
    double rec_seq[9];
    for (int _li = 0; _li < 9; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 8.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[9];
    for (int _li = 0; _li < 9; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 8.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 0;
        double mag = _c7r; /* +_c7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        double ang = _c8r; /* +_c8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = k + _c9r; _add10i = 0 + _c9i;
        for (int j = 1; j < (int)(_add10r); j++) {
            double _arr11r = 0, _arr11i = 0;
            { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 9) ? rec_seq[_idx] : 0.0; _arr11i = 0; }
            double _mul12r = 0, _mul12i = 0;
            c_mul(_arr11r, _arr11i, j, 0, &_mul12r, &_mul12i);
            double _sin13r = 0, _sin13i = 0;
            c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
            double _arr14r = 0, _arr14i = 0;
            { int _idx = (j - 1); _arr14r = (_idx >= 0 && _idx < 9) ? imc_seq[_idx] : 0.0; _arr14i = 0; }
            double _mul15r = 0, _mul15i = 0;
            c_mul(_arr14r, _arr14i, j, 0, &_mul15r, &_mul15i);
            double _cos16r = 0, _cos16i = 0;
            c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_sin13r, _sin13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
            mag += _mul17r;
            double _arr18r = 0, _arr18i = 0;
            { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 9) ? rec_seq[_idx] : 0.0; _arr18i = 0; }
            double _arr19r = 0, _arr19i = 0;
            { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 9) ? imc_seq[_idx] : 0.0; _arr19i = 0; }
            double _c20r = 0, _c20i = 0;
            _c20r = 0.0; _c20i = 1.0;
            double _mul21r = 0, _mul21i = 0;
            c_mul(_arr19r, _arr19i, _c20r, _c20i, &_mul21r, &_mul21i);
            double _add22r = 0, _add22i = 0;
            _add22r = _arr18r + _mul21r; _add22i = _arr18i + _mul21i;
            double _ang23r = 0, _ang23i = 0;
            _ang23r = c_arg(_add22r, _add22i); _ang23i = 0;
            double _mul24r = 0, _mul24i = 0;
            c_mul(_ang23r, _ang23i, j, 0, &_mul24r, &_mul24i);
            ang += _mul24r;
        }
        double _arr25r = 0, _arr25i = 0;
        { int _idx = (k - 1); _arr25r = (_idx >= 0 && _idx < 9) ? rec_seq[_idx] : 0.0; _arr25i = 0; }
        double _arr26r = 0, _arr26i = 0;
        { int _idx = (k - 1); _arr26r = (_idx >= 0 && _idx < 9) ? imc_seq[_idx] : 0.0; _arr26i = 0; }
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_arr26r, _arr26i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _arr25r + _mul28r; _add29i = _arr25i + _mul28i;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(_add29r, _add29i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _abs30r + _c31r; _add32i = _abs30i + _c31i;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(mag, 0, _log33r, _log33i, &_mul34r, &_mul34i);
        mag = _mul34r;
        double _div35r = 0, _div35i = 0;
        c_div(ang, 0, k, 0, &_div35r, &_div35i);
        ang = _div35r;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_654_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 9; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 3.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mod2r + _c3r; _add4i = _mod2i + _c3i;
        double k = _add4r; /* +_add4ii */
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_abs6r, _abs6i, j, 0, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _abs5r + _mul7r; _add8i = _abs5i + _mul7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.5; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_powr(j, 0, 1.5, &_pow11r, &_pow11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log9r, _log9i, _pow11r, _pow11i, &_mul12r, &_mul12i);
        double r = _mul12r; /* +_mul12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_ang13r, _ang13i, k, &_pow14r, &_pow14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_powr(_ang15r, _ang15i, j, &_pow16r, &_pow16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _pow14r - _pow16r; _sub17i = _pow14i - _pow16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _sub17r + _sin21r; _add22i = _sub17i + _sin21i;
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
        c_mul(r, 0, _add27r, _add27i, &_mul28r, &_mul28i);
        { int _idx = j; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_655_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double mag = 0;
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        if (_mod2r == _c3r) {
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
            double _sqrt9r = 0, _sqrt9i = 0;
            c_powr(j, 0, 0.5, &_sqrt9r, &_sqrt9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(_log8r, _log8i, _sqrt9r, _sqrt9i, &_mul10r, &_mul10i);
            mag = _mul10r;
        } else {
            double _abs11r = 0, _abs11i = 0;
            _abs11r = c_abs(x2r, x2i); _abs11i = 0;
            double _pow12r = 0, _pow12i = 0;
            c_powr(_abs11r, _abs11i, j, &_pow12r, &_pow12i);
            double _c13r = 0, _c13i = 0;
            _c13r = 1.0; _c13i = 0;
            double _add14r = 0, _add14i = 0;
            _add14r = _c13r + j; _add14i = _c13i + 0;
            double _div15r = 0, _div15i = 0;
            c_div(_pow12r, _pow12i, _add14r, _add14i, &_div15r, &_div15i);
            mag = _div15r;
        }
        double angle = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _mod17r = 0, _mod17i = 0;
        _mod17r = fmod(j, _c16r); _mod17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 0;
        if (_mod17r == _c18r) {
            double _ang19r = 0, _ang19i = 0;
            _ang19r = c_arg(x1r, x1i); _ang19i = 0;
            double _sin20r = 0, _sin20i = 0;
            c_sin(j, 0, &_sin20r, &_sin20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(_ang19r, _ang19i, _sin20r, _sin20i, &_mul21r, &_mul21i);
            double _ang22r = 0, _ang22i = 0;
            _ang22r = c_arg(x2r, x2i); _ang22i = 0;
            double _mul23r = 0, _mul23i = 0;
            c_mul(j, 0, _ang22r, _ang22i, &_mul23r, &_mul23i);
            double _cos24r = 0, _cos24i = 0;
            c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
            double _add25r = 0, _add25i = 0;
            _add25r = _mul21r + _cos24r; _add25i = _mul21i + _cos24i;
            angle = _add25r;
        } else {
            double _ang26r = 0, _ang26i = 0;
            _ang26r = c_arg(x1r, x1i); _ang26i = 0;
            double _mul27r = 0, _mul27i = 0;
            c_mul(j, 0, _ang26r, _ang26i, &_mul27r, &_mul27i);
            double _sin28r = 0, _sin28i = 0;
            c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
            double _ang29r = 0, _ang29i = 0;
            _ang29r = c_arg(x2r, x2i); _ang29i = 0;
            double _mul30r = 0, _mul30i = 0;
            c_mul(j, 0, _ang29r, _ang29i, &_mul30r, &_mul30i);
            double _cos31r = 0, _cos31i = 0;
            c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
            double _sub32r = 0, _sub32i = 0;
            _sub32r = _sin28r - _cos31r; _sub32i = _sin28i - _cos31i;
            angle = _sub32r;
        }
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c33r, _c33i, angle, 0, &_mul34r, &_mul34i);
        double _exp35r = 0, _exp35i = 0;
        c_exp2(_mul34r, _mul34i, &_exp35r, &_exp35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag, 0, _exp35r, _exp35i, &_mul36r, &_mul36i);
        double _attr37r = 0, _attr37i = 0;
        _attr37r = x1r; _attr37i = 0;
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x2i; _attr38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_attr38r, _attr38i, j, 0, &_div39r, &_div39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _attr37r + _div39r; _add40i = _attr37i + _div39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_mul36r, _mul36i, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_656_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x1r, x1i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log4r, _log4i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double mag_part1 = _mul8r; /* +_mul8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log12r, _log12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double mag_part2 = _mul16r; /* +_mul16ii */
        double _add17r = 0, _add17i = 0;
        _add17r = mag_part1 + mag_part2; _add17i = 0 + 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_mul(j, 0, j, 0, &_pow19r, &_pow19i);
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x2r, x2i); _abs21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _abs21r; _add22i = _abs20i + _abs21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _add22r + _c23r; _add24i = _add22i + _c23i;
        double _div25r = 0, _div25i = 0;
        c_div(_pow19r, _pow19i, _add24r, _add24i, &_div25r, &_div25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _add17r + _div25r; _add26i = _add17i + _div25i;
        double magnitude = _add26r; /* +_add26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _cos28r = 0, _cos28i = 0;
        c_cos(j, 0, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang27r, _ang27i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double angle_part1 = _mul29r; /* +_mul29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(j, 0, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang30r, _ang30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double angle_part2 = _mul32r; /* +_mul32ii */
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x1r; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _attr36r = 0, _attr36i = 0;
        _attr36r = x2i; _attr36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, _attr36r, _attr36i, &_mul37r, &_mul37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_mul37r, _mul37i, &_cos38r, &_cos38i);
        double _sub39r = 0, _sub39i = 0;
        _sub39r = _sin35r - _cos38r; _sub39i = _sin35i - _cos38i;
        double angle_part3 = _sub39r; /* +_sub39ii */
        double _add40r = 0, _add40i = 0;
        _add40r = angle_part1 + angle_part2; _add40i = 0 + 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _add40r + angle_part3; _add41i = _add40i + 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_657_c(double x1r, double x1i, double x2r, double x2i,
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
        double _div3r = 0, _div3i = 0;
        c_div(j, 0, degree, 0, &_div3r, &_div3i);
        double r = _div3r; /* +_div3ii */
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs4r + _abs5r; _add6i = _abs4i + _abs5i;
        double _add7r = 0, _add7i = 0;
        _add7r = _add6r + r; _add7i = _add6i + 0;
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
        double mag = _mul15r; /* +_mul15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_mul(r, 0, r, 0, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang16r, _ang16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _c21r - r; _sub22i = _c21i - 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(_sub22r, _sub22i, _sub22r, _sub22i, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang20r, _ang20i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul19r + _mul25r; _add26i = _mul19i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 3.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add26r + _cos30r; _add31i = _add26i + _cos30i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_658_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + j; _add6i = _add5i + 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _add6r + _c7r; _add8i = _add6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.5; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_add11r, _add11i, 1.5, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(_ang18r, _ang18i, &_cos19r, &_cos19i);
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _sin17r - _cos19r; _sub20i = _sin17i - _cos19i;
        double ang = _sub20r; /* +_sub20ii */
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c21r, _c21i, ang, 0, &_mul22r, &_mul22i);
        double _exp23r = 0, _exp23i = 0;
        c_exp2(_mul22r, _mul22i, &_exp23r, &_exp23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(mag, 0, _exp23r, _exp23i, &_mul24r, &_mul24i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_659_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int r = 0; r < 9; r++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(_attr1r, _attr1i, _attr1r, _attr1i, &_pow3r, &_pow3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(_pow3r, _pow3i, r, 0, &_mul4r, &_mul4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul4r + _log8r; _add9i = _mul4i + _log8i;
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(r, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _add9r + _sin12r; _add13i = _add9i + _sin12i;
        double mag = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang14r, _ang14i, r, 0, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2i; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(r, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _sub19r = 0, _sub19i = 0;
        _sub19r = _mul15r - _cos18r; _sub19i = _mul15i - _cos18i;
        double angle = _sub19r; /* +_sub19ii */
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
        { int _idx = r; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_660_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + j; _add6i = _add5i + 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _add6r + _c7r; _add8i = _add6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 5.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double mag_component = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _cos16r = 0, _cos16i = 0;
        c_cos(j, 0, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _cos16r, _cos16i, &_mul17r, &_mul17i);
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
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _mul17r - _mul22r; _sub23i = _mul17i - _mul22i;
        double angle_component = _sub23r; /* +_sub23ii */
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1r; _attr24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _mod26r = 0, _mod26i = 0;
        _mod26r = fmod(j, _c25r); _mod26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _mod26r + _c27r; _add28i = _mod26i + _c27i;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_attr24r, _attr24i, _add28r, &_pow29r, &_pow29i);
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2r; _attr30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _mod32r = 0, _mod32i = 0;
        _mod32r = fmod(j, _c31r); _mod32i = 0;
        double _sub33r = 0, _sub33i = 0;
        _sub33r = degree - _mod32r; _sub33i = 0 - _mod32i;
        double _pow34r = 0, _pow34i = 0;
        c_powr(_attr30r, _attr30i, _sub33r, &_pow34r, &_pow34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _pow29r + _pow34r; _add35i = _pow29i + _pow34i;
        double real_part = _add35r; /* +_add35ii */
        double _attr36r = 0, _attr36i = 0;
        _attr36r = x2i; _attr36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, M_PI, 0, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 4.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_attr36r, _attr36i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double imag_part = _mul41r; /* +_mul41ii */
        double _add42r = 0, _add42i = 0;
        _add42r = mag_component + real_part; _add42i = 0 + 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _add44r = 0, _add44i = 0;
        _add44r = angle_component + imag_part; _add44i = 0 + 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c43r, _c43i, _add44r, _add44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _add42r + _mul45r; _add46i = _add42i + _mul45i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_661_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 9.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_attr3r, _attr3i, _sub5r, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double r_part = _add7r; /* +_add7ii */
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
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _mul10r - _mul13r; _sub14i = _mul10i - _mul13i;
        double i_part = _sub14r; /* +_sub14ii */
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = r_part + _c15r; _add16i = 0 + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _add18r = 0, _add18i = 0;
        _add18r = x1r + x2r; _add18i = x1i + x2i;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(_add18r, _add18i); _abs19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log17r, _log17i, _abs19r, _abs19i, &_mul20r, &_mul20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(r_part, 0, &_sin21r, &_sin21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(i_part, 0, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin21r, _sin21i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
        double magnitude = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_mul(j, 0, j, 0, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang25r, _ang25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_ang29r, _ang29i, j, 0, &_div30r, &_div30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _mul28r - _div30r; _sub31i = _mul28i - _div30i;
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, _ang32r, _ang32i, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _sub31r + _sin34r; _add35i = _sub31i + _sin34i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_662_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(k, 0, _attr3r, _attr3i, &_mul4r, &_mul4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2i; _attr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(k, 0, _attr5r, _attr5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _mul4r + _cos7r; _add8i = _mul4i + _cos7i;
        double _sin9r = 0, _sin9i = 0;
        c_sin(_add8r, _add8i, &_sin9r, &_sin9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _abs11r; _add12i = _abs10i + _abs11i;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _add12r + _c13r; _add14i = _add12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _sin9r + _log15r; _add16i = _sin9i + _log15i;
        double j = _add16r; /* +_add16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, _ang20r, _ang20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_cos19r, _cos19i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1r; _attr24i = 0;
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_attr24r, _attr24i, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul23r + _mul26r; _add27i = _mul23i + _mul26i;
        double r = _add27r; /* +_add27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(j, 0, j, 0, &_pow29r, &_pow29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(r, 0, r, 0, &_pow31r, &_pow31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _pow29r + _pow31r; _add32i = _pow29i + _pow31i;
        double _sqrt33r = 0, _sqrt33i = 0;
        c_powr(_add32r, _add32i, 0.5, &_sqrt33r, &_sqrt33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = k + _c34r; _add35i = 0 + _c34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_sqrt33r, _sqrt33i, _add35r, _add35i, &_mul36r, &_mul36i);
        double magnitude = _mul36r; /* +_mul36ii */
        double _at237r = 0, _at237i = 0;
        _at237r = atan2(r, j); _at237i = 0;
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x1r; _attr38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(k, 0, _attr38r, _attr38i, &_mul39r, &_mul39i);
        double _attr40r = 0, _attr40i = 0;
        _attr40r = x2i; _attr40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_mul39r, _mul39i, _attr40r, _attr40i, &_mul41r, &_mul41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_mul41r, _mul41i, &_sin42r, &_sin42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _at237r + _sin42r; _add43i = _at237i + _sin42i;
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
        c_mul(magnitude, 0, _add48r, _add48i, &_mul49r, &_mul49i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_663_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double r = j; /* +0i */
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_attr1r, _attr1i, r, &_pow2r, &_pow2i);
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2i; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 5.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(r, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_attr3r, _attr3i, _add7r, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow2r + _pow8r; _add9i = _pow2i + _pow8i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_add9r, _add9i, _log13r, _log13i, &_mul14r, &_mul14i);
        double term1 = _mul14r; /* +_mul14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(r, 0, _ang16r, _ang16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_abs15r, _abs15i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(r, 0, &_sin20r, &_sin20i);
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2r; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin20r, _sin20i, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul19r + _mul22r; _add23i = _mul19i + _mul22i;
        double term2 = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(r, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _cos28r = 0, _cos28i = 0;
        c_cos(r, 0, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang27r, _ang27i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul26r - _mul29r; _sub30i = _mul26i - _mul29i;
        double angle = _sub30r; /* +_sub30ii */
        double _add31r = 0, _add31i = 0;
        _add31r = term1 + term2; _add31i = 0 + 0;
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
        c_mul(_add31r, _add31i, _add36r, _add36i, &_mul37r, &_mul37i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_664_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 9; j++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(j, 0, x2r, x2i, &_mul1r, &_mul1i);
        double _add2r = 0, _add2i = 0;
        _add2r = x1r + _mul1r; _add2i = x1i + _mul1i;
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(_add2r, _add2i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 4.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log6r, _log6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul12r, &_mul12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(_mul12r, _mul12i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul11r + _cos15r; _add16i = _mul11i + _cos15i;
        double mag = _add16r; /* +_add16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_ang17r, _ang17i, j, &_pow18r, &_pow18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 8.0; _c20i = 0;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _c20r - j; _sub21i = _c20i - 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_ang19r, _ang19i, _sub21r, &_pow22r, &_pow22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _pow18r - _pow22r; _sub23i = _pow18i - _pow22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sub23r + _sin27r; _add28i = _sub23i + _sin27i;
        double angle = _add28r; /* +_add28ii */
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
        { int _idx = j; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_665_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 8;
    for (int _i = 0; _i < 8; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 9; j++) {
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
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double mag = _mul7r; /* +_mul7ii */
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x2r, x2i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _cos10r + _sin13r; _add14i = _cos10i + _sin13i;
        double angle = _add14r; /* +_add14ii */
        double _cos15r = 0, _cos15i = 0;
        c_cos(angle, 0, &_cos15r, &_cos15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(angle, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c16r, _c16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _cos15r + _mul18r; _add19i = _cos15i + _mul18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(mag, 0, _add19r, _add19i, &_mul20r, &_mul20i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 8) { cRe[_idx] = _mul20r; cIm[_idx] = _mul20i; } }
    }
    for (int _i = 0; _i < 8; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_666_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 8;
    for (int _i = 0; _i < 8; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double rec1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec2 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double imc1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc2 = _attr4r; /* +_attr4ii */
    for (int j = 1; j < 9; j++) {
        double _add5r = 0, _add5i = 0;
        _add5r = rec1 + j; _add5i = 0 + 0;
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
        double r_part = _mul14r; /* +_mul14ii */
        double _sub15r = 0, _sub15i = 0;
        _sub15r = imc2 - j; _sub15i = 0 - 0;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_sub15r, _sub15i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log19r, _log19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double i_part = _mul24r; /* +_mul24ii */
        double _add25r = 0, _add25i = 0;
        _add25r = r_part + i_part; _add25i = 0 + 0;
        double magnitude = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(j, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul28r + _mul31r; _add32i = _mul28i + _mul31i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 8) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 8; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_667_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double mag_sum = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double angle_sum = _c4r; /* +_c4ii */
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
            double _add12r = 0, _add12i = 0;
            _add12r = _mul11r + j; _add12i = _mul11i + 0;
            double _sin13r = 0, _sin13i = 0;
            c_sin(_add12r, _add12i, &_sin13r, &_sin13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(_log9r, _log9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
            mag_sum += _mul14r;
            double _mul15r = 0, _mul15i = 0;
            c_mul(k, 0, M_PI, 0, &_mul15r, &_mul15i);
            double _c16r = 0, _c16i = 0;
            _c16r = 1.0; _c16i = 0;
            double _add17r = 0, _add17i = 0;
            _add17r = j + _c16r; _add17i = 0 + _c16i;
            double _div18r = 0, _div18i = 0;
            c_div(_mul15r, _mul15i, _add17r, _add17i, &_div18r, &_div18i);
            double _cos19r = 0, _cos19i = 0;
            c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
            angle_sum += _cos19r;
        }
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _c20r + j; _add21i = _c20i + 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(mag_sum, 0, _add21r, _add21i, &_mul22r, &_mul22i);
        double magnitude = _mul22r; /* +_mul22ii */
        double _conj23r = 0, _conj23i = 0;
        _conj23r = x1r; _conj23i = -(x1i);
        double _conj24r = 0, _conj24i = 0;
        _conj24r = x2r; _conj24i = -(x2i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_conj23r, _conj23i, _conj24r, _conj24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(_mul25r, _mul25i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_mul(j, 0, j, 0, &_pow28r, &_pow28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang26r, _ang26i, _pow28r, _pow28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = angle_sum + _mul29r; _add30i = 0 + _mul29i;
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
    double _conj35r = 0, _conj35i = 0;
    _conj35r = x1r; _conj35i = -(x1i);
    double _conj36r = 0, _conj36i = 0;
    _conj36r = x2r; _conj36i = -(x2i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_conj35r, _conj35i, _conj36r, _conj36i, &_mul37r, &_mul37i);
    double _unk38r = 0, _unk38i = 0;
    /* WARNING: unhandled node Slice(lower=Constant(value=0), upper=Name(id='degree', ctx=Load())) */
    double _cf39r = 0, _cf39i = 0;
    { int _idx = (int)(_unk38r); if (_idx >= 0 && _idx < 36) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
    double _abs40r = 0, _abs40i = 0;
    _abs40r = c_abs(_cf39r, _cf39i); _abs40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = _mul37r + _abs40r; _add41i = _mul37i + _abs40i;
    { int _idx = (int)(degree); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_668_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double k = j; /* +0i */
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr1r, _attr1i, _log4r, _log4i, &_mul5r, &_mul5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr6r, _attr6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul5r + _mul11r; _add12i = _mul5i + _mul11i;
        double r_part = _add12r; /* +_add12ii */
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(k, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 5.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_attr13r, _attr13i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = k + _c20r; _add21i = 0 + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr19r, _attr19i, _log22r, _log22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul18r + _mul23r; _add24i = _mul18i + _mul23i;
        double i_part = _add24r; /* +_add24ii */
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(r_part, 0, r_part, 0, &_pow26r, &_pow26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_mul(i_part, 0, i_part, 0, &_pow28r, &_pow28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _pow26r + _pow28r; _add29i = _pow26i + _pow28i;
        double _sqrt30r = 0, _sqrt30i = 0;
        c_powr(_add29r, _add29i, 0.5, &_sqrt30r, &_sqrt30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.1; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, j, 0, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _c31r + _mul33r; _add34i = _c31i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sqrt30r, _sqrt30i, _add34r, _add34i, &_mul35r, &_mul35i);
        double magnitude = _mul35r; /* +_mul35ii */
        double _at236r = 0, _at236i = 0;
        _at236r = atan2(i_part, r_part); _at236i = 0;
        double _attr37r = 0, _attr37i = 0;
        _attr37r = x2r; _attr37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, _attr37r, _attr37i, &_mul38r, &_mul38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_mul38r, _mul38i, &_cos39r, &_cos39i);
        double _attr40r = 0, _attr40i = 0;
        _attr40r = x1i; _attr40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, _attr40r, _attr40i, &_mul41r, &_mul41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_mul41r, _mul41i, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_cos39r, _cos39i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _at236r + _mul43r; _add44i = _at236i + _mul43i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_669_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_pow2r, _pow2i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2i; _attr7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(j, _c8r); _mod9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_attr7r, _attr7i, _mod9r, &_pow10r, &_pow10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(_abs13r, _abs13i, &_sin14r, &_sin14i);
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _mul12r - _sin14r; _sub15i = _mul12i - _sin14i;
        double _cos16r = 0, _cos16i = 0;
        c_cos(_sub15r, _sub15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_pow10r, _pow10i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul6r + _mul17r; _add18i = _mul6i + _mul17i;
        double term1 = _add18r; /* +_add18ii */
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _mod21r = 0, _mod21i = 0;
        _mod21r = fmod(j, _c20r); _mod21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_attr19r, _attr19i, _mod21r, &_pow22r, &_pow22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _abs25r, _abs25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        _sin27r = sin(_mul26r); _sin27i = 0;
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul24r - _sin27r; _sub28i = _mul24i - _sin27i;
        double _cos29r = 0, _cos29i = 0;
        _cos29r = cos(_sub28r); _cos29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow22r, _pow22i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double term2 = _mul30r; /* +_mul30ii */
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x1r, x1i); _abs31i = 0;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x2r, x2i); _abs32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _abs31r + _abs32r; _add33i = _abs31i + _abs32i;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _add33r + _c34r; _add35i = _add33i + _c34i;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _attr37r = 0, _attr37i = 0;
        _attr37r = x1r; _attr37i = 0;
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x2i; _attr38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_attr37r, _attr37i, _attr38r, _attr38i, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 3.0; _c40i = 0;
        double _mod41r = 0, _mod41i = 0;
        _mod41r = fmod(j, _c40r); _mod41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = _mod41r + _c42r; _add43i = _mod41i + _c42i;
        double _pow44r = 0, _pow44i = 0;
        c_powr(_mul39r, _mul39i, _add43r, &_pow44r, &_pow44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_log36r, _log36i, _pow44r, _pow44i, &_mul45r, &_mul45i);
        double term3 = _mul45r; /* +_mul45ii */
        double _add46r = 0, _add46i = 0;
        _add46r = term1 + term2; _add46i = 0 + 0;
        double _add47r = 0, _add47i = 0;
        _add47r = _add46r + term3; _add47i = _add46i + 0;
        double magnitude = _add47r; /* +_add47ii */
        double _ang48r = 0, _ang48i = 0;
        _ang48r = c_arg(x1r, x1i); _ang48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_ang48r, _ang48i, j, 0, &_mul49r, &_mul49i);
        double _ang50r = 0, _ang50i = 0;
        _ang50r = c_arg(x2r, x2i); _ang50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 10.0; _c51i = 0;
        double _sub52r = 0, _sub52i = 0;
        _sub52r = _c51r - j; _sub52i = _c51i - 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_ang50r, _ang50i, _sub52r, _sub52i, &_mul53r, &_mul53i);
        double _sub54r = 0, _sub54i = 0;
        _sub54r = _mul49r - _mul53r; _sub54i = _mul49i - _mul53i;
        double _attr55r = 0, _attr55i = 0;
        _attr55r = x2r; _attr55i = 0;
        double _mul56r = 0, _mul56i = 0;
        c_mul(j, 0, _attr55r, _attr55i, &_mul56r, &_mul56i);
        double _sin57r = 0, _sin57i = 0;
        c_sin(_mul56r, _mul56i, &_sin57r, &_sin57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _sub54r + _sin57r; _add58i = _sub54i + _sin57i;
        double angle = _add58r; /* +_add58ii */
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
        c_mul(magnitude, 0, _add63r, _add63i, &_mul64r, &_mul64i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul64r; cIm[_idx] = _mul64i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_670_c(double x1r, double x1i, double x2r, double x2i,
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
        double _sin8r = 0, _sin8i = 0;
        c_sin(j, 0, &_sin8r, &_sin8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow7r + _sin8r; _add9i = _pow7i + _sin8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log5r, _log5i, _add9r, _add9i, &_mul10r, &_mul10i);
        double magnitude = _mul10r; /* +_mul10ii */
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_ang11r, _ang11i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x2r, x2i); _ang14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(j, 0, j, 0, &_pow16r, &_pow16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_pow16r, _pow16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang14r, _ang14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul13r + _mul18r; _add19i = _mul13i + _mul18i;
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
        double _conj26r = 0, _conj26i = 0;
        _conj26r = x1r; _conj26i = -(x1i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_conj26r, _conj26i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _mul28r; _add29i = _mul25i + _mul28i;
        double _conj30r = 0, _conj30i = 0;
        _conj30r = x2r; _conj30i = -(x2i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(j, 0, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_conj30r, _conj30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _add29r - _mul32r; _sub33i = _add29i - _mul32i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _sub33r; cIm[_idx] = _sub33i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_671_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul4r = 0, _mul4i = 0;
        c_mul(_attr3r, _attr3i, j, 0, &_mul4r, &_mul4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2r; _attr5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = degree - j; _sub6i = 0 - 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr5r, _attr5i, _sub6r, _sub6i, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _mul4r + _mul7r; _add8i = _mul4i + _mul7i;
        double real_part = _add8r; /* +_add8ii */
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1i; _attr9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr9r, _attr9i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _cos13r = 0, _cos13i = 0;
        c_cos(j, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr12r, _attr12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _mul11r - _mul14r; _sub15i = _mul11i - _mul14i;
        double imag_part = _sub15r; /* +_sub15ii */
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
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log22r, _log22i, _add24r, _add24i, &_mul25r, &_mul25i);
        double magnitude = _mul25r; /* +_mul25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _cos27r = 0, _cos27i = 0;
        c_cos(j, 0, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(j, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul28r + _mul31r; _add32i = _mul28i + _mul31i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _add40r = 0, _add40i = 0;
    _add40r = degree + _c39r; _add40i = 0 + _c39i;
    for (int k = 1; k < (int)(_add40r); k++) {
        double _cf41r = 0, _cf41i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(k, 0, M_PI, 0, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 4.0; _c44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(_mul43r, _mul43i, _c44r, _c44i, &_div45r, &_div45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_div45r, _div45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c42r, _c42i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _exp48r = 0, _exp48i = 0;
        c_exp2(_mul47r, _mul47i, &_exp48r, &_exp48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_cf41r, _cf41i, _exp48r, _exp48i, &_mul49r, &_mul49i);
        double _cf50r = 0, _cf50i = 0;
        { int _idx = (((int)(degree) + 1) - k); if (_idx >= 0 && _idx < 36) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
        double _conj51r = 0, _conj51i = 0;
        _conj51r = _cf50r; _conj51i = -(_cf50i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul49r + _conj51r; _add52i = _mul49i + _conj51i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_672_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _sin8r + _cos11r; _add12i = _sin8i + _cos11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log5r, _log5i, _add12r, _add12i, &_mul13r, &_mul13i);
        double magnitude = _mul13r; /* +_mul13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _sqrt15r = 0, _sqrt15i = 0;
        c_powr(j, 0, 0.5, &_sqrt15r, &_sqrt15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang14r, _ang14i, _sqrt15r, _sqrt15i, &_mul16r, &_mul16i);
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
        double angle = _add21r; /* +_add21ii */
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c22r, _c22i, angle, 0, &_mul23r, &_mul23i);
        double _exp24r = 0, _exp24i = 0;
        c_exp2(_mul23r, _mul23i, &_exp24r, &_exp24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(magnitude, 0, _exp24r, _exp24i, &_mul25r, &_mul25i);
        double _conj26r = 0, _conj26i = 0;
        _conj26r = x1r; _conj26i = -(x1i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(j, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_conj26r, _conj26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _mul28r; _add29i = _mul25i + _mul28i;
        double _conj30r = 0, _conj30i = 0;
        _conj30r = x2r; _conj30i = -(x2i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(j, 0, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_conj30r, _conj30i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _add29r - _mul32r; _sub33i = _add29i - _mul32i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub33r; cIm[_idx] = _sub33i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_673_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _abs4r, _abs4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _abs3r + _mul5r; _add6i = _abs3i + _mul5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _add6r + _c7r; _add8i = _add6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.5; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_add11r, _add11i, 1.5, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double mag = _mul14r; /* +_mul14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_powr(_ang15r, _ang15i, j, &_pow16r, &_pow16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _pow16r - _sin19r; _sub20i = _pow16i - _sin19i;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1r; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x2i; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul22r, _mul22i, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _sub20r + _cos25r; _add26i = _sub20i + _cos25i;
        double angle = _add26r; /* +_add26ii */
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
        c_mul(mag, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_674_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    for (int j = 0; j < 9; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs1r + _c2r; _add3i = _abs1i + _c2i;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log4r, _log4i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double mag_part1 = _mul8r; /* +_mul8ii */
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 0.5; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs12r, _abs12i, 0.5, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_cos11r, _cos11i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double mag_part2 = _mul15r; /* +_mul15ii */
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _mod17r = 0, _mod17i = 0;
        _mod17r = fmod(j, _c16r); _mod17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 0;
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x1r; _attr19i = 0;
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2r; _attr20i = 0;
        double _cos21r = 0, _cos21i = 0;
        c_cos(_attr20r, _attr20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_attr19r, _attr19i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1i; _attr23i = 0;
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2r; _attr24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _attr23r + _attr24r; _add25i = _attr23i + _attr24i;
        double _tern26r = 0, _tern26i = 0;
        if (_mod17r == _c18r) { _tern26r = _mul22r; _tern26i = _mul22i; }
        else { _tern26r = _add25r; _tern26i = _add25i; }
        double mag_part3 = _tern26r; /* +_tern26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + mag_part2; _add27i = 0 + 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _add27r + mag_part3; _add28i = _add27i + 0;
        double magnitude = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(_ang29r, _ang29i, j, &_pow30r, &_pow30i);
        double angle_part1 = _pow30r; /* +_pow30ii */
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x2r; _attr31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_attr31r, _attr31i, j, 0, &_mul32r, &_mul32i);
        double angle_part2 = _mul32r; /* +_mul32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 3.0; _c33i = 0;
        double _mod34r = 0, _mod34i = 0;
        _mod34r = fmod(j, _c33r); _mod34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 0;
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x1r, x1i); _ang36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, _ang36r, _ang36i, &_mul37r, &_mul37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_mul37r, _mul37i, &_sin38r, &_sin38i);
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x2r, x2i); _ang39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, _ang39r, _ang39i, &_mul40r, &_mul40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_mul40r, _mul40i, &_cos41r, &_cos41i);
        double _tern42r = 0, _tern42i = 0;
        if (_mod34r == _c35r) { _tern42r = _sin38r; _tern42i = _sin38i; }
        else { _tern42r = _cos41r; _tern42i = _cos41i; }
        double angle_part3 = _tern42r; /* +_tern42ii */
        double _add43r = 0, _add43i = 0;
        _add43r = angle_part1 + angle_part2; _add43i = 0 + 0;
        double _add44r = 0, _add44i = 0;
        _add44r = _add43r + angle_part3; _add44i = _add43i + 0;
        double angle = _add44r; /* +_add44ii */
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, angle, 0, &_mul46r, &_mul46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(magnitude, 0, _exp47r, _exp47i, &_mul48r, &_mul48i);
        { int _idx = j; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_675_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_sum = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double angle_sum = _c4r; /* +_c4ii */
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
            double _add12r = 0, _add12i = 0;
            _add12r = _mul11r + j; _add12i = _mul11i + 0;
            double _sin13r = 0, _sin13i = 0;
            c_sin(_add12r, _add12i, &_sin13r, &_sin13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(_log9r, _log9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
            mag_sum += _mul14r;
            double _sub15r = 0, _sub15i = 0;
            _sub15r = x1r - x2r; _sub15i = x1i - x2i;
            double _ang16r = 0, _ang16i = 0;
            _ang16r = c_arg(_sub15r, _sub15i); _ang16i = 0;
            double _mul17r = 0, _mul17i = 0;
            c_mul(k, 0, _ang16r, _ang16i, &_mul17r, &_mul17i);
            double _cos18r = 0, _cos18i = 0;
            c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
            angle_sum += _cos18r;
        }
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _mod20r = 0, _mod20i = 0;
        _mod20r = fmod(j, _c19r); _mod20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _mod20r + _c21r; _add22i = _mod20i + _c21i;
        for (int r = 1; r < (int)(_add22r); r++) {
            double _attr23r = 0, _attr23i = 0;
            _attr23r = x1r; _attr23i = 0;
            double _attr24r = 0, _attr24i = 0;
            _attr24r = x2i; _attr24i = 0;
            double _prod25r = 0, _prod25i = 0;
            c_mul(_attr23r, _attr23i, _attr24r, _attr24i, &_prod25r, &_prod25i);
            double _prod26r = 0, _prod26i = 0;
            c_mul(_prod25r, _prod25i, r, 0, &_prod26r, &_prod26i);
            mag_sum += _prod26r;
            double _conj27r = 0, _conj27i = 0;
            _conj27r = x1r; _conj27i = -(x1i);
            double _conj28r = 0, _conj28i = 0;
            _conj28r = x2r; _conj28i = -(x2i);
            double _mul29r = 0, _mul29i = 0;
            c_mul(_conj27r, _conj27i, _conj28r, _conj28i, &_mul29r, &_mul29i);
            double _ang30r = 0, _ang30i = 0;
            _ang30r = c_arg(_mul29r, _mul29i); _ang30i = 0;
            double _mul31r = 0, _mul31i = 0;
            c_mul(_ang30r, _ang30i, r, 0, &_mul31r, &_mul31i);
            angle_sum += _mul31r;
        }
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _c32r + j; _add33i = _c32i + 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(mag_sum, 0, _add33r, _add33i, &_mul34r, &_mul34i);
        double magnitude = _mul34r; /* +_mul34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x1r, x1i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang35r, _ang35i, j, 0, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = angle_sum + _mul36r; _add37i = 0 + _mul36i;
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 2.0; _c39i = 0;
        double _pow40r = 0, _pow40i = 0;
        c_mul(j, 0, j, 0, &_pow40r, &_pow40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang38r, _ang38i, _pow40r, _pow40i, &_mul41r, &_mul41i);
        double _sub42r = 0, _sub42i = 0;
        _sub42r = _add37r - _mul41r; _sub42i = _add37i - _mul41i;
        double angle = _sub42r; /* +_sub42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, angle, 0, &_mul44r, &_mul44i);
        double _exp45r = 0, _exp45i = 0;
        c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _exp45r, _exp45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_676_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double r = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2i; _attr2i = 0;
    double m = _attr2r; /* +_attr2ii */
    for (int j = 1; j < 10; j++) {
        double _pow3r = 0, _pow3i = 0;
        c_powr(r, 0, j, &_pow3r, &_pow3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 3.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(j, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double _pow8r = 0, _pow8i = 0;
        c_powr(m, 0, _add7r, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow3r + _pow8r; _add9i = _pow3i + _pow8i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _abs11r; _add12i = _abs10i + _abs11i;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _add12r + _c13r; _add14i = _add12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_add9r, _add9i, _log15r, _log15i, &_mul16r, &_mul16i);
        double mag = _mul16r; /* +_mul16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_ang17r, _ang17i, j, &_pow18r, &_pow18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _pow18r - _ang19r; _sub20i = _pow18i - _ang19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, r, 0, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, m, 0, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin22r, _sin22i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _sub20r + _mul25r; _add26i = _sub20i + _mul25i;
        double angle = _add26r; /* +_add26ii */
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
        c_mul(mag, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_677_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_attr3r, _attr3i, j, 0, &_mul4r, &_mul4i);
        double _div5r = 0, _div5i = 0;
        c_div(_mul4r, _mul4i, degree, 0, &_div5r, &_div5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = degree - j; _sub7i = 0 - 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _sub7r + _c8r; _add9i = _sub7i + _c8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr6r, _attr6i, _add9r, _add9i, &_mul10r, &_mul10i);
        double _div11r = 0, _div11i = 0;
        c_div(_mul10r, _mul10i, degree, 0, &_div11r, &_div11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _div5r + _div11r; _add12i = _div5i + _div11i;
        double r = _add12r; /* +_add12ii */
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
        double theta = _add19r; /* +_add19ii */
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 4.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log23r, _log23i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = degree + _c30r; _add31i = 0 + _c30i;
        double _div32r = 0, _div32i = 0;
        c_div(_mul29r, _mul29i, _add31r, _add31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul28r + _cos33r; _add34i = _mul28i + _cos33i;
        double mag = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, theta, 0, &_mul36r, &_mul36i);
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

static void poly_678_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double k = j; /* +0i */
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_ang1r, _ang1i, _log4r, _log4i, &_mul5r, &_mul5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _sin7r = 0, _sin7i = 0;
        c_sin(k, 0, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_ang6r, _ang6i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _mul8r; _add9i = _mul5i + _mul8i;
        double angle = _add9r; /* +_add9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_powr(_abs10r, _abs10i, k, &_pow11r, &_pow11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 9.0; _c13i = 0;
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _c13r - k; _sub14i = _c13i - 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs12r, _abs12i, _sub14r, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _pow11r + _pow15r; _add16i = _pow11i + _pow15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(k, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add16r + _cos20r; _add21i = _add16i + _cos20i;
        double magnitude = _add21r; /* +_add21ii */
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c22r, _c22i, angle, 0, &_mul23r, &_mul23i);
        double _exp24r = 0, _exp24i = 0;
        c_exp2(_mul23r, _mul23i, &_exp24r, &_exp24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(magnitude, 0, _exp24r, _exp24i, &_mul25r, &_mul25i);
        double _conj26r = 0, _conj26i = 0;
        _conj26r = x1r; _conj26i = -(x1i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(angle, 0, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_conj26r, _conj26i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(k, 0, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul28r, _mul28i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_679_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _add4r + _c5r; _add6i = _add4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_add9r, _add9i, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_sin12r, _sin12i); _abs13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log7r, _log7i, _abs13r, _abs13i, &_mul14r, &_mul14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs15r + degree; _add16i = _abs15i + 0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _add16r - j; _sub17i = _add16i - 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _sub17r + _c18r; _add19i = _sub17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_add22r, _add22i, _ang23r, _ang23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_cos25r, _cos25i); _abs26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log20r, _log20i, _abs26r, _abs26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul14r + _mul27r; _add28i = _mul14i + _mul27i;
        double mag_part = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = j + _c30r; _add31i = 0 + _c30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang29r, _ang29i, _add31r, _add31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _sub34r = 0, _sub34i = 0;
        _sub34r = degree - j; _sub34i = 0 - 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang33r, _ang33i, _sub34r, _sub34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul32r + _mul35r; _add36i = _mul32i + _mul35i;
        double _sin37r = 0, _sin37i = 0;
        c_sin(j, 0, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add36r + _sin37r; _add38i = _add36i + _sin37i;
        double angle_part = _add38r; /* +_add38ii */
        double _cos39r = 0, _cos39i = 0;
        c_cos(angle_part, 0, &_cos39r, &_cos39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _sin41r = 0, _sin41i = 0;
        c_sin(angle_part, 0, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c40r, _c40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cos39r + _mul42r; _add43i = _cos39i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag_part, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul45r, &_mul45i);
        double _conj46r = 0, _conj46i = 0;
        _conj46r = _mul45r; _conj46i = -(_mul45i);
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _add48r = 0, _add48i = 0;
        _add48r = j + _c47r; _add48i = 0 + _c47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_conj46r, _conj46i, _add48r, _add48i, &_mul49r, &_mul49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 1.0; _c50i = 0;
        double _add51r = 0, _add51i = 0;
        _add51r = degree + _c50r; _add51i = 0 + _c50i;
        double _div52r = 0, _div52i = 0;
        c_div(_mul49r, _mul49i, _add51r, _add51i, &_div52r, &_div52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul44r + _div52r; _add53i = _mul44i + _div52i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add53r; cIm[_idx] = _add53i; } }
    }
    double _c54r = 0, _c54i = 0;
    _c54r = 1.0; _c54i = 0;
    double _add55r = 0, _add55i = 0;
    _add55r = degree + _c54r; _add55i = 0 + _c54i;
    for (int j = 0; j < (int)(_add55r); j++) {
        double _conj56r = 0, _conj56i = 0;
        _conj56r = x1r; _conj56i = -(x1i);
        double _attr57r = 0, _attr57i = 0;
        _attr57r = x1r; _attr57i = 0;
        double _mul58r = 0, _mul58i = 0;
        c_mul(j, 0, _attr57r, _attr57i, &_mul58r, &_mul58i);
        double _sin59r = 0, _sin59i = 0;
        c_sin(_mul58r, _mul58i, &_sin59r, &_sin59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_conj56r, _conj56i, _sin59r, _sin59i, &_mul60r, &_mul60i);
        double _attr61r = 0, _attr61i = 0;
        _attr61r = x2i; _attr61i = 0;
        double _mul62r = 0, _mul62i = 0;
        c_mul(j, 0, _attr61r, _attr61i, &_mul62r, &_mul62i);
        double _cos63r = 0, _cos63i = 0;
        c_cos(_mul62r, _mul62i, &_cos63r, &_cos63i);
        double _add64r = 0, _add64i = 0;
        _add64r = _mul60r + _cos63r; _add64i = _mul60i + _cos63i;
        cRe[j] += _add64r; cIm[j] += _add64i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_680_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs5r + _c6r; _add7i = _abs5i + _c6i;
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
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _cos15r = 0, _cos15i = 0;
        c_cos(_add14r, _add14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul12r + _cos17r; _add18i = _mul12i + _cos17i;
        double magnitude = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang19r, _ang19i, j, 0, &_mul20r, &_mul20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x2r, x2i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 4.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_div27r, _div27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log24r, _log24i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul20r - _mul29r; _sub30i = _mul20i - _mul29i;
        double angle = _sub30r; /* +_sub30ii */
        double _cos31r = 0, _cos31i = 0;
        c_cos(angle, 0, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(magnitude, 0, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x1r; _attr33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 3.0; _c34i = 0;
        double _mod35r = 0, _mod35i = 0;
        _mod35r = fmod(j, _c34r); _mod35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_powr(_attr33r, _attr33i, _mod35r, &_pow36r, &_pow36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul32r + _pow36r; _add37i = _mul32i + _pow36i;
        double real_part = _add37r; /* +_add37ii */
        double _sin38r = 0, _sin38i = 0;
        c_sin(angle, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude, 0, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _attr40r = 0, _attr40i = 0;
        _attr40r = x2i; _attr40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 2.0; _c41i = 0;
        double _mod42r = 0, _mod42i = 0;
        _mod42r = fmod(j, _c41r); _mod42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = _mod42r + _c43r; _add44i = _mod42i + _c43i;
        double _pow45r = 0, _pow45i = 0;
        c_powr(_attr40r, _attr40i, _add44r, &_pow45r, &_pow45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul39r + _pow45r; _add46i = _mul39i + _pow45i;
        double imag_part = _add46r; /* +_add46ii */
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c47r, _c47i, imag_part, 0, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = real_part + _mul48r; _add49i = 0 + _mul48i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_681_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 3.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _c1r, _c1i, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mul2r + _c3r; _add4i = _mul2i + _c3i;
        double _c5r = 0, _c5i = 0;
        _c5r = 8.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(_add4r, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double k = _add8r; /* +_add8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _abs10r; _add11i = _abs9i + _abs10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log12r, _log12i, j, 0, &_mul13r, &_mul13i);
        double r = _mul13r; /* +_mul13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs14r, _abs14i, j, &_pow15r, &_pow15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_abs16r, _abs16i, k, &_pow17r, &_pow17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _pow15r + _pow17r; _add18i = _pow15i + _pow17i;
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _ang19r, _ang19i, &_mul20r, &_mul20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_mul20r, _mul20i, &_sin21r, &_sin21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(k, 0, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin21r, _sin21i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _add18r + _mul25r; _add26i = _add18i + _mul25i;
        double mag = _add26r; /* +_add26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang27r, _ang27i, j, 0, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang29r, _ang29i, k, 0, &_mul30r, &_mul30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _mul28r - _mul30r; _sub31i = _mul28i - _mul30i;
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, _ang32r, _ang32i, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _sub31r + _sin34r; _add35i = _sub31i + _sin34i;
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(k, 0, _ang36r, _ang36i, &_mul37r, &_mul37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_mul37r, _mul37i, &_cos38r, &_cos38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add35r + _cos38r; _add39i = _add35i + _cos38i;
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

static void poly_682_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 8;
    for (int _i = 0; _i < 8; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 8; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double mag = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double ang = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        for (int k = 1; k < (int)(_add4r); k++) {
            double _mul5r = 0, _mul5i = 0;
            c_mul(x1r, x1i, k, 0, &_mul5r, &_mul5i);
            double _abs6r = 0, _abs6i = 0;
            _abs6r = c_abs(_mul5r, _mul5i); _abs6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 1.0; _c7i = 0;
            double _add8r = 0, _add8i = 0;
            _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
            double _log9r = 0, _log9i = 0;
            c_log(_add8r, _add8i, &_log9r, &_log9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, M_PI, 0, &_mul10r, &_mul10i);
            double _c11r = 0, _c11i = 0;
            _c11r = 4.0; _c11i = 0;
            double _div12r = 0, _div12i = 0;
            c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
            double _sin13r = 0, _sin13i = 0;
            c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(_log9r, _log9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
            mag += _mul14r;
            double _ang15r = 0, _ang15i = 0;
            _ang15r = c_arg(x2r, x2i); _ang15i = 0;
            double _mul16r = 0, _mul16i = 0;
            c_mul(k, 0, M_PI, 0, &_mul16r, &_mul16i);
            double _c17r = 0, _c17i = 0;
            _c17r = 3.0; _c17i = 0;
            double _div18r = 0, _div18i = 0;
            c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
            double _cos19r = 0, _cos19i = 0;
            c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
            double _mul20r = 0, _mul20i = 0;
            c_mul(_ang15r, _ang15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
            ang += _mul20r;
        }
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c21r, _c21i, ang, 0, &_mul22r, &_mul22i);
        double _exp23r = 0, _exp23i = 0;
        c_exp2(_mul22r, _mul22i, &_exp23r, &_exp23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(mag, 0, _exp23r, _exp23i, &_mul24r, &_mul24i);
        double _conj25r = 0, _conj25i = 0;
        _conj25r = x1r; _conj25i = -(x1i);
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = j + _c26r; _add27i = 0 + _c26i;
        double _pow28r = 0, _pow28i = 0;
        c_powr(_conj25r, _conj25i, _add27r, &_pow28r, &_pow28i);
        double _im29r = 0, _im29i = 0;
        _im29r = x2i; _im29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow28r, _pow28i, _im29r, _im29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul24r + _mul30r; _add31i = _mul24i + _mul30i;
        { int _idx = j; if (_idx >= 0 && _idx < 8) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    for (int _i = 0; _i < 8; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_683_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
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
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
    for (int j = 0; j < (int)(_add6r); j++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.5; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_powr(j, 0, 1.5, &_pow9r, &_pow9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _abs7r + _pow9r; _add10i = _abs7i + _pow9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 3.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _c12r + _sin16r; _add17i = _c12i + _sin16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log11r, _log11i, _add17r, _add17i, &_mul18r, &_mul18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 4.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_cos22r, _cos22i, _abs23r, _abs23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul18r + _mul24r; _add25i = _mul18i + _mul24i;
        double mag = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang26r, _ang26i, j, 0, &_mul27r, &_mul27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 5.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul27r + _sin31r; _add32i = _mul27i + _sin31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _add32r - _cos36r; _sub37i = _add32i - _cos36i;
        double ang = _sub37r; /* +_sub37ii */
        double _cos38r = 0, _cos38i = 0;
        c_cos(ang, 0, &_cos38r, &_cos38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _sin40r = 0, _sin40i = 0;
        c_sin(ang, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c39r, _c39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos38r + _mul41r; _add42i = _cos38i + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(mag, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_684_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _add3r + _pow5r; _add6i = _add3i + _pow5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x1r, x1i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log7r, _log7i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _ang12r, _ang12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul11r + _cos14r; _add15i = _mul11i + _cos14i;
        double mag = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang16r, _ang16i, j, 0, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _div21r = 0, _div21i = 0;
        c_div(_ang18r, _ang18i, _add20r, _add20i, &_div21r, &_div21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _mul17r - _div21r; _sub22i = _mul17i - _div21i;
        double angle = _sub22r; /* +_sub22ii */
        double _cos23r = 0, _cos23i = 0;
        c_cos(angle, 0, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(mag, 0, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(angle, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag, 0, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul26r, _mul26i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul24r + _mul28r; _add29i = _mul24i + _mul28i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_685_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 3.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _c6r + _sin10r; _add11i = _c6i + _sin10i;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log5r, _log5i, _add11r, _add11i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 4.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _c13r + _cos17r; _add18i = _c13i + _cos17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul12r, _mul12i, _add18r, _add18i, &_mul19r, &_mul19i);
        double mag = _mul19r; /* +_mul19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _sqrt21r = 0, _sqrt21i = 0;
        c_powr(j, 0, 0.5, &_sqrt21r, &_sqrt21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang20r, _ang20i, _sqrt21r, _sqrt21i, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _div26r = 0, _div26i = 0;
        c_div(_ang23r, _ang23i, _add25r, _add25i, &_div26r, &_div26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul22r + _div26r; _add27i = _mul22i + _div26i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_686_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 8.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double r1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double im1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double r2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double im2 = _attr4r; /* +_attr4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _abs10r, _abs10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(j, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin12r, _sin12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _log9r + _mul14r; _add15i = _log9i + _mul14i;
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1r; _attr16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_attr16r, _attr16i, j, &_pow17r, &_pow17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _c18r + j; _add19i = _c18i + 0;
        double _div20r = 0, _div20i = 0;
        c_div(_pow17r, _pow17i, _add19r, _add19i, &_div20r, &_div20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add15r + _div20r; _add21i = _add15i + _div20i;
        double mag = _add21r; /* +_add21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, r1, 0, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, im2, 0, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _sin23r + _cos25r; _add26i = _sin23i + _cos25i;
        double _attr27r = 0, _attr27i = 0;
        _attr27r = x2i; _attr27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_attr27r, _attr27i, _add29r, _add29i, &_div30r, &_div30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add26r + _div30r; _add31i = _add26i + _div30i;
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

static void poly_687_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag = _log10r; /* +_log10ii */
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1i; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 9.0; _c14i = 0;
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _c14r - j; _sub15i = _c14i - 0;
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_sub15r, _sub15i, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _sin13r + _cos18r; _add19i = _sin13i + _cos18i;
        double ang = _add19r; /* +_add19ii */
        double _cos20r = 0, _cos20i = 0;
        c_cos(ang, 0, &_cos20r, &_cos20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(ang, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c21r, _c21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _cos20r + _mul23r; _add24i = _cos20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag, 0, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_688_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 3.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mod2r + _c3r; _add4i = _mod2i + _c3i;
        double k = _add4r; /* +_add4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_attr5r, _attr5i, j, 0, &_mul6r, &_mul6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2i; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr7r, _attr7i, k, 0, &_mul8r, &_mul8i);
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _mul6r - _mul8r; _sub9i = _mul6i - _mul8i;
        double _add10r = 0, _add10i = 0;
        _add10r = j + k; _add10i = 0 + 0;
        double _div11r = 0, _div11i = 0;
        c_div(_sub9r, _sub9i, _add10r, _add10i, &_div11r, &_div11i);
        double r = _div11r; /* +_div11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log15r, _log15i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1i; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul19r + _cos22r; _add23i = _mul19i + _cos22i;
        double mag = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_ang24r, _ang24i, j, &_pow25r, &_pow25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_ang26r, _ang26i, k, &_pow27r, &_pow27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _pow25r + _pow27r; _add28i = _pow25i + _pow27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, k, 0, &_mul29r, &_mul29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_mul29r, _mul29i, &_sin30r, &_sin30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add28r + _sin30r; _add31i = _add28i + _sin30i;
        double ang = _add31r; /* +_add31ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_689_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 10; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_abs1r, _abs1i, k, &_pow2r, &_pow2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x2r, x2i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 9.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - k; _sub5i = _c4i - 0;
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
        c_mul(k, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 9.0; _c14i = 0;
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _c14r - k; _sub15i = _c14i - 0;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_690_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + j; _add6i = _add5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log7r, _log7i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1i; _attr12i = 0;
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
        double ang_part = _add22r; /* +_add22ii */
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _attr23r + _attr24r; _add25i = _attr23i + _attr24i;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = j + _c26r; _add27i = 0 + _c26i;
        double _div28r = 0, _div28i = 0;
        c_div(_add25r, _add25i, _add27r, _add27i, &_div28r, &_div28i);
        double _add29r = 0, _add29i = 0;
        _add29r = mag_part + _div28r; _add29i = 0 + _div28i;
        double magnitude = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _ang30r, _ang30i, &_mul31r, &_mul31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_mul31r, _mul31i, &_sin32r, &_sin32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _ang33r, _ang33i, &_mul34r, &_mul34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_mul34r, _mul34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_sin32r, _sin32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = ang_part + _mul36r; _add37i = 0 + _mul36i;
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

static void poly_691_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 10; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _sin2r = 0, _sin2i = 0;
        c_sin(j, 0, &_sin2r, &_sin2i);
        double _pow3r = 0, _pow3i = 0;
        c_powr(j, 0, _sin2r, &_pow3r, &_pow3i);
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
        double _add9r = 0, _add9i = 0;
        _add9r = _mul4r + _div8r; _add9i = _mul4i + _div8i;
        double r_part = _add9r; /* +_add9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _cos11r = 0, _cos11i = 0;
        c_cos(j, 0, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr10r, _attr10i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2i; _attr13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(j, 0, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_attr13r, _attr13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul12r + _mul17r; _add18i = _mul12i + _mul17i;
        double im_part = _add18r; /* +_add18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs19r + j; _add20i = _abs19i + 0;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(j, 0, &_sin22r, &_sin22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_sin22r, _sin22i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log21r, _log21i, _abs23r, _abs23i, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul24r + _log28r; _add29i = _mul24i + _log28i;
        double mag = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang30r, _ang30i, j, 0, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 3.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(j, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul31r + _mul36r; _add37i = _mul31i + _mul36i;
        double angle = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, im_part, 0, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = r_part + _mul39r; _add40i = 0 + _mul39i;
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_add40r, _add40i, _exp43r, _exp43i, &_mul44r, &_mul44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_mul44r, _mul44i, mag, 0, &_mul45r, &_mul45i);
        double coeff = _mul45r; /* +_mul45ii */
        double _conj46r = 0, _conj46i = 0;
        _conj46r = x1r; _conj46i = -(x1i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(j, 0, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_conj46r, _conj46i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = coeff + _mul48r; _add49i = 0 + _mul48i;
        double _cos50r = 0, _cos50i = 0;
        c_cos(j, 0, &_cos50r, &_cos50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 2.0; _c51i = 0;
        double _div52r = 0, _div52i = 0;
        c_div(j, 0, _c51r, _c51i, &_div52r, &_div52i);
        double _sin53r = 0, _sin53i = 0;
        c_sin(_div52r, _div52i, &_sin53r, &_sin53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_cos50r, _cos50i, _sin53r, _sin53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _add49r + _mul54r; _add55i = _add49i + _mul54i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_692_c(double x1r, double x1i, double x2r, double x2i,
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
        double radius = _log10r; /* +_log10ii */
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
        c_mul(radius, 0, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_693_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 9; j++) {
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
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 4.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log5r, _log5i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 6.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul10r + _cos14r; _add15i = _mul10i + _cos14i;
        double mag = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _cos17r = 0, _cos17i = 0;
        c_cos(j, 0, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
        double ang = _add22r; /* +_add22ii */
        double _cos23r = 0, _cos23i = 0;
        c_cos(ang, 0, &_cos23r, &_cos23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(ang, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c24r, _c24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _cos23r + _mul26r; _add27i = _cos23i + _mul26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag, 0, _add27r, _add27i, &_mul28r, &_mul28i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    double _attr29r = 0, _attr29i = 0;
    _attr29r = x1r; _attr29i = 0;
    double _c30r = 0, _c30i = 0;
    _c30r = 2.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(_attr29r, _attr29i, _attr29r, _attr29i, &_pow31r, &_pow31i);
    double _attr32r = 0, _attr32i = 0;
    _attr32r = x2i; _attr32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_attr32r, _attr32i, _attr32r, _attr32i, &_pow34r, &_pow34i);
    double _sub35r = 0, _sub35i = 0;
    _sub35r = _pow31r - _pow34r; _sub35i = _pow31i - _pow34i;
    double _attr36r = 0, _attr36i = 0;
    _attr36r = x1r; _attr36i = 0;
    double _sin37r = 0, _sin37i = 0;
    c_sin(_attr36r, _attr36i, &_sin37r, &_sin37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _sub35r + _sin37r; _add38i = _sub35i + _sin37i;
    { int _idx = 8; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_694_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 5.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log4r, _log4i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double mag_part1 = _mul9r; /* +_mul9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 0.5; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs13r, _abs13i, 0.5, &_pow15r, &_pow15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_cos12r, _cos12i, _pow15r, _pow15i, &_mul16r, &_mul16i);
        double mag_part2 = _mul16r; /* +_mul16ii */
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _mod18r = 0, _mod18i = 0;
        _mod18r = fmod(j, _c17r); _mod18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.0; _c19i = 0;
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2i; _attr21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _attr20r + _attr21r; _add22i = _attr20i + _attr21i;
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2r; _attr24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _attr23r + _attr24r; _add25i = _attr23i + _attr24i;
        double _tern26r = 0, _tern26i = 0;
        if (_mod18r == _c19r) { _tern26r = _add22r; _tern26i = _add22i; }
        else { _tern26r = _add25r; _tern26i = _add25i; }
        double mag_part3 = _tern26r; /* +_tern26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + mag_part2; _add27i = 0 + 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _add27r + mag_part3; _add28i = _add27i + 0;
        double magnitude = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(_ang29r, _ang29i, j, &_pow30r, &_pow30i);
        double angle_part1 = _pow30r; /* +_pow30ii */
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x2r; _attr31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_attr31r, _attr31i, j, 0, &_mul32r, &_mul32i);
        double angle_part2 = _mul32r; /* +_mul32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 3.0; _c33i = 0;
        double _mod34r = 0, _mod34i = 0;
        _mod34r = fmod(j, _c33r); _mod34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 0;
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x1r, x1i); _ang36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, _ang36r, _ang36i, &_mul37r, &_mul37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_mul37r, _mul37i, &_sin38r, &_sin38i);
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x2r, x2i); _ang39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, _ang39r, _ang39i, &_mul40r, &_mul40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_mul40r, _mul40i, &_cos41r, &_cos41i);
        double _tern42r = 0, _tern42i = 0;
        if (_mod34r == _c35r) { _tern42r = _sin38r; _tern42i = _sin38i; }
        else { _tern42r = _cos41r; _tern42i = _cos41i; }
        double angle_part3 = _tern42r; /* +_tern42ii */
        double _add43r = 0, _add43i = 0;
        _add43r = angle_part1 + angle_part2; _add43i = 0 + 0;
        double _add44r = 0, _add44i = 0;
        _add44r = _add43r + angle_part3; _add44i = _add43i + 0;
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
        double base = _mul50r; /* +_mul50ii */
        double _attr51r = 0, _attr51i = 0;
        _attr51r = x1r; _attr51i = 0;
        double _pow52r = 0, _pow52i = 0;
        c_powr(_attr51r, _attr51i, 0, &_pow52r, &_pow52i);
        double _attr53r = 0, _attr53i = 0;
        _attr53r = x2i; _attr53i = 0;
        double _pow54r = 0, _pow54i = 0;
        c_powr(_attr53r, _attr53i, 0, &_pow54r, &_pow54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _pow52r + _pow54r; _add55i = _pow52i + _pow54i;
        double sum_part = _add55r; /* +_add55ii */
        double _add56r = 0, _add56i = 0;
        _add56r = x1r + x2r; _add56i = x1i + x2i;
        double _conj57r = 0, _conj57i = 0;
        _conj57r = _add56r; _conj57i = -(_add56i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(sum_part, 0, _conj57r, _conj57i, &_mul58r, &_mul58i);
        double additional = _mul58r; /* +_mul58ii */
        double _add59r = 0, _add59i = 0;
        _add59r = base + additional; _add59i = 0 + 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int k = 1; k < 10; k++) {
        double _cf60r = 0, _cf60i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
        double _attr61r = 0, _attr61i = 0;
        _attr61r = x1r; _attr61i = 0;
        double _mul62r = 0, _mul62i = 0;
        c_mul(k, 0, _attr61r, _attr61i, &_mul62r, &_mul62i);
        double _sin63r = 0, _sin63i = 0;
        c_sin(_mul62r, _mul62i, &_sin63r, &_sin63i);
        double _attr64r = 0, _attr64i = 0;
        _attr64r = x2i; _attr64i = 0;
        double _mul65r = 0, _mul65i = 0;
        c_mul(k, 0, _attr64r, _attr64i, &_mul65r, &_mul65i);
        double _cos66r = 0, _cos66i = 0;
        c_cos(_mul65r, _mul65i, &_cos66r, &_cos66i);
        double _add67r = 0, _add67i = 0;
        _add67r = _sin63r + _cos66r; _add67i = _sin63i + _cos66i;
        double _mul68r = 0, _mul68i = 0;
        c_mul(_cf60r, _cf60i, _add67r, _add67i, &_mul68r, &_mul68i);
        double _sub69r = 0, _sub69i = 0;
        _sub69r = x1r - x2r; _sub69i = x1i - x2i;
        double _abs70r = 0, _abs70i = 0;
        _abs70r = c_abs(_sub69r, _sub69i); _abs70i = 0;
        double _mul71r = 0, _mul71i = 0;
        c_mul(_mul68r, _mul68i, _abs70r, _abs70i, &_mul71r, &_mul71i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul71r; cIm[_idx] = _mul71i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_695_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 9; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double k = _add2r; /* +_add2ii */
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
        c_mul(k, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin10r + _cos13r; _add14i = _sin10i + _cos13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log7r, _log7i, _add14r, _add14i, &_mul15r, &_mul15i);
        double mag = _mul15r; /* +_mul15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang16r, _ang16i, j, 0, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang18r, _ang18i, k, 0, &_mul19r, &_mul19i);
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _mul17r - _mul19r; _sub20i = _mul17i - _mul19i;
        double angle = _sub20r; /* +_sub20ii */
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c21r, _c21i, angle, 0, &_mul22r, &_mul22i);
        double _exp23r = 0, _exp23i = 0;
        c_exp2(_mul22r, _mul22i, &_exp23r, &_exp23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(mag, 0, _exp23r, _exp23i, &_mul24r, &_mul24i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    }
    double _cf25r = 0, _cf25i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 9) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
    double _conj26r = 0, _conj26i = 0;
    _conj26r = _cf25r; _conj26i = -(_cf25i);
    double _sum27r = 0, _sum27i = 0;
    _sum27r = 0; _sum27i = 0;
    for (int _si = 1; _si < 8; _si++) { _sum27r += cRe[_si]; _sum27i += cIm[_si]; }
    double _c28r = 0, _c28i = 0;
    _c28r = 0.5; _c28i = 0;
    double _mul29r = 0, _mul29i = 0;
    c_mul(_sum27r, _sum27i, _c28r, _c28i, &_mul29r, &_mul29i);
    double _add30r = 0, _add30i = 0;
    _add30r = _conj26r + _mul29r; _add30i = _conj26i + _mul29i;
    { int _idx = 8; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_696_c(double x1r, double x1i, double x2r, double x2i,
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
        double _pow4r = 0, _pow4i = 0;
        c_powr(_abs3r, _abs3i, j, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = degree - j; _sub6i = 0 - 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_abs5r, _abs5i, _sub6r, &_pow7r, &_pow7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow4r + _pow7r; _add8i = _pow4i + _pow7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _add8r + _c9r; _add10i = _add8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double mag = _log11r; /* +_log11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _ang12r, _ang12i, &_mul13r, &_mul13i);
        double _sub14r = 0, _sub14i = 0;
        _sub14r = degree - j; _sub14i = 0 - 0;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sub14r, _sub14i, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul13r + _cos17r; _add18i = _mul13i + _cos17i;
        double _sin19r = 0, _sin19i = 0;
        c_sin(_add18r, _add18i, &_sin19r, &_sin19i);
        double ang = _sin19r; /* +_sin19ii */
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c20r, _c20i, ang, 0, &_mul21r, &_mul21i);
        double _exp22r = 0, _exp22i = 0;
        c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(mag, 0, _exp22r, _exp22i, &_mul23r, &_mul23i);
        double _conj24r = 0, _conj24i = 0;
        _conj24r = x1r; _conj24i = -(x1i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_conj24r, _conj24i, _add26r, _add26i, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = degree + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_mul27r, _mul27i, _add29r, _add29i, &_div30r, &_div30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul23r + _div30r; _add31i = _mul23i + _div30i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_697_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _pow10r = 0, _pow10i = 0;
        c_powr(_add8r, _add8i, _sin9r, &_pow10r, &_pow10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log6r, _log6i, _pow10r, _pow10i, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _sqrt14r = 0, _sqrt14i = 0;
        c_powr(_add13r, _add13i, 0.5, &_sqrt14r, &_sqrt14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sqrt14r, _sqrt14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul11r + _mul18r; _add19i = _mul11i + _mul18i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int k = 0; k < (int)(degree); k++) {
        double _cf33r = 0, _cf33i = 0;
        { int _idx = (k + 1); if (_idx >= 0 && _idx < 36) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = k + _c35r; _add36i = 0 + _c35i;
        double _sin37r = 0, _sin37i = 0;
        c_sin(_add36r, _add36i, &_sin37r, &_sin37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = k + _c38r; _add39i = 0 + _c38i;
        double _cos40r = 0, _cos40i = 0;
        c_cos(_add39r, _add39i, &_cos40r, &_cos40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _sin37r + _cos40r; _add41i = _sin37i + _cos40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c34r, _c34i, _add41r, _add41i, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_cf33r, _cf33i, _exp43r, _exp43i, &_mul44r, &_mul44i);
        { int _idx = (k + 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    double _abs45r = 0, _abs45i = 0;
    _abs45r = c_abs(x1r, x1i); _abs45i = 0;
    double _abs46r = 0, _abs46i = 0;
    _abs46r = c_abs(x2r, x2i); _abs46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = _abs45r + _abs46r; _add47i = _abs45i + _abs46i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    double _conj48r = 0, _conj48i = 0;
    _conj48r = x1r; _conj48i = -(x1i);
    double _conj49r = 0, _conj49i = 0;
    _conj49r = x2r; _conj49i = -(x2i);
    double _mul50r = 0, _mul50i = 0;
    c_mul(_conj48r, _conj48i, _conj49r, _conj49i, &_mul50r, &_mul50i);
    { int _idx = (int)(degree); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_698_c(double x1r, double x1i, double x2r, double x2i,
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
        double angle_part = 0;
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(angle_part, 0, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _sin13r + _cos14r; _add15i = _sin13i + _cos14i;
        angle_part = _add15r;
        double _cos16r = 0, _cos16i = 0;
        c_cos(angle_part, 0, &_cos16r, &_cos16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 0.0; _c17i = 1.0;
        double _sin18r = 0, _sin18i = 0;
        c_sin(angle_part, 0, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_c17r, _c17i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _cos16r + _mul19r; _add20i = _cos16i + _mul19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(mag_part, 0, _add20r, _add20i, &_mul21r, &_mul21i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul21r; cIm[_idx] = _mul21i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_699_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int _i = 0; _i < 9; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 9; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_attr1r, _attr1i, j, 0, &_mul2r, &_mul2i);
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 9.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_attr3r, _attr3i, _sub5r, _sub5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _mul2r + _mul6r; _add7i = _mul2i + _mul6i;
        double r = _add7r; /* +_add7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _div11r = 0, _div11i = 0;
        c_div(_attr8r, _attr8i, _add10r, _add10i, &_div11r, &_div11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 10.0; _c13i = 0;
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _c13r - j; _sub14i = _c13i - 0;
        double _div15r = 0, _div15i = 0;
        c_div(_attr12r, _attr12i, _sub14r, _sub14i, &_div15r, &_div15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = _div11r - _div15r; _sub16i = _div11i - _div15i;
        double im = _sub16r; /* +_sub16ii */
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
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(j, 0, j, 0, &_pow23r, &_pow23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log21r, _log21i, _pow23r, _pow23i, &_mul24r, &_mul24i);
        double mag = _mul24r; /* +_mul24ii */
        double _sin25r = 0, _sin25i = 0;
        c_sin(r, 0, &_sin25r, &_sin25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(im, 0, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _sin25r + _cos26r; _add27i = _sin25i + _cos26i;
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _ang29r, _ang29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add27r + _mul30r; _add31i = _add27i + _mul30i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    double _conj38r = 0, _conj38i = 0;
    _conj38r = x1r; _conj38i = -(x1i);
    double _conj39r = 0, _conj39i = 0;
    _conj39r = x2r; _conj39i = -(x2i);
    double _add40r = 0, _add40i = 0;
    _add40r = _conj38r + _conj39r; _add40i = _conj38i + _conj39i;
    double _attr41r = 0, _attr41i = 0;
    _attr41r = x1r; _attr41i = 0;
    double _attr42r = 0, _attr42i = 0;
    _attr42r = x2r; _attr42i = 0;
    double _mul43r = 0, _mul43i = 0;
    c_mul(_attr41r, _attr41i, _attr42r, _attr42i, &_mul43r, &_mul43i);
    double _sin44r = 0, _sin44i = 0;
    c_sin(_mul43r, _mul43i, &_sin44r, &_sin44i);
    double _add45r = 0, _add45i = 0;
    _add45r = _add40r + _sin44r; _add45i = _add40i + _sin44i;
    { int _idx = 8; if (_idx >= 0 && _idx < 9) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_700_c(double x1r, double x1i, double x2r, double x2i,
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
        double _sin8r = 0, _sin8i = 0;
        c_sin(j, 0, &_sin8r, &_sin8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow7r + _sin8r; _add9i = _pow7i + _sin8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log5r, _log5i, _add9r, _add9i, &_mul10r, &_mul10i);
        double magnitude = _mul10r; /* +_mul10ii */
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_ang11r, _ang11i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x2r, x2i); _ang14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_mul(j, 0, j, 0, &_pow16r, &_pow16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_pow16r, _pow16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang14r, _ang14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul13r + _mul18r; _add19i = _mul13i + _mul18i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 9) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 9; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
