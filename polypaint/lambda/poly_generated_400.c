/* AUTO-GENERATED from poly400.py — do not edit manually */
/* 100 coefficient functions */

static void poly_301_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _mul4r = 0, _mul4i = 0;
        c_mul(_re3r, _re3i, j, 0, &_mul4r, &_mul4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(_mul4r, _mul4i); _abs5i = 0;
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
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.5; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_abs18r, _abs18i, 0.5, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_cos17r, _cos17i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double mag_part2 = _mul21r; /* +_mul21ii */
        double _add22r = 0, _add22i = 0;
        _add22r = mag_part1 + mag_part2; _add22i = 0 + 0;
        double _div23r = 0, _div23i = 0;
        c_div(0, 0, 0, 0, &_div23r, &_div23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _add22r + _div23r; _add24i = _add22i + _div23i;
        double magnitude = _add24r; /* +_add24ii */
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
        double angle_part1 = _mul29r; /* +_mul29ii */
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
        double angle_part2 = _mul34r; /* +_mul34ii */
        double _add35r = 0, _add35i = 0;
        _add35r = angle_part1 + angle_part2; _add35i = 0 + 0;
        double _sin36r = 0, _sin36i = 0;
        c_sin(j, 0, &_sin36r, &_sin36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 2.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(j, 0, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_sin36r, _sin36i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _add35r + _mul40r; _add41i = _add35i + _mul40i;
        double phase = _add41r; /* +_add41ii */
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, phase, 0, &_mul43r, &_mul43i);
        double _exp44r = 0, _exp44i = 0;
        c_exp2(_mul43r, _mul43i, &_exp44r, &_exp44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(magnitude, 0, _exp44r, _exp44i, &_mul45r, &_mul45i);
        double _conj46r = 0, _conj46i = 0;
        _conj46r = x1r; _conj46i = -(x1i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(j, 0, M_PI, 0, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 6.0; _c48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(_mul47r, _mul47i, _c48r, _c48i, &_div49r, &_div49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(_div49r, _div49i, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_conj46r, _conj46i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul45r + _mul51r; _add52i = _mul45i + _mul51i;
        double _conj53r = 0, _conj53i = 0;
        _conj53r = x2r; _conj53i = -(x2i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(j, 0, M_PI, 0, &_mul54r, &_mul54i);
        double _c55r = 0, _c55i = 0;
        _c55r = 5.0; _c55i = 0;
        double _div56r = 0, _div56i = 0;
        c_div(_mul54r, _mul54i, _c55r, _c55i, &_div56r, &_div56i);
        double _cos57r = 0, _cos57i = 0;
        c_cos(_div56r, _div56i, &_cos57r, &_cos57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_conj53r, _conj53i, _cos57r, _cos57i, &_mul58r, &_mul58i);
        double _sub59r = 0, _sub59i = 0;
        _sub59r = _add52r - _mul58r; _sub59i = _add52i - _mul58i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub59r; cIm[_idx] = _sub59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_302_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr11i = 0; }
        double _arr12r = 0, _arr12i = 0;
        { int _idx = (j - 1); _arr12r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr12i = 0; }
        double _mul13r = 0, _mul13i = 0;
        c_mul(_arr11r, _arr11i, _arr12r, _arr12i, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c10r + _sin14r; _add15i = _c10i + _sin14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log9r, _log9i, _add15r, _add15i, &_mul16r, &_mul16i);
        double _re17r = 0, _re17i = 0;
        _re17r = x1r; _re17i = 0;
        double _im18r = 0, _im18i = 0;
        _im18r = x2i; _im18i = 0;
        double _prod19r = 0, _prod19i = 0;
        c_mul(_re17r, _re17i, _im18r, _im18i, &_prod19r, &_prod19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_prod19r, _prod19i, _add21r, _add21i, &_div22r, &_div22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul16r + _div22r; _add23i = _mul16i + _div22i;
        double mag = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(j, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _cos28r = 0, _cos28i = 0;
        c_cos(j, 0, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang27r, _ang27i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul26r + _mul29r; _add30i = _mul26i + _mul29i;
        double _arr31r = 0, _arr31i = 0;
        { int _idx = (j - 1); _arr31r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr31i = 0; }
        double _arr32r = 0, _arr32i = 0;
        { int _idx = (j - 1); _arr32r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr32i = 0; }
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _arr32r + _c33r; _add34i = _arr32i + _c33i;
        double _div35r = 0, _div35i = 0;
        c_div(_arr31r, _arr31i, _add34r, _add34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add30r + _sin36r; _add37i = _add30i + _sin36i;
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
        c_mul(mag, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x1r; _conj44i = -(x1i);
        double _c45r = 0, _c45i = 0;
        _c45r = 3.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(j, 0, _c45r, _c45i, &_div46r, &_div46i);
        double _cos47r = 0, _cos47i = 0;
        c_cos(_div46r, _div46i, &_cos47r, &_cos47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_conj44r, _conj44i, _cos47r, _cos47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul43r + _mul48r; _add49i = _mul43i + _mul48i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_303_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_305_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double real_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        real_seq[_li] = _re1r + (_re2r - _re1r) * _li / 34.0;
    }
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imag_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imag_seq[_li] = _im3r + (_im4r - _im3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int k = 1; k < (int)(_add6r); k++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (k - 1); _arr7r = (_idx >= 0 && _idx < 35) ? real_seq[_idx] : 0.0; _arr7i = 0; }
        double r = _arr7r; /* +_arr7ii */
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (k - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imag_seq[_idx] : 0.0; _arr8i = 0; }
        double im = _arr8r; /* +_arr8ii */
        double _mul9r = 0, _mul9i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_mul9r, _mul9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(k, 0, k, 0, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _abs10r + _pow12r; _add13i = _abs10i + _pow12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(k, 0, &_sin16r, &_sin16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(k, 0, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sin16r, _sin16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _c15r + _mul20r; _add21i = _c15i + _mul20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log14r, _log14i, _add21r, _add21i, &_mul22r, &_mul22i);
        double mag_pattern = _mul22r; /* +_mul22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(k, 0, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang23r, _ang23i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 4.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(k, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul27r + _mul32r; _add33i = _mul27i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(k, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 5.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add33r + _sin37r; _add38i = _add33i + _sin37i;
        double angle_pattern = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle_pattern, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(mag_pattern, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, angle_pattern, 0, &_mul44r, &_mul44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 2.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(_mul44r, _mul44i, _c45r, _c45i, &_div46r, &_div46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_div46r, _div46i, &_exp47r, &_exp47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(mag_pattern, 0, _exp47r, _exp47i, &_mul48r, &_mul48i);
        double _conj49r = 0, _conj49i = 0;
        _conj49r = _mul48r; _conj49i = -(_mul48i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul42r + _conj49r; _add50i = _mul42i + _conj49i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_306_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c7r = 0, _c7i = 0;
        _c7r = 5.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(j, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
        double k = _add10r; /* +_add10ii */
        double _mul11r = 0, _mul11i = 0;
        c_mul(rec1, 0, rec2, 0, &_mul11r, &_mul11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_mul11r, _mul11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul16r, &_mul16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_mul16r, _mul16i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _log15r + _log20r; _add21i = _log15i + _log20i;
        double r = _add21r; /* +_add21ii */
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
        double angle = _add28r; /* +_add28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(_abs29r, _abs29i, k, &_pow30r, &_pow30i);
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x2r, x2i); _abs31i = 0;
        double _sub32r = 0, _sub32i = 0;
        _sub32r = n - k; _sub32i = 0 - 0;
        double _pow33r = 0, _pow33i = 0;
        c_powr(_abs31r, _abs31i, _sub32r, &_pow33r, &_pow33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _pow30r + _pow33r; _add34i = _pow30i + _pow33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 7.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(r, 0, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _add34r + _mul39r; _add40i = _add34i + _mul39i;
        double magnitude = _add40r; /* +_add40ii */
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, M_PI, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 5.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(_mul41r, _mul41i, _c42r, _c42i, &_div43r, &_div43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_div43r, _div43i, &_cos44r, &_cos44i);
        double _add45r = 0, _add45i = 0;
        _add45r = angle + _cos44r; _add45i = 0 + _cos44i;
        double phase = _add45r; /* +_add45ii */
        double _cos46r = 0, _cos46i = 0;
        c_cos(phase, 0, &_cos46r, &_cos46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _sin48r = 0, _sin48i = 0;
        c_sin(phase, 0, &_sin48r, &_sin48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c47r, _c47i, _sin48r, _sin48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _cos46r + _mul49r; _add50i = _cos46i + _mul49i;
        double _mul51r = 0, _mul51i = 0;
        c_mul(magnitude, 0, _add50r, _add50i, &_mul51r, &_mul51i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
        double _c52r = 0, _c52i = 0;
        _c52r = 7.0; _c52i = 0;
        double _mod53r = 0, _mod53i = 0;
        _mod53r = fmod(j, _c52r); _mod53i = 0;
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 0;
        if (_mod53r == _c54r) {
            double _cf55r = 0, _cf55i = 0;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
            double _conj56r = 0, _conj56i = 0;
            _conj56r = x1r; _conj56i = -(x1i);
            double _mul57r = 0, _mul57i = 0;
            c_mul(_cf55r, _cf55i, _conj56r, _conj56i, &_mul57r, &_mul57i);
            double _conj58r = 0, _conj58i = 0;
            _conj58r = x2r; _conj58i = -(x2i);
            double _add59r = 0, _add59i = 0;
            _add59r = _mul57r + _conj58r; _add59i = _mul57i + _conj58i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
        }
        double _c60r = 0, _c60i = 0;
        _c60r = 3.0; _c60i = 0;
        double _mod61r = 0, _mod61i = 0;
        _mod61r = fmod(j, _c60r); _mod61i = 0;
        double _c62r = 0, _c62i = 0;
        _c62r = 0.0; _c62i = 0;
        if (_mod61r == _c62r) {
            double _mul63r = 0, _mul63i = 0;
            c_mul(x1r, x1i, j, 0, &_mul63r, &_mul63i);
            double _sin64r = 0, _sin64i = 0;
            c_sin(_mul63r, _mul63i, &_sin64r, &_sin64i);
            double _div65r = 0, _div65i = 0;
            c_div(x2r, x2i, j, 0, &_div65r, &_div65i);
            double _cos66r = 0, _cos66i = 0;
            c_cos(_div65r, _div65i, &_cos66r, &_cos66i);
            double _mul67r = 0, _mul67i = 0;
            c_mul(_sin64r, _sin64i, _cos66r, _cos66i, &_mul67r, &_mul67i);
            cRe[(j - 1)] += _mul67r; cIm[(j - 1)] += _mul67i;
        }
        double _c68r = 0, _c68i = 0;
        _c68r = 4.0; _c68i = 0;
        double _mod69r = 0, _mod69i = 0;
        _mod69r = fmod(j, _c68r); _mod69i = 0;
        double _c70r = 0, _c70i = 0;
        _c70r = 0.0; _c70i = 0;
        if (_mod69r == _c70r) {
            double _c71r = 0, _c71i = 0;
            _c71r = 0.0; _c71i = 1.0;
            double _mul72r = 0, _mul72i = 0;
            c_mul(rec1, 0, j, 0, &_mul72r, &_mul72i);
            double _sub73r = 0, _sub73i = 0;
            _sub73r = _mul72r - imc2; _sub73i = _mul72i - 0;
            double _mul74r = 0, _mul74i = 0;
            c_mul(_c71r, _c71i, _sub73r, _sub73i, &_mul74r, &_mul74i);
            double _exp75r = 0, _exp75i = 0;
            c_exp2(_mul74r, _mul74i, &_exp75r, &_exp75i);
            cRe[(j - 1)] += _exp75r; cIm[(j - 1)] += _exp75i;
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_307_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _add15r = 0, _add15i = 0;
        _add15r = _sin10r + _cos14r; _add15i = _sin10i + _cos14i;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang16r, _ang16i, j, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 10.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add15r + _div19r; _add20i = _add15i + _div19i;
        double phase = _add20r; /* +_add20ii */
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs21r + j; _add22i = _abs21i + 0;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 6.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _c24r + _sin28r; _add29i = _c24i + _sin28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_log23r, _log23i, _add29r, _add29i, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.5; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_powr(0, 0, 0.5, &_pow32r, &_pow32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 8.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_pow32r, _pow32i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul30r + _mul37r; _add38i = _mul30i + _mul37i;
        double magnitude = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, phase, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(magnitude, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        double _conj43r = 0, _conj43i = 0;
        _conj43r = x2r; _conj43i = -(x2i);
        double _c44r = 0, _c44i = 0;
        _c44r = 5.0; _c44i = 0;
        double _mod45r = 0, _mod45i = 0;
        _mod45r = fmod(j, _c44r); _mod45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_conj43r, _conj43i, _mod45r, _mod45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _mul42r + _mul46r; _add47i = _mul42i + _mul46i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_308_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(j, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr11i = 0; }
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_arr11r, _arr11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_abs10r, _abs10i, _log15r, _log15i, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul16r, _mul16i, j, 0, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr19i = 0; }
        double _sin20r = 0, _sin20i = 0;
        c_sin(_arr19r, _arr19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_abs18r, _abs18i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = x1r + x2r; _add22i = x1i + x2i;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_add22r, _add22i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _div26r = 0, _div26i = 0;
        c_div(_abs23r, _abs23i, _add25r, _add25i, &_div26r, &_div26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul21r + _div26r; _add27i = _mul21i + _div26i;
        double _tern28r = 0, _tern28i = 0;
        if (_mod8r == _c9r) { _tern28r = _mul17r; _tern28i = _mul17i; }
        else { _tern28r = _add27r; _tern28i = _add27i; }
        double magnitude = _tern28r; /* +_tern28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(n, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _arr32r = 0, _arr32i = 0;
        { int _idx = (j - 1); _arr32r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr32i = 0; }
        double _mul33r = 0, _mul33i = 0;
        c_mul(_arr32r, _arr32i, M_PI, 0, &_mul33r, &_mul33i);
        double _div34r = 0, _div34i = 0;
        c_div(_mul33r, _mul33i, j, 0, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _ang31r + _sin35r; _add36i = _ang31i + _sin35i;
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _arr38r = 0, _arr38i = 0;
        { int _idx = (j - 1); _arr38r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr38i = 0; }
        double _mul39r = 0, _mul39i = 0;
        c_mul(_arr38r, _arr38i, M_PI, 0, &_mul39r, &_mul39i);
        double _div40r = 0, _div40i = 0;
        c_div(_mul39r, _mul39i, j, 0, &_div40r, &_div40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _ang37r + _cos41r; _add42i = _ang37i + _cos41i;
        double _tern43r = 0, _tern43i = 0;
        if (j <= _div30r) { _tern43r = _add36r; _tern43i = _add36i; }
        else { _tern43r = _add42r; _tern43i = _add42i; }
        double angle = _tern43r; /* +_tern43ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    }
    double _c50r = 0, _c50i = 0;
    _c50r = 1.0; _c50i = 0;
    double _add51r = 0, _add51i = 0;
    _add51r = n + _c50r; _add51i = 0 + _c50i;
    for (int k = 1; k < (int)(_add51r); k++) {
        double _cf52r = 0, _cf52i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
        double _conj53r = 0, _conj53i = 0;
        _conj53r = _cf52r; _conj53i = -(_cf52i);
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 1.0;
        double _sin55r = 0, _sin55i = 0;
        c_sin(k, 0, &_sin55r, &_sin55i);
        double _cos56r = 0, _cos56i = 0;
        c_cos(k, 0, &_cos56r, &_cos56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _sin55r + _cos56r; _add57i = _sin55i + _cos56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(_c54r, _c54i, _add57r, _add57i, &_mul58r, &_mul58i);
        double _exp59r = 0, _exp59i = 0;
        c_exp2(_mul58r, _mul58i, &_exp59r, &_exp59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_conj53r, _conj53i, _exp59r, _exp59i, &_mul60r, &_mul60i);
        cRe[(k - 1)] += _mul60r; cIm[(k - 1)] += _mul60i;
    }
    double _c61r = 0, _c61i = 0;
    _c61r = 1.0; _c61i = 0;
    double _add62r = 0, _add62i = 0;
    _add62r = n + _c61r; _add62i = 0 + _c61i;
    for (int r = 1; r < (int)(_add62r); r++) {
        double _c63r = 0, _c63i = 0;
        _c63r = 1.0; _c63i = 0;
        double _cf64r = 0, _cf64i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf64r = cRe[_idx]; _cf64i = cIm[_idx]; } }
        double _abs65r = 0, _abs65i = 0;
        _abs65r = c_abs(_cf64r, _cf64i); _abs65i = 0;
        double _c66r = 0, _c66i = 0;
        _c66r = 1.0; _c66i = 0;
        double _add67r = 0, _add67i = 0;
        _add67r = _abs65r + _c66r; _add67i = _abs65i + _c66i;
        double _log68r = 0, _log68i = 0;
        c_log(_add67r, _add67i, &_log68r, &_log68i);
        double _add69r = 0, _add69i = 0;
        _add69r = _c63r + _log68r; _add69i = _c63i + _log68i;
        double _c70r = 0, _c70i = 0;
        _c70r = 1.0; _c70i = 0;
        double _div71r = 0, _div71i = 0;
        c_div(r, 0, n, 0, &_div71r, &_div71i);
        double _add72r = 0, _add72i = 0;
        _add72r = _c70r + _div71r; _add72i = _c70i + _div71i;
        double _div73r = 0, _div73i = 0;
        c_div(_add69r, _add69i, _add72r, _add72i, &_div73r, &_div73i);
        { double _tr = cRe[(r - 1)]*_div73r - cIm[(r - 1)]*_div73i; cIm[(r - 1)] = cRe[(r - 1)]*_div73i + cIm[(r - 1)]*_div73r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_309_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr8i = 0; }
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 1.0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_arr8r, _arr8i, _c9r, _c9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _arr7r + _mul10r; _add11i = _arr7i + _mul10i;
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
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _c16r + _sin20r; _add21i = _c16i + _sin20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log15r, _log15i, _add21r, _add21i, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 5.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _c23r + _cos27r; _add28i = _c23i + _cos27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_mul22r, _mul22i, _add28r, _add28i, &_mul29r, &_mul29i);
        double mag = _mul29r; /* +_mul29ii */
        double _arr30r = 0, _arr30i = 0;
        { int _idx = (j - 1); _arr30r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr30i = 0; }
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _arr30r, _arr30i, &_mul31r, &_mul31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_mul31r, _mul31i, &_sin32r, &_sin32i);
        double _arr33r = 0, _arr33i = 0;
        { int _idx = (j - 1); _arr33r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr33i = 0; }
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _arr33r, _arr33i, &_mul34r, &_mul34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_mul34r, _mul34i, &_cos35r, &_cos35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _sin32r + _cos35r; _add36i = _sin32i + _cos35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul37r, &_mul37i);
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(_mul37r, _mul37i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = j + _c39r; _add40i = 0 + _c39i;
        double _div41r = 0, _div41i = 0;
        c_div(_ang38r, _ang38i, _add40r, _add40i, &_div41r, &_div41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _add36r + _div41r; _add42i = _add36i + _div41i;
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
        _conj47r = x2r; _conj47i = -(x2i);
        double _div48r = 0, _div48i = 0;
        c_div(j, 0, n, 0, &_div48r, &_div48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_div48r, _div48i, M_PI, 0, &_mul49r, &_mul49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(_mul49r, _mul49i, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_conj47r, _conj47i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul46r + _mul51r; _add52i = _mul46i + _mul51i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_310_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log6r, _log6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 7.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log15r, _log15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul11r + _mul20r; _add21i = _mul11i + _mul20i;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.5; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(j, 0, 1.5, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _add21r + _pow23r; _add24i = _add21i + _pow23i;
        double mag_part = _add24r; /* +_add24ii */
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
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 6.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang31r, _ang31i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul30r + _mul36r; _add37i = _mul30i + _mul36i;
        double _c38r = 0, _c38i = 0;
        _c38r = 3.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(j, 0, _c38r, _c38i, &_div39r, &_div39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _add37r + _sin40r; _add41i = _add37i + _sin40i;
        double ang_part = _add41r; /* +_add41ii */
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, ang_part, 0, &_mul43r, &_mul43i);
        double _exp44r = 0, _exp44i = 0;
        c_exp2(_mul43r, _mul43i, &_exp44r, &_exp44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag_part, 0, _exp44r, _exp44i, &_mul45r, &_mul45i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_311_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _mul8r = 0, _mul8i = 0;
        c_mul(_arr7r, _arr7i, x1r, x1i, &_mul8r, &_mul8i);
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (j - 1); _arr9r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr9i = 0; }
        double _mul10r = 0, _mul10i = 0;
        c_mul(_arr9r, _arr9i, x2r, x2i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul8r + _mul10r; _add11i = _mul8i + _mul10i;
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
        double _cos18r = 0, _cos18i = 0;
        c_cos(j, 0, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_sin17r, _sin17i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _c16r + _mul19r; _add20i = _c16i + _mul19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log15r, _log15i, _add20r, _add20i, &_mul21r, &_mul21i);
        double mag_part = _mul21r; /* +_mul21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _arr23r = 0, _arr23i = 0;
        { int _idx = (j - 1); _arr23r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr23i = 0; }
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul22r, _mul22i, _arr23r, _arr23i, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_mul24r, _mul24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _arr27r = 0, _arr27i = 0;
        { int _idx = (j - 1); _arr27r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr27i = 0; }
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul26r, _mul26i, _arr27r, _arr27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _sin25r + _cos29r; _add30i = _sin25i + _cos29i;
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _add30r + _ang31r; _add32i = _add30i + _ang31i;
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _add32r - _ang33r; _sub34i = _add32i - _ang33i;
        double angle_part = _sub34r; /* +_sub34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, angle_part, 0, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(mag_part, 0, _exp37r, _exp37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    double _c39r = 0, _c39i = 0;
    _c39r = 1.0; _c39i = 0;
    double _add40r = 0, _add40i = 0;
    _add40r = n + _c39r; _add40i = 0 + _c39i;
    for (int k = 1; k < (int)(_add40r); k++) {
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = k + _c41r; _add42i = 0 + _c41i;
        double _log43r = 0, _log43i = 0;
        c_log(_add42r, _add42i, &_log43r, &_log43i);
        double r = _log43r; /* +_log43ii */
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _sin45r = 0, _sin45i = 0;
        c_sin(r, 0, &_sin45r, &_sin45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _c44r + _sin45r; _add46i = _c44i + _sin45i;
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _cos48r = 0, _cos48i = 0;
        c_cos(r, 0, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c47r, _c47i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _add46r + _mul49r; _add50i = _add46i + _mul49i;
        { double _tr = cRe[(k - 1)]*_add50r - cIm[(k - 1)]*_add50i; cIm[(k - 1)] = cRe[(k - 1)]*_add50i + cIm[(k - 1)]*_add50r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_312_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    for (int j = 1; j < 36; j++) {
        double _arr5r = 0, _arr5i = 0;
        { int _idx = (j - 1); _arr5r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr5i = 0; }
        double _arr6r = 0, _arr6i = 0;
        { int _idx = (j - 1); _arr6r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr6i = 0; }
        double _mul7r = 0, _mul7i = 0;
        c_mul(_arr5r, _arr5i, _arr6r, _arr6i, &_mul7r, &_mul7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_mul7r, _mul7i); _abs8i = 0;
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
        _c18r = 3.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul16r + _cos20r; _add21i = _mul16i + _cos20i;
        double magnitude = _add21r; /* +_add21ii */
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
        double _arr29r = 0, _arr29i = 0;
        { int _idx = (j - 1); _arr29r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr29i = 0; }
        double _arr30r = 0, _arr30i = 0;
        { int _idx = (j - 1); _arr30r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr30i = 0; }
        double _add31r = 0, _add31i = 0;
        _add31r = _arr29r + _arr30r; _add31i = _arr29i + _arr30i;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(_add31r, _add31i); _abs32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _abs32r + _c33r; _add34i = _abs32i + _c33i;
        double _log35r = 0, _log35i = 0;
        c_log(_add34r, _add34i, &_log35r, &_log35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add28r + _log35r; _add36i = _add28i + _log35i;
        double angle = _add36r; /* +_add36ii */
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, angle, 0, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(magnitude, 0, _exp39r, _exp39i, &_mul40r, &_mul40i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_313_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double ang = 0;
        double mag = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 4.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(j, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        if (_mod8r == _c9r) {
            double _arr10r = 0, _arr10i = 0;
            { int _idx = (j - 1); _arr10r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr10i = 0; }
            double _arr11r = 0, _arr11i = 0;
            { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr11i = 0; }
            double _mul12r = 0, _mul12i = 0;
            c_mul(_arr10r, _arr10i, _arr11r, _arr11i, &_mul12r, &_mul12i);
            double _abs13r = 0, _abs13i = 0;
            _abs13r = c_abs(_mul12r, _mul12i); _abs13i = 0;
            double _c14r = 0, _c14i = 0;
            _c14r = 1.0; _c14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
            double _log16r = 0, _log16i = 0;
            c_log(_add15r, _add15i, &_log16r, &_log16i);
            double _c17r = 0, _c17i = 0;
            _c17r = 1.3; _c17i = 0;
            double _pow18r = 0, _pow18i = 0;
            c_powr(j, 0, 1.3, &_pow18r, &_pow18i);
            double _sqrt19r = 0, _sqrt19i = 0;
            c_powr(j, 0, 0.5, &_sqrt19r, &_sqrt19i);
            double _add20r = 0, _add20i = 0;
            _add20r = _pow18r + _sqrt19r; _add20i = _pow18i + _sqrt19i;
            double _mul21r = 0, _mul21i = 0;
            c_mul(_log16r, _log16i, _add20r, _add20i, &_mul21r, &_mul21i);
            mag = _mul21r;
            double _mul22r = 0, _mul22i = 0;
            c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
            double _arr23r = 0, _arr23i = 0;
            { int _idx = (j - 1); _arr23r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr23i = 0; }
            double _mul24r = 0, _mul24i = 0;
            c_mul(_mul22r, _mul22i, _arr23r, _arr23i, &_mul24r, &_mul24i);
            double _sin25r = 0, _sin25i = 0;
            c_sin(_mul24r, _mul24i, &_sin25r, &_sin25i);
            double _c26r = 0, _c26i = 0;
            _c26r = 2.0; _c26i = 0;
            double _pow27r = 0, _pow27i = 0;
            c_mul(j, 0, j, 0, &_pow27r, &_pow27i);
            double _mul28r = 0, _mul28i = 0;
            c_mul(_pow27r, _pow27i, M_PI, 0, &_mul28r, &_mul28i);
            double _arr29r = 0, _arr29i = 0;
            { int _idx = (j - 1); _arr29r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr29i = 0; }
            double _mul30r = 0, _mul30i = 0;
            c_mul(_mul28r, _mul28i, _arr29r, _arr29i, &_mul30r, &_mul30i);
            double _cos31r = 0, _cos31i = 0;
            c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
            double _add32r = 0, _add32i = 0;
            _add32r = _sin25r + _cos31r; _add32i = _sin25i + _cos31i;
            double _ang33r = 0, _ang33i = 0;
            _ang33r = c_arg(x1r, x1i); _ang33i = 0;
            double _re34r = 0, _re34i = 0;
            _re34r = x2r; _re34i = 0;
            double _mul35r = 0, _mul35i = 0;
            c_mul(_ang33r, _ang33i, _re34r, _re34i, &_mul35r, &_mul35i);
            double _add36r = 0, _add36i = 0;
            _add36r = _add32r + _mul35r; _add36i = _add32i + _mul35i;
            ang = _add36r;
        } else {
            double _c37r = 0, _c37i = 0;
            _c37r = 4.0; _c37i = 0;
            double _mod38r = 0, _mod38i = 0;
            _mod38r = fmod(j, _c37r); _mod38i = 0;
            double _c39r = 0, _c39i = 0;
            _c39r = 2.0; _c39i = 0;
            if (_mod38r == _c39r) {
                double _arr40r = 0, _arr40i = 0;
                { int _idx = (j - 1); _arr40r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr40i = 0; }
                double _arr41r = 0, _arr41i = 0;
                { int _idx = (j - 1); _arr41r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr41i = 0; }
                double _add42r = 0, _add42i = 0;
                _add42r = _arr40r + _arr41r; _add42i = _arr40i + _arr41i;
                double _abs43r = 0, _abs43i = 0;
                _abs43r = c_abs(_add42r, _add42i); _abs43i = 0;
                double _c44r = 0, _c44i = 0;
                _c44r = 1.0; _c44i = 0;
                double _add45r = 0, _add45i = 0;
                _add45r = _abs43r + _c44r; _add45i = _abs43i + _c44i;
                double _log46r = 0, _log46i = 0;
                c_log(_add45r, _add45i, &_log46r, &_log46i);
                double _c47r = 0, _c47i = 0;
                _c47r = 0.05; _c47i = 0;
                double _mul48r = 0, _mul48i = 0;
                c_mul(_c47r, _c47i, j, 0, &_mul48r, &_mul48i);
                double _exp49r = 0, _exp49i = 0;
                c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
                double _add50r = 0, _add50i = 0;
                _add50r = _exp49r + j; _add50i = _exp49i + 0;
                double _mul51r = 0, _mul51i = 0;
                c_mul(_log46r, _log46i, _add50r, _add50i, &_mul51r, &_mul51i);
                mag = _mul51r;
                double _mul52r = 0, _mul52i = 0;
                c_mul(j, 0, M_PI, 0, &_mul52r, &_mul52i);
                double _arr53r = 0, _arr53i = 0;
                { int _idx = (j - 1); _arr53r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr53i = 0; }
                double _mul54r = 0, _mul54i = 0;
                c_mul(_mul52r, _mul52i, _arr53r, _arr53i, &_mul54r, &_mul54i);
                double _cos55r = 0, _cos55i = 0;
                c_cos(_mul54r, _mul54i, &_cos55r, &_cos55i);
                double _c56r = 0, _c56i = 0;
                _c56r = 1.5; _c56i = 0;
                double _pow57r = 0, _pow57i = 0;
                c_powr(j, 0, 1.5, &_pow57r, &_pow57i);
                double _mul58r = 0, _mul58i = 0;
                c_mul(_pow57r, _pow57i, M_PI, 0, &_mul58r, &_mul58i);
                double _arr59r = 0, _arr59i = 0;
                { int _idx = (j - 1); _arr59r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr59i = 0; }
                double _mul60r = 0, _mul60i = 0;
                c_mul(_mul58r, _mul58i, _arr59r, _arr59i, &_mul60r, &_mul60i);
                double _sin61r = 0, _sin61i = 0;
                c_sin(_mul60r, _mul60i, &_sin61r, &_sin61i);
                double _sub62r = 0, _sub62i = 0;
                _sub62r = _cos55r - _sin61r; _sub62i = _cos55i - _sin61i;
                double _ang63r = 0, _ang63i = 0;
                _ang63r = c_arg(x2r, x2i); _ang63i = 0;
                double _im64r = 0, _im64i = 0;
                _im64r = x1i; _im64i = 0;
                double _mul65r = 0, _mul65i = 0;
                c_mul(_ang63r, _ang63i, _im64r, _im64i, &_mul65r, &_mul65i);
                double _add66r = 0, _add66i = 0;
                _add66r = _sub62r + _mul65r; _add66i = _sub62i + _mul65i;
                ang = _add66r;
            } else {
                double _c67r = 0, _c67i = 0;
                _c67r = 4.0; _c67i = 0;
                double _mod68r = 0, _mod68i = 0;
                _mod68r = fmod(j, _c67r); _mod68i = 0;
                double _c69r = 0, _c69i = 0;
                _c69r = 3.0; _c69i = 0;
                if (_mod68r == _c69r) {
                    double _arr70r = 0, _arr70i = 0;
                    { int _idx = (j - 1); _arr70r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr70i = 0; }
                    double _arr71r = 0, _arr71i = 0;
                    { int _idx = (j - 1); _arr71r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr71i = 0; }
                    double _sub72r = 0, _sub72i = 0;
                    _sub72r = _arr70r - _arr71r; _sub72i = _arr70i - _arr71i;
                    double _abs73r = 0, _abs73i = 0;
                    _abs73r = c_abs(_sub72r, _sub72i); _abs73i = 0;
                    double _c74r = 0, _c74i = 0;
                    _c74r = 1.0; _c74i = 0;
                    double _add75r = 0, _add75i = 0;
                    _add75r = _abs73r + _c74r; _add75i = _abs73i + _c74i;
                    double _log76r = 0, _log76i = 0;
                    c_log(_add75r, _add75i, &_log76r, &_log76i);
                    double _c77r = 0, _c77i = 0;
                    _c77r = 2.0; _c77i = 0;
                    double _pow78r = 0, _pow78i = 0;
                    c_mul(j, 0, j, 0, &_pow78r, &_pow78i);
                    double _c79r = 0, _c79i = 0;
                    _c79r = 1.0; _c79i = 0;
                    double _add80r = 0, _add80i = 0;
                    _add80r = _c79r + j; _add80i = _c79i + 0;
                    double _div81r = 0, _div81i = 0;
                    c_div(_pow78r, _pow78i, _add80r, _add80i, &_div81r, &_div81i);
                    double _mul82r = 0, _mul82i = 0;
                    c_mul(_log76r, _log76i, _div81r, _div81i, &_mul82r, &_mul82i);
                    mag = _mul82r;
                    double _c83r = 0, _c83i = 0;
                    _c83r = 2.0; _c83i = 0;
                    double _pow84r = 0, _pow84i = 0;
                    c_mul(j, 0, j, 0, &_pow84r, &_pow84i);
                    double _mul85r = 0, _mul85i = 0;
                    c_mul(_pow84r, _pow84i, M_PI, 0, &_mul85r, &_mul85i);
                    double _arr86r = 0, _arr86i = 0;
                    { int _idx = (j - 1); _arr86r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr86i = 0; }
                    double _mul87r = 0, _mul87i = 0;
                    c_mul(_mul85r, _mul85i, _arr86r, _arr86i, &_mul87r, &_mul87i);
                    double _sin88r = 0, _sin88i = 0;
                    c_sin(_mul87r, _mul87i, &_sin88r, &_sin88i);
                    double _mul89r = 0, _mul89i = 0;
                    c_mul(j, 0, M_PI, 0, &_mul89r, &_mul89i);
                    double _arr90r = 0, _arr90i = 0;
                    { int _idx = (j - 1); _arr90r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr90i = 0; }
                    double _mul91r = 0, _mul91i = 0;
                    c_mul(_mul89r, _mul89i, _arr90r, _arr90i, &_mul91r, &_mul91i);
                    double _cos92r = 0, _cos92i = 0;
                    c_cos(_mul91r, _mul91i, &_cos92r, &_cos92i);
                    double _mul93r = 0, _mul93i = 0;
                    c_mul(_sin88r, _sin88i, _cos92r, _cos92i, &_mul93r, &_mul93i);
                    double _ang94r = 0, _ang94i = 0;
                    _ang94r = c_arg(x1r, x1i); _ang94i = 0;
                    double _ang95r = 0, _ang95i = 0;
                    _ang95r = c_arg(x2r, x2i); _ang95i = 0;
                    double _mul96r = 0, _mul96i = 0;
                    c_mul(_ang94r, _ang94i, _ang95r, _ang95i, &_mul96r, &_mul96i);
                    double _add97r = 0, _add97i = 0;
                    _add97r = _mul93r + _mul96r; _add97i = _mul93i + _mul96i;
                    ang = _add97r;
                } else {
                    double _arr98r = 0, _arr98i = 0;
                    { int _idx = (j - 1); _arr98r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr98i = 0; }
                    double _c99r = 0, _c99i = 0;
                    _c99r = 2.0; _c99i = 0;
                    double _pow100r = 0, _pow100i = 0;
                    c_mul(_arr98r, _arr98i, _arr98r, _arr98i, &_pow100r, &_pow100i);
                    double _arr101r = 0, _arr101i = 0;
                    { int _idx = (j - 1); _arr101r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr101i = 0; }
                    double _c102r = 0, _c102i = 0;
                    _c102r = 2.0; _c102i = 0;
                    double _pow103r = 0, _pow103i = 0;
                    c_mul(_arr101r, _arr101i, _arr101r, _arr101i, &_pow103r, &_pow103i);
                    double _add104r = 0, _add104i = 0;
                    _add104r = _pow100r + _pow103r; _add104i = _pow100i + _pow103i;
                    double _abs105r = 0, _abs105i = 0;
                    _abs105r = c_abs(_add104r, _add104i); _abs105i = 0;
                    double _c106r = 0, _c106i = 0;
                    _c106r = 1.0; _c106i = 0;
                    double _add107r = 0, _add107i = 0;
                    _add107r = _abs105r + _c106r; _add107i = _abs105i + _c106i;
                    double _log108r = 0, _log108i = 0;
                    c_log(_add107r, _add107i, &_log108r, &_log108i);
                    double _sqrt109r = 0, _sqrt109i = 0;
                    c_powr(j, 0, 0.5, &_sqrt109r, &_sqrt109i);
                    double _mul110r = 0, _mul110i = 0;
                    c_mul(_log108r, _log108i, _sqrt109r, _sqrt109i, &_mul110r, &_mul110i);
                    double _c111r = 0, _c111i = 0;
                    _c111r = 1.0; _c111i = 0;
                    double _log112r = 0, _log112i = 0;
                    c_log(j, 0, &_log112r, &_log112i);
                    double _add113r = 0, _add113i = 0;
                    _add113r = _c111r + _log112r; _add113i = _c111i + _log112i;
                    double _mul114r = 0, _mul114i = 0;
                    c_mul(_mul110r, _mul110i, _add113r, _add113i, &_mul114r, &_mul114i);
                    mag = _mul114r;
                    double _mul115r = 0, _mul115i = 0;
                    c_mul(j, 0, M_PI, 0, &_mul115r, &_mul115i);
                    double _arr116r = 0, _arr116i = 0;
                    { int _idx = (j - 1); _arr116r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr116i = 0; }
                    double _mul117r = 0, _mul117i = 0;
                    c_mul(_mul115r, _mul115i, _arr116r, _arr116i, &_mul117r, &_mul117i);
                    double _c118r = 0, _c118i = 0;
                    _c118r = 2.0; _c118i = 0;
                    double _div119r = 0, _div119i = 0;
                    c_div(_mul117r, _mul117i, _c118r, _c118i, &_div119r, &_div119i);
                    double _sin120r = 0, _sin120i = 0;
                    c_sin(_div119r, _div119i, &_sin120r, &_sin120i);
                    double _c121r = 0, _c121i = 0;
                    _c121r = 3.0; _c121i = 0;
                    double _pow122r = 0, _pow122i = 0;
                    c_mul(j, 0, j, 0, &_pow122r, &_pow122i);
                    c_mul(_pow122r, _pow122i, j, 0, &_pow122r, &_pow122i);
                    double _mul123r = 0, _mul123i = 0;
                    c_mul(_pow122r, _pow122i, M_PI, 0, &_mul123r, &_mul123i);
                    double _arr124r = 0, _arr124i = 0;
                    { int _idx = (j - 1); _arr124r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr124i = 0; }
                    double _mul125r = 0, _mul125i = 0;
                    c_mul(_mul123r, _mul123i, _arr124r, _arr124i, &_mul125r, &_mul125i);
                    double _c126r = 0, _c126i = 0;
                    _c126r = 3.0; _c126i = 0;
                    double _div127r = 0, _div127i = 0;
                    c_div(_mul125r, _mul125i, _c126r, _c126i, &_div127r, &_div127i);
                    double _cos128r = 0, _cos128i = 0;
                    c_cos(_div127r, _div127i, &_cos128r, &_cos128i);
                    double _add129r = 0, _add129i = 0;
                    _add129r = _sin120r + _cos128r; _add129i = _sin120i + _cos128i;
                    ang = _add129r;
                }
            }
        }
        double _c130r = 0, _c130i = 0;
        _c130r = 0.0; _c130i = 1.0;
        double _mul131r = 0, _mul131i = 0;
        c_mul(_c130r, _c130i, ang, 0, &_mul131r, &_mul131i);
        double _exp132r = 0, _exp132i = 0;
        c_exp2(_mul131r, _mul131i, &_exp132r, &_exp132i);
        double _mul133r = 0, _mul133i = 0;
        c_mul(mag, 0, _exp132r, _exp132i, &_mul133r, &_mul133i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul133r; cIm[_idx] = _mul133i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_314_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double j = k; /* +0i */
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(j, 0, j, 0, &_pow4r, &_pow4i);
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _im6r = 0, _im6i = 0;
        _im6r = x2i; _im6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_re5r, _re5i, _im6r, _im6i, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow4r + _mul7r; _add8i = _pow4i + _mul7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 7.0; _c9i = 0;
        double _mod10r = 0, _mod10i = 0;
        _mod10r = fmod(_add8r, _c9r); _mod10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _mod10r + _c11r; _add12i = _mod10i + _c11i;
        double r = _add12r; /* +_add12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _div15r = 0, _div15i = 0;
        c_div(_mul14r, _mul14i, r, 0, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang13r, _ang13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = r + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_mul19r, _mul19i, _add21r, _add21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang18r, _ang18i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul17r + _mul24r; _add25i = _mul17i + _mul24i;
        double angle = _add25r; /* +_add25ii */
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.5; _c27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, j, 0, &_mul28r, &_mul28i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(_abs26r, _abs26i, _mul28r, &_pow29r, &_pow29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.3; _c31i = 0;
        double _sub32r = 0, _sub32i = 0;
        _sub32r = n - j; _sub32i = 0 - 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _sub32r + _c33r; _add34i = _sub32i + _c33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c31r, _c31i, _add34r, _add34i, &_mul35r, &_mul35i);
        double _pow36r = 0, _pow36i = 0;
        c_powr(_abs30r, _abs30i, _mul35r, &_pow36r, &_pow36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _pow29r + _pow36r; _add37i = _pow29i + _pow36i;
        double magnitude = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, angle, 0, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub49r; cIm[_idx] = _sub49i; } }
        double _cf50r = 0, _cf50i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
        double _cf51r = 0, _cf51i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
        double _abs52r = 0, _abs52i = 0;
        _abs52r = c_abs(_cf51r, _cf51i); _abs52i = 0;
        double _c53r = 0, _c53i = 0;
        _c53r = 1.0; _c53i = 0;
        double _add54r = 0, _add54i = 0;
        _add54r = _abs52r + _c53r; _add54i = _abs52i + _c53i;
        double _log55r = 0, _log55i = 0;
        c_log(_add54r, _add54i, &_log55r, &_log55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_cf50r, _cf50i, _log55r, _log55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _mul56r + 0; _add57i = _mul56i + 0;
        double _add58r = 0, _add58i = 0;
        _add58r = _add57r + j; _add58i = _add57i + 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_315_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_re3r, _re3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _re8r = 0, _re8i = 0;
        _re8r = x2r; _re8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_re8r, _re8i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _mul10r; _add11i = _mul7i + _mul10i;
        double r = _add11r; /* +_add11ii */
        double _im12r = 0, _im12i = 0;
        _im12r = x1i; _im12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(j, 0, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_im12r, _im12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _im17r = 0, _im17i = 0;
        _im17r = x2i; _im17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(_mul18r, _mul18i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _abs19r; _add20i = 0 + _abs19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_im17r, _im17i, _log21r, _log21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul16r + _mul22r; _add23i = _mul16i + _mul22i;
        double q = _add23r; /* +_add23ii */
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(r, 0); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 5.0; _c25i = 0;
        double _mod26r = 0, _mod26i = 0;
        _mod26r = fmod(j, _c25r); _mod26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _mod26r + _c27r; _add28i = _mod26i + _c27i;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_abs24r, _abs24i, _add28r, &_pow29r, &_pow29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 3.0; _c31i = 0;
        double _mod32r = 0, _mod32i = 0;
        _mod32r = fmod(j, _c31r); _mod32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _mod32r + _c33r; _add34i = _mod32i + _c33i;
        double _pow35r = 0, _pow35i = 0;
        c_powr(_abs30r, _abs30i, _add34r, &_pow35r, &_pow35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _pow29r + _pow35r; _add36i = _pow29i + _pow35i;
        double magnitude = _add36r; /* +_add36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(q, 0); _ang37i = 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(j, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang37r, _ang37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(x2r, x2i); _ang40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 3.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(j, 0, _c41r, _c41i, &_div42r, &_div42i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(_div42r, _div42i, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_ang40r, _ang40i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _mul39r - _mul44r; _sub45i = _mul39i - _mul44i;
        double angle = _sub45r; /* +_sub45ii */
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

static void poly_316_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 3.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        double k = _add4r; /* +_add4ii */
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 8.0; _c7i = 0;
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
        double r = _add17r; /* +_add17ii */
        double _im18r = 0, _im18i = 0;
        _im18r = x1i; _im18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 7.0; _c20i = 0;
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
        _c26r = 9.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_im24r, _im24i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul23r - _mul29r; _sub30i = _mul23i - _mul29i;
        double im = _sub30r; /* +_sub30ii */
        double _mul31r = 0, _mul31i = 0;
        c_mul(r, 0, im, 0, &_mul31r, &_mul31i);
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(_mul31r, _mul31i); _abs32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _abs32r + _c33r; _add34i = _abs32i + _c33i;
        double _log35r = 0, _log35i = 0;
        c_log(_add34r, _add34i, &_log35r, &_log35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(k, 0, M_PI, 0, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 4.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _c36r + _sin40r; _add41i = _c36i + _sin40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_log35r, _log35i, _add41r, _add41i, &_mul42r, &_mul42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_mul42r, _mul42i, 0, 0, &_mul43r, &_mul43i);
        double _div44r = 0, _div44i = 0;
        c_div(_mul43r, _mul43i, n, 0, &_div44r, &_div44i);
        double mag = _div44r; /* +_div44ii */
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x1r, x1i); _ang45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(k, 0, M_PI, 0, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 6.0; _c47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(_mul46r, _mul46i, _c47r, _c47i, &_div48r, &_div48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_div48r, _div48i, &_cos49r, &_cos49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_ang45r, _ang45i, _cos49r, _cos49i, &_mul50r, &_mul50i);
        double _ang51r = 0, _ang51i = 0;
        _ang51r = c_arg(x2r, x2i); _ang51i = 0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(k, 0, M_PI, 0, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 10.0; _c53i = 0;
        double _div54r = 0, _div54i = 0;
        c_div(_mul52r, _mul52i, _c53r, _c53i, &_div54r, &_div54i);
        double _sin55r = 0, _sin55i = 0;
        c_sin(_div54r, _div54i, &_sin55r, &_sin55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_ang51r, _ang51i, _sin55r, _sin55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _mul50r + _mul56r; _add57i = _mul50i + _mul56i;
        double ang = _add57r; /* +_add57ii */
        double _cos58r = 0, _cos58i = 0;
        c_cos(ang, 0, &_cos58r, &_cos58i);
        double _c59r = 0, _c59i = 0;
        _c59r = 0.0; _c59i = 1.0;
        double _sin60r = 0, _sin60i = 0;
        c_sin(ang, 0, &_sin60r, &_sin60i);
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

static void poly_317_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log9r, _log9i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(j, 0, j, 0, &_pow13r, &_pow13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_pow13r, _pow13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul11r + _cos14r; _add15i = _mul11i + _cos14i;
        double mag_part = _add15r; /* +_add15ii */
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
        double angle_part = _add22r; /* +_add22ii */
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, angle_part, 0, &_mul24r, &_mul24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(mag_part, 0, _exp25r, _exp25i, &_mul26r, &_mul26i);
        double _conj27r = 0, _conj27i = 0;
        _conj27r = x1r; _conj27i = -(x1i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_conj27r, _conj27i, 0, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = j + _c29r; _add30i = 0 + _c29i;
        double _div31r = 0, _div31i = 0;
        c_div(_mul28r, _mul28i, _add30r, _add30i, &_div31r, &_div31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul26r + _div31r; _add32i = _mul26i + _div31i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    }
    double _c33r = 0, _c33i = 0;
    _c33r = 1.0; _c33i = 0;
    double _add34r = 0, _add34i = 0;
    _add34r = n + _c33r; _add34i = 0 + _c33i;
    for (int k = 1; k < (int)(_add34r); k++) {
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 0.05; _c36i = 0;
        double _cos37r = 0, _cos37i = 0;
        c_cos(k, 0, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c36r, _c36i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _c35r + _mul38r; _add39i = _c35i + _mul38i;
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 0.03;
        double _sin41r = 0, _sin41i = 0;
        c_sin(k, 0, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c40r, _c40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add39r + _mul42r; _add43i = _add39i + _mul42i;
        { double _tr = cRe[(k - 1)]*_add43r - cIm[(k - 1)]*_add43i; cIm[(k - 1)] = cRe[(k - 1)]*_add43i + cIm[(k - 1)]*_add43r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_318_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double rec1 = _re1r; /* +_re1ii */
    double _im2r = 0, _im2i = 0;
    _im2r = x1i; _im2i = 0;
    double imc1 = _im2r; /* +_im2ii */
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec2 = _re3r; /* +_re3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc2 = _im4r; /* +_im4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, rec1, 0, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, imc2, 0, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _sin8r + _cos10r; _add11i = _sin8i + _cos10i;
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 0.1; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = j + _c15r; _add16i = 0 + _c15i;
        double _div17r = 0, _div17i = 0;
        c_div(_mul14r, _mul14i, _add16r, _add16i, &_div17r, &_div17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _add11r + _div17r; _add18i = _add11i + _div17i;
        double angle = _add18r; /* +_add18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_abs19r, _abs19i, j, &_pow20r, &_pow20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x2r, x2i); _abs21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs21r + j; _add22i = _abs21i + 0;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_pow20r, _pow20i, _log23r, _log23i, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _neg27r = 0, _neg27i = 0;
        _neg27r = -(_c26r); _neg27i = -(_c26i);
        double _pow28r = 0, _pow28i = 0;
        c_powr(_neg27r, _neg27i, j, &_pow28r, &_pow28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.5; _c29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow28r, _pow28i, _c29r, _c29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _c25r + _mul30r; _add31i = _c25i + _mul30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_mul24r, _mul24i, _add31r, _add31i, &_mul32r, &_mul32i);
        double magnitude = _mul32r; /* +_mul32ii */
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

static void poly_319_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_re3r, _re3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _re8r = 0, _re8i = 0;
        _re8r = x2r; _re8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 7.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_re8r, _re8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul7r + _mul13r; _add14i = _mul7i + _mul13i;
        double r = _add14r; /* +_add14ii */
        double _im15r = 0, _im15i = 0;
        _im15r = x1i; _im15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 5.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_im15r, _im15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _im21r = 0, _im21i = 0;
        _im21r = x2i; _im21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = j + _c22r; _add23i = 0 + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_im21r, _im21i, _log24r, _log24i, &_mul25r, &_mul25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul20r - _mul25r; _sub26i = _mul20i - _mul25i;
        double q = _sub26r; /* +_sub26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _add28r = 0, _add28i = 0;
        _add28r = r + _c27r; _add28i = 0 + _c27i;
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(_add28r, _add28i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs29r + _c30r; _add31i = _abs29i + _c30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 4.0; _c34i = 0;
        double _mod35r = 0, _mod35i = 0;
        _mod35r = fmod(j, _c34r); _mod35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _c33r + _mod35r; _add36i = _c33i + _mod35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_log32r, _log32i, _add36r, _add36i, &_mul37r, &_mul37i);
        double magnitude = _mul37r; /* +_mul37ii */
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
        double _add46r = 0, _add46i = 0;
        _add46r = _mul40r + _mul45r; _add46i = _mul40i + _mul45i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_320_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _mul6r = 0, _mul6i = 0;
        c_mul(_arr5r, _arr5i, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _div9r = 0, _div9i = 0;
        c_div(_mul6r, _mul6i, _add8r, _add8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double mag_part1 = _sin10r; /* +_sin10ii */
        double _arr11r = 0, _arr11i = 0;
        { int _idx = j; _arr11r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr11i = 0; }
        double _mul12r = 0, _mul12i = 0;
        c_mul(_arr11r, _arr11i, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _div15r = 0, _div15i = 0;
        c_div(_mul12r, _mul12i, _add14r, _add14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double mag_part2 = _cos16r; /* +_cos16ii */
        double _add17r = 0, _add17i = 0;
        _add17r = mag_part1 + mag_part2; _add17i = 0 + 0;
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
        c_mul(_add17r, _add17i, _log22r, _log22i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(j, 0, n, 0, &_div25r, &_div25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _c24r + _div25r; _add26i = _c24i + _div25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul23r, _mul23i, _add26r, _add26i, &_mul27r, &_mul27i);
        double mag = _mul27r; /* +_mul27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _arr29r = 0, _arr29i = 0;
        { int _idx = j; _arr29r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr29i = 0; }
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _arr29r, _arr29i, &_mul30r, &_mul30i);
        double _div31r = 0, _div31i = 0;
        c_div(_mul30r, _mul30i, n, 0, &_div31r, &_div31i);
        double ang_part1 = _div31r; /* +_div31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _arr33r = 0, _arr33i = 0;
        { int _idx = j; _arr33r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr33i = 0; }
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang32r, _ang32i, _arr33r, _arr33i, &_mul34r, &_mul34i);
        double _div35r = 0, _div35i = 0;
        c_div(_mul34r, _mul34i, n, 0, &_div35r, &_div35i);
        double ang_part2 = _div35r; /* +_div35ii */
        double _sub36r = 0, _sub36i = 0;
        _sub36r = ang_part1 - ang_part2; _sub36i = 0 - 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, M_PI, 0, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 5.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _sub36r + _sin40r; _add41i = _sub36i + _sin40i;
        double angle = _add41r; /* +_add41ii */
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, angle, 0, &_mul43r, &_mul43i);
        double _exp44r = 0, _exp44i = 0;
        c_exp2(_mul43r, _mul43i, &_exp44r, &_exp44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag, 0, _exp44r, _exp44i, &_mul45r, &_mul45i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_321_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
    for (int j = 0; j < (int)(n); j++) {
        double _c5r = 0, _c5i = 0;
        _c5r = 1.8; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(j, 0, 1.8, &_pow6r, &_pow6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + j; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _pow6r + _log11r; _add12i = _pow6i + _log11i;
        double _re13r = 0, _re13i = 0;
        _re13r = x1r; _re13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _re13r, _re13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _im16r, _im16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _sin15r + _cos18r; _add19i = _sin15i + _cos18i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_add19r, _add19i); _abs20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_add12r, _add12i, _abs20r, _abs20i, &_mul21r, &_mul21i);
        double mag_part = _mul21r; /* +_mul21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = j + _c23r; _add24i = 0 + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang22r, _ang22i, _log25r, _log25i, &_mul26r, &_mul26i);
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
        double _add32r = 0, _add32i = 0;
        _add32r = _mul26r + _mul31r; _add32i = _mul26i + _mul31i;
        double angle_part = _add32r; /* +_add32ii */
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
        c_mul(mag_part, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_322_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double d = _arr6r; /* +_arr6ii */
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 1.0;
        double _add8r = 0, _add8i = 0;
        _add8r = r + _c7r; _add8i = 0 + _c7i;
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
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 5.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
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
        _ang25r = c_arg(d, 0); _ang25i = 0;
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
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 6.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang31r, _ang31i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul30r + _mul36r; _add37i = _mul30i + _mul36i;
        double angle = _add37r; /* +_add37ii */
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
        c_mul(mag, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_323_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 5.0; _c2i = 0;
        double _mod3r = 0, _mod3i = 0;
        _mod3r = fmod(j, _c2r); _mod3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _mod3r + _c4r; _add5i = _mod3i + _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_re1r, _re1i, _add5r, &_pow6r, &_pow6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_pow6r, _pow6i, _log9r, _log9i, &_mul10r, &_mul10i);
        double _im11r = 0, _im11i = 0;
        _im11r = x1i; _im11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 7.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_im11r, _im11i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul10r + _mul16r; _add17i = _mul10i + _mul16i;
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(j, 0, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang18r, _ang18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 4.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(j, 0, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang23r, _ang23i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul22r + _mul27r; _add28i = _mul22i + _mul27i;
        double angle = _add28r; /* +_add28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, angle, 0, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag, 0, _exp31r, _exp31i, &_mul32r, &_mul32i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_324_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double d = _arr6r; /* +_arr6ii */
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_mul(r, 0, r, 0, &_pow8r, &_pow8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_pow8r, _pow8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_c14r, _c14i, M_PI, 0, &_mul15r, &_mul15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_mul15r, _mul15i, r, 0, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul16r, _mul16i, j, 0, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _c13r + _sin18r; _add19i = _c13i + _sin18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log12r, _log12i, _add19r, _add19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(M_PI, 0, _c22r, _c22i, &_mul23r, &_mul23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul23r, _mul23i, j, 0, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _c21r + _cos25r; _add26i = _c21i + _cos25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul20r, _mul20i, _add26r, _add26i, &_mul27r, &_mul27i);
        double mag = _mul27r; /* +_mul27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(d, 0, _c28r, _c28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = r + _mul29r; _add30i = 0 + _mul29i;
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(_add30r, _add30i); _ang31i = 0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(j, 0, &_sin32r, &_sin32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _add34r = 0, _add34i = 0;
        _add34r = r + _c33r; _add34i = 0 + _c33i;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(_add34r, _add34i); _abs35i = 0;
        double _log36r = 0, _log36i = 0;
        c_log(_abs35r, _abs35i, &_log36r, &_log36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sin32r, _sin32i, _log36r, _log36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _ang31r + _mul37r; _add38i = _ang31i + _mul37i;
        double _cos39r = 0, _cos39i = 0;
        c_cos(j, 0, &_cos39r, &_cos39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _sub41r = 0, _sub41i = 0;
        _sub41r = r - _c40r; _sub41i = 0 - _c40i;
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(_sub41r, _sub41i); _ang42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_cos39r, _cos39i, _ang42r, _ang42i, &_mul43r, &_mul43i);
        double _sub44r = 0, _sub44i = 0;
        _sub44r = _add38r - _mul43r; _sub44i = _add38i - _mul43i;
        double ang = _sub44r; /* +_sub44ii */
        double _cos45r = 0, _cos45i = 0;
        c_cos(ang, 0, &_cos45r, &_cos45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(ang, 0, &_sin46r, &_sin46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_sin46r, _sin46i, _c47r, _c47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _cos45r + _mul48r; _add49i = _cos45i + _mul48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(mag, 0, _add49r, _add49i, &_mul50r, &_mul50i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_325: too complex for auto-transpile, stubbed */
static void poly_325_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_326_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c5r = 0, _c5i = 0;
        _c5r = 7.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(j, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double k = _add8r; /* +_add8ii */
        double _add9r = 0, _add9i = 0;
        _add9r = j + k; _add9i = 0 + 0;
        double r = _add9r; /* +_add9ii */
        double _arr10r = 0, _arr10i = 0;
        { int _idx = j; _arr10r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr10i = 0; }
        double _arr11r = 0, _arr11i = 0;
        { int _idx = j; _arr11r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr11i = 0; }
        double _mul12r = 0, _mul12i = 0;
        c_mul(_arr10r, _arr10i, _arr11r, _arr11i, &_mul12r, &_mul12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_mul12r, _mul12i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(j, 0, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log16r, _log16i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _add24r = 0, _add24i = 0;
        _add24r = x1r + x2r; _add24i = x1i + x2i;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_add24r, _add24i); _abs25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_cos23r, _cos23i, _abs25r, _abs25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul20r + _mul26r; _add27i = _mul20i + _mul26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 6.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang28r, _ang28i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 8.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang34r, _ang34i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul33r + _mul39r; _add40i = _mul33i + _mul39i;
        double _sin41r = 0, _sin41i = 0;
        c_sin(j, 0, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _add40r + _sin41r; _add42i = _add40i + _sin41i;
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
        double _conj49r = 0, _conj49i = 0;
        _conj49r = x1r; _conj49i = -(x1i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(j, 0, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_conj49r, _conj49i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul48r + _mul51r; _add52i = _mul48i + _mul51i;
        double _conj53r = 0, _conj53i = 0;
        _conj53r = x2r; _conj53i = -(x2i);
        double _cos54r = 0, _cos54i = 0;
        c_cos(j, 0, &_cos54r, &_cos54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_conj53r, _conj53i, _cos54r, _cos54i, &_mul55r, &_mul55i);
        double _sub56r = 0, _sub56i = 0;
        _sub56r = _add52r - _mul55r; _sub56i = _add52i - _mul55i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub56r; cIm[_idx] = _sub56i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_327_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
    for (int j = 0; j < (int)(n); j++) {
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 4.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _re9r = 0, _re9i = 0;
        _re9r = x1r; _re9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 0.5; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_powr(_re9r, _re9i, 0.5, &_pow11r, &_pow11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sin8r, _sin8i, _pow11r, _pow11i, &_mul12r, &_mul12i);
        double part1 = _mul12r; /* +_mul12ii */
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 3.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _im17r = 0, _im17i = 0;
        _im17r = x2i; _im17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.3; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_im17r, _im17i, 0.3, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_cos16r, _cos16i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double part2 = _mul20r; /* +_mul20ii */
        double _arr21r = 0, _arr21i = 0;
        { int _idx = j; _arr21r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr21i = 0; }
        double _arr22r = 0, _arr22i = 0;
        { int _idx = j; _arr22r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr22i = 0; }
        double _mul23r = 0, _mul23i = 0;
        c_mul(_arr21r, _arr21i, _arr22r, _arr22i, &_mul23r, &_mul23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_mul23r, _mul23i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double part3 = _log27r; /* +_log27ii */
        double _add28r = 0, _add28i = 0;
        _add28r = part1 + part2; _add28i = 0 + 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _add28r + part3; _add29i = _add28i + 0;
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
        c_mul(_c37r, _c37i, angle, 0, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(magnitude, 0, _exp39r, _exp39i, &_mul40r, &_mul40i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_328_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c5r = 0, _c5i = 0;
        _c5r = 5.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(j, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double k = _add8r; /* +_add8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_mul(j, 0, j, 0, &_pow10r, &_pow10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(j, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin11r, _sin11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _pow10r + _mul13r; _add14i = _pow10i + _mul13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _log18r + _c19r; _add20i = _log18i + _c19i;
        double _div21r = 0, _div21i = 0;
        c_div(_add14r, _add14i, _add20r, _add20i, &_div21r, &_div21i);
        double r = _div21r; /* +_div21ii */
        double mag_variation = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(n, 0, _c22r, _c22i, &_div23r, &_div23i);
        if (j <= _div23r) {
            double _c24r = 0, _c24i = 0;
            _c24r = 1.0; _c24i = 0;
            double _mul25r = 0, _mul25i = 0;
            c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
            double _c26r = 0, _c26i = 0;
            _c26r = 7.0; _c26i = 0;
            double _div27r = 0, _div27i = 0;
            c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
            double _sin28r = 0, _sin28i = 0;
            c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
            double _add29r = 0, _add29i = 0;
            _add29r = _c24r + _sin28r; _add29i = _c24i + _sin28i;
            double _mul30r = 0, _mul30i = 0;
            c_mul(r, 0, _add29r, _add29i, &_mul30r, &_mul30i);
            mag_variation = _mul30r;
        } else {
            double _c31r = 0, _c31i = 0;
            _c31r = 1.0; _c31i = 0;
            double _mul32r = 0, _mul32i = 0;
            c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
            double _c33r = 0, _c33i = 0;
            _c33r = 5.0; _c33i = 0;
            double _div34r = 0, _div34i = 0;
            c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
            double _cos35r = 0, _cos35i = 0;
            c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
            double _add36r = 0, _add36i = 0;
            _add36r = _c31r + _cos35r; _add36i = _c31i + _cos35i;
            double _mul37r = 0, _mul37i = 0;
            c_mul(r, 0, _add36r, _add36i, &_mul37r, &_mul37i);
            mag_variation = _mul37r;
        }
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(j, 0, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang38r, _ang38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _cos42r = 0, _cos42i = 0;
        c_cos(j, 0, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang41r, _ang41i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul40r + _mul43r; _add44i = _mul40i + _mul43i;
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = mag_variation + _c45r; _add46i = 0 + _c45i;
        double _log47r = 0, _log47i = 0;
        c_log(_add46r, _add46i, &_log47r, &_log47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _add44r + _log47r; _add48i = _add44i + _log47i;
        double angle_variation = _add48r; /* +_add48ii */
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c49r, _c49i, angle_variation, 0, &_mul50r, &_mul50i);
        double _exp51r = 0, _exp51i = 0;
        c_exp2(_mul50r, _mul50i, &_exp51r, &_exp51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(mag_variation, 0, _exp51r, _exp51i, &_mul52r, &_mul52i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_329_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr6i = 0; }
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
        double mag_part1 = _log11r; /* +_log11ii */
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 4.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sin15r, _sin15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double mag_part2 = _mul20r; /* +_mul20ii */
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _c21r + mag_part2; _add22i = _c21i + 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(mag_part1, 0, _add22r, _add22i, &_mul23r, &_mul23i);
        double magnitude = _mul23r; /* +_mul23ii */
        double _conj24r = 0, _conj24i = 0;
        _conj24r = x1r; _conj24i = -(x1i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_conj24r, _conj24i, x2r, x2i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(_mul25r, _mul25i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 5.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _ang26r + _sin30r; _add31i = _ang26i + _sin30i;
        double angle_part1 = _add31r; /* +_add31ii */
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 7.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _arr36r = 0, _arr36i = 0;
        { int _idx = j; _arr36r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr36i = 0; }
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _arr38r = 0, _arr38i = 0;
        { int _idx = j; _arr38r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr38i = 0; }
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c37r, _c37i, _arr38r, _arr38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _arr36r + _mul39r; _add40i = _arr36i + _mul39i;
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(_add40r, _add40i); _ang41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_cos35r, _cos35i, _ang41r, _ang41i, &_mul42r, &_mul42i);
        double angle_part2 = _mul42r; /* +_mul42ii */
        double _add43r = 0, _add43i = 0;
        _add43r = angle_part1 + angle_part2; _add43i = 0 + 0;
        double angle = _add43r; /* +_add43ii */
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, angle, 0, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(magnitude, 0, _exp46r, _exp46i, &_mul47r, &_mul47i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_330_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
    for (int j = 0; j < (int)(n); j++) {
        double _arr5r = 0, _arr5i = 0;
        { int _idx = j; _arr5r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr5i = 0; }
        double rec = _arr5r; /* +_arr5ii */
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr6i = 0; }
        double imc = _arr6r; /* +_arr6ii */
        double _mul7r = 0, _mul7i = 0;
        c_mul(rec, 0, j, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(j, 0, j, 0, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(imc, 0, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _mul10r; _add11i = _mul7i + _mul10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 4.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log15r, _log15i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = rec - imc; _sub25i = 0 - 0;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_sub25r, _sub25i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_cos24r, _cos24i, _add28r, _add28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul20r + _mul29r; _add30i = _mul20i + _mul29i;
        double mag = _add30r; /* +_add30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 6.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang31r, _ang31i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 8.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang37r, _ang37i, _cos41r, _cos41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul36r + _mul42r; _add43i = _mul36i + _mul42i;
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
        double _sin49r = 0, _sin49i = 0;
        c_sin(ang, 0, &_sin49r, &_sin49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_sin49r, _sin49i, _c50r, _c50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _cos48r + _mul51r; _add52i = _cos48i + _mul51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(mag, 0, _add52r, _add52i, &_mul53r, &_mul53i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_331_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double mag_part1 = _log3r; /* +_log3ii */
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + n; _add5i = _abs4i + 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = _add5r - j; _sub6i = _add5i - 0;
        double _log7r = 0, _log7i = 0;
        c_log(_sub6r, _sub6i, &_log7r, &_log7i);
        double mag_part2 = _log7r; /* +_log7ii */
        double _mul8r = 0, _mul8i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(j, 0, _c10r, _c10i, &_div11r, &_div11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_div11r, _div11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin9r, _sin9i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _mul13r; _add14i = _mul8i + _mul13i;
        double mag = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(j, 0, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang15r, _ang15i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double angle_part1 = _mul19r; /* +_mul19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double angle_part2 = _mul24r; /* +_mul24ii */
        double _add25r = 0, _add25i = 0;
        _add25r = angle_part1 + angle_part2; _add25i = 0 + 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 7.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add25r + _sin29r; _add30i = _add25i + _sin29i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_332: too complex for auto-transpile, stubbed */
static void poly_332_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_333_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _im1r = 0, _im1i = 0;
        _im1r = x1i; _im1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _im1r, _im1i, &_mul2r, &_mul2i);
        double _sin3r = 0, _sin3i = 0;
        c_sin(_mul2r, _mul2i, &_sin3r, &_sin3i);
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _re4r, _re4i, &_mul5r, &_mul5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_mul5r, _mul5i, &_cos6r, &_cos6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _sin3r + _cos6r; _add7i = _sin3i + _cos6i;
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x1r, x1i); _ang8i = 0;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_ang8r, _ang8i, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _div11r = 0, _div11i = 0;
        c_div(_mul10r, _mul10i, j, 0, &_div11r, &_div11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _add7r + _div11r; _add12i = _add7i + _div11i;
        double angle = _add12r; /* +_add12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.5; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(j, 0, 1.5, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log16r, _log16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _neg20r = 0, _neg20i = 0;
        _neg20r = -(j); _neg20i = -(0);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x2r, x2i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
        double _div24r = 0, _div24i = 0;
        c_div(_neg20r, _neg20i, _add23r, _add23i, &_div24r, &_div24i);
        double _exp25r = 0, _exp25i = 0;
        c_exp2(_div24r, _div24i, &_exp25r, &_exp25i);
        double _sqrt26r = 0, _sqrt26i = 0;
        c_powr(j, 0, 0.5, &_sqrt26r, &_sqrt26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_exp25r, _exp25i, _sqrt26r, _sqrt26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul19r + _mul27r; _add28i = _mul19i + _mul27i;
        double magnitude = _add28r; /* +_add28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, angle, 0, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(magnitude, 0, _exp31r, _exp31i, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _neg34r = 0, _neg34i = 0;
        _neg34r = -(_c33r); _neg34i = -(_c33i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_neg34r, _neg34i, angle, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = j + _c36r; _add37i = 0 + _c36i;
        double _div38r = 0, _div38i = 0;
        c_div(_mul35r, _mul35i, _add37r, _add37i, &_div38r, &_div38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_div38r, _div38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(magnitude, 0, _exp39r, _exp39i, &_mul40r, &_mul40i);
        double _conj41r = 0, _conj41i = 0;
        _conj41r = _mul40r; _conj41i = -(_mul40i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul32r + _conj41r; _add42i = _mul32i + _conj41i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_334_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 5.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 7.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _div11r + _ang12r; _add13i = _div11i + _ang12i;
        double _cos14r = 0, _cos14i = 0;
        c_cos(_add13r, _add13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _sin8r + _cos14r; _add15i = _sin8i + _cos14i;
        double angle_part = _add15r; /* +_add15ii */
        double _arr16r = 0, _arr16i = 0;
        { int _idx = j; _arr16r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr16i = 0; }
        double _arr17r = 0, _arr17i = 0;
        { int _idx = j; _arr17r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr17i = 0; }
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
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x1r, x1i); _abs23i = 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _abs24r; _add25i = _abs23i + _abs24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log22r, _log22i, _add25r, _add25i, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_mul26r, _mul26i, _add28r, _add28i, &_div29r, &_div29i);
        double magnitude_part = _div29r; /* +_div29ii */
        double _arr30r = 0, _arr30i = 0;
        { int _idx = j; _arr30r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr30i = 0; }
        double _c31r = 0, _c31i = 0;
        _c31r = 3.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(_arr30r, _arr30i, _arr30r, _arr30i, &_pow32r, &_pow32i);
        c_mul(_pow32r, _pow32i, _arr30r, _arr30i, &_pow32r, &_pow32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _arr34r = 0, _arr34i = 0;
        { int _idx = j; _arr34r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr34i = 0; }
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_mul(_arr34r, _arr34i, _arr34r, _arr34i, &_pow36r, &_pow36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c33r, _c33i, _pow36r, _pow36i, &_mul37r, &_mul37i);
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _pow32r - _mul37r; _sub38i = _pow32i - _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, M_PI, 0, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 3.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_mul39r, _mul39i, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_sub38r, _sub38i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double intricate_term = _mul43r; /* +_mul43ii */
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, angle_part, 0, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(magnitude_part, 0, _exp46r, _exp46i, &_mul47r, &_mul47i);
        double _conj48r = 0, _conj48i = 0;
        _conj48r = x1r; _conj48i = -(x1i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_conj48r, _conj48i, intricate_term, 0, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul47r + _mul49r; _add50i = _mul47i + _mul49i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_335_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double rec_t1 = _re1r; /* +_re1ii */
    double _im2r = 0, _im2i = 0;
    _im2r = x1i; _im2i = 0;
    double imc_t1 = _im2r; /* +_im2ii */
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec_t2 = _re3r; /* +_re3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc_t2 = _im4r; /* +_im4ii */
    for (int j = 0; j < (int)(n); j++) {
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 7.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 5.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_div11r, _div11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin8r, _sin8i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang14r, _ang14i, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul13r + _mul16r; _add17i = _mul13i + _mul16i;
        double angle_part = _add17r; /* +_add17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
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
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log21r, _log21i, _pow23r, _pow23i, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _c25r + j; _add26i = _c25i + 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul24r, _mul24i, _add26r, _add26i, &_div27r, &_div27i);
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x2r, x2i); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(j, 0, &_sin30r, &_sin30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _c29r + _sin30r; _add31i = _c29i + _sin30i;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_abs28r, _abs28i, _add31r, &_pow32r, &_pow32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _div27r + _pow32r; _add33i = _div27i + _pow32i;
        double magnitude_part = _add33r; /* +_add33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _im35r = 0, _im35i = 0;
        _im35r = x1i; _im35i = 0;
        double _re36r = 0, _re36i = 0;
        _re36r = x2r; _re36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_im35r, _im35i, _re36r, _re36i, &_mul37r, &_mul37i);
        double _div38r = 0, _div38i = 0;
        c_div(_mul37r, _mul37i, j, 0, &_div38r, &_div38i);
        double _add39r = 0, _add39i = 0;
        _add39r = angle_part + _div38r; _add39i = 0 + _div38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c34r, _c34i, _add39r, _add39i, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double phase_shift = _exp41r; /* +_exp41ii */
        double _mul42r = 0, _mul42i = 0;
        c_mul(magnitude_part, 0, phase_shift, 0, &_mul42r, &_mul42i);
        double _conj43r = 0, _conj43i = 0;
        _conj43r = x1r; _conj43i = -(x1i);
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x2r; _conj44i = -(x2i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_conj43r, _conj43i, _conj44r, _conj44i, &_mul45r, &_mul45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 1.0; _c46i = 0;
        double _add47r = 0, _add47i = 0;
        _add47r = j + _c46r; _add47i = 0 + _c46i;
        double _div48r = 0, _div48i = 0;
        c_div(_mul45r, _mul45i, _add47r, _add47i, &_div48r, &_div48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul42r + _div48r; _add49i = _mul42i + _div48i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
    }
    for (int k = 0; k < (int)(n); k++) {
        double _c50r = 0, _c50i = 0;
        _c50r = 5.0; _c50i = 0;
        double _mod51r = 0, _mod51i = 0;
        _mod51r = fmod(k, _c50r); _mod51i = 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 0;
        if (_mod51r == _c52r) {
            double _c53r = 0, _c53i = 0;
            _c53r = 1.0; _c53i = 0;
            double _c54r = 0, _c54i = 0;
            _c54r = 0.5; _c54i = 0;
            double _mul55r = 0, _mul55i = 0;
            c_mul(k, 0, M_PI, 0, &_mul55r, &_mul55i);
            double _c56r = 0, _c56i = 0;
            _c56r = 3.0; _c56i = 0;
            double _div57r = 0, _div57i = 0;
            c_div(_mul55r, _mul55i, _c56r, _c56i, &_div57r, &_div57i);
            double _cos58r = 0, _cos58i = 0;
            c_cos(_div57r, _div57i, &_cos58r, &_cos58i);
            double _mul59r = 0, _mul59i = 0;
            c_mul(_c54r, _c54i, _cos58r, _cos58i, &_mul59r, &_mul59i);
            double _add60r = 0, _add60i = 0;
            _add60r = _c53r + _mul59r; _add60i = _c53i + _mul59i;
            { double _tr = cRe[k]*_add60r - cIm[k]*_add60i; cIm[k] = cRe[k]*_add60i + cIm[k]*_add60r; cRe[k] = _tr; }
        } else {
            double _c61r = 0, _c61i = 0;
            _c61r = 3.0; _c61i = 0;
            double _mod62r = 0, _mod62i = 0;
            _mod62r = fmod(k, _c61r); _mod62i = 0;
            double _c63r = 0, _c63i = 0;
            _c63r = 0.0; _c63i = 0;
            if (_mod62r == _c63r) {
                double _c64r = 0, _c64i = 0;
                _c64r = 1.0; _c64i = 0;
                double _c65r = 0, _c65i = 0;
                _c65r = 0.3; _c65i = 0;
                double _mul66r = 0, _mul66i = 0;
                c_mul(k, 0, M_PI, 0, &_mul66r, &_mul66i);
                double _c67r = 0, _c67i = 0;
                _c67r = 4.0; _c67i = 0;
                double _div68r = 0, _div68i = 0;
                c_div(_mul66r, _mul66i, _c67r, _c67i, &_div68r, &_div68i);
                double _sin69r = 0, _sin69i = 0;
                c_sin(_div68r, _div68i, &_sin69r, &_sin69i);
                double _mul70r = 0, _mul70i = 0;
                c_mul(_c65r, _c65i, _sin69r, _sin69i, &_mul70r, &_mul70i);
                double _add71r = 0, _add71i = 0;
                _add71r = _c64r + _mul70r; _add71i = _c64i + _mul70i;
                { double _tr = cRe[k]*_add71r - cIm[k]*_add71i; cIm[k] = cRe[k]*_add71i + cIm[k]*_add71r; cRe[k] = _tr; }
            } else {
                double _c72r = 0, _c72i = 0;
                _c72r = 1.0; _c72i = 0;
                double _c73r = 0, _c73i = 0;
                _c73r = 0.2; _c73i = 0;
                double _c74r = 0, _c74i = 0;
                _c74r = 1.0; _c74i = 0;
                double _add75r = 0, _add75i = 0;
                _add75r = k + _c74r; _add75i = 0 + _c74i;
                double _log76r = 0, _log76i = 0;
                c_log(_add75r, _add75i, &_log76r, &_log76i);
                double _mul77r = 0, _mul77i = 0;
                c_mul(_c73r, _c73i, _log76r, _log76i, &_mul77r, &_mul77i);
                double _add78r = 0, _add78i = 0;
                _add78r = _c72r + _mul77r; _add78i = _c72i + _mul77i;
                { double _tr = cRe[k]*_add78r - cIm[k]*_add78i; cIm[k] = cRe[k]*_add78i + cIm[k]*_add78r; cRe[k] = _tr; }
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_336_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 7.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mod2r + _c3r; _add4i = _mod2i + _c3i;
        double k = _add4r; /* +_add4ii */
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 6.0; _c7i = 0;
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
        double r = _add17r; /* +_add17ii */
        double _im18r = 0, _im18i = 0;
        _im18r = x1i; _im18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 4.0; _c20i = 0;
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
        _c26r = 3.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_im24r, _im24i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul23r - _mul29r; _sub30i = _mul23i - _mul29i;
        double s = _sub30r; /* +_sub30ii */
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x1r, x1i); _abs31i = 0;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x2r, x2i); _abs32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _abs31r + _abs32r; _add33i = _abs31i + _abs32i;
        double _add34r = 0, _add34i = 0;
        _add34r = _add33r + j; _add34i = _add33i + 0;
        double _log35r = 0, _log35i = 0;
        c_log(_add34r, _add34i, &_log35r, &_log35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _sin37r = 0, _sin37i = 0;
        c_sin(r, 0, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _c36r + _sin37r; _add38i = _c36i + _sin37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_log35r, _log35i, _add38r, _add38i, &_mul39r, &_mul39i);
        double _abs40r = 0, _abs40i = 0;
        _abs40r = c_abs(s, 0); _abs40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _mul39r + _abs40r; _add41i = _mul39i + _abs40i;
        double magnitude = _add41r; /* +_add41ii */
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x1r, x1i); _ang42i = 0;
        double _cos43r = 0, _cos43i = 0;
        c_cos(r, 0, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_ang42r, _ang42i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x2r, x2i); _ang45i = 0;
        double _sin46r = 0, _sin46i = 0;
        c_sin(s, 0, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_ang45r, _ang45i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul44r + _mul47r; _add48i = _mul44i + _mul47i;
        double angle = _add48r; /* +_add48ii */
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c49r, _c49i, angle, 0, &_mul50r, &_mul50i);
        double _exp51r = 0, _exp51i = 0;
        c_exp2(_mul50r, _mul50i, &_exp51r, &_exp51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(magnitude, 0, _exp51r, _exp51i, &_mul52r, &_mul52i);
        double _conj53r = 0, _conj53i = 0;
        _conj53r = x1r; _conj53i = -(x1i);
        double _pow54r = 0, _pow54i = 0;
        c_powr(_conj53r, _conj53i, k, &_pow54r, &_pow54i);
        double _conj55r = 0, _conj55i = 0;
        _conj55r = x2r; _conj55i = -(x2i);
        double _pow56r = 0, _pow56i = 0;
        c_powr(_conj55r, _conj55i, k, &_pow56r, &_pow56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_pow54r, _pow54i, _pow56r, _pow56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _mul52r + _mul57r; _add58i = _mul52i + _mul57i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_337_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < 35; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _re1r, _re1i, &_mul2r, &_mul2i);
        double _sin3r = 0, _sin3i = 0;
        c_sin(_mul2r, _mul2i, &_sin3r, &_sin3i);
        double _im4r = 0, _im4i = 0;
        _im4r = x2i; _im4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _im4r, _im4i, &_mul5r, &_mul5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_mul5r, _mul5i, &_cos6r, &_cos6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _sin3r + _cos6r; _add7i = _sin3i + _cos6i;
        double _re8r = 0, _re8i = 0;
        _re8r = x1r; _re8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _re8r + _c9r; _add10i = _re8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log11r, _log11i, _ang12r, _ang12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add7r + _mul13r; _add14i = _add7i + _mul13i;
        double phase = _add14r; /* +_add14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_powr(_abs15r, _abs15i, j, &_pow16r, &_pow16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 35.0; _c18i = 0;
        double _sub19r = 0, _sub19i = 0;
        _sub19r = _c18r - j; _sub19i = _c18i - 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_abs17r, _abs17i, _sub19r, &_pow20r, &_pow20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _pow16r + _pow20r; _add21i = _pow16i + _pow20i;
        double _c22r = 0, _c22i = 0;
        _c22r = 7.0; _c22i = 0;
        double _mod23r = 0, _mod23i = 0;
        _mod23r = fmod(j, _c22r); _mod23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _mod23r + _c24r; _add25i = _mod23i + _c24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_add21r, _add21i, _add25r, _add25i, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_mul26r, _mul26i, _add28r, _add28i, &_div29r, &_div29i);
        double magnitude = _div29r; /* +_div29ii */
        double _cos30r = 0, _cos30i = 0;
        c_cos(phase, 0, &_cos30r, &_cos30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(phase, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c31r, _c31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _cos30r + _mul33r; _add34i = _cos30i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(magnitude, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_338_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(k, 0, M_PI, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 5.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul4r, _mul4i, _c5r, _c5i, &_div6r, &_div6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_div6r, _div6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re3r, _re3i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _re9r = 0, _re9i = 0;
        _re9r = x2r; _re9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, M_PI, 0, &_mul10r, &_mul10i);
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
        double real_part = _add15r; /* +_add15ii */
        double _im16r = 0, _im16i = 0;
        _im16r = x1i; _im16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(k, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 6.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_im16r, _im16i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _im22r = 0, _im22i = 0;
        _im22r = x2i; _im22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(k, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 8.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_im22r, _im22i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul21r - _mul27r; _sub28i = _mul21i - _mul27i;
        double imag_part = _sub28r; /* +_sub28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_mul(real_part, 0, real_part, 0, &_pow30r, &_pow30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(imag_part, 0, imag_part, 0, &_pow32r, &_pow32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _pow30r + _pow32r; _add33i = _pow30i + _pow32i;
        double _sqrt34r = 0, _sqrt34i = 0;
        c_powr(_add33r, _add33i, 0.5, &_sqrt34r, &_sqrt34i);
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(k, 0); _abs35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _abs35r + _c36r; _add37i = _abs35i + _c36i;
        double _log38r = 0, _log38i = 0;
        c_log(_add37r, _add37i, &_log38r, &_log38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_sqrt34r, _sqrt34i, _log38r, _log38i, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _sin41r = 0, _sin41i = 0;
        c_sin(k, 0, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _c40r + _sin41r; _add42i = _c40i + _sin41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_mul39r, _mul39i, _add42r, _add42i, &_mul43r, &_mul43i);
        double magnitude = _mul43r; /* +_mul43ii */
        double _at244r = 0, _at244i = 0;
        _at244r = atan2(imag_part, real_part); _at244i = 0;
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x1r, x1i); _ang45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(k, 0, _ang45r, _ang45i, &_mul46r, &_mul46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(_mul46r, _mul46i, &_sin47r, &_sin47i);
        double _ang48r = 0, _ang48i = 0;
        _ang48r = c_arg(x2r, x2i); _ang48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(k, 0, _ang48r, _ang48i, &_mul49r, &_mul49i);
        double _cos50r = 0, _cos50i = 0;
        c_cos(_mul49r, _mul49i, &_cos50r, &_cos50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_sin47r, _sin47i, _cos50r, _cos50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _at244r + _mul51r; _add52i = _at244i + _mul51i;
        double angle = _add52r; /* +_add52ii */
        double _c53r = 0, _c53i = 0;
        _c53r = 0.0; _c53i = 1.0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c53r, _c53i, angle, 0, &_mul54r, &_mul54i);
        double _exp55r = 0, _exp55i = 0;
        c_exp2(_mul54r, _mul54i, &_exp55r, &_exp55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(magnitude, 0, _exp55r, _exp55i, &_mul56r, &_mul56i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    }
    double _c57r = 0, _c57i = 0;
    _c57r = 1.0; _c57i = 0;
    double _add58r = 0, _add58i = 0;
    _add58r = n + _c57r; _add58i = 0 + _c57i;
    for (int r = 1; r < (int)(_add58r); r++) {
        double _cf59r = 0, _cf59i = 0;
        { int _idx = ((int)(n) - r); if (_idx >= 0 && _idx < 36) { _cf59r = cRe[_idx]; _cf59i = cIm[_idx]; } }
        double _conj60r = 0, _conj60i = 0;
        _conj60r = _cf59r; _conj60i = -(_cf59i);
        double _mul61r = 0, _mul61i = 0;
        c_mul(r, 0, M_PI, 0, &_mul61r, &_mul61i);
        double _c62r = 0, _c62i = 0;
        _c62r = 10.0; _c62i = 0;
        double _div63r = 0, _div63i = 0;
        c_div(_mul61r, _mul61i, _c62r, _c62i, &_div63r, &_div63i);
        double _sin64r = 0, _sin64i = 0;
        c_sin(_div63r, _div63i, &_sin64r, &_sin64i);
        double _mul65r = 0, _mul65i = 0;
        c_mul(_conj60r, _conj60i, _sin64r, _sin64i, &_mul65r, &_mul65i);
        cRe[(r - 1)] += _mul65r; cIm[(r - 1)] += _mul65i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_339: too complex for auto-transpile, stubbed */
static void poly_339_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_340_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _re4r, _re4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(_sin6r, _sin6i); _abs7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log3r, _log3i, _abs7r, _abs7i, &_mul8r, &_mul8i);
        double _sqrt9r = 0, _sqrt9i = 0;
        c_powr(j, 0, 0.5, &_sqrt9r, &_sqrt9i);
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _im10r, _im10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_cos12r, _cos12i); _abs13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sqrt9r, _sqrt9i, _abs13r, _abs13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul8r + _mul14r; _add15i = _mul8i + _mul14i;
        double mag_part = _add15r; /* +_add15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang16r, _ang16i, j, 0, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(j, 0, &_sin18r, &_sin18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul17r + _sin18r; _add19i = _mul17i + _sin18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(j, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _add19r + _cos22r; _add23i = _add19i + _cos22i;
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
        _conj28r = x2r; _conj28i = -(x2i);
        double _c29r = 0, _c29i = 0;
        _c29r = 5.0; _c29i = 0;
        double _mod30r = 0, _mod30i = 0;
        _mod30r = fmod(j, _c29r); _mod30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _mod30r + _c31r; _add32i = _mod30i + _c31i;
        double _pow33r = 0, _pow33i = 0;
        c_powr(_conj28r, _conj28i, _add32r, &_pow33r, &_pow33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul27r + _pow33r; _add34i = _mul27i + _pow33i;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add34r; cIm[_idx] = _add34i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_341_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _log4r = 0, _log4i = 0;
        c_log(_add3r, _add3i, &_log4r, &_log4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_abs1r, _abs1i, _log4r, _log4i, &_mul5r, &_mul5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _sqrt7r = 0, _sqrt7i = 0;
        c_powr(j, 0, 0.5, &_sqrt7r, &_sqrt7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_abs6r, _abs6i, _sqrt7r, _sqrt7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _mul8r; _add9i = _mul5i + _mul8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.3; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(j, 0, 1.3, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _c10r + _pow12r; _add13i = _c10i + _pow12i;
        double _div14r = 0, _div14i = 0;
        c_div(_add9r, _add9i, _add13r, _add13i, &_div14r, &_div14i);
        double mag = _div14r; /* +_div14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(j, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(j, 0, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang18r, _ang18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul17r + _mul22r; _add23i = _mul17i + _mul22i;
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(j, 0, _c24r, _c24i, &_div25r, &_div25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_div25r, _div25i, M_PI, 0, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _add23r + _sin27r; _add28i = _add23i + _sin27i;
        double angle = _add28r; /* +_add28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _c30r = 0, _c30i = 0;
        _c30r = 4.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_div31r, _div31i, M_PI, 0, &_mul32r, &_mul32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_mul32r, _mul32i, &_sin33r, &_sin33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 5.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(j, 0, _c34r, _c34i, &_div35r, &_div35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_div35r, _div35i, M_PI, 0, &_mul36r, &_mul36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_mul36r, _mul36i, &_cos37r, &_cos37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sin33r + _cos37r; _add38i = _sin33i + _cos37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c29r, _c29i, _add38r, _add38i, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double perturb = _exp40r; /* +_exp40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag, 0, _exp43r, _exp43i, &_mul44r, &_mul44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_mul44r, _mul44i, perturb, 0, &_mul45r, &_mul45i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_342_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_re5r, _re5i, _log8r, _log8i, &_mul9r, &_mul9i);
        double r = _mul9r; /* +_mul9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x2i; _im10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(k, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 5.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_im10r, _im10i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double s = _mul15r; /* +_mul15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(k, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 3.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang16r, _ang16i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(k, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 4.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul21r + _sin25r; _add26i = _mul21i + _sin25i;
        double theta = _add26r; /* +_add26ii */
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x1r, x1i); _abs27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_powr(_abs27r, _abs27i, k, &_pow28r, &_pow28i);
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x2r, x2i); _abs29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs29r + j; _add30i = _abs29i + 0;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _pow28r + _log31r; _add32i = _pow28i + _log31i;
        double magnitude = _add32r; /* +_add32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(s, 0, _c33r, _c33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = r + _mul34r; _add35i = 0 + _mul34i;
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, theta, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_add35r, _add35i, _exp38r, _exp38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = x1r + x2r; _add40i = x1i + x2i;
        double _conj41r = 0, _conj41i = 0;
        _conj41r = _add40r; _conj41i = -(_add40i);
        double _pow42r = 0, _pow42i = 0;
        c_powr(_conj41r, _conj41i, k, &_pow42r, &_pow42i);
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x1r, x1i); _ang43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(j, 0, _ang43r, _ang43i, &_mul44r, &_mul44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_mul44r, _mul44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_pow42r, _pow42i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _mul39r + _mul46r; _add47i = _mul39i + _mul46i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_343_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double rec1 = _re1r; /* +_re1ii */
    double _im2r = 0, _im2i = 0;
    _im2r = x1i; _im2i = 0;
    double imc1 = _im2r; /* +_im2ii */
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec2 = _re3r; /* +_re3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc2 = _im4r; /* +_im4ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _mul7r = 0, _mul7i = 0;
        c_mul(rec1, 0, _add6r, _add6i, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        double _div10r = 0, _div10i = 0;
        c_div(imc2, 0, _add9r, _add9i, &_div10r, &_div10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _div10r; _add11i = _mul7i + _div10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
        double _log13r = 0, _log13i = 0;
        c_log(_abs12r, _abs12i, &_log13r, &_log13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(rec1, 0, _add15r, _add15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _div20r = 0, _div20i = 0;
        c_div(imc2, 0, _add19r, _add19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin17r, _sin17i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _log13r + _mul22r; _add23i = _log13i + _mul22i;
        double mag_part = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 0.5; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_powr(j, 0, 0.5, &_pow26r, &_pow26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang24r, _ang24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _sqrt29r = 0, _sqrt29i = 0;
        c_powr(j, 0, 0.5, &_sqrt29r, &_sqrt29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _sqrt29r, _sqrt29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul27r + _mul30r; _add31i = _mul27i + _mul30i;
        double _re32r = 0, _re32i = 0;
        _re32r = x1r; _re32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, _re32r, _re32i, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add31r + _sin34r; _add35i = _add31i + _sin34i;
        double _im36r = 0, _im36i = 0;
        _im36r = x2i; _im36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, _im36r, _im36i, &_mul37r, &_mul37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_mul37r, _mul37i, &_cos38r, &_cos38i);
        double _sub39r = 0, _sub39i = 0;
        _sub39r = _add35r - _cos38r; _sub39i = _add35i - _cos38i;
        double angle_part = _sub39r; /* +_sub39ii */
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

static void poly_344_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
    for (int j = 0; j < (int)(n); j++) {
        double _arr5r = 0, _arr5i = 0;
        { int _idx = j; _arr5r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr5i = 0; }
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr6i = 0; }
        double _mul7r = 0, _mul7i = 0;
        c_mul(_arr5r, _arr5i, _arr6r, _arr6i, &_mul7r, &_mul7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_mul7r, _mul7i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
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
        double _c19r = 0, _c19i = 0;
        _c19r = 4.0; _c19i = 0;
        double _mod20r = 0, _mod20i = 0;
        _mod20r = fmod(j, _c19r); _mod20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _mod20r + _c21r; _add22i = _mod20i + _c21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul18r, _mul18i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 5.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_div27r, _div27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang24r, _ang24i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, M_PI, 0, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 7.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_mul31r, _mul31i, _c32r, _c32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang30r, _ang30i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul29r + _mul35r; _add36i = _mul29i + _mul35i;
        double _arr37r = 0, _arr37i = 0;
        { int _idx = j; _arr37r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr37i = 0; }
        double _arr38r = 0, _arr38i = 0;
        { int _idx = j; _arr38r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr38i = 0; }
        double _add39r = 0, _add39i = 0;
        _add39r = _arr37r + _arr38r; _add39i = _arr37i + _arr38i;
        double _abs40r = 0, _abs40i = 0;
        _abs40r = c_abs(_add39r, _add39i); _abs40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _abs40r + _c41r; _add42i = _abs40i + _c41i;
        double _log43r = 0, _log43i = 0;
        c_log(_add42r, _add42i, &_log43r, &_log43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _add36r + _log43r; _add44i = _add36i + _log43i;
        double ang = _add44r; /* +_add44ii */
        double _cos45r = 0, _cos45i = 0;
        c_cos(ang, 0, &_cos45r, &_cos45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _sin47r = 0, _sin47i = 0;
        c_sin(ang, 0, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c46r, _c46i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _cos45r + _mul48r; _add49i = _cos45i + _mul48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(mag, 0, _add49r, _add49i, &_mul50r, &_mul50i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_345_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(_arr5r, _arr5i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log9r, _log9i, 0, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _c11r; _add12i = 0 + _c11i;
        double _div13r = 0, _div13i = 0;
        c_div(_mul10r, _mul10i, _add12r, _add12i, &_div13r, &_div13i);
        double mag_part = _div13r; /* +_div13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _sin15r = 0, _sin15i = 0;
        c_sin(j, 0, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang14r, _ang14i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 3.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang17r, _ang17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul16r + _mul21r; _add22i = _mul16i + _mul21i;
        double angle_part = _add22r; /* +_add22ii */
        double _cos23r = 0, _cos23i = 0;
        c_cos(angle_part, 0, &_cos23r, &_cos23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(angle_part, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c24r, _c24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _cos23r + _mul26r; _add27i = _cos23i + _mul26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_part, 0, _add27r, _add27i, &_mul28r, &_mul28i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
        double _cf29r = 0, _cf29i = 0;
        { int _idx = j; if (_idx >= 0 && _idx < 36) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
        double _conj30r = 0, _conj30i = 0;
        _conj30r = _cf29r; _conj30i = -(_cf29i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, M_PI, 0, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 4.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_mul31r, _mul31i, _c32r, _c32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_conj30r, _conj30i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        cRe[j] += _mul35r; cIm[j] += _mul35i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_346_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _div3r = 0, _div3i = 0;
        c_div(j, 0, _c2r, _c2i, &_div3r, &_div3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_div3r, _div3i, &_sin4r, &_sin4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_re1r, _re1i, _sin4r, _sin4i, &_mul5r, &_mul5i);
        double _re6r = 0, _re6i = 0;
        _re6r = x2r; _re6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 3.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(j, 0, _c7r, _c7i, &_div8r, &_div8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_div8r, _div8i, &_cos9r, &_cos9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_re6r, _re6i, _cos9r, _cos9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul5r + _mul10r; _add11i = _mul5i + _mul10i;
        double rec_part = _add11r; /* +_add11ii */
        double _im12r = 0, _im12i = 0;
        _im12r = x1i; _im12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 4.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(j, 0, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_im12r, _im12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _im17r = 0, _im17i = 0;
        _im17r = x2i; _im17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_im17r, _im17i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _mul16r - _mul21r; _sub22i = _mul16i - _mul21i;
        double imc_part = _sub22r; /* +_sub22ii */
        double _add23r = 0, _add23i = 0;
        _add23r = rec_part + imc_part; _add23i = 0 + 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_add23r, _add23i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.2; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_powr(j, 0, 1.2, &_pow29r, &_pow29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_log27r, _log27i, _pow29r, _pow29i, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 6.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _c31r + _sin35r; _add36i = _c31i + _sin35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_mul30r, _mul30i, _add36r, _add36i, &_mul37r, &_mul37i);
        double magnitude = _mul37r; /* +_mul37ii */
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 7.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(j, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang38r, _ang38i, _cos41r, _cos41i, &_mul42r, &_mul42i);
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x2r, x2i); _ang43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 8.0; _c44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(j, 0, _c44r, _c44i, &_div45r, &_div45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_div45r, _div45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_ang43r, _ang43i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul42r + _mul47r; _add48i = _mul42i + _mul47i;
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
        c_mul(magnitude, 0, _add53r, _add53i, &_mul54r, &_mul54i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_347: too complex for auto-transpile, stubbed */
static void poly_347_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_348_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        _c14r = 5.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_cos16r, _cos16i, _abs17r, _abs17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul12r + _mul18r; _add19i = _mul12i + _mul18i;
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
        double angle = _add26r; /* +_add26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, angle, 0, &_mul28r, &_mul28i);
        double _exp29r = 0, _exp29i = 0;
        c_exp2(_mul28r, _mul28i, &_exp29r, &_exp29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(magnitude, 0, _exp29r, _exp29i, &_mul30r, &_mul30i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int k = 0; k < (int)(n); k++) {
        double _re31r = 0, _re31i = 0;
        _re31r = x1r; _re31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 5.0; _c32i = 0;
        double _mod33r = 0, _mod33i = 0;
        _mod33r = fmod(k, _c32r); _mod33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _mod33r + _c34r; _add35i = _mod33i + _c34i;
        double _pow36r = 0, _pow36i = 0;
        c_powr(_re31r, _re31i, _add35r, &_pow36r, &_pow36i);
        double _im37r = 0, _im37i = 0;
        _im37r = x2i; _im37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 3.0; _c38i = 0;
        double _mod39r = 0, _mod39i = 0;
        _mod39r = fmod(k, _c38r); _mod39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _mod39r + _c40r; _add41i = _mod39i + _c40i;
        double _pow42r = 0, _pow42i = 0;
        c_powr(_im37r, _im37i, _add41r, &_pow42r, &_pow42i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = _pow36r - _pow42r; _sub43i = _pow36i - _pow42i;
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _sin45r = 0, _sin45i = 0;
        c_sin(k, 0, &_sin45r, &_sin45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(k, 0, &_cos46r, &_cos46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _sin45r + _cos46r; _add47i = _sin45i + _cos46i;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c44r, _c44i, _add47r, _add47i, &_mul48r, &_mul48i);
        double _exp49r = 0, _exp49i = 0;
        c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_sub43r, _sub43i, _exp49r, _exp49i, &_mul50r, &_mul50i);
        cRe[k] += _mul50r; cIm[k] += _mul50i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_349: too complex for auto-transpile, stubbed */
static void poly_349_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_350_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < 35; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, M_PI, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 6.0; _c5i = 0;
        double _div6r = 0, _div6i = 0;
        c_div(_mul4r, _mul4i, _c5r, _c5i, &_div6r, &_div6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_div6r, _div6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log3r, _log3i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double mag_part1 = _mul8r; /* +_mul8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs9r + j; _add10i = _abs9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 8.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log11r, _log11i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double mag_part2 = _mul16r; /* +_mul16ii */
        double _add17r = 0, _add17i = 0;
        _add17r = mag_part1 + mag_part2; _add17i = 0 + 0;
        double _sqrt18r = 0, _sqrt18i = 0;
        c_powr(j, 0, 0.5, &_sqrt18r, &_sqrt18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _add17r + _sqrt18r; _add19i = _add17i + _sqrt18i;
        double magnitude = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double angle_part1 = _mul24r; /* +_mul24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 4.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_div27r, _div27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double angle_part2 = _mul29r; /* +_mul29ii */
        double _add30r = 0, _add30i = 0;
        _add30r = angle_part1 + angle_part2; _add30i = 0 + 0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(j, 0, &_sin31r, &_sin31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sin31r, _sin31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add30r + _mul35r; _add36i = _add30i + _mul35i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_351_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double rec1 = _re1r; /* +_re1ii */
    double _im2r = 0, _im2i = 0;
    _im2r = x1i; _im2i = 0;
    double imc1 = _im2r; /* +_im2ii */
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double rec2 = _re3r; /* +_re3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc2 = _im4r; /* +_im4ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c5r = 0, _c5i = 0;
        _c5r = 1.5; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(j, 0, 1.5, &_pow6r, &_pow6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(rec1, 0, _pow6r, _pow6i, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        double _div10r = 0, _div10i = 0;
        c_div(imc2, 0, _add9r, _add9i, &_div10r, &_div10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _div10r; _add11i = _mul7i + _div10i;
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
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _c16r + _sin20r; _add21i = _c16i + _sin20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log15r, _log15i, _add21r, _add21i, &_mul22r, &_mul22i);
        double mag_part = _mul22r; /* +_mul22ii */
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
        _c29r = 5.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul27r + _mul32r; _add33i = _mul27i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 7.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add33r + _sin37r; _add38i = _add33i + _sin37i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_352_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double sum_real = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double sum_imag = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        for (int k = 1; k < (int)(_add4r); k++) {
            double _re5r = 0, _re5i = 0;
            _re5r = x1r; _re5i = 0;
            double _pow6r = 0, _pow6i = 0;
            c_powr(_re5r, _re5i, k, &_pow6r, &_pow6i);
            double _mul7r = 0, _mul7i = 0;
            c_mul(k, 0, M_PI, 0, &_mul7r, &_mul7i);
            double _div8r = 0, _div8i = 0;
            c_div(_mul7r, _mul7i, j, 0, &_div8r, &_div8i);
            double _cos9r = 0, _cos9i = 0;
            c_cos(_div8r, _div8i, &_cos9r, &_cos9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(_pow6r, _pow6i, _cos9r, _cos9i, &_mul10r, &_mul10i);
            sum_real += _mul10r;
            double _im11r = 0, _im11i = 0;
            _im11r = x2i; _im11i = 0;
            double _pow12r = 0, _pow12i = 0;
            c_powr(_im11r, _im11i, k, &_pow12r, &_pow12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(k, 0, M_PI, 0, &_mul13r, &_mul13i);
            double _div14r = 0, _div14i = 0;
            c_div(_mul13r, _mul13i, j, 0, &_div14r, &_div14i);
            double _sin15r = 0, _sin15i = 0;
            c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_pow12r, _pow12i, _sin15r, _sin15i, &_mul16r, &_mul16i);
            sum_imag += _mul16r;
        }
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(sum_real, 0); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 0.5; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs21r, _abs21i, 0.5, &_pow23r, &_pow23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log20r, _log20i, _pow23r, _pow23i, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(sum_imag, 0); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x2r, x2i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 0.3; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(_abs29r, _abs29i, 0.3, &_pow31r, &_pow31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_log28r, _log28i, _pow31r, _pow31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul24r + _mul32r; _add33i = _mul24i + _mul32i;
        double mag = _add33r; /* +_add33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _sin35r = 0, _sin35i = 0;
        c_sin(j, 0, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang34r, _ang34i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 2.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(j, 0, _c38r, _c38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang37r, _ang37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul36r + _mul41r; _add42i = _mul36i + _mul41i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    }
    for (int r = 0; r < (int)(n); r++) {
        double _c49r = 0, _c49i = 0;
        _c49r = 1.0; _c49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 0.1; _c50i = 0;
        double _sin51r = 0, _sin51i = 0;
        c_sin(r, 0, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c50r, _c50i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _c49r + _mul52r; _add53i = _c49i + _mul52i;
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 0.1;
        double _cos55r = 0, _cos55i = 0;
        c_cos(r, 0, &_cos55r, &_cos55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c54r, _c54i, _cos55r, _cos55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _add53r + _mul56r; _add57i = _add53i + _mul56i;
        { double _tr = cRe[r]*_add57r - cIm[r]*_add57i; cIm[r] = cRe[r]*_add57i + cIm[r]*_add57r; cRe[r] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_353_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr6i = 0; }
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
        _c12r = 1.0; _c12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(j, 0, &_sin13r, &_sin13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _c12r + _sin13r; _add14i = _c12i + _sin13i;
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(j, 0, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _add14r + _cos17r; _add18i = _add14i + _cos17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log11r, _log11i, _add18r, _add18i, &_mul19r, &_mul19i);
        double mag_variation = _mul19r; /* +_mul19ii */
        double _arr20r = 0, _arr20i = 0;
        { int _idx = j; _arr20r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr20i = 0; }
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _arr22r = 0, _arr22i = 0;
        { int _idx = j; _arr22r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr22i = 0; }
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c21r, _c21i, _arr22r, _arr22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _arr20r + _mul23r; _add24i = _arr20i + _mul23i;
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(_add24r, _add24i); _ang25i = 0;
        double _arr26r = 0, _arr26i = 0;
        { int _idx = j; _arr26r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr26i = 0; }
        double _mul27r = 0, _mul27i = 0;
        c_mul(_arr26r, _arr26i, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_mul27r, _mul27i, _add29r, _add29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _ang25r + _sin31r; _add32i = _ang25i + _sin31i;
        double _arr33r = 0, _arr33i = 0;
        { int _idx = j; _arr33r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr33i = 0; }
        double _mul34r = 0, _mul34i = 0;
        c_mul(_arr33r, _arr33i, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = j + _c35r; _add36i = 0 + _c35i;
        double _div37r = 0, _div37i = 0;
        c_div(_mul34r, _mul34i, _add36r, _add36i, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _sub39r = 0, _sub39i = 0;
        _sub39r = _add32r - _cos38r; _sub39i = _add32i - _cos38i;
        double angle_variation = _sub39r; /* +_sub39ii */
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, angle_variation, 0, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(mag_variation, 0, _exp42r, _exp42i, &_mul43r, &_mul43i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_354_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 5.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sin8r, _sin8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double part1 = _mul12r; /* +_mul12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs13r + j; _add14i = _abs13i + 0;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log15r, _log15i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double part2 = _mul20r; /* +_mul20ii */
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 4.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 6.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos24r + _sin28r; _add29i = _cos24i + _sin28i;
        double part3 = _add29r; /* +_add29ii */
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.5; _c31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, j, 0, &_mul32r, &_mul32i);
        double _pow33r = 0, _pow33i = 0;
        c_powr(_abs30r, _abs30i, _mul32r, &_pow33r, &_pow33i);
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(j, 0); _abs34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _abs34r + _c35r; _add36i = _abs34i + _c35i;
        double _log37r = 0, _log37i = 0;
        c_log(_add36r, _add36i, &_log37r, &_log37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 7.0; _c38i = 0;
        double _mod39r = 0, _mod39i = 0;
        _mod39r = fmod(j, _c38r); _mod39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _mod39r + _c40r; _add41i = _mod39i + _c40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_log37r, _log37i, _add41r, _add41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _pow33r + _mul42r; _add43i = _pow33i + _mul42i;
        double magnitude = _add43r; /* +_add43ii */
        double _add44r = 0, _add44i = 0;
        _add44r = part1 + part2; _add44i = 0 + 0;
        double _add45r = 0, _add45i = 0;
        _add45r = _add44r + part3; _add45i = _add44i + 0;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_355_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < 35; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double mag = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double angle = _c2r; /* +_c2ii */
        for (int k = 1; k < 36; k++) {
            double _re3r = 0, _re3i = 0;
            _re3r = x1r; _re3i = 0;
            double _pow4r = 0, _pow4i = 0;
            c_powr(_re3r, _re3i, k, &_pow4r, &_pow4i);
            double _abs5r = 0, _abs5i = 0;
            _abs5r = c_abs(x2r, x2i); _abs5i = 0;
            double _add6r = 0, _add6i = 0;
            _add6r = _abs5r + k; _add6i = _abs5i + 0;
            double _log7r = 0, _log7i = 0;
            c_log(_add6r, _add6i, &_log7r, &_log7i);
            double _mul8r = 0, _mul8i = 0;
            c_mul(_pow4r, _pow4i, _log7r, _log7i, &_mul8r, &_mul8i);
            double _c9r = 0, _c9i = 0;
            _c9r = 1.0; _c9i = 0;
            double _c10r = 0, _c10i = 0;
            _c10r = 2.0; _c10i = 0;
            double _pow11r = 0, _pow11i = 0;
            c_mul(k, 0, k, 0, &_pow11r, &_pow11i);
            double _add12r = 0, _add12i = 0;
            _add12r = _c9r + _pow11r; _add12i = _c9i + _pow11i;
            double _div13r = 0, _div13i = 0;
            c_div(_mul8r, _mul8i, _add12r, _add12i, &_div13r, &_div13i);
            mag += _div13r;
            double _ang14r = 0, _ang14i = 0;
            _ang14r = c_arg(x1r, x1i); _ang14i = 0;
            double _mul15r = 0, _mul15i = 0;
            c_mul(k, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
            double _sin16r = 0, _sin16i = 0;
            c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
            double _ang17r = 0, _ang17i = 0;
            _ang17r = c_arg(x2r, x2i); _ang17i = 0;
            double _mul18r = 0, _mul18i = 0;
            c_mul(k, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
            double _cos19r = 0, _cos19i = 0;
            c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
            double _mul20r = 0, _mul20i = 0;
            c_mul(_sin16r, _sin16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
            angle += _mul20r;
        }
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    for (int r = 1; r < 36; r++) {
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _re28r = 0, _re28i = 0;
        _re28r = x1r; _re28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_re28r, _re28i, r, 0, &_mul29r, &_mul29i);
        double _im30r = 0, _im30i = 0;
        _im30r = x2i; _im30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = r + _c31r; _add32i = 0 + _c31i;
        double _div33r = 0, _div33i = 0;
        c_div(_im30r, _im30i, _add32r, _add32i, &_div33r, &_div33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul29r - _div33r; _sub34i = _mul29i - _div33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c27r, _c27i, _sub34r, _sub34i, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        { double _tr = cRe[(r - 1)]*_exp36r - cIm[(r - 1)]*_exp36i; cIm[(r - 1)] = cRe[(r - 1)]*_exp36i + cIm[(r - 1)]*_exp36r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_356_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < 35; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double temp = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        for (int k = 1; k < (int)(_add3r); k++) {
            double _re4r = 0, _re4i = 0;
            _re4r = x1r; _re4i = 0;
            double _pow5r = 0, _pow5i = 0;
            c_powr(_re4r, _re4i, k, &_pow5r, &_pow5i);
            double _ang6r = 0, _ang6i = 0;
            _ang6r = c_arg(x2r, x2i); _ang6i = 0;
            double _mul7r = 0, _mul7i = 0;
            c_mul(k, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
            double _sin8r = 0, _sin8i = 0;
            c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
            double _mul9r = 0, _mul9i = 0;
            c_mul(_pow5r, _pow5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
            double _im10r = 0, _im10i = 0;
            _im10r = x2i; _im10i = 0;
            double _pow11r = 0, _pow11i = 0;
            c_powr(_im10r, _im10i, k, &_pow11r, &_pow11i);
            double _ang12r = 0, _ang12i = 0;
            _ang12r = c_arg(x1r, x1i); _ang12i = 0;
            double _mul13r = 0, _mul13i = 0;
            c_mul(k, 0, _ang12r, _ang12i, &_mul13r, &_mul13i);
            double _cos14r = 0, _cos14i = 0;
            c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
            double _mul15r = 0, _mul15i = 0;
            c_mul(_pow11r, _pow11i, _cos14r, _cos14i, &_mul15r, &_mul15i);
            double _add16r = 0, _add16i = 0;
            _add16r = _mul9r + _mul15r; _add16i = _mul9i + _mul15i;
            temp += _add16r;
        }
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
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log21r, _log21i, temp, 0, &_mul22r, &_mul22i);
        double magnitude = _mul22r; /* +_mul22ii */
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
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _mul28r; _add29i = _mul25i + _mul28i;
        double angle = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, angle, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(magnitude, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int r = 0; r < 35; r++) {
        double _re34r = 0, _re34i = 0;
        _re34r = x1r; _re34i = 0;
        double _re35r = 0, _re35i = 0;
        _re35r = x2r; _re35i = 0;
        double _sub36r = 0, _sub36i = 0;
        _sub36r = _re34r - _re35r; _sub36i = _re34i - _re35i;
        double _sin37r = 0, _sin37i = 0;
        c_sin(r, 0, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_sub36r, _sub36i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _im39r = 0, _im39i = 0;
        _im39r = x1i; _im39i = 0;
        double _im40r = 0, _im40i = 0;
        _im40r = x2i; _im40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _im39r + _im40r; _add41i = _im39i + _im40i;
        double _cos42r = 0, _cos42i = 0;
        c_cos(r, 0, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_add41r, _add41i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul38r + _mul43r; _add44i = _mul38i + _mul43i;
        cRe[r] += _add44r; cIm[r] += _add44i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_357_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = _abs1r + j; _add2i = _abs1i + 0;
        double _log3r = 0, _log3i = 0;
        c_log(_add2r, _add2i, &_log3r, &_log3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _div5r = 0, _div5i = 0;
        c_div(j, 0, _c4r, _c4i, &_div5r, &_div5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_div5r, _div5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _sqrt9r = 0, _sqrt9i = 0;
        c_powr(_abs8r, _abs8i, 0.5, &_sqrt9r, &_sqrt9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 3.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(j, 0, _c10r, _c10i, &_div11r, &_div11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_div11r, _div11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sqrt9r, _sqrt9i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul7r + _mul13r; _add14i = _mul7i + _mul13i;
        double magnitude = _add14r; /* +_add14ii */
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
        double angle = _add20r; /* +_add20ii */
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
        c_mul(magnitude, 0, _add25r, _add25i, &_mul26r, &_mul26i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    for (int k = 0; k < (int)(n); k++) {
        double _re27r = 0, _re27i = 0;
        _re27r = x1r; _re27i = 0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(k, 0, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_re27r, _re27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _im30r = 0, _im30i = 0;
        _im30r = x2i; _im30i = 0;
        double _cos31r = 0, _cos31i = 0;
        c_cos(k, 0, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_im30r, _im30i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _mul29r - _mul32r; _sub33i = _mul29i - _mul32i;
        cRe[k] += _sub33r; cIm[k] += _sub33i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_358_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x2r; _re1i = 0;
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _sub3r = 0, _sub3i = 0;
    _sub3r = _re1r - _re2r; _sub3i = _re1i - _re2i;
    double real_diff = _sub3r; /* +_sub3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double _im5r = 0, _im5i = 0;
    _im5r = x1i; _im5i = 0;
    double _sub6r = 0, _sub6i = 0;
    _sub6r = _im4r - _im5r; _sub6i = _im4i - _im5i;
    double im_diff = _sub6r; /* +_sub6ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(n, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _sub10r = 0, _sub10i = 0;
        _sub10r = j - _div9r; _sub10i = 0 - _div9i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_sub10r, _sub10i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_abs11r, _abs11i, _log14r, _log14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c7r + _mul15r; _add16i = _c7i + _mul15i;
        double mag = _add16r; /* +_add16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(j, 0, n, 0, &_div18r, &_div18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_div18r, _div18i, _ang19r, _ang19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _ang17r + _mul20r; _add21i = _ang17i + _mul20i;
        double _sin22r = 0, _sin22i = 0;
        c_sin(j, 0, &_sin22r, &_sin22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(j, 0, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_sin22r, _sin22i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add21r + _mul26r; _add27i = _add21i + _mul26i;
        double angle = _add27r; /* +_add27ii */
        for (int k = 1; k < 4; k++) {
            double _c28r = 0, _c28i = 0;
            _c28r = 1.0; _c28i = 0;
            double _c29r = 0, _c29i = 0;
            _c29r = 0.1; _c29i = 0;
            double _mul30r = 0, _mul30i = 0;
            c_mul(_c29r, _c29i, k, 0, &_mul30r, &_mul30i);
            double _mul31r = 0, _mul31i = 0;
            c_mul(j, 0, k, 0, &_mul31r, &_mul31i);
            double _div32r = 0, _div32i = 0;
            c_div(_mul31r, _mul31i, n, 0, &_div32r, &_div32i);
            double _sin33r = 0, _sin33i = 0;
            c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
            double _mul34r = 0, _mul34i = 0;
            c_mul(_mul30r, _mul30i, _sin33r, _sin33i, &_mul34r, &_mul34i);
            double _add35r = 0, _add35i = 0;
            _add35r = _c28r + _mul34r; _add35i = _c28i + _mul34i;
            mag *= _add35r;
            double _c36r = 0, _c36i = 0;
            _c36r = 0.5; _c36i = 0;
            double _mul37r = 0, _mul37i = 0;
            c_mul(_c36r, _c36i, k, 0, &_mul37r, &_mul37i);
            double _mul38r = 0, _mul38i = 0;
            c_mul(j, 0, k, 0, &_mul38r, &_mul38i);
            double _div39r = 0, _div39i = 0;
            c_div(_mul38r, _mul38i, n, 0, &_div39r, &_div39i);
            double _cos40r = 0, _cos40i = 0;
            c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
            double _mul41r = 0, _mul41i = 0;
            c_mul(_mul37r, _mul37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
            angle += _mul41r;
            for (int r = 1; r < 3; r++) {
                double _c42r = 0, _c42i = 0;
                _c42r = 0.05; _c42i = 0;
                double _mul43r = 0, _mul43i = 0;
                c_mul(_c42r, _c42i, r, 0, &_mul43r, &_mul43i);
                double _add44r = 0, _add44i = 0;
                _add44r = j + r; _add44i = 0 + 0;
                double _log45r = 0, _log45i = 0;
                c_log(_add44r, _add44i, &_log45r, &_log45i);
                double _mul46r = 0, _mul46i = 0;
                c_mul(_mul43r, _mul43i, _log45r, _log45i, &_mul46r, &_mul46i);
                mag += _mul46r;
                double _c47r = 0, _c47i = 0;
                _c47r = 0.3; _c47i = 0;
                double _mul48r = 0, _mul48i = 0;
                c_mul(_c47r, _c47i, r, 0, &_mul48r, &_mul48i);
                double _mul49r = 0, _mul49i = 0;
                c_mul(j, 0, r, 0, &_mul49r, &_mul49i);
                double _div50r = 0, _div50i = 0;
                c_div(_mul49r, _mul49i, n, 0, &_div50r, &_div50i);
                double _sin51r = 0, _sin51i = 0;
                c_sin(_div50r, _div50i, &_sin51r, &_sin51i);
                double _mul52r = 0, _mul52i = 0;
                c_mul(_mul48r, _mul48i, _sin51r, _sin51i, &_mul52r, &_mul52i);
                angle += _mul52r;
            }
        }
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
        c_mul(mag, 0, _add57r, _add57i, &_mul58r, &_mul58i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_359_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _conj1r = 0, _conj1i = 0;
    _conj1r = x1r; _conj1i = -(x1i);
    double _conj2r = 0, _conj2i = 0;
    _conj2r = x2r; _conj2i = -(x2i);
    double _add3r = 0, _add3i = 0;
    _add3r = _conj1r + _conj2r; _add3i = _conj1i + _conj2i;
    double t_conj = _add3r; /* +_add3ii */
    for (int j = 0; j < (int)(n); j++) {
        double _re4r = 0, _re4i = 0;
        _re4r = x1r; _re4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, M_PI, 0, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 7.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_mul5r, _mul5i, _c6r, _c6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_re4r, _re4i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _re10r = 0, _re10i = 0;
        _re10r = x2r; _re10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 5.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_re10r, _re10i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul9r + _mul15r; _add16i = _mul9i + _mul15i;
        double real_part = _add16r; /* +_add16ii */
        double _im17r = 0, _im17i = 0;
        _im17r = x1i; _im17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 6.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_im17r, _im17i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _im23r = 0, _im23i = 0;
        _im23r = x2i; _im23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 4.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_im23r, _im23i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul22r - _mul28r; _sub29i = _mul22i - _mul28i;
        double imag_part = _sub29r; /* +_sub29ii */
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(t_conj, 0); _abs30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs30r + j; _add31i = _abs30i + 0;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(real_part, 0); _abs33i = 0;
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(imag_part, 0); _abs34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _abs33r + _abs34r; _add35i = _abs33i + _abs34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_log32r, _log32i, _add35r, _add35i, &_mul36r, &_mul36i);
        double magnitude = _mul36r; /* +_mul36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x1r, x1i); _ang37i = 0;
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(t_conj, 0); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(j, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang38r, _ang38i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _ang37r + _mul42r; _add43i = _ang37i + _mul42i;
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 4.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(j, 0, _c45r, _c45i, &_div46r, &_div46i);
        double _cos47r = 0, _cos47i = 0;
        c_cos(_div46r, _div46i, &_cos47r, &_cos47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang44r, _ang44i, _cos47r, _cos47i, &_mul48r, &_mul48i);
        double _sub49r = 0, _sub49i = 0;
        _sub49r = _add43r - _mul48r; _sub49i = _add43i - _mul48i;
        double angle = _sub49r; /* +_sub49ii */
        double _cos50r = 0, _cos50i = 0;
        c_cos(angle, 0, &_cos50r, &_cos50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(angle, 0, &_sin51r, &_sin51i);
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_sin51r, _sin51i, _c52r, _c52i, &_mul53r, &_mul53i);
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

static void poly_360_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr6i = 0; }
        double _mul7r = 0, _mul7i = 0;
        c_mul(_arr5r, _arr5i, _arr6r, _arr6i, &_mul7r, &_mul7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_mul7r, _mul7i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _pow13r = 0, _pow13i = 0;
        c_powr(j, 0, _sin12r, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log11r, _log11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _cos16r = 0, _cos16i = 0;
        c_cos(j, 0, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _c15r + _cos16r; _add17i = _c15i + _cos16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul14r, _mul14i, _add17r, _add17i, &_mul18r, &_mul18i);
        double magnitude = _mul18r; /* +_mul18ii */
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c19r, _c19i, M_PI, 0, &_mul20r, &_mul20i);
        double _arr21r = 0, _arr21i = 0;
        { int _idx = j; _arr21r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr21i = 0; }
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul20r, _mul20i, _arr21r, _arr21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, M_PI, 0, &_mul25r, &_mul25i);
        double _arr26r = 0, _arr26i = 0;
        { int _idx = j; _arr26r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr26i = 0; }
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul25r, _mul25i, _arr26r, _arr26i, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _sin23r + _cos28r; _add29i = _sin23i + _cos28i;
        double _arr30r = 0, _arr30i = 0;
        { int _idx = j; _arr30r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr30i = 0; }
        double _arr31r = 0, _arr31i = 0;
        { int _idx = j; _arr31r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr31i = 0; }
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_361_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double re1 = _re1r; /* +_re1ii */
    double _im2r = 0, _im2i = 0;
    _im2r = x1i; _im2i = 0;
    double im1 = _im2r; /* +_im2ii */
    double _re3r = 0, _re3i = 0;
    _re3r = x2r; _re3i = 0;
    double re2 = _re3r; /* +_re3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double im2 = _im4r; /* +_im4ii */
    for (int j = 0; j < (int)(n); j++) {
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(j, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, re1, 0, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _mul9r; _add10i = _abs8i + _mul9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, im2, 0, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c12r + _sin14r; _add15i = _c12i + _sin14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log11r, _log11i, _add15r, _add15i, &_mul16r, &_mul16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, im1, 0, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _mul18r; _add19i = _abs17i + _mul18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, re2, 0, &_mul22r, &_mul22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _c21r + _cos23r; _add24i = _c21i + _cos23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log20r, _log20i, _add24r, _add24i, &_mul25r, &_mul25i);
        double _tern26r = 0, _tern26i = 0;
        if (_mod6r == _c7r) { _tern26r = _mul16r; _tern26i = _mul16i; }
        else { _tern26r = _mul25r; _tern26i = _mul25i; }
        double mag = _tern26r; /* +_tern26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(n, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul29r, _mul29i, re1, 0, &_mul30r, &_mul30i);
        double _div31r = 0, _div31i = 0;
        c_div(_mul30r, _mul30i, n, 0, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_mul33r, _mul33i, im2, 0, &_mul34r, &_mul34i);
        double _div35r = 0, _div35i = 0;
        c_div(_mul34r, _mul34i, n, 0, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sin32r + _cos36r; _add37i = _sin32i + _cos36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_mul38r, _mul38i, re2, 0, &_mul39r, &_mul39i);
        double _div40r = 0, _div40i = 0;
        c_div(_mul39r, _mul39i, n, 0, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, M_PI, 0, &_mul42r, &_mul42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_mul42r, _mul42i, im1, 0, &_mul43r, &_mul43i);
        double _div44r = 0, _div44i = 0;
        c_div(_mul43r, _mul43i, n, 0, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _sub46r = 0, _sub46i = 0;
        _sub46r = _sin41r - _cos45r; _sub46i = _sin41i - _cos45i;
        double _tern47r = 0, _tern47i = 0;
        if (j <= _div28r) { _tern47r = _add37r; _tern47i = _add37i; }
        else { _tern47r = _sub46r; _tern47i = _sub46i; }
        double angle = _tern47r; /* +_tern47ii */
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
        c_mul(mag, 0, _add52r, _add52i, &_mul53r, &_mul53i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_362_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    for (int j = 0; j < (int)(n); j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 4.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _c1r, _c1i, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mul2r + _c3r; _add4i = _mul2i + _c3i;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(_add4r, n); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double k = _add7r; /* +_add7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 6.0; _c8i = 0;
        double _fdiv9r = 0, _fdiv9i = 0;
        c_div(j, 0, _c8r, _c8i, &_fdiv9r, &_fdiv9i);
        _fdiv9r = floor(_fdiv9r); _fdiv9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _fdiv9r + _c10r; _add11i = _fdiv9i + _c10i;
        double r = _add11r; /* +_add11ii */
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
        _c17r = 2.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(j, 0, _c17r, _c17i, &_div18r, &_div18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(k, 0, _c19r, _c19i, &_div20r, &_div20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _div18r + _div20r; _add21i = _div18i + _div20i;
        double _sin22r = 0, _sin22i = 0;
        c_sin(_add21r, _add21i, &_sin22r, &_sin22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_sin22r, _sin22i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log16r, _log16i, _abs23r, _abs23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = j + r; _add25i = 0 + 0;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul24r + _log26r; _add27i = _mul24i + _log26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = k + _c29r; _add30i = 0 + _c29i;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _add30r, _add30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang28r, _ang28i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = r + _c35r; _add36i = 0 + _c35i;
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, _add36r, _add36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang34r, _ang34i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul33r + _mul39r; _add40i = _mul33i + _mul39i;
        double _re41r = 0, _re41i = 0;
        _re41r = x1r; _re41i = 0;
        double _im42r = 0, _im42i = 0;
        _im42r = x2i; _im42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_re41r, _re41i, _im42r, _im42i, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = j + _c44r; _add45i = 0 + _c44i;
        double _div46r = 0, _div46i = 0;
        c_div(_mul43r, _mul43i, _add45r, _add45i, &_div46r, &_div46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _add40r + _div46r; _add47i = _add40i + _div46i;
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
        c_mul(mag, 0, _add52r, _add52i, &_mul53r, &_mul53i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_363_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x1r, x1i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x2r, x2i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _sin9r + _cos12r; _add13i = _sin9i + _cos12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(j, 0, j, 0, &_pow15r, &_pow15i);
        double _div16r = 0, _div16i = 0;
        c_div(_pow15r, _pow15i, n, 0, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _add13r + _sin17r; _add18i = _add13i + _sin17i;
        double angle = _add18r; /* +_add18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log22r, _log22i, j, 0, &_mul23r, &_mul23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 0.5; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_abs24r, _abs24i, 0.5, &_pow26r, &_pow26i);
        double _sqrt27r = 0, _sqrt27i = 0;
        c_powr(j, 0, 0.5, &_sqrt27r, &_sqrt27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_pow26r, _pow26i, _sqrt27r, _sqrt27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul23r + _mul28r; _add29i = _mul23i + _mul28i;
        double magnitude = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, angle, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(magnitude, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        double _conj34r = 0, _conj34i = 0;
        _conj34r = x1r; _conj34i = -(x1i);
        double _conj35r = 0, _conj35i = 0;
        _conj35r = x2r; _conj35i = -(x2i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_conj34r, _conj34i, _conj35r, _conj35i, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = j + _c37r; _add38i = 0 + _c37i;
        double _div39r = 0, _div39i = 0;
        c_div(_mul36r, _mul36i, _add38r, _add38i, &_div39r, &_div39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul33r + _div39r; _add40i = _mul33i + _div39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    double _c41r = 0, _c41i = 0;
    _c41r = 2.0; _c41i = 0;
    double _fdiv42r = 0, _fdiv42i = 0;
    c_div(n, 0, _c41r, _c41i, &_fdiv42r, &_fdiv42i);
    _fdiv42r = floor(_fdiv42r); _fdiv42i = 0;
    double _c43r = 0, _c43i = 0;
    _c43r = 1.0; _c43i = 0;
    double _add44r = 0, _add44i = 0;
    _add44r = _fdiv42r + _c43r; _add44i = _fdiv42i + _c43i;
    for (int k = 1; k < (int)(_add44r); k++) {
        double _re45r = 0, _re45i = 0;
        _re45r = x1r; _re45i = 0;
        double _re46r = 0, _re46i = 0;
        _re46r = x2r; _re46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_re45r, _re45i, _re46r, _re46i, &_mul47r, &_mul47i);
        double _im48r = 0, _im48i = 0;
        _im48r = x1i; _im48i = 0;
        double _im49r = 0, _im49i = 0;
        _im49r = x2i; _im49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_im48r, _im48i, _im49r, _im49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul47r + _mul50r; _add51i = _mul47i + _mul50i;
        double _sin52r = 0, _sin52i = 0;
        c_sin(k, 0, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_add51r, _add51i, _sin52r, _sin52i, &_mul53r, &_mul53i);
        cRe[(k - 1)] += _mul53r; cIm[(k - 1)] += _mul53i;
    }
    double _c54r = 0, _c54i = 0;
    _c54r = 4.0; _c54i = 0;
    double _sub55r = 0, _sub55i = 0;
    _sub55r = n - _c54r; _sub55i = 0 - _c54i;
    double _c56r = 0, _c56i = 0;
    _c56r = 1.0; _c56i = 0;
    double _add57r = 0, _add57i = 0;
    _add57r = n + _c56r; _add57i = 0 + _c56i;
    for (int r = (int)(_sub55r); r < (int)(_add57r); r++) {
        double _abs58r = 0, _abs58i = 0;
        _abs58r = c_abs(x1r, x1i); _abs58i = 0;
        double _abs59r = 0, _abs59i = 0;
        _abs59r = c_abs(x2r, x2i); _abs59i = 0;
        double _prod60r = 0, _prod60i = 0;
        c_mul(_abs58r, _abs58i, _abs59r, _abs59i, &_prod60r, &_prod60i);
        double _prod61r = 0, _prod61i = 0;
        c_mul(_prod60r, _prod60i, r, 0, &_prod61r, &_prod61i);
        double _cos62r = 0, _cos62i = 0;
        c_cos(r, 0, &_cos62r, &_cos62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(_prod61r, _prod61i, _cos62r, _cos62i, &_mul63r, &_mul63i);
        cRe[(r - 1)] += _mul63r; cIm[(r - 1)] += _mul63i;
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_364_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _sin14r = 0, _sin14i = 0;
        c_sin(j, 0, &_sin14r, &_sin14i);
        double _pow15r = 0, _pow15i = 0;
        c_powr(j, 0, _sin14r, &_pow15r, &_pow15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log13r, _log13i, _pow15r, _pow15i, &_mul16r, &_mul16i);
        double _sqrt17r = 0, _sqrt17i = 0;
        c_powr(j, 0, 0.5, &_sqrt17r, &_sqrt17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 3.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_cos21r, _cos21i); _abs22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sqrt17r, _sqrt17i, _abs22r, _abs22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul16r + _mul23r; _add24i = _mul16i + _mul23i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_365: too complex for auto-transpile, stubbed */
static void poly_365_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_366_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 4.0; _c3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _c3r, _c3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _mod7r = 0, _mod7i = 0;
        _mod7r = fmod(_add6r, n); _mod7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _mod7r + _c8r; _add9i = _mod7i + _c8i;
        double k = _add9r; /* +_add9ii */
        double _c10r = 0, _c10i = 0;
        _c10r = 3.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _mod12r = 0, _mod12i = 0;
        _mod12r = fmod(_add11r, n); _mod12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _mod12r + _c13r; _add14i = _mod12i + _c13i;
        double r = _add14r; /* +_add14ii */
        double _re15r = 0, _re15i = 0;
        _re15r = x1r; _re15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(j, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_re15r, _re15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _re18r = 0, _re18i = 0;
        _re18r = x2r; _re18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(k, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_re18r, _re18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
        double rec = _add21r; /* +_add21ii */
        double _im22r = 0, _im22i = 0;
        _im22r = x1i; _im22i = 0;
        double _cos23r = 0, _cos23i = 0;
        c_cos(r, 0, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_im22r, _im22i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _im25r = 0, _im25i = 0;
        _im25r = x2i; _im25i = 0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(k, 0, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_im25r, _im25i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul24r - _mul27r; _sub28i = _mul24i - _mul27i;
        double imc = _sub28r; /* +_sub28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs29r + _c30r; _add31i = _abs29i + _c30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(rec, 0); _abs33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_log32r, _log32i, _abs33r, _abs33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(j, 0, &_sin35r, &_sin35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(r, 0, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sin35r, _sin35i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul34r + _mul37r; _add38i = _mul34i + _mul37i;
        double _re39r = 0, _re39i = 0;
        _re39r = x1r; _re39i = 0;
        double _im40r = 0, _im40i = 0;
        _im40r = x2i; _im40i = 0;
        double _prod41r = 0, _prod41i = 0;
        c_mul(_re39r, _re39i, _im40r, _im40i, &_prod41r, &_prod41i);
        double _prod42r = 0, _prod42i = 0;
        c_mul(_prod41r, _prod41i, j, 0, &_prod42r, &_prod42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add38r + _prod42r; _add43i = _add38i + _prod42i;
        double mag = _add43r; /* +_add43ii */
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x1r, x1i); _ang44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_ang44r, _ang44i, k, 0, &_mul45r, &_mul45i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x2r, x2i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_ang46r, _ang46i, r, 0, &_mul47r, &_mul47i);
        double _sub48r = 0, _sub48i = 0;
        _sub48r = _mul45r - _mul47r; _sub48i = _mul45i - _mul47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(j, 0, M_PI, 0, &_mul49r, &_mul49i);
        double _div50r = 0, _div50i = 0;
        c_div(_mul49r, _mul49i, n, 0, &_div50r, &_div50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(_div50r, _div50i, &_sin51r, &_sin51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _sub48r + _sin51r; _add52i = _sub48i + _sin51i;
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
        c_mul(mag, 0, _add57r, _add57i, &_mul58r, &_mul58i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_368_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _c6r + _sin9r; _add10i = _c6i + _sin9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log5r, _log5i, _add10r, _add10i, &_mul11r, &_mul11i);
        double mag = _mul11r; /* +_mul11ii */
        double _re12r = 0, _re12i = 0;
        _re12r = x1r; _re12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _re12r, _re12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _im15r = 0, _im15i = 0;
        _im15r = x2i; _im15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _im15r, _im15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_mul(_sin17r, _sin17i, _sin17r, _sin17i, &_pow19r, &_pow19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _cos14r + _pow19r; _add20i = _cos14i + _pow19i;
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
        c_mul(mag, 0, _add25r, _add25i, &_mul26r, &_mul26i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    double _c27r = 0, _c27i = 0;
    _c27r = 1.0; _c27i = 0;
    double _add28r = 0, _add28i = 0;
    _add28r = n + _c27r; _add28i = 0 + _c27i;
    for (int k = 1; k < (int)(_add28r); k++) {
        double _re29r = 0, _re29i = 0;
        _re29r = x1r; _re29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(_re29r, _re29i, k, &_pow30r, &_pow30i);
        double _im31r = 0, _im31i = 0;
        _im31r = x2i; _im31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_im31r, _im31i, k, &_pow32r, &_pow32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _pow30r - _pow32r; _sub33i = _pow30i - _pow32i;
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(k, 0, x2r, x2i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = x1r + _mul35r; _add36i = x1i + _mul35i;
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(_add36r, _add36i); _ang37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c34r, _c34i, _ang37r, _ang37i, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_sub33r, _sub33i, _exp39r, _exp39i, &_mul40r, &_mul40i);
        cRe[(k - 1)] += _mul40r; cIm[(k - 1)] += _mul40i;
    }
    double _c41r = 0, _c41i = 0;
    _c41r = 1.0; _c41i = 0;
    double _add42r = 0, _add42i = 0;
    _add42r = n + _c41r; _add42i = 0 + _c41i;
    for (int r = 1; r < (int)(_add42r); r++) {
        double _mul43r = 0, _mul43i = 0;
        c_mul(r, 0, x2r, x2i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = x1r + _mul43r; _add44i = x1i + _mul43i;
        double _abs45r = 0, _abs45i = 0;
        _abs45r = c_abs(_add44r, _add44i); _abs45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 1.0; _c46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 10.0; _c47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(r, 0, _c47r, _c47i, &_div48r, &_div48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _c46r + _div48r; _add49i = _c46i + _div48i;
        double _pow50r = 0, _pow50i = 0;
        c_powr(_abs45r, _abs45i, _add49r, &_pow50r, &_pow50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul51r, &_mul51i);
        double _ang52r = 0, _ang52i = 0;
        _ang52r = c_arg(_mul51r, _mul51i); _ang52i = 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(r, 0, _ang52r, _ang52i, &_mul53r, &_mul53i);
        double _cos54r = 0, _cos54i = 0;
        c_cos(_mul53r, _mul53i, &_cos54r, &_cos54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_pow50r, _pow50i, _cos54r, _cos54i, &_mul55r, &_mul55i);
        { double _tr = cRe[(r - 1)]*_mul55r - cIm[(r - 1)]*_mul55i; cIm[(r - 1)] = cRe[(r - 1)]*_mul55i + cIm[(r - 1)]*_mul55r; cRe[(r - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_369_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_371_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(x2r, x2i); _ang8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _ang8r, _ang8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(_sin10r, _sin10i, _sin10r, _sin10i, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _pow12r; _add13i = _mul7i + _pow12i;
        double mag_part = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _cos15r = 0, _cos15i = 0;
        c_cos(j, 0, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang14r, _ang14i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _sin18r = 0, _sin18i = 0;
        c_sin(j, 0, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang17r, _ang17i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul16r + _mul19r; _add20i = _mul16i + _mul19i;
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
        double _conj27r = 0, _conj27i = 0;
        _conj27r = x1r; _conj27i = -(x1i);
        double _pow28r = 0, _pow28i = 0;
        c_powr(_conj27r, _conj27i, j, &_pow28r, &_pow28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul26r + _pow28r; _add29i = _mul26i + _pow28i;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _abs30r + _c31r; _add32i = _abs30i + _c31i;
        double _log33r = 0, _log33i = 0;
        c_log(_add32r, _add32i, &_log33r, &_log33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(j, 0, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_log33r, _log33i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _sub36r = 0, _sub36i = 0;
        _sub36r = _add29r - _mul35r; _sub36i = _add29i - _mul35i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_372_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
    for (int k = 1; k < (int)(_add6r); k++) {
        double _mul7r = 0, _mul7i = 0;
        c_mul(x1r, x1i, k, 0, &_mul7r, &_mul7i);
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_mul7r, _mul7i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_ang12r, _ang12i, k, 0, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log11r, _log11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double mag1 = _mul15r; /* +_mul15ii */
        double _div16r = 0, _div16i = 0;
        c_div(x2r, x2i, k, 0, &_div16r, &_div16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_div16r, _div16i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = k + _c22r; _add23i = 0 + _c22i;
        double _div24r = 0, _div24i = 0;
        c_div(_ang21r, _ang21i, _add23r, _add23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log20r, _log20i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double mag2 = _mul26r; /* +_mul26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = mag1 + mag2; _add27i = 0 + 0;
        double mag = _add27r; /* +_add27ii */
        double _arr28r = 0, _arr28i = 0;
        { int _idx = (k - 1); _arr28r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr28i = 0; }
        double _mul29r = 0, _mul29i = 0;
        c_mul(_arr28r, _arr28i, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = k + _c30r; _add31i = 0 + _c30i;
        double _div32r = 0, _div32i = 0;
        c_div(_mul29r, _mul29i, _add31r, _add31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _arr34r = 0, _arr34i = 0;
        { int _idx = (k - 1); _arr34r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr34i = 0; }
        double _mul35r = 0, _mul35i = 0;
        c_mul(_arr34r, _arr34i, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = k + _c36r; _add37i = 0 + _c36i;
        double _div38r = 0, _div38i = 0;
        c_div(_mul35r, _mul35i, _add37r, _add37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _sin33r + _cos39r; _add40i = _sin33i + _cos39i;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_373_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_re4r, _re4i, j, 0, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _re3r + _mul5r; _add6i = _re3i + _mul5i;
        double r = _add6r; /* +_add6ii */
        double _im7r = 0, _im7i = 0;
        _im7r = x1i; _im7i = 0;
        double _im8r = 0, _im8i = 0;
        _im8r = x2i; _im8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_im8r, _im8i, j, 0, &_mul9r, &_mul9i);
        double _sub10r = 0, _sub10i = 0;
        _sub10r = _im7r - _mul9r; _sub10i = _im7i - _mul9i;
        double im = _sub10r; /* +_sub10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(j, 0, j, 0, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _abs11r + _pow13r; _add14i = _abs11i + _pow13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 7.0; _c22i = 0;
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
        double _cos29r = 0, _cos29i = 0;
        c_cos(j, 0, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(j, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
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
        c_mul(mag, 0, _add39r, _add39i, &_mul40r, &_mul40i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_375_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_ang3r, _ang3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(j, 0, &_sin8r, &_sin8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_sin8r, _sin8i, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _c11r; _add12i = 0 + _c11i;
        double _div13r = 0, _div13i = 0;
        c_div(_mul10r, _mul10i, _add12r, _add12i, &_div13r, &_div13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul7r + _div13r; _add14i = _mul7i + _div13i;
        double angle = _add14r; /* +_add14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _sqrt16r = 0, _sqrt16i = 0;
        c_powr(j, 0, 0.5, &_sqrt16r, &_sqrt16i);
        double _pow17r = 0, _pow17i = 0;
        c_powr(_abs15r, _abs15i, _sqrt16r, &_pow17r, &_pow17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_c20r, _c20i, j, 0, &_div21r, &_div21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _c19r + _div21r; _add22i = _c19i + _div21i;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs18r, _abs18i, _add22r, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _pow17r + _pow23r; _add24i = _pow17i + _pow23i;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(n, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = j - _div26r; _sub27i = 0 - _div26i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_sub27r, _sub27i); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs28r + _c29r; _add30i = _abs28i + _c29i;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _add24r + _log31r; _add32i = _add24i + _log31i;
        double magnitude = _add32r; /* +_add32ii */
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
        double _re40r = 0, _re40i = 0;
        _re40r = x2r; _re40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj39r, _conj39i, _re40r, _re40i, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = j + _c42r; _add43i = 0 + _c42i;
        double _div44r = 0, _div44i = 0;
        c_div(_mul41r, _mul41i, _add43r, _add43i, &_div44r, &_div44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul38r + _div44r; _add45i = _mul38i + _div44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_378_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 1.0;
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_c8r, _c8i, M_PI, 0, &_mul9r, &_mul9i);
        double _arr10r = 0, _arr10i = 0;
        { int _idx = (j - 1); _arr10r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr10i = 0; }
        double _mul11r = 0, _mul11i = 0;
        c_mul(_mul9r, _mul9i, _arr10r, _arr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_c7r, _c7i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _exp14r = 0, _exp14i = 0;
        c_exp2(_mul13r, _mul13i, &_exp14r, &_exp14i);
        double term1 = _exp14r; /* +_exp14ii */
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 1.0;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, M_PI, 0, &_mul17r, &_mul17i);
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr18i = 0; }
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul17r, _mul17i, _arr18r, _arr18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c15r, _c15i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _exp22r = 0, _exp22i = 0;
        c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
        double term2 = _exp22r; /* +_exp22ii */
        double _arr23r = 0, _arr23i = 0;
        { int _idx = (j - 1); _arr23r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr23i = 0; }
        double _arr24r = 0, _arr24i = 0;
        { int _idx = (j - 1); _arr24r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr24i = 0; }
        double _mul25r = 0, _mul25i = 0;
        c_mul(_arr23r, _arr23i, _arr24r, _arr24i, &_mul25r, &_mul25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_mul25r, _mul25i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double term3 = _log29r; /* +_log29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(j, 0, j, 0, &_pow31r, &_pow31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(term3, 0, _pow31r, _pow31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul32r + 0; _add33i = _mul32i + 0;
        double mag = _add33r; /* +_add33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(term1, 0); _ang34i = 0;
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(term2, 0); _ang35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _ang34r + _ang35r; _add36i = _ang34i + _ang35i;
        double _add37r = 0, _add37i = 0;
        _add37r = x1r + x2r; _add37i = x1i + x2i;
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(_add37r, _add37i); _ang38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, _ang38r, _ang38i, &_mul39r, &_mul39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_mul39r, _mul39i, &_sin40r, &_sin40i);
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
        c_mul(mag, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_379_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 6.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _c6r + _sin10r; _add11i = _c6i + _sin10i;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log5r, _log5i, _add11r, _add11i, &_mul12r, &_mul12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 4.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_cos16r, _cos16i, _abs17r, _abs17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul12r + _mul18r; _add19i = _mul12i + _mul18i;
        double magnitude = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 5.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_div27r, _div27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang25r, _ang25i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul24r + _mul29r; _add30i = _mul24i + _mul29i;
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
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x1r; _conj37i = -(x1i);
        double _c38r = 0, _c38i = 0;
        _c38r = 7.0; _c38i = 0;
        double _mod39r = 0, _mod39i = 0;
        _mod39r = fmod(j, _c38r); _mod39i = 0;
        double _pow40r = 0, _pow40i = 0;
        c_powr(_conj37r, _conj37i, _mod39r, &_pow40r, &_pow40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, _ang41r, _ang41i, &_mul42r, &_mul42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_mul42r, _mul42i, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_pow40r, _pow40i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul36r + _mul44r; _add45i = _mul36i + _mul44i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_380_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
        double r = _arr7r; /* +_arr7ii */
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr8i = 0; }
        double m = _arr8r; /* +_arr8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_mul(r, 0, r, 0, &_pow10r, &_pow10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(m, 0, m, 0, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _pow10r + _pow12r; _add13i = _pow10i + _pow12i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_add13r, _add13i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _sin20r = 0, _sin20i = 0;
        c_sin(r, 0, &_sin20r, &_sin20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(m, 0, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _sin20r + _cos21r; _add22i = _sin20i + _cos21i;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_add19r, _add19i, _add22r, &_pow23r, &_pow23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log17r, _log17i, _pow23r, _pow23i, &_mul24r, &_mul24i);
        double mag = _mul24r; /* +_mul24ii */
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, r, 0, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, m, 0, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _sin26r + _cos28r; _add29i = _sin26i + _cos28i;
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(m, 0, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang30r, _ang30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _add29r + _mul32r; _add33i = _add29i + _mul32i;
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _cos35r = 0, _cos35i = 0;
        c_cos(r, 0, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang34r, _ang34i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _add33r - _mul36r; _sub37i = _add33i - _mul36i;
        double angle = _sub37r; /* +_sub37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, angle, 0, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(mag, 0, _exp40r, _exp40i, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_381_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double rec_t1 = _re1r; /* +_re1ii */
    double _re2r = 0, _re2i = 0;
    _re2r = x2r; _re2i = 0;
    double rec_t2 = _re2r; /* +_re2ii */
    double _im3r = 0, _im3i = 0;
    _im3r = x1i; _im3i = 0;
    double imc_t1 = _im3r; /* +_im3ii */
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double imc_t2 = _im4r; /* +_im4ii */
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
        double phase = _mul15r; /* +_mul15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log19r, _log19i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(j, 0, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log25r, _log25i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul21r + _mul27r; _add28i = _mul21i + _mul27i;
        double magnitude = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 0.5; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(j, 0, 0.5, &_pow31r, &_pow31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang29r, _ang29i, _pow31r, _pow31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_ang33r, _ang33i, _add35r, _add35i, &_div36r, &_div36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul32r + _div36r; _add37i = _mul32i + _div36i;
        double angle = _add37r; /* +_add37ii */
        double _add38r = 0, _add38i = 0;
        _add38r = angle + phase; _add38i = 0 + 0;
        double _cos39r = 0, _cos39i = 0;
        c_cos(_add38r, _add38i, &_cos39r, &_cos39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _sub41r = 0, _sub41i = 0;
        _sub41r = angle - phase; _sub41i = 0 - 0;
        double _sin42r = 0, _sin42i = 0;
        c_sin(_sub41r, _sub41i, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c40r, _c40i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _cos39r + _mul43r; _add44i = _cos39i + _mul43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(magnitude, 0, _add44r, _add44i, &_mul45r, &_mul45i);
        double _conj46r = 0, _conj46i = 0;
        _conj46r = x1r; _conj46i = -(x1i);
        double _pow47r = 0, _pow47i = 0;
        c_powr(_conj46r, _conj46i, j, &_pow47r, &_pow47i);
        double _re48r = 0, _re48i = 0;
        _re48r = x2r; _re48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_pow47r, _pow47i, _re48r, _re48i, &_mul49r, &_mul49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 2.0; _c50i = 0;
        double _add51r = 0, _add51i = 0;
        _add51r = j + _c50r; _add51i = 0 + _c50i;
        double _div52r = 0, _div52i = 0;
        c_div(_mul49r, _mul49i, _add51r, _add51i, &_div52r, &_div52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul45r + _div52r; _add53i = _mul45i + _div52i;
        double _add54r = 0, _add54i = 0;
        _add54r = x1r + x2r; _add54i = x1i + x2i;
        double _im55r = 0, _im55i = 0;
        _im55r = _add54i; _im55i = 0;
        double _ang56r = 0, _ang56i = 0;
        _ang56r = c_arg(x1r, x1i); _ang56i = 0;
        double _mul57r = 0, _mul57i = 0;
        c_mul(j, 0, _ang56r, _ang56i, &_mul57r, &_mul57i);
        double _cos58r = 0, _cos58i = 0;
        c_cos(_mul57r, _mul57i, &_cos58r, &_cos58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(_im55r, _im55i, _cos58r, _cos58i, &_mul59r, &_mul59i);
        double _ang60r = 0, _ang60i = 0;
        _ang60r = c_arg(x2r, x2i); _ang60i = 0;
        double _mul61r = 0, _mul61i = 0;
        c_mul(j, 0, _ang60r, _ang60i, &_mul61r, &_mul61i);
        double _sin62r = 0, _sin62i = 0;
        c_sin(_mul61r, _mul61i, &_sin62r, &_sin62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(_mul59r, _mul59i, _sin62r, _sin62i, &_mul63r, &_mul63i);
        double _add64r = 0, _add64i = 0;
        _add64r = _add53r + _mul63r; _add64i = _add53i + _mul63i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_382_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double coeff_real = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double coeff_imag = _c4r; /* +_c4ii */
        for (int k = 1; k < 6; k++) {
            double _re5r = 0, _re5i = 0;
            _re5r = x1r; _re5i = 0;
            double _pow6r = 0, _pow6i = 0;
            c_powr(_re5r, _re5i, k, &_pow6r, &_pow6i);
            double _mul7r = 0, _mul7i = 0;
            c_mul(j, 0, k, 0, &_mul7r, &_mul7i);
            double _mul8r = 0, _mul8i = 0;
            c_mul(_mul7r, _mul7i, M_PI, 0, &_mul8r, &_mul8i);
            double _c9r = 0, _c9i = 0;
            _c9r = 4.0; _c9i = 0;
            double _div10r = 0, _div10i = 0;
            c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_pow6r, _pow6i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            double _re13r = 0, _re13i = 0;
            _re13r = x2r; _re13i = 0;
            double _c14r = 0, _c14i = 0;
            _c14r = 2.0; _c14i = 0;
            double _div15r = 0, _div15i = 0;
            c_div(k, 0, _c14r, _c14i, &_div15r, &_div15i);
            double _pow16r = 0, _pow16i = 0;
            c_powr(_re13r, _re13i, _div15r, &_pow16r, &_pow16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(j, 0, k, 0, &_mul17r, &_mul17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(_mul17r, _mul17i, M_PI, 0, &_mul18r, &_mul18i);
            double _c19r = 0, _c19i = 0;
            _c19r = 3.0; _c19i = 0;
            double _div20r = 0, _div20i = 0;
            c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
            double _cos21r = 0, _cos21i = 0;
            c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
            double _mul22r = 0, _mul22i = 0;
            c_mul(_pow16r, _pow16i, _cos21r, _cos21i, &_mul22r, &_mul22i);
            double _add23r = 0, _add23i = 0;
            _add23r = _mul12r + _mul22r; _add23i = _mul12i + _mul22i;
            coeff_real += _add23r;
            double _im24r = 0, _im24i = 0;
            _im24r = x1i; _im24i = 0;
            double _pow25r = 0, _pow25i = 0;
            c_powr(_im24r, _im24i, k, &_pow25r, &_pow25i);
            double _mul26r = 0, _mul26i = 0;
            c_mul(j, 0, k, 0, &_mul26r, &_mul26i);
            double _mul27r = 0, _mul27i = 0;
            c_mul(_mul26r, _mul26i, M_PI, 0, &_mul27r, &_mul27i);
            double _c28r = 0, _c28i = 0;
            _c28r = 5.0; _c28i = 0;
            double _div29r = 0, _div29i = 0;
            c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
            double _cos30r = 0, _cos30i = 0;
            c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
            double _mul31r = 0, _mul31i = 0;
            c_mul(_pow25r, _pow25i, _cos30r, _cos30i, &_mul31r, &_mul31i);
            double _im32r = 0, _im32i = 0;
            _im32r = x2i; _im32i = 0;
            double _c33r = 0, _c33i = 0;
            _c33r = 2.0; _c33i = 0;
            double _div34r = 0, _div34i = 0;
            c_div(k, 0, _c33r, _c33i, &_div34r, &_div34i);
            double _pow35r = 0, _pow35i = 0;
            c_powr(_im32r, _im32i, _div34r, &_pow35r, &_pow35i);
            double _mul36r = 0, _mul36i = 0;
            c_mul(j, 0, k, 0, &_mul36r, &_mul36i);
            double _mul37r = 0, _mul37i = 0;
            c_mul(_mul36r, _mul36i, M_PI, 0, &_mul37r, &_mul37i);
            double _c38r = 0, _c38i = 0;
            _c38r = 6.0; _c38i = 0;
            double _div39r = 0, _div39i = 0;
            c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
            double _sin40r = 0, _sin40i = 0;
            c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
            double _mul41r = 0, _mul41i = 0;
            c_mul(_pow35r, _pow35i, _sin40r, _sin40i, &_mul41r, &_mul41i);
            double _sub42r = 0, _sub42i = 0;
            _sub42r = _mul31r - _mul41r; _sub42i = _mul31i - _mul41i;
            coeff_imag += _sub42r;
        }
        for (int r = 1; r < 4; r++) {
            double _add43r = 0, _add43i = 0;
            _add43r = x1r + r; _add43i = x1i + 0;
            double _abs44r = 0, _abs44i = 0;
            _abs44r = c_abs(_add43r, _add43i); _abs44i = 0;
            double _c45r = 0, _c45i = 0;
            _c45r = 1.0; _c45i = 0;
            double _add46r = 0, _add46i = 0;
            _add46r = _abs44r + _c45r; _add46i = _abs44i + _c45i;
            double _log47r = 0, _log47i = 0;
            c_log(_add46r, _add46i, &_log47r, &_log47i);
            double _mul48r = 0, _mul48i = 0;
            c_mul(j, 0, r, 0, &_mul48r, &_mul48i);
            double _mul49r = 0, _mul49i = 0;
            c_mul(_mul48r, _mul48i, M_PI, 0, &_mul49r, &_mul49i);
            double _c50r = 0, _c50i = 0;
            _c50r = 7.0; _c50i = 0;
            double _div51r = 0, _div51i = 0;
            c_div(_mul49r, _mul49i, _c50r, _c50i, &_div51r, &_div51i);
            double _sin52r = 0, _sin52i = 0;
            c_sin(_div51r, _div51i, &_sin52r, &_sin52i);
            double _mul53r = 0, _mul53i = 0;
            c_mul(_log47r, _log47i, _sin52r, _sin52i, &_mul53r, &_mul53i);
            double _add54r = 0, _add54i = 0;
            _add54r = x1r + r; _add54i = x1i + 0;
            double _ang55r = 0, _ang55i = 0;
            _ang55r = c_arg(_add54r, _add54i); _ang55i = 0;
            double _mul56r = 0, _mul56i = 0;
            c_mul(_mul53r, _mul53i, _ang55r, _ang55i, &_mul56r, &_mul56i);
            coeff_real += _mul56r;
            double _sub57r = 0, _sub57i = 0;
            _sub57r = x2r - r; _sub57i = x2i - 0;
            double _abs58r = 0, _abs58i = 0;
            _abs58r = c_abs(_sub57r, _sub57i); _abs58i = 0;
            double _c59r = 0, _c59i = 0;
            _c59r = 1.0; _c59i = 0;
            double _add60r = 0, _add60i = 0;
            _add60r = _abs58r + _c59r; _add60i = _abs58i + _c59i;
            double _log61r = 0, _log61i = 0;
            c_log(_add60r, _add60i, &_log61r, &_log61i);
            double _mul62r = 0, _mul62i = 0;
            c_mul(j, 0, r, 0, &_mul62r, &_mul62i);
            double _mul63r = 0, _mul63i = 0;
            c_mul(_mul62r, _mul62i, M_PI, 0, &_mul63r, &_mul63i);
            double _c64r = 0, _c64i = 0;
            _c64r = 8.0; _c64i = 0;
            double _div65r = 0, _div65i = 0;
            c_div(_mul63r, _mul63i, _c64r, _c64i, &_div65r, &_div65i);
            double _cos66r = 0, _cos66i = 0;
            c_cos(_div65r, _div65i, &_cos66r, &_cos66i);
            double _mul67r = 0, _mul67i = 0;
            c_mul(_log61r, _log61i, _cos66r, _cos66i, &_mul67r, &_mul67i);
            double _sub68r = 0, _sub68i = 0;
            _sub68r = x2r - r; _sub68i = x2i - 0;
            double _ang69r = 0, _ang69i = 0;
            _ang69r = c_arg(_sub68r, _sub68i); _ang69i = 0;
            double _mul70r = 0, _mul70i = 0;
            c_mul(_mul67r, _mul67i, _ang69r, _ang69i, &_mul70r, &_mul70i);
            coeff_imag += _mul70r;
        }
        double _c71r = 0, _c71i = 0;
        _c71r = 0.0; _c71i = 1.0;
        double _mul72r = 0, _mul72i = 0;
        c_mul(_c71r, _c71i, coeff_imag, 0, &_mul72r, &_mul72i);
        double _add73r = 0, _add73i = 0;
        _add73r = coeff_real + _mul72r; _add73i = 0 + _mul72i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add73r; cIm[_idx] = _add73i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_383_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _div12r = 0, _div12i = 0;
        c_div(0, 0, _add11r, _add11i, &_div12r, &_div12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul9r + _div12r; _add13i = _mul9i + _div12i;
        double mag_part = _add13r; /* +_add13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _im15r = 0, _im15i = 0;
        _im15r = x2i; _im15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _im15r, _im15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang14r, _ang14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(j, 0, &_sin19r, &_sin19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_sin19r, _sin19i, _ang20r, _ang20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
        double angle_part = _add22r; /* +_add22ii */
        double _cos23r = 0, _cos23i = 0;
        c_cos(angle_part, 0, &_cos23r, &_cos23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(angle_part, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c24r, _c24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _cos23r + _mul26r; _add27i = _cos23i + _mul26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_part, 0, _add27r, _add27i, &_mul28r, &_mul28i);
        double _conj29r = 0, _conj29i = 0;
        _conj29r = x1r; _conj29i = -(x1i);
        double _pow30r = 0, _pow30i = 0;
        c_powr(_conj29r, _conj29i, j, &_pow30r, &_pow30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul28r + _pow30r; _add31i = _mul28i + _pow30i;
        double _conj32r = 0, _conj32i = 0;
        _conj32r = x2r; _conj32i = -(x2i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = n - j; _sub33i = 0 - 0;
        double _pow34r = 0, _pow34i = 0;
        c_powr(_conj32r, _conj32i, _sub33r, &_pow34r, &_pow34i);
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _add31r - _pow34r; _sub35i = _add31i - _pow34i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub35r; cIm[_idx] = _sub35i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_384_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_re3r, _re3i, k, &_pow4r, &_pow4i);
        double _re5r = 0, _re5i = 0;
        _re5r = x2r; _re5i = 0;
        double _sub6r = 0, _sub6i = 0;
        _sub6r = n - k; _sub6i = 0 - 0;
        double _pow7r = 0, _pow7i = 0;
        c_powr(_re5r, _re5i, _sub6r, &_pow7r, &_pow7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow4r + _pow7r; _add8i = _pow4i + _pow7i;
        double real_part = _add8r; /* +_add8ii */
        double _im9r = 0, _im9i = 0;
        _im9r = x1i; _im9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 5.0; _c10i = 0;
        double _mod11r = 0, _mod11i = 0;
        _mod11r = fmod(k, _c10r); _mod11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _mod11r + _c12r; _add13i = _mod11i + _c12i;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_im9r, _im9i, _add13r, &_pow14r, &_pow14i);
        double _im15r = 0, _im15i = 0;
        _im15r = x2i; _im15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _fdiv17r = 0, _fdiv17i = 0;
        c_div(k, 0, _c16r, _c16i, &_fdiv17r, &_fdiv17i);
        _fdiv17r = floor(_fdiv17r); _fdiv17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _fdiv17r + _c18r; _add19i = _fdiv17i + _c18i;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_im15r, _im15i, _add19r, &_pow20r, &_pow20i);
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _pow14r - _pow20r; _sub21i = _pow14i - _pow20i;
        double imag_part = _sub21r; /* +_sub21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _abs23r; _add24i = _abs22i + _abs23i;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_add24r, _add24i, k, &_pow25r, &_pow25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = k + _c26r; _add27i = 0 + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_pow25r, _pow25i, _log28r, _log28i, &_mul29r, &_mul29i);
        double magnitude = _mul29r; /* +_mul29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(k, 0, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang30r, _ang30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _cos34r = 0, _cos34i = 0;
        c_cos(k, 0, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang33r, _ang33i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul32r + _mul35r; _add36i = _mul32i + _mul35i;
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_386_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double j = k; /* +0i */
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + k; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(j, 0, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log5r, _log5i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(j, 0, &_cos8r, &_cos8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_cos8r, _cos8i, _log12r, _log12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul7r + _mul13r; _add14i = _mul7i + _mul13i;
        double mag_part = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.5; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(j, 0, 0.5, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang15r, _ang15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang19r, _ang19i, _log22r, _log22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul18r + _mul23r; _add24i = _mul18i + _mul23i;
        double _re25r = 0, _re25i = 0;
        _re25r = x1r; _re25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _re25r, _re25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _add24r + _sin27r; _add28i = _add24i + _sin27i;
        double _im29r = 0, _im29i = 0;
        _im29r = x2i; _im29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _im29r, _im29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _add28r - _cos31r; _sub32i = _add28i - _cos31i;
        double angle_part = _sub32r; /* +_sub32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c33r, _c33i, angle_part, 0, &_mul34r, &_mul34i);
        double _exp35r = 0, _exp35i = 0;
        c_exp2(_mul34r, _mul34i, &_exp35r, &_exp35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag_part, 0, _exp35r, _exp35i, &_mul36r, &_mul36i);
        { int _idx = ((int)(j) - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_388_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _mul4r = 0, _mul4i = 0;
        c_mul(_re3r, _re3i, j, 0, &_mul4r, &_mul4i);
        double _re5r = 0, _re5i = 0;
        _re5r = x2r; _re5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _div8r = 0, _div8i = 0;
        c_div(_re5r, _re5i, _add7r, _add7i, &_div8r, &_div8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul4r + _div8r; _add9i = _mul4i + _div8i;
        double r_part = _add9r; /* +_add9ii */
        double _im10r = 0, _im10i = 0;
        _im10r = x1i; _im10i = 0;
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_im10r, _im10i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _im13r = 0, _im13i = 0;
        _im13r = x2i; _im13i = 0;
        double _cos14r = 0, _cos14i = 0;
        c_cos(j, 0, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_im13r, _im13i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _mul15r; _add16i = _mul12i + _mul15i;
        double i_part = _add16r; /* +_add16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs17r + j; _add18i = _abs17i + 0;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _mod21r = 0, _mod21i = 0;
        _mod21r = fmod(j, _c20r); _mod21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _mod21r + _c22r; _add23i = _mod21i + _c22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log19r, _log19i, _add23r, _add23i, &_mul24r, &_mul24i);
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
        double angle = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, i_part, 0, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = r_part + _mul37r; _add38i = 0 + _mul37i;
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_add38r, _add38i, _exp41r, _exp41i, &_mul42r, &_mul42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_mul42r, _mul42i, mag, 0, &_mul43r, &_mul43i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_389_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double coeff_real = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double coeff_imag = _c4r; /* +_c4ii */
        for (int k = 1; k < 6; k++) {
            double _re5r = 0, _re5i = 0;
            _re5r = x1r; _re5i = 0;
            double _pow6r = 0, _pow6i = 0;
            c_powr(_re5r, _re5i, k, &_pow6r, &_pow6i);
            double _ang7r = 0, _ang7i = 0;
            _ang7r = c_arg(x2r, x2i); _ang7i = 0;
            double _mul8r = 0, _mul8i = 0;
            c_mul(k, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
            double _add9r = 0, _add9i = 0;
            _add9r = _mul8r + j; _add9i = _mul8i + 0;
            double _cos10r = 0, _cos10i = 0;
            c_cos(_add9r, _add9i, &_cos10r, &_cos10i);
            double _mul11r = 0, _mul11i = 0;
            c_mul(_pow6r, _pow6i, _cos10r, _cos10i, &_mul11r, &_mul11i);
            coeff_real += _mul11r;
            double _im12r = 0, _im12i = 0;
            _im12r = x2i; _im12i = 0;
            double _pow13r = 0, _pow13i = 0;
            c_powr(_im12r, _im12i, k, &_pow13r, &_pow13i);
            double _ang14r = 0, _ang14i = 0;
            _ang14r = c_arg(x1r, x1i); _ang14i = 0;
            double _mul15r = 0, _mul15i = 0;
            c_mul(k, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
            double _add16r = 0, _add16i = 0;
            _add16r = _mul15r + j; _add16i = _mul15i + 0;
            double _sin17r = 0, _sin17i = 0;
            c_sin(_add16r, _add16i, &_sin17r, &_sin17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(_pow13r, _pow13i, _sin17r, _sin17i, &_mul18r, &_mul18i);
            coeff_imag += _mul18r;
        }
        for (int r = 1; r < 4; r++) {
            double _add19r = 0, _add19i = 0;
            _add19r = x1r + x2r; _add19i = x1i + x2i;
            double _abs20r = 0, _abs20i = 0;
            _abs20r = c_abs(_add19r, _add19i); _abs20i = 0;
            double _c21r = 0, _c21i = 0;
            _c21r = 1.0; _c21i = 0;
            double _add22r = 0, _add22i = 0;
            _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
            double _log23r = 0, _log23i = 0;
            c_log(_add22r, _add22i, &_log23r, &_log23i);
            double _re24r = 0, _re24i = 0;
            _re24r = x1r; _re24i = 0;
            double _mul25r = 0, _mul25i = 0;
            c_mul(_log23r, _log23i, _re24r, _re24i, &_mul25r, &_mul25i);
            double _div26r = 0, _div26i = 0;
            c_div(_mul25r, _mul25i, r, 0, &_div26r, &_div26i);
            coeff_real += _div26r;
            double _sub27r = 0, _sub27i = 0;
            _sub27r = x1r - x2r; _sub27i = x1i - x2i;
            double _abs28r = 0, _abs28i = 0;
            _abs28r = c_abs(_sub27r, _sub27i); _abs28i = 0;
            double _c29r = 0, _c29i = 0;
            _c29r = 1.0; _c29i = 0;
            double _add30r = 0, _add30i = 0;
            _add30r = _abs28r + _c29r; _add30i = _abs28i + _c29i;
            double _log31r = 0, _log31i = 0;
            c_log(_add30r, _add30i, &_log31r, &_log31i);
            double _im32r = 0, _im32i = 0;
            _im32r = x2i; _im32i = 0;
            double _mul33r = 0, _mul33i = 0;
            c_mul(_log31r, _log31i, _im32r, _im32i, &_mul33r, &_mul33i);
            double _div34r = 0, _div34i = 0;
            c_div(_mul33r, _mul33i, r, 0, &_div34r, &_div34i);
            coeff_imag += _div34r;
        }
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_mul(coeff_real, 0, coeff_real, 0, &_pow36r, &_pow36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 2.0; _c37i = 0;
        double _pow38r = 0, _pow38i = 0;
        c_mul(coeff_imag, 0, coeff_imag, 0, &_pow38r, &_pow38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _pow36r + _pow38r; _add39i = _pow36i + _pow38i;
        double _sqrt40r = 0, _sqrt40i = 0;
        c_powr(_add39r, _add39i, 0.5, &_sqrt40r, &_sqrt40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(j, 0, n, 0, &_div42r, &_div42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _c41r + _div42r; _add43i = _c41i + _div42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_sqrt40r, _sqrt40i, _add43r, _add43i, &_mul44r, &_mul44i);
        double magnitude = _mul44r; /* +_mul44ii */
        double _at245r = 0, _at245i = 0;
        _at245r = atan2(coeff_imag, coeff_real); _at245i = 0;
        double _sin46r = 0, _sin46i = 0;
        c_sin(j, 0, &_sin46r, &_sin46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 2.0; _c47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(j, 0, _c47r, _c47i, &_div48r, &_div48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_div48r, _div48i, &_cos49r, &_cos49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_sin46r, _sin46i, _cos49r, _cos49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _at245r + _mul50r; _add51i = _at245i + _mul50i;
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

/* poly_390: too complex for auto-transpile, stubbed */
static void poly_390_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_391: too complex for auto-transpile, stubbed */
static void poly_391_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_392_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double mag = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double ang = _c4r; /* +_c4ii */
        for (int k = 1; k < 6; k++) {
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
            double _mul10r = 0, _mul10i = 0;
            c_mul(j, 0, k, 0, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_mul9r, _mul9i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            double term1 = _mul12r; /* +_mul12ii */
            double _im13r = 0, _im13i = 0;
            _im13r = x2i; _im13i = 0;
            double _add14r = 0, _add14i = 0;
            _add14r = j + k; _add14i = 0 + 0;
            double _cos15r = 0, _cos15i = 0;
            c_cos(_add14r, _add14i, &_cos15r, &_cos15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_im13r, _im13i, _cos15r, _cos15i, &_mul16r, &_mul16i);
            double term2 = _mul16r; /* +_mul16ii */
            double _c17r = 0, _c17i = 0;
            _c17r = 2.0; _c17i = 0;
            double _pow18r = 0, _pow18i = 0;
            c_mul(term1, 0, term1, 0, &_pow18r, &_pow18i);
            double _c19r = 0, _c19i = 0;
            _c19r = 2.0; _c19i = 0;
            double _pow20r = 0, _pow20i = 0;
            c_mul(term2, 0, term2, 0, &_pow20r, &_pow20i);
            double _add21r = 0, _add21i = 0;
            _add21r = _pow18r + _pow20r; _add21i = _pow18i + _pow20i;
            mag += _add21r;
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
            double _sub28r = 0, _sub28i = 0;
            _sub28r = _mul24r - _mul27r; _sub28i = _mul24i - _mul27i;
            ang += _sub28r;
        }
        for (int r = 1; r < 4; r++) {
            double _add29r = 0, _add29i = 0;
            _add29r = x1r + r; _add29i = x1i + 0;
            double _abs30r = 0, _abs30i = 0;
            _abs30r = c_abs(_add29r, _add29i); _abs30i = 0;
            double _sqrt31r = 0, _sqrt31i = 0;
            c_powr(r, 0, 0.5, &_sqrt31r, &_sqrt31i);
            double _mul32r = 0, _mul32i = 0;
            c_mul(_abs30r, _abs30i, _sqrt31r, _sqrt31i, &_mul32r, &_mul32i);
            double _add33r = 0, _add33i = 0;
            _add33r = j + r; _add33i = 0 + 0;
            double _div34r = 0, _div34i = 0;
            c_div(_mul32r, _mul32i, _add33r, _add33i, &_div34r, &_div34i);
            mag += _div34r;
            double _mul35r = 0, _mul35i = 0;
            c_mul(r, 0, M_PI, 0, &_mul35r, &_mul35i);
            double _div36r = 0, _div36i = 0;
            c_div(_mul35r, _mul35i, j, 0, &_div36r, &_div36i);
            double _sin37r = 0, _sin37i = 0;
            c_sin(_div36r, _div36i, &_sin37r, &_sin37i);
            double _cos38r = 0, _cos38i = 0;
            c_cos(r, 0, &_cos38r, &_cos38i);
            double _mul39r = 0, _mul39i = 0;
            c_mul(_sin37r, _sin37i, _cos38r, _cos38i, &_mul39r, &_mul39i);
            ang += _mul39r;
        }
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(j, 0, n, 0, &_div41r, &_div41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _c40r + _div41r; _add42i = _c40i + _div41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(mag, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        double varied_mag = _mul43r; /* +_mul43ii */
        double _abs44r = 0, _abs44i = 0;
        _abs44r = c_abs(x1r, x1i); _abs44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = _abs44r + _c45r; _add46i = _abs44i + _c45i;
        double _log47r = 0, _log47i = 0;
        c_log(_add46r, _add46i, &_log47r, &_log47i);
        double _add48r = 0, _add48i = 0;
        _add48r = ang + _log47r; _add48i = 0 + _log47i;
        double _abs49r = 0, _abs49i = 0;
        _abs49r = c_abs(x2r, x2i); _abs49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 1.0; _c50i = 0;
        double _add51r = 0, _add51i = 0;
        _add51r = _abs49r + _c50r; _add51i = _abs49i + _c50i;
        double _log52r = 0, _log52i = 0;
        c_log(_add51r, _add51i, &_log52r, &_log52i);
        double _sub53r = 0, _sub53i = 0;
        _sub53r = _add48r - _log52r; _sub53i = _add48i - _log52i;
        double varied_ang = _sub53r; /* +_sub53ii */
        double _cos54r = 0, _cos54i = 0;
        c_cos(varied_ang, 0, &_cos54r, &_cos54i);
        double _c55r = 0, _c55i = 0;
        _c55r = 0.0; _c55i = 1.0;
        double _sin56r = 0, _sin56i = 0;
        c_sin(varied_ang, 0, &_sin56r, &_sin56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_c55r, _c55i, _sin56r, _sin56i, &_mul57r, &_mul57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _cos54r + _mul57r; _add58i = _cos54i + _mul57i;
        double _mul59r = 0, _mul59i = 0;
        c_mul(varied_mag, 0, _add58r, _add58i, &_mul59r, &_mul59i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_393_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _mul8r = 0, _mul8i = 0;
        c_mul(_arr7r, _arr7i, x1r, x1i, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_mul8r, _mul8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double mag_part1 = _log12r; /* +_log12ii */
        double _arr13r = 0, _arr13i = 0;
        { int _idx = (j - 1); _arr13r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr13i = 0; }
        double _cos14r = 0, _cos14i = 0;
        c_cos(_arr13r, _arr13i, &_cos14r, &_cos14i);
        double _arr15r = 0, _arr15i = 0;
        { int _idx = (j - 1); _arr15r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr15i = 0; }
        double _sin16r = 0, _sin16i = 0;
        c_sin(_arr15r, _arr15i, &_sin16r, &_sin16i);
        double _sum17r = 0, _sum17i = 0;
        _sum17r = _cos14r + _sin16r; _sum17i = _cos14i + _sin16i;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.5; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(j, 0, 0.5, &_pow19r, &_pow19i);
        double _div20r = 0, _div20i = 0;
        c_div(_sum17r, _sum17i, _pow19r, _pow19i, &_div20r, &_div20i);
        double mag_part2 = _div20r; /* +_div20ii */
        double _mul21r = 0, _mul21i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 7.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _c22r + _sin26r; _add27i = _c22i + _sin26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul21r, _mul21i, _add27r, _add27i, &_mul28r, &_mul28i);
        double mag = _mul28r; /* +_mul28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double ang_part1 = _mul33r; /* +_mul33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 3.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, _c35r, _c35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang34r, _ang34i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double ang_part2 = _mul38r; /* +_mul38ii */
        double _add39r = 0, _add39i = 0;
        _add39r = ang_part1 + ang_part2; _add39i = 0 + 0;
        double _abs40r = 0, _abs40i = 0;
        _abs40r = c_abs(j, 0); _abs40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _abs40r + _c41r; _add42i = _abs40i + _c41i;
        double _log43r = 0, _log43i = 0;
        c_log(_add42r, _add42i, &_log43r, &_log43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _add39r + _log43r; _add44i = _add39i + _log43i;
        double ang = _add44r; /* +_add44ii */
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, ang, 0, &_mul46r, &_mul46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(mag, 0, _exp47r, _exp47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = x1r + x2r; _add49i = x1i + x2i;
        double _conj50r = 0, _conj50i = 0;
        _conj50r = _add49r; _conj50i = -(_add49i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(j, 0, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_conj50r, _conj50i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 1.0; _c53i = 0;
        double _add54r = 0, _add54i = 0;
        _add54r = j + _c53r; _add54i = 0 + _c53i;
        double _div55r = 0, _div55i = 0;
        c_div(_mul52r, _mul52i, _add54r, _add54i, &_div55r, &_div55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _mul48r + _div55r; _add56i = _mul48i + _div55i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_396_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_re3r, _re3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _im8r = 0, _im8i = 0;
        _im8r = x2i; _im8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 8.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_im8r, _im8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul7r + _mul13r; _add14i = _mul7i + _mul13i;
        double r_part = _add14r; /* +_add14ii */
        double _re15r = 0, _re15i = 0;
        _re15r = x2r; _re15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 7.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_re15r, _re15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _im21r = 0, _im21i = 0;
        _im21r = x1i; _im21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 5.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_im21r, _im21i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _mul20r - _mul26r; _sub27i = _mul20i - _mul26i;
        double i_part = _sub27r; /* +_sub27ii */
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(r_part, 0, r_part, 0, &_pow29r, &_pow29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(i_part, 0, i_part, 0, &_pow31r, &_pow31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _pow29r + _pow31r; _add32i = _pow29i + _pow31i;
        double _sqrt33r = 0, _sqrt33i = 0;
        c_powr(_add32r, _add32i, 0.5, &_sqrt33r, &_sqrt33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 2.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(0, 0, _add35r, _add35i, &_div36r, &_div36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sqrt33r + _div36r; _add37i = _sqrt33i + _div36i;
        double mag = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, i_part, 0, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = r_part + _mul39r; _add40i = 0 + _mul39i;
        double temp = _add40r; /* +_add40ii */
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(temp, 0); _ang41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(j, 0, M_PI, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 6.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _ang41r + _cos45r; _add46i = _ang41i + _cos45i;
        double theta = _add46r; /* +_add46ii */
        double _cos47r = 0, _cos47i = 0;
        c_cos(theta, 0, &_cos47r, &_cos47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _sin49r = 0, _sin49i = 0;
        c_sin(theta, 0, &_sin49r, &_sin49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c48r, _c48i, _sin49r, _sin49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _cos47r + _mul50r; _add51i = _cos47i + _mul50i;
        double _mul52r = 0, _mul52i = 0;
        c_mul(mag, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_397_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(n, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _sub10r = 0, _sub10i = 0;
        _sub10r = j - _div9r; _sub10i = 0 - _div9i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_sub10r, _sub10i); _abs11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_abs7r, _abs7i, _abs11r, &_pow12r, &_pow12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 3.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(n, 0, _c14r, _c14i, &_div15r, &_div15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = j - _div15r; _sub16i = 0 - _div15i;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_sub16r, _sub16i); _abs17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs13r, _abs13i, _abs17r, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _pow12r + _pow18r; _add19i = _pow12i + _pow18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _add19r + _c20r; _add21i = _add19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double magnitude_part = _log22r; /* +_log22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _ang23r, _ang23i, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_mul24r, _mul24i, &_sin25r, &_sin25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _ang26r, _ang26i, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin25r, _sin25i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _im30r = 0, _im30i = 0;
        _im30r = x1i; _im30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_im30r, _im30i, M_PI, 0, &_mul31r, &_mul31i);
        double _div32r = 0, _div32i = 0;
        c_div(_mul31r, _mul31i, j, 0, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul29r + _sin33r; _add34i = _mul29i + _sin33i;
        double angle_part = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 3.0; _c35i = 0;
        double _mod36r = 0, _mod36i = 0;
        _mod36r = fmod(j, _c35r); _mod36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 0;
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang38r, _ang38i, j, 0, &_mul39r, &_mul39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_mul39r, _mul39i, &_cos40r, &_cos40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x2r, x2i); _ang41i = 0;
        double _sub42r = 0, _sub42i = 0;
        _sub42r = n - j; _sub42i = 0 - 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang41r, _ang41i, _sub42r, _sub42i, &_mul43r, &_mul43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_mul43r, _mul43i, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_cos40r, _cos40i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x1r, x1i); _ang46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _add48r = 0, _add48i = 0;
        _add48r = j + _c47r; _add48i = 0 + _c47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_ang46r, _ang46i, _add48r, _add48i, &_mul49r, &_mul49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(_mul49r, _mul49i, &_sin50r, &_sin50i);
        double _ang51r = 0, _ang51i = 0;
        _ang51r = c_arg(x2r, x2i); _ang51i = 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 2.0; _c52i = 0;
        double _add53r = 0, _add53i = 0;
        _add53r = j + _c52r; _add53i = 0 + _c52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_ang51r, _ang51i, _add53r, _add53i, &_mul54r, &_mul54i);
        double _cos55r = 0, _cos55i = 0;
        c_cos(_mul54r, _mul54i, &_cos55r, &_cos55i);
        double _sub56r = 0, _sub56i = 0;
        _sub56r = _sin50r - _cos55r; _sub56i = _sin50i - _cos55i;
        double _tern57r = 0, _tern57i = 0;
        if (_mod36r == _c37r) { _tern57r = _mul45r; _tern57i = _mul45i; }
        else { _tern57r = _sub56r; _tern57i = _sub56i; }
        double variation = _tern57r; /* +_tern57ii */
        double _c58r = 0, _c58i = 0;
        _c58r = 0.0; _c58i = 1.0;
        double _add59r = 0, _add59i = 0;
        _add59r = angle_part + variation; _add59i = 0 + 0;
        double _mul60r = 0, _mul60i = 0;
        c_mul(_c58r, _c58i, _add59r, _add59i, &_mul60r, &_mul60i);
        double _exp61r = 0, _exp61i = 0;
        c_exp2(_mul60r, _mul60i, &_exp61r, &_exp61i);
        double _mul62r = 0, _mul62i = 0;
        c_mul(magnitude_part, 0, _exp61r, _exp61i, &_mul62r, &_mul62i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul62r; cIm[_idx] = _mul62i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_398_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        c_mul(_arr7r, _arr7i, M_PI, 0, &_mul8r, &_mul8i);
        double _div9r = 0, _div9i = 0;
        c_div(_mul8r, _mul8i, n, 0, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr11i = 0; }
        double _mul12r = 0, _mul12i = 0;
        c_mul(_arr11r, _arr11i, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_c13r, _c13i, n, 0, &_mul14r, &_mul14i);
        double _div15r = 0, _div15i = 0;
        c_div(_mul12r, _mul12i, _mul14r, _mul14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_sin10r, _sin10i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double term1 = _mul17r; /* +_mul17ii */
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr18i = 0; }
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr19i = 0; }
        double _add20r = 0, _add20i = 0;
        _add20r = _arr18r + _arr19r; _add20i = _arr18i + _arr19i;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(_add20r, _add20i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double term2 = _log24r; /* +_log24ii */
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_abs25r, _abs25i, j, &_pow26r, &_pow26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _c27r + j; _add28i = _c27i + 0;
        double _div29r = 0, _div29i = 0;
        c_div(_pow26r, _pow26i, _add28r, _add28i, &_div29r, &_div29i);
        double term3 = _div29r; /* +_div29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang30r, _ang30i, j, 0, &_mul31r, &_mul31i);
        double term4 = _mul31r; /* +_mul31ii */
        double _add32r = 0, _add32i = 0;
        _add32r = term1 + term2; _add32i = 0 + 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _add32r + term3; _add33i = _add32i + 0;
        double magnitude = _add33r; /* +_add33ii */
        double _arr34r = 0, _arr34i = 0;
        { int _idx = (j - 1); _arr34r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr34i = 0; }
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, _arr34r, _arr34i, &_mul35r, &_mul35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_mul35r, _mul35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = term4 + _sin36r; _add37i = 0 + _sin36i;
        double _arr38r = 0, _arr38i = 0;
        { int _idx = (j - 1); _arr38r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr38i = 0; }
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, _arr38r, _arr38i, &_mul39r, &_mul39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_mul39r, _mul39i, &_cos40r, &_cos40i);
        double _sub41r = 0, _sub41i = 0;
        _sub41r = _add37r - _cos40r; _sub41i = _add37i - _cos40i;
        double angle = _sub41r; /* +_sub41ii */
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
