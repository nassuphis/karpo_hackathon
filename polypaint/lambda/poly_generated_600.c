/* AUTO-GENERATED from poly600.py — do not edit manually */
/* 100 coefficient functions */

static void poly_501_c(double x1r, double x1i, double x2r, double x2i,
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
        _c4r = 5.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(j, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_abs3r, _abs3i, _add7r, &_pow8r, &_pow8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs9r + j; _add10i = _abs9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_pow8r, _pow8i, _log11r, _log11i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 0.5; _c13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _c13r, _c13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_mul12r, _mul12i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.3; _c18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, _c18r, _c18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang17r, _ang17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul16r + _mul21r; _add22i = _mul16i + _mul21i;
        double mag = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 0.5; _c24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, _c24r, _c24i, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang23r, _ang23i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 0.3; _c29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, _c29r, _c29i, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul27r + _mul32r; _add33i = _mul27i + _mul32i;
        double angle = _add33r; /* +_add33ii */
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
        _c38r = 3.0; _c38i = 0;
        double _mod39r = 0, _mod39i = 0;
        _mod39r = fmod(j, _c38r); _mod39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        if (_mod39r == _c40r) {
            double _conj41r = 0, _conj41i = 0;
            _conj41r = x1r; _conj41i = -(x1i);
            double _attr42r = 0, _attr42i = 0;
            _attr42r = x2r; _attr42i = 0;
            double _mul43r = 0, _mul43i = 0;
            c_mul(j, 0, _attr42r, _attr42i, &_mul43r, &_mul43i);
            double _sin44r = 0, _sin44i = 0;
            c_sin(_mul43r, _mul43i, &_sin44r, &_sin44i);
            double _mul45r = 0, _mul45i = 0;
            c_mul(_conj41r, _conj41i, _sin44r, _sin44i, &_mul45r, &_mul45i);
            double _conj46r = 0, _conj46i = 0;
            _conj46r = x2r; _conj46i = -(x2i);
            double _attr47r = 0, _attr47i = 0;
            _attr47r = x1i; _attr47i = 0;
            double _mul48r = 0, _mul48i = 0;
            c_mul(j, 0, _attr47r, _attr47i, &_mul48r, &_mul48i);
            double _cos49r = 0, _cos49i = 0;
            c_cos(_mul48r, _mul48i, &_cos49r, &_cos49i);
            double _mul50r = 0, _mul50i = 0;
            c_mul(_conj46r, _conj46i, _cos49r, _cos49i, &_mul50r, &_mul50i);
            double _sub51r = 0, _sub51i = 0;
            _sub51r = _mul45r - _mul50r; _sub51i = _mul45i - _mul50i;
            cRe[(j - 1)] += _sub51r; cIm[(j - 1)] += _sub51i;
        } else {
            double _c52r = 0, _c52i = 0;
            _c52r = 3.0; _c52i = 0;
            double _mod53r = 0, _mod53i = 0;
            _mod53r = fmod(j, _c52r); _mod53i = 0;
            double _c54r = 0, _c54i = 0;
            _c54r = 2.0; _c54i = 0;
            if (_mod53r == _c54r) {
                double _attr55r = 0, _attr55i = 0;
                _attr55r = x1r; _attr55i = 0;
                double _attr56r = 0, _attr56i = 0;
                _attr56r = x2i; _attr56i = 0;
                double _mul57r = 0, _mul57i = 0;
                c_mul(_attr55r, _attr55i, _attr56r, _attr56i, &_mul57r, &_mul57i);
                double _sin58r = 0, _sin58i = 0;
                c_sin(j, 0, &_sin58r, &_sin58i);
                double _mul59r = 0, _mul59i = 0;
                c_mul(_mul57r, _mul57i, _sin58r, _sin58i, &_mul59r, &_mul59i);
                cRe[(j - 1)] += _mul59r; cIm[(j - 1)] += _mul59i;
            } else {
                double _attr60r = 0, _attr60i = 0;
                _attr60r = x2r; _attr60i = 0;
                double _attr61r = 0, _attr61i = 0;
                _attr61r = x1i; _attr61i = 0;
                double _mul62r = 0, _mul62i = 0;
                c_mul(_attr60r, _attr60i, _attr61r, _attr61i, &_mul62r, &_mul62i);
                double _cos63r = 0, _cos63i = 0;
                c_cos(j, 0, &_cos63r, &_cos63i);
                double _mul64r = 0, _mul64i = 0;
                c_mul(_mul62r, _mul62i, _cos63r, _cos63i, &_mul64r, &_mul64i);
                cRe[(j - 1)] += _mul64r; cIm[(j - 1)] += _mul64i;
            }
        }
    }
    double _c65r = 0, _c65i = 0;
    _c65r = 1.0; _c65i = 0;
    double _add66r = 0, _add66i = 0;
    _add66r = n + _c65r; _add66i = 0 + _c65i;
    for (int k = 1; k < (int)(_add66r); k++) {
        double _cf67r = 0, _cf67i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
        double _c68r = 0, _c68i = 0;
        _c68r = 1.0; _c68i = 0;
        double _c69r = 0, _c69i = 0;
        _c69r = 0.05; _c69i = 0;
        double _mul70r = 0, _mul70i = 0;
        c_mul(_c69r, _c69i, k, 0, &_mul70r, &_mul70i);
        double _add71r = 0, _add71i = 0;
        _add71r = _c68r + _mul70r; _add71i = _c68i + _mul70i;
        double _mul72r = 0, _mul72i = 0;
        c_mul(_cf67r, _cf67i, _add71r, _add71i, &_mul72r, &_mul72i);
        double _c73r = 0, _c73i = 0;
        _c73r = 0.02; _c73i = 0;
        double _ang74r = 0, _ang74i = 0;
        _ang74r = c_arg(x1r, x1i); _ang74i = 0;
        double _mul75r = 0, _mul75i = 0;
        c_mul(k, 0, _ang74r, _ang74i, &_mul75r, &_mul75i);
        double _sin76r = 0, _sin76i = 0;
        c_sin(_mul75r, _mul75i, &_sin76r, &_sin76i);
        double _mul77r = 0, _mul77i = 0;
        c_mul(_c73r, _c73i, _sin76r, _sin76i, &_mul77r, &_mul77i);
        double _ang78r = 0, _ang78i = 0;
        _ang78r = c_arg(x2r, x2i); _ang78i = 0;
        double _mul79r = 0, _mul79i = 0;
        c_mul(k, 0, _ang78r, _ang78i, &_mul79r, &_mul79i);
        double _cos80r = 0, _cos80i = 0;
        c_cos(_mul79r, _mul79i, &_cos80r, &_cos80i);
        double _mul81r = 0, _mul81i = 0;
        c_mul(_mul77r, _mul77i, _cos80r, _cos80i, &_mul81r, &_mul81i);
        double _add82r = 0, _add82i = 0;
        _add82r = _mul72r + _mul81r; _add82i = _mul72i + _mul81i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add82r; cIm[_idx] = _add82i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_502_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _attr3r, _attr3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _sin5r + _cos8r; _add9i = _sin5i + _cos8i;
        double phase_r = _add9r; /* +_add9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = _cos12r - _sin15r; _sub16i = _cos12i - _sin15i;
        double phase_i = _sub16r; /* +_sub16ii */
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
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _add22r + _c23r; _add24i = _add22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double magnitude = _log25r; /* +_log25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang26r, _ang26i, j, 0, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _sub29r = 0, _sub29i = 0;
        _sub29r = n - j; _sub29i = 0 - 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _sub29r, _sub29i, &_mul30r, &_mul30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _mul27r - _mul30r; _sub31i = _mul27i - _mul30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(phase_r, 0, phase_i, 0, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _sub31r + _mul32r; _add33i = _sub31i + _mul32i;
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

static void poly_503_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(j, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin11r, _sin11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _c10r + _mul13r; _add14i = _c10i + _mul13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log9r, _log9i, _add14r, _add14i, &_mul15r, &_mul15i);
        double mag_part1 = _mul15r; /* +_mul15ii */
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
        double _add22r = 0, _add22i = 0;
        _add22r = _mul17r + _div21r; _add22i = _mul17i + _div21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, rec1, 0, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _add22r + _sin24r; _add25i = _add22i + _sin24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, imc2, 0, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _add25r - _cos27r; _sub28i = _add25i - _cos27i;
        double mag_part2 = _sub28r; /* +_sub28ii */
        double _mul29r = 0, _mul29i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul29r, &_mul29i);
        double magnitude = _mul29r; /* +_mul29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang30r, _ang30i, j, 0, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _div35r = 0, _div35i = 0;
        c_div(_ang32r, _ang32i, _add34r, _add34i, &_div35r, &_div35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul31r + _div35r; _add36i = _mul31i + _div35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, rec1, 0, &_mul37r, &_mul37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_mul37r, _mul37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add36r + _sin38r; _add39i = _add36i + _sin38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, imc2, 0, &_mul40r, &_mul40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_mul40r, _mul40i, &_cos41r, &_cos41i);
        double _sub42r = 0, _sub42i = 0;
        _sub42r = _add39r - _cos41r; _sub42i = _add39i - _cos41i;
        double angle = _sub42r; /* +_sub42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, angle, 0, &_mul44r, &_mul44i);
        double _exp45r = 0, _exp45i = 0;
        c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(magnitude, 0, _exp45r, _exp45i, &_mul46r, &_mul46i);
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
        double _add54r = 0, _add54i = 0;
        _add54r = _add50r + _mul53r; _add54i = _add50i + _mul53i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add54r; cIm[_idx] = _add54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_504_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs14r, _abs14i, j, &_pow15r, &_pow15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = n - j; _sub17i = 0 - 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_abs16r, _abs16i, _sub17r, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _pow15r + _pow18r; _add19i = _pow15i + _pow18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log13r, _log13i, _add19r, _add19i, &_mul20r, &_mul20i);
        double magnitude = _mul20r; /* +_mul20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(j, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang21r, _ang21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul23r + _mul28r; _add29i = _mul23i + _mul28i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    double _c36r = 0, _c36i = 0;
    _c36r = 2.0; _c36i = 0;
    double _div37r = 0, _div37i = 0;
    c_div(n, 0, _c36r, _c36i, &_div37r, &_div37i);
    double _flr38r = 0, _flr38i = 0;
    _flr38r = floor(_div37r); _flr38i = 0;
    double _c39r = 0, _c39i = 0;
    _c39r = 1.0; _c39i = 0;
    double _add40r = 0, _add40i = 0;
    _add40r = _flr38r + _c39r; _add40i = _flr38i + _c39i;
    for (int k = 1; k < (int)(_add40r); k++) {
        if (k <= n) {
            double _prod41r = 0, _prod41i = 0;
            /* WARNING: unhandled np.prod */
            double _conj42r = 0, _conj42i = 0;
            _conj42r = x2r; _conj42i = -(x2i);
            double _pow43r = 0, _pow43i = 0;
            c_powr(_conj42r, _conj42i, k, &_pow43r, &_pow43i);
            double _mul44r = 0, _mul44i = 0;
            c_mul(_prod41r, _prod41i, _pow43r, _pow43i, &_mul44r, &_mul44i);
            cRe[(k - 1)] += _mul44r; cIm[(k - 1)] += _mul44i;
            double _sum45r = 0, _sum45i = 0;
            /* WARNING: unhandled np.sum */
            double _abs46r = 0, _abs46i = 0;
            _abs46r = c_abs(x1r, x1i); _abs46i = 0;
            double _mul47r = 0, _mul47i = 0;
            c_mul(_abs46r, _abs46i, k, 0, &_mul47r, &_mul47i);
            double _sin48r = 0, _sin48i = 0;
            c_sin(_mul47r, _mul47i, &_sin48r, &_sin48i);
            double _mul49r = 0, _mul49i = 0;
            c_mul(_sum45r, _sum45i, _sin48r, _sin48i, &_mul49r, &_mul49i);
            double _abs50r = 0, _abs50i = 0;
            _abs50r = c_abs(x2r, x2i); _abs50i = 0;
            double _c51r = 0, _c51i = 0;
            _c51r = 1.0; _c51i = 0;
            double _add52r = 0, _add52i = 0;
            _add52r = k + _c51r; _add52i = 0 + _c51i;
            double _div53r = 0, _div53i = 0;
            c_div(_abs50r, _abs50i, _add52r, _add52i, &_div53r, &_div53i);
            double _cos54r = 0, _cos54i = 0;
            c_cos(_div53r, _div53i, &_cos54r, &_cos54i);
            double _mul55r = 0, _mul55i = 0;
            c_mul(_mul49r, _mul49i, _cos54r, _cos54i, &_mul55r, &_mul55i);
            cRe[((int)(n) - k)] += _mul55r; cIm[((int)(n) - k)] += _mul55i;
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_505_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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

static void poly_506_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x2r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _attr3r + _mul5r; _add6i = _attr3i + _mul5i;
        double r = _add6r; /* +_add6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1i; _attr7i = 0;
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sub10r = 0, _sub10i = 0;
        _sub10r = _attr7r - _mul9r; _sub10i = _attr7i - _mul9i;
        double im = _sub10r; /* +_sub10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs11r + j; _add12i = _abs11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _sin16r + _cos19r; _add20i = _sin16i + _cos19i;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(_add20r, _add20i); _abs21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log13r, _log13i, _abs21r, _abs21i, &_mul22r, &_mul22i);
        double mag = _mul22r; /* +_mul22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang23r, _ang23i, j, 0, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _sub26r = 0, _sub26i = 0;
        _sub26r = n - j; _sub26i = 0 - 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _sub26r, _sub26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul24r + _mul27r; _add28i = _mul24i + _mul27i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    double _c35r = 0, _c35i = 0;
    _c35r = 1.0; _c35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = n + _c35r; _add36i = 0 + _c35i;
    for (int k = 1; k < (int)(_add36r); k++) {
        double _cf37r = 0, _cf37i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
        double _conj38r = 0, _conj38i = 0;
        _conj38r = _cf37r; _conj38i = -(_cf37i);
        double _attr39r = 0, _attr39i = 0;
        _attr39r = x1r; _attr39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(k, 0, _attr39r, _attr39i, &_mul40r, &_mul40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_mul40r, _mul40i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj38r, _conj38i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = _c43r + k; _add44i = _c43i + 0;
        double _div45r = 0, _div45i = 0;
        c_div(_mul42r, _mul42i, _add44r, _add44i, &_div45r, &_div45i);
        cRe[(k - 1)] += _div45r; cIm[(k - 1)] += _div45i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_507_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_mul9r, _mul9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 4.0; _c14i = 0;
        double _mod15r = 0, _mod15i = 0;
        _mod15r = fmod(j, _c14r); _mod15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _mod15r + _c16r; _add17i = _mod15i + _c16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log13r, _log13i, _add17r, _add17i, &_mul18r, &_mul18i);
        double mag = _mul18r; /* +_mul18ii */
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_mul19r, _mul19i, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul23r, _mul23i, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _sin22r + _cos26r; _add27i = _sin22i + _cos26i;
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x1r; _attr28i = 0;
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(_attr28r, _attr28i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 0.5; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(_abs29r, _abs29i, 0.5, &_pow31r, &_pow31i);
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2i; _attr32i = 0;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_attr32r, _attr32i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 0.3; _c34i = 0;
        double _pow35r = 0, _pow35i = 0;
        c_powr(_abs33r, _abs33i, 0.3, &_pow35r, &_pow35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_pow31r, _pow31i, _pow35r, _pow35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add27r + _mul36r; _add37i = _add27i + _mul36i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    double _c44r = 0, _c44i = 0;
    _c44r = 1.0; _c44i = 0;
    double _add45r = 0, _add45i = 0;
    _add45r = n + _c44r; _add45i = 0 + _c44i;
    for (int k = 1; k < (int)(_add45r); k++) {
        double _cf46r = 0, _cf46i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 0.5; _c48i = 0;
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 3.0; _c50i = 0;
        double _sub51r = 0, _sub51i = 0;
        _sub51r = k - _c50r; _sub51i = 0 - _c50i;
        double _max52r = 0, _max52i = 0;
        _max52r = fmax(_c49r, _sub51r); _max52i = 0;
        double _cf53r = 0, _cf53i = 0;
        { int _idx = (int)(_max52r); if (_idx >= 0 && _idx < 36) { _cf53r = cRe[_idx]; _cf53i = cIm[_idx]; } }
        double _conj54r = 0, _conj54i = 0;
        _conj54r = _cf53r; _conj54i = -(_cf53i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c48r, _c48i, _conj54r, _conj54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _c47r + _mul55r; _add56i = _c47i + _mul55i;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_cf46r, _cf46i, _add56r, _add56i, &_mul57r, &_mul57i);
        double _c58r = 0, _c58i = 0;
        _c58r = 0.3; _c58i = 0;
        double _sin59r = 0, _sin59i = 0;
        c_sin(k, 0, &_sin59r, &_sin59i);
        double _mul60r = 0, _mul60i = 0;
        c_mul(_c58r, _c58i, _sin59r, _sin59i, &_mul60r, &_mul60i);
        double _cos61r = 0, _cos61i = 0;
        c_cos(k, 0, &_cos61r, &_cos61i);
        double _mul62r = 0, _mul62i = 0;
        c_mul(_mul60r, _mul60i, _cos61r, _cos61i, &_mul62r, &_mul62i);
        double _add63r = 0, _add63i = 0;
        _add63r = _mul57r + _mul62r; _add63i = _mul57i + _mul62i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add63r; cIm[_idx] = _add63i; } }
    }
    double _c64r = 0, _c64i = 0;
    _c64r = 1.0; _c64i = 0;
    double _add65r = 0, _add65i = 0;
    _add65r = n + _c64r; _add65i = 0 + _c64i;
    for (int r = 1; r < (int)(_add65r); r++) {
        double _cf66r = 0, _cf66i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf66r = cRe[_idx]; _cf66i = cIm[_idx]; } }
        double _c67r = 0, _c67i = 0;
        _c67r = 1.0; _c67i = 0;
        double _c68r = 0, _c68i = 0;
        _c68r = 0.5; _c68i = 0;
        double _c69r = 0, _c69i = 0;
        _c69r = 0.0; _c69i = 0;
        double _c70r = 0, _c70i = 0;
        _c70r = 2.0; _c70i = 0;
        double _sub71r = 0, _sub71i = 0;
        _sub71r = r - _c70r; _sub71i = 0 - _c70i;
        double _max72r = 0, _max72i = 0;
        _max72r = fmax(_c69r, _sub71r); _max72i = 0;
        double _cf73r = 0, _cf73i = 0;
        { int _idx = (int)(_max72r); if (_idx >= 0 && _idx < 36) { _cf73r = cRe[_idx]; _cf73i = cIm[_idx]; } }
        double _conj74r = 0, _conj74i = 0;
        _conj74r = _cf73r; _conj74i = -(_cf73i);
        double _mul75r = 0, _mul75i = 0;
        c_mul(_c68r, _c68i, _conj74r, _conj74i, &_mul75r, &_mul75i);
        double _add76r = 0, _add76i = 0;
        _add76r = _c67r + _mul75r; _add76i = _c67i + _mul75i;
        double _mul77r = 0, _mul77i = 0;
        c_mul(_cf66r, _cf66i, _add76r, _add76i, &_mul77r, &_mul77i);
        double _c78r = 0, _c78i = 0;
        _c78r = 0.3; _c78i = 0;
        double _sin79r = 0, _sin79i = 0;
        c_sin(r, 0, &_sin79r, &_sin79i);
        double _mul80r = 0, _mul80i = 0;
        c_mul(_c78r, _c78i, _sin79r, _sin79i, &_mul80r, &_mul80i);
        double _cos81r = 0, _cos81i = 0;
        c_cos(r, 0, &_cos81r, &_cos81i);
        double _mul82r = 0, _mul82i = 0;
        c_mul(_mul80r, _mul80i, _cos81r, _cos81i, &_mul82r, &_mul82i);
        double _add83r = 0, _add83i = 0;
        _add83r = _mul77r + _mul82r; _add83i = _mul77i + _mul82i;
        cRe[(r - 1)] += _add83r; cIm[(r - 1)] += _add83i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_508_c(double x1r, double x1i, double x2r, double x2i,
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
        _c3r = 2.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(j, 0, j, 0, &_pow4r, &_pow4i);
        double k = _pow4r; /* +_pow4ii */
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x1r, x1i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 4.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_ang5r, _ang5i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x2r, x2i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 6.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang11r, _ang11i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul10r + _mul16r; _add17i = _mul10i + _mul16i;
        double angle_part = _add17r; /* +_add17ii */
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
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(j, 0, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _c23r + _sin26r; _add27i = _c23i + _sin26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log22r, _log22i, _add27r, _add27i, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _c29r + _cos32r; _add33i = _c29i + _cos32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_mul28r, _mul28i, _add33r, _add33i, &_mul34r, &_mul34i);
        double magnitude_part = _mul34r; /* +_mul34ii */
        double _conj35r = 0, _conj35i = 0;
        _conj35r = x1r; _conj35i = -(x1i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.5; _c36i = 0;
        double _pow37r = 0, _pow37i = 0;
        c_powr(_conj35r, _conj35i, 0.5, &_pow37r, &_pow37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(j, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_pow37r, _pow37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x2r; _conj40i = -(x2i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.3; _c41i = 0;
        double _pow42r = 0, _pow42i = 0;
        c_powr(_conj40r, _conj40i, 0.3, &_pow42r, &_pow42i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(j, 0, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_pow42r, _pow42i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _mul39r + _mul44r; _add45i = _mul39i + _mul44i;
        double perturbation = _add45r; /* +_add45ii */
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c46r, _c46i, angle_part, 0, &_mul47r, &_mul47i);
        double _exp48r = 0, _exp48i = 0;
        c_exp2(_mul47r, _mul47i, &_exp48r, &_exp48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(magnitude_part, 0, _exp48r, _exp48i, &_mul49r, &_mul49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 0.1; _c50i = 0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c50r, _c50i, perturbation, 0, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul49r + _mul51r; _add52i = _mul49i + _mul51i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    }
    for (int k = 1; k < 6; k++) {
        for (int r = 1; r < 8; r++) {
            double _c53r = 0, _c53i = 0;
            _c53r = 1.0; _c53i = 0;
            double _sub54r = 0, _sub54i = 0;
            _sub54r = k - _c53r; _sub54i = 0 - _c53i;
            double _c55r = 0, _c55i = 0;
            _c55r = 7.0; _c55i = 0;
            double _mul56r = 0, _mul56i = 0;
            c_mul(_sub54r, _sub54i, _c55r, _c55i, &_mul56r, &_mul56i);
            double _add57r = 0, _add57i = 0;
            _add57r = _mul56r + r; _add57i = _mul56i + 0;
            double index = _add57r; /* +_add57ii */
            if (index <= n) {
                double _cf58r = 0, _cf58i = 0;
                { int _idx = ((int)(index) - 1); if (_idx >= 0 && _idx < 36) { _cf58r = cRe[_idx]; _cf58i = cIm[_idx]; } }
                double _c59r = 0, _c59i = 0;
                _c59r = 1.0; _c59i = 0;
                double _c60r = 0, _c60i = 0;
                _c60r = 0.05; _c60i = 0;
                double _mul61r = 0, _mul61i = 0;
                c_mul(k, 0, r, 0, &_mul61r, &_mul61i);
                double _sin62r = 0, _sin62i = 0;
                c_sin(_mul61r, _mul61i, &_sin62r, &_sin62i);
                double _mul63r = 0, _mul63i = 0;
                c_mul(_c60r, _c60i, _sin62r, _sin62i, &_mul63r, &_mul63i);
                double _add64r = 0, _add64i = 0;
                _add64r = _c59r + _mul63r; _add64i = _c59i + _mul63i;
                double _mul65r = 0, _mul65i = 0;
                c_mul(_cf58r, _cf58i, _add64r, _add64i, &_mul65r, &_mul65i);
                double _c66r = 0, _c66i = 0;
                _c66r = 0.02; _c66i = 0;
                double _add67r = 0, _add67i = 0;
                _add67r = k + r; _add67i = 0 + 0;
                double _cos68r = 0, _cos68i = 0;
                c_cos(_add67r, _add67i, &_cos68r, &_cos68i);
                double _mul69r = 0, _mul69i = 0;
                c_mul(_c66r, _c66i, _cos68r, _cos68i, &_mul69r, &_mul69i);
                double _add70r = 0, _add70i = 0;
                _add70r = _mul65r + _mul69r; _add70i = _mul65i + _mul69i;
                { int _idx = ((int)(index) - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add70r; cIm[_idx] = _add70i; } }
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_509_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 4.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(j, 0, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 6.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(j, 0, _c10r, _c10i, &_div11r, &_div11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_div11r, _div11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_sin9r, _sin9i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _c6r + _mul13r; _add14i = _c6i + _mul13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log5r, _log5i, _add14r, _add14i, &_mul15r, &_mul15i);
        double mag_part = _mul15r; /* +_mul15ii */
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _sqrt17r = 0, _sqrt17i = 0;
        c_powr(j, 0, 0.5, &_sqrt17r, &_sqrt17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang16r, _ang16i, _sqrt17r, _sqrt17i, &_mul18r, &_mul18i);
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
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _mul18r - _mul23r; _sub24i = _mul18i - _mul23i;
        double angle_part = _sub24r; /* +_sub24ii */
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
        double _conj31r = 0, _conj31i = 0;
        _conj31r = x1r; _conj31i = -(x1i);
        double _pow32r = 0, _pow32i = 0;
        c_powr(x2r, x2i, j, &_pow32r, &_pow32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_conj31r, _conj31i, _pow32r, _pow32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul30r + _mul33r; _add34i = _mul30i + _mul33i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add34r; cIm[_idx] = _add34i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_510_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
        double _sqrt12r = 0, _sqrt12i = 0;
        c_powr(j, 0, 0.5, &_sqrt12r, &_sqrt12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log11r, _log11i, _sqrt12r, _sqrt12i, &_mul13r, &_mul13i);
        double mag_part1 = _mul13r; /* +_mul13ii */
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(j, 0, _c15r, _c15i, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_abs14r, _abs14i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double mag_part2 = _mul18r; /* +_mul18ii */
        double _add19r = 0, _add19i = 0;
        _add19r = mag_part1 + mag_part2; _add19i = 0 + 0;
        double magnitude = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 5.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _ang20r + _cos24r; _add25i = _ang20i + _cos24i;
        double ang_part1 = _add25r; /* +_add25ii */
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 7.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_sin29r, _sin29i, _ang30r, _ang30i, &_mul31r, &_mul31i);
        double ang_part2 = _mul31r; /* +_mul31ii */
        double _add32r = 0, _add32i = 0;
        _add32r = ang_part1 + ang_part2; _add32i = 0 + 0;
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

static void poly_511_c(double x1r, double x1i, double x2r, double x2i,
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
        double _sin3r = 0, _sin3i = 0;
        c_sin(j, 0, &_sin3r, &_sin3i);
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_sin3r, _sin3i, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _c6r, _c6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_cos8r, _cos8i, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul5r + _mul10r; _add11i = _mul5i + _mul10i;
        double term1 = _add11r; /* +_add11ii */
        double _c12r = 0, _c12i = 0;
        _c12r = 3.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(j, 0, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x2r; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_cos14r, _cos14i, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 4.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(j, 0, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1i; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_sin19r, _sin19i, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _mul16r - _mul21r; _sub22i = _mul16i - _mul21i;
        double term2 = _sub22r; /* +_sub22ii */
        double _add23r = 0, _add23i = 0;
        _add23r = term1 + term2; _add23i = 0 + 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_add23r, _add23i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log27r, _log27i, j, 0, &_mul28r, &_mul28i);
        double magnitude = _mul28r; /* +_mul28ii */
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
        double _add39r = 0, _add39i = 0;
        _add39r = _mul33r + _mul38r; _add39i = _mul33i + _mul38i;
        double _sin40r = 0, _sin40i = 0;
        c_sin(j, 0, &_sin40r, &_sin40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 2.0; _c41i = 0;
        double _pow42r = 0, _pow42i = 0;
        c_mul(_sin40r, _sin40i, _sin40r, _sin40i, &_pow42r, &_pow42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add39r + _pow42r; _add43i = _add39i + _pow42i;
        double angle = _add43r; /* +_add43ii */
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, angle, 0, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(magnitude, 0, _exp46r, _exp46i, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.3; _c48i = 0;
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _c50r = 0, _c50i = 0;
        _c50r = 2.0; _c50i = 0;
        double _div51r = 0, _div51i = 0;
        c_div(angle, 0, _c50r, _c50i, &_div51r, &_div51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c49r, _c49i, _div51r, _div51i, &_mul52r, &_mul52i);
        double _exp53r = 0, _exp53i = 0;
        c_exp2(_mul52r, _mul52i, &_exp53r, &_exp53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c48r, _c48i, _exp53r, _exp53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _mul47r + _mul54r; _add55i = _mul47i + _mul54i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_512_c(double x1r, double x1i, double x2r, double x2i,
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
        double _sub9r = 0, _sub9i = 0;
        _sub9r = x1r - x2r; _sub9i = x1i - x2i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_sub9r, _sub9i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _abs10r; _add11i = 0 + _abs10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _add8r + _log12r; _add13i = _add8i + _log12i;
        double mag = _add13r; /* +_add13ii */
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
        double _sub19r = 0, _sub19i = 0;
        _sub19r = _mul15r - _mul18r; _sub19i = _mul15i - _mul18i;
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(j, 0, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin20r, _sin20i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _sub19r + _mul22r; _add23i = _sub19i + _mul22i;
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
    for (int k = 1; k < 6; k++) {
        double _conj30r = 0, _conj30i = 0;
        _conj30r = x1r; _conj30i = -(x1i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(k, 0, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_conj30r, _conj30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        { double _tr = cRe[(k - 1)]*_mul32r - cIm[(k - 1)]*_mul32i; cIm[(k - 1)] = cRe[(k - 1)]*_mul32i + cIm[(k - 1)]*_mul32r; cRe[(k - 1)] = _tr; }
    }
    for (int r = 1; r < 6; r++) {
        double _conj33r = 0, _conj33i = 0;
        _conj33r = x2r; _conj33i = -(x2i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(r, 0, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_conj33r, _conj33i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        { double _tr = cRe[((int)(n) - r)]*_mul35r - cIm[((int)(n) - r)]*_mul35i; cIm[((int)(n) - r)] = cRe[((int)(n) - r)]*_mul35i + cIm[((int)(n) - r)]*_mul35r; cRe[((int)(n) - r)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_513_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double real_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        real_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imag_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imag_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _add7r = 0, _add7i = 0;
        _add7r = x1r + j; _add7i = x1i + 0;
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
        _c13r = 7.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log11r, _log11i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_cos20r, _cos20i, 0, 0, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul16r + _mul21r; _add22i = _mul16i + _mul21i;
        double magnitude = _add22r; /* +_add22ii */
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
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 4.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_sin27r, _sin27i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _ang23r + _mul32r; _add33i = _ang23i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 6.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _tsin37r = 0, _tsin37i = 0;
        double _tcos38r = 0, _tcos38i = 0;
        double _tan39r = 0, _tan39i = 0;
        c_sin(_div36r, _div36i, &_tsin37r, &_tsin37i);
        c_cos(_div36r, _div36i, &_tcos38r, &_tcos38i);
        c_div(_tsin37r, _tsin37i, _tcos38r, _tcos38i, &_tan39r, &_tan39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _add33r + _tan39r; _add40i = _add33i + _tan39i;
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
    _c47r = 2.0; _c47i = 0;
    double _fdiv48r = 0, _fdiv48i = 0;
    c_div(n, 0, _c47r, _c47i, &_fdiv48r, &_fdiv48i);
    _fdiv48r = floor(_fdiv48r); _fdiv48i = 0;
    double _c49r = 0, _c49i = 0;
    _c49r = 1.0; _c49i = 0;
    double _add50r = 0, _add50i = 0;
    _add50r = _fdiv48r + _c49r; _add50i = _fdiv48i + _c49i;
    for (int k = 1; k < (int)(_add50r); k++) {
        double _c51r = 0, _c51i = 0;
        _c51r = 2.0; _c51i = 0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(k, 0, _c51r, _c51i, &_mul52r, &_mul52i);
        double idx = _mul52r; /* +_mul52ii */
        if (idx <= n) {
            double _c53r = 0, _c53i = 0;
            _c53r = 0.0; _c53i = 1.0;
            double _arr54r = 0, _arr54i = 0;
            { int _idx = (k - 1); _arr54r = (_idx >= 0 && _idx < 35) ? real_seq[_idx] : 0.0; _arr54i = 0; }
            double _mul55r = 0, _mul55i = 0;
            c_mul(_c53r, _c53i, _arr54r, _arr54i, &_mul55r, &_mul55i);
            double _arr56r = 0, _arr56i = 0;
            { int _idx = (k - 1); _arr56r = (_idx >= 0 && _idx < 35) ? imag_seq[_idx] : 0.0; _arr56i = 0; }
            double _abs57r = 0, _abs57i = 0;
            _abs57r = c_abs(_arr56r, _arr56i); _abs57i = 0;
            double _c58r = 0, _c58i = 0;
            _c58r = 1.0; _c58i = 0;
            double _add59r = 0, _add59i = 0;
            _add59r = _abs57r + _c58r; _add59i = _abs57i + _c58i;
            double _div60r = 0, _div60i = 0;
            c_div(_mul55r, _mul55i, _add59r, _add59i, &_div60r, &_div60i);
            double _exp61r = 0, _exp61i = 0;
            c_exp2(_div60r, _div60i, &_exp61r, &_exp61i);
            { double _tr = cRe[((int)(idx) - 1)]*_exp61r - cIm[((int)(idx) - 1)]*_exp61i; cIm[((int)(idx) - 1)] = cRe[((int)(idx) - 1)]*_exp61i + cIm[((int)(idx) - 1)]*_exp61r; cRe[((int)(idx) - 1)] = _tr; }
        }
    }
    double _c62r = 0, _c62i = 0;
    _c62r = 3.0; _c62i = 0;
    double _mod63r = 0, _mod63i = 0;
    _mod63r = fmod(n, _c62r); _mod63i = 0;
    double _c64r = 0, _c64i = 0;
    _c64r = 2.0; _c64i = 0;
    double _add65r = 0, _add65i = 0;
    _add65r = _mod63r + _c64r; _add65i = _mod63i + _c64i;
    for (int r = 1; r < (int)(_add65r); r++) {
        double _cf66r = 0, _cf66i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf66r = cRe[_idx]; _cf66i = cIm[_idx]; } }
        double _c67r = 0, _c67i = 0;
        _c67r = 2.0; _c67i = 0;
        double _pow68r = 0, _pow68i = 0;
        c_mul(_cf66r, _cf66i, _cf66r, _cf66i, &_pow68r, &_pow68i);
        double _c69r = 0, _c69i = 0;
        _c69r = 1.0; _c69i = 0;
        double _cf70r = 0, _cf70i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf70r = cRe[_idx]; _cf70i = cIm[_idx]; } }
        double _abs71r = 0, _abs71i = 0;
        _abs71r = c_abs(_cf70r, _cf70i); _abs71i = 0;
        double _add72r = 0, _add72i = 0;
        _add72r = _c69r + _abs71r; _add72i = _c69i + _abs71i;
        double _div73r = 0, _div73i = 0;
        c_div(_pow68r, _pow68i, _add72r, _add72i, &_div73r, &_div73i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div73r; cIm[_idx] = _div73i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_514_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
    for (int r = 1; r < (int)(_add6r); r++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _abs8r; _add9i = _abs7i + _abs8i;
        double _add10r = 0, _add10i = 0;
        _add10r = _add9r + r; _add10i = _add9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _arr12r = 0, _arr12i = 0;
        { int _idx = (r - 1); _arr12r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr12i = 0; }
        double _mul13r = 0, _mul13i = 0;
        c_mul(_arr12r, _arr12i, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = r + _c14r; _add15i = 0 + _c14i;
        double _div16r = 0, _div16i = 0;
        c_div(_mul13r, _mul13i, _add15r, _add15i, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (r - 1); _arr18r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr18i = 0; }
        double _mul19r = 0, _mul19i = 0;
        c_mul(_arr18r, _arr18i, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = r + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_mul19r, _mul19i, _add21r, _add21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _sin17r + _cos23r; _add24i = _sin17i + _cos23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log11r, _log11i, _add24r, _add24i, &_mul25r, &_mul25i);
        double mag_part = _mul25r; /* +_mul25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 5.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(r, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 7.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(r, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul30r + _mul35r; _add36i = _mul30i + _mul35i;
        double angle_part = _add36r; /* +_add36ii */
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 0;
        double intricate_sum = _c37r; /* +_c37ii */
        for (int j = 1; j < 4; j++) {
            double _attr38r = 0, _attr38i = 0;
            _attr38r = x1r; _attr38i = 0;
            double _pow39r = 0, _pow39i = 0;
            c_powr(_attr38r, _attr38i, j, &_pow39r, &_pow39i);
            double _attr40r = 0, _attr40i = 0;
            _attr40r = x2i; _attr40i = 0;
            double _pow41r = 0, _pow41i = 0;
            c_powr(_attr40r, _attr40i, j, &_pow41r, &_pow41i);
            double _sub42r = 0, _sub42i = 0;
            _sub42r = _pow39r - _pow41r; _sub42i = _pow39i - _pow41i;
            double _arr43r = 0, _arr43i = 0;
            { int _idx = (r - 1); _arr43r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr43i = 0; }
            double _mul44r = 0, _mul44i = 0;
            c_mul(j, 0, _arr43r, _arr43i, &_mul44r, &_mul44i);
            double _sin45r = 0, _sin45i = 0;
            c_sin(_mul44r, _mul44i, &_sin45r, &_sin45i);
            double _mul46r = 0, _mul46i = 0;
            c_mul(_sub42r, _sub42i, _sin45r, _sin45i, &_mul46r, &_mul46i);
            double _arr47r = 0, _arr47i = 0;
            { int _idx = (r - 1); _arr47r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr47i = 0; }
            double _mul48r = 0, _mul48i = 0;
            c_mul(j, 0, _arr47r, _arr47i, &_mul48r, &_mul48i);
            double _cos49r = 0, _cos49i = 0;
            c_cos(_mul48r, _mul48i, &_cos49r, &_cos49i);
            double _mul50r = 0, _mul50i = 0;
            c_mul(_mul46r, _mul46i, _cos49r, _cos49i, &_mul50r, &_mul50i);
            intricate_sum += _mul50r;
        }
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c51r, _c51i, angle_part, 0, &_mul52r, &_mul52i);
        double _exp53r = 0, _exp53i = 0;
        c_exp2(_mul52r, _mul52i, &_exp53r, &_exp53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(mag_part, 0, _exp53r, _exp53i, &_mul54r, &_mul54i);
        double _conj55r = 0, _conj55i = 0;
        _conj55r = x1r; _conj55i = -(x1i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(intricate_sum, 0, _conj55r, _conj55i, &_mul56r, &_mul56i);
        double _pow57r = 0, _pow57i = 0;
        c_powr(x2r, x2i, r, &_pow57r, &_pow57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_mul56r, _mul56i, _pow57r, _pow57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _mul54r + _mul58r; _add59i = _mul54i + _mul58i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_515_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _attr3r, _attr3i, &_mul4r, &_mul4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2i; _attr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _attr5r, _attr5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _mul4r + _cos7r; _add8i = _mul4i + _cos7i;
        double _sin9r = 0, _sin9i = 0;
        c_sin(_add8r, _add8i, &_sin9r, &_sin9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1i; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _sub15r = 0, _sub15i = 0;
        _sub15r = _mul11r - _sin14r; _sub15i = _mul11i - _sin14i;
        double _cos16r = 0, _cos16i = 0;
        c_cos(_sub15r, _sub15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _sin9r + _cos16r; _add17i = _sin9i + _cos16i;
        double phase = _add17r; /* +_add17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _sqrt22r = 0, _sqrt22i = 0;
        c_powr(j, 0, 0.5, &_sqrt22r, &_sqrt22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x2r; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sqrt22r, _sqrt22i, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _log21r + _mul24r; _add25i = _log21i + _mul24i;
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x1i; _attr26i = 0;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_attr26r, _attr26i); _abs27i = 0;
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _add25r - _abs27r; _sub28i = _add25i - _abs27i;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1r; _attr29i = 0;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x2r; _attr30i = 0;
        double _prod31r = 0, _prod31i = 0;
        c_mul(_attr29r, _attr29i, _attr30r, _attr30i, &_prod31r, &_prod31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = j + _c32r; _add33i = 0 + _c32i;
        double _div34r = 0, _div34i = 0;
        c_div(_prod31r, _prod31i, _add33r, _add33i, &_div34r, &_div34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _sub28r + _div34r; _add35i = _sub28i + _div34i;
        double magnitude = _add35r; /* +_add35ii */
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

static void poly_516_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double rec = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2i; _attr2i = 0;
    double imc = _attr2r; /* +_attr2ii */
    double _c3r = 0, _c3i = 0;
    _c3r = 1.0; _c3i = 0;
    double _add4r = 0, _add4i = 0;
    _add4r = n + _c3r; _add4i = 0 + _c3i;
    for (int j = 1; j < (int)(_add4r); j++) {
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
        double mag_part = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 0.5; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(j, 0, 0.5, &_pow21r, &_pow21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang19r, _ang19i, _pow21r, _pow21i, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang23r, _ang23i, _log26r, _log26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul22r + _mul27r; _add28i = _mul22i + _mul27i;
        double angle_part = _add28r; /* +_add28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, angle_part, 0, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag_part, 0, _exp31r, _exp31i, &_mul32r, &_mul32i);
        double _conj33r = 0, _conj33i = 0;
        _conj33r = x1r; _conj33i = -(x1i);
        double _pow34r = 0, _pow34i = 0;
        c_powr(_conj33r, _conj33i, j, &_pow34r, &_pow34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 5.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_pow34r, _pow34i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul32r + _mul39r; _add40i = _mul32i + _mul39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_517_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int r = 1; r < (int)(_add2r); r++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _abs4r; _add5i = _abs3i + _abs4i;
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + r; _add6i = _add5i + 0;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(r, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _c10r + r; _add11i = _c10i + 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul9r, _mul9i, _add11r, _add11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log7r, _log7i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(r, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _c17r + r; _add18i = _c17i + 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul16r, _mul16i, _add18r, _add18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul14r + _cos20r; _add21i = _mul14i + _cos20i;
        double mag_part = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, r, 0, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _sub25r = 0, _sub25i = 0;
        _sub25r = n - r; _sub25i = 0 - 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _sub25r, _sub25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul23r + _mul26r; _add27i = _mul23i + _mul26i;
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x1r; _attr28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(r, 0, _attr28r, _attr28i, &_mul29r, &_mul29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_mul29r, _mul29i, &_sin30r, &_sin30i);
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x2i; _attr31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(r, 0, _attr31r, _attr31i, &_mul32r, &_mul32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_mul32r, _mul32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin30r, _sin30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add27r + _mul34r; _add35i = _add27i + _mul34i;
        double angle_part = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, angle_part, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag_part, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _neg41r = 0, _neg41i = 0;
        _neg41r = -(_c40r); _neg41i = -(_c40i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_neg41r, _neg41i, angle_part, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag_part, 0, _exp43r, _exp43i, &_mul44r, &_mul44i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = _mul44r; _conj45i = -(_mul44i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul39r + _conj45r; _add46i = _mul39i + _conj45i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_518_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(j, 0, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _sin11r + _cos12r; _add13i = _sin11i + _cos12i;
        double _pow14r = 0, _pow14i = 0;
        c_powr(j, 0, _add13r, &_pow14r, &_pow14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log10r, _log10i, _pow14r, _pow14i, &_mul15r, &_mul15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 7.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 5.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sin19r, _sin19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul15r + _mul24r; _add25i = _mul15i + _mul24i;
        double mag = _add25r; /* +_add25ii */
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
        double _c42r = 0, _c42i = 0;
        _c42r = 5.0; _c42i = 0;
        double _min43r = 0, _min43i = 0;
        _min43r = fmin(j, _c42r); _min43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = _min43r + _c44r; _add45i = _min43i + _c44i;
        for (int k = 1; k < (int)(_add45r); k++) {
            double _pow46r = 0, _pow46i = 0;
            c_powr(r1, 0, k, &_pow46r, &_pow46i);
            double _sub47r = 0, _sub47i = 0;
            _sub47r = j - k; _sub47i = 0 - 0;
            double _pow48r = 0, _pow48i = 0;
            c_powr(i2, 0, _sub47r, &_pow48r, &_pow48i);
            double _mul49r = 0, _mul49i = 0;
            c_mul(_pow46r, _pow46i, _pow48r, _pow48i, &_mul49r, &_mul49i);
            double _c50r = 0, _c50i = 0;
            _c50r = 1.0; _c50i = 0;
            double _add51r = 0, _add51i = 0;
            _add51r = k + _c50r; _add51i = 0 + _c50i;
            double _log52r = 0, _log52i = 0;
            c_log(_add51r, _add51i, &_log52r, &_log52i);
            double _mul53r = 0, _mul53i = 0;
            c_mul(_mul49r, _mul49i, _log52r, _log52i, &_mul53r, &_mul53i);
            mag += _mul53r;
            double _ang54r = 0, _ang54i = 0;
            _ang54r = c_arg(x1r, x1i); _ang54i = 0;
            double _pow55r = 0, _pow55i = 0;
            c_powr(_ang54r, _ang54i, k, &_pow55r, &_pow55i);
            double _ang56r = 0, _ang56i = 0;
            _ang56r = c_arg(x2r, x2i); _ang56i = 0;
            double _sub57r = 0, _sub57i = 0;
            _sub57r = j - k; _sub57i = 0 - 0;
            double _pow58r = 0, _pow58i = 0;
            c_powr(_ang56r, _ang56i, _sub57r, &_pow58r, &_pow58i);
            double _mul59r = 0, _mul59i = 0;
            c_mul(k, 0, M_PI, 0, &_mul59r, &_mul59i);
            double _c60r = 0, _c60i = 0;
            _c60r = 8.0; _c60i = 0;
            double _div61r = 0, _div61i = 0;
            c_div(_mul59r, _mul59i, _c60r, _c60i, &_div61r, &_div61i);
            double _cos62r = 0, _cos62i = 0;
            c_cos(_div61r, _div61i, &_cos62r, &_cos62i);
            double _mul63r = 0, _mul63i = 0;
            c_mul(_pow58r, _pow58i, _cos62r, _cos62i, &_mul63r, &_mul63i);
            double _sub64r = 0, _sub64i = 0;
            _sub64r = _pow55r - _mul63r; _sub64i = _pow55i - _mul63i;
            angle += _sub64r;
        }
        double _c65r = 0, _c65i = 0;
        _c65r = 0.0; _c65i = 1.0;
        double _mul66r = 0, _mul66i = 0;
        c_mul(_c65r, _c65i, angle, 0, &_mul66r, &_mul66i);
        double _exp67r = 0, _exp67i = 0;
        c_exp2(_mul66r, _mul66i, &_exp67r, &_exp67i);
        double _mul68r = 0, _mul68i = 0;
        c_mul(mag, 0, _exp67r, _exp67i, &_mul68r, &_mul68i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul68r; cIm[_idx] = _mul68i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_519_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 7.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_sin9r, _sin9i); _abs10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log5r, _log5i, _abs10r, _abs10i, &_mul11r, &_mul11i);
        double _sqrt12r = 0, _sqrt12i = 0;
        c_powr(j, 0, 0.5, &_sqrt12r, &_sqrt12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_sqrt12r, _sqrt12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul11r + _mul16r; _add17i = _mul11i + _mul16i;
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _sin19r = 0, _sin19i = 0;
        c_sin(j, 0, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul20r + _mul25r; _add26i = _mul20i + _mul25i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_520_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _add19r = 0, _add19i = 0;
        _add19r = _c14r + _sin18r; _add19i = _c14i + _sin18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log13r, _log13i, _add19r, _add19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 7.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _c21r + _cos25r; _add26i = _c21i + _cos25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_mul20r, _mul20i, _add26r, _add26i, &_mul27r, &_mul27i);
        double magnitude = _mul27r; /* +_mul27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 4.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(j, 0, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang33r, _ang33i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul32r + _mul37r; _add38i = _mul32i + _mul37i;
        double _attr39r = 0, _attr39i = 0;
        _attr39r = x1r; _attr39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, _attr39r, _attr39i, &_mul40r, &_mul40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_mul40r, _mul40i, &_sin41r, &_sin41i);
        double _attr42r = 0, _attr42i = 0;
        _attr42r = x2i; _attr42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(j, 0, _attr42r, _attr42i, &_mul43r, &_mul43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_mul43r, _mul43i, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_sin41r, _sin41i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _add38r + _mul45r; _add46i = _add38i + _mul45i;
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
    double _c53r = 0, _c53i = 0;
    _c53r = 1.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = n + _c53r; _add54i = 0 + _c53i;
    for (int k = 1; k < (int)(_add54r); k++) {
        double _c55r = 0, _c55i = 0;
        _c55r = 1.0; _c55i = 0;
        double _c56r = 0, _c56i = 0;
        _c56r = 0.5; _c56i = 0;
        double _c57r = 0, _c57i = 0;
        _c57r = 0.0; _c57i = 0;
        double _c58r = 0, _c58i = 0;
        _c58r = 2.0; _c58i = 0;
        double _sub59r = 0, _sub59i = 0;
        _sub59r = k - _c58r; _sub59i = 0 - _c58i;
        double _max60r = 0, _max60i = 0;
        _max60r = fmax(_c57r, _sub59r); _max60i = 0;
        double _cf61r = 0, _cf61i = 0;
        { int _idx = (int)(_max60r); if (_idx >= 0 && _idx < 36) { _cf61r = cRe[_idx]; _cf61i = cIm[_idx]; } }
        double _conj62r = 0, _conj62i = 0;
        _conj62r = _cf61r; _conj62i = -(_cf61i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(_c56r, _c56i, _conj62r, _conj62i, &_mul63r, &_mul63i);
        double _add64r = 0, _add64i = 0;
        _add64r = _c55r + _mul63r; _add64i = _c55i + _mul63i;
        double _c65r = 0, _c65i = 0;
        _c65r = 0.3; _c65i = 0;
        double _sin66r = 0, _sin66i = 0;
        c_sin(k, 0, &_sin66r, &_sin66i);
        double _mul67r = 0, _mul67i = 0;
        c_mul(_c65r, _c65i, _sin66r, _sin66i, &_mul67r, &_mul67i);
        double _cos68r = 0, _cos68i = 0;
        c_cos(k, 0, &_cos68r, &_cos68i);
        double _mul69r = 0, _mul69i = 0;
        c_mul(_mul67r, _mul67i, _cos68r, _cos68i, &_mul69r, &_mul69i);
        double _add70r = 0, _add70i = 0;
        _add70r = _add64r + _mul69r; _add70i = _add64i + _mul69i;
        { double _tr = cRe[(k - 1)]*_add70r - cIm[(k - 1)]*_add70i; cIm[(k - 1)] = cRe[(k - 1)]*_add70i + cIm[(k - 1)]*_add70r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_521_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_part1 = _log5r; /* +_log5ii */
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(j, 0, _c9r, _c9i, &_div10r, &_div10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_div10r, _div10i, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin8r + _cos13r; _add14i = _sin8i + _cos13i;
        double mag_part2 = _add14r; /* +_add14ii */
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_mul(mag_part2, 0, mag_part2, 0, &_pow17r, &_pow17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _c15r + _pow17r; _add18i = _c15i + _pow17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(mag_part1, 0, _add18r, _add18i, &_mul19r, &_mul19i);
        double magnitude = _mul19r; /* +_mul19ii */
        double _add20r = 0, _add20i = 0;
        _add20r = x1r + j; _add20i = x1i + 0;
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(_add20r, _add20i); _ang21i = 0;
        double angle_part1 = _ang21r; /* +_ang21ii */
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x2i; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _attr22r, _attr22i, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double angle_part2 = _cos24r; /* +_cos24ii */
        double _add25r = 0, _add25i = 0;
        _add25r = angle_part1 + angle_part2; _add25i = 0 + 0;
        double angle = _add25r; /* +_add25ii */
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
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_522_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double angle = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        for (int k = 1; k < (int)(_add6r); k++) {
            double _attr7r = 0, _attr7i = 0;
            _attr7r = x1r; _attr7i = 0;
            double _pow8r = 0, _pow8i = 0;
            c_powr(_attr7r, _attr7i, k, &_pow8r, &_pow8i);
            double _attr9r = 0, _attr9i = 0;
            _attr9r = x2r; _attr9i = 0;
            double _sub10r = 0, _sub10i = 0;
            _sub10r = j - k; _sub10i = 0 - 0;
            double _pow11r = 0, _pow11i = 0;
            c_powr(_attr9r, _attr9i, _sub10r, &_pow11r, &_pow11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_pow8r, _pow8i, _pow11r, _pow11i, &_mul12r, &_mul12i);
            double _attr13r = 0, _attr13i = 0;
            _attr13r = x1i; _attr13i = 0;
            double _pow14r = 0, _pow14i = 0;
            c_powr(_attr13r, _attr13i, k, &_pow14r, &_pow14i);
            double _attr15r = 0, _attr15i = 0;
            _attr15r = x2i; _attr15i = 0;
            double _sub16r = 0, _sub16i = 0;
            _sub16r = j - k; _sub16i = 0 - 0;
            double _pow17r = 0, _pow17i = 0;
            c_powr(_attr15r, _attr15i, _sub16r, &_pow17r, &_pow17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(_pow14r, _pow14i, _pow17r, _pow17i, &_mul18r, &_mul18i);
            double _add19r = 0, _add19i = 0;
            _add19r = _mul12r + _mul18r; _add19i = _mul12i + _mul18i;
            mag += _add19r;
            double _ang20r = 0, _ang20i = 0;
            _ang20r = c_arg(x1r, x1i); _ang20i = 0;
            double _sin21r = 0, _sin21i = 0;
            c_sin(k, 0, &_sin21r, &_sin21i);
            double _mul22r = 0, _mul22i = 0;
            c_mul(_ang20r, _ang20i, _sin21r, _sin21i, &_mul22r, &_mul22i);
            double _ang23r = 0, _ang23i = 0;
            _ang23r = c_arg(x2r, x2i); _ang23i = 0;
            double _sub24r = 0, _sub24i = 0;
            _sub24r = j - k; _sub24i = 0 - 0;
            double _cos25r = 0, _cos25i = 0;
            c_cos(_sub24r, _sub24i, &_cos25r, &_cos25i);
            double _mul26r = 0, _mul26i = 0;
            c_mul(_ang23r, _ang23i, _cos25r, _cos25i, &_mul26r, &_mul26i);
            double _sub27r = 0, _sub27i = 0;
            _sub27r = _mul22r - _mul26r; _sub27i = _mul22i - _mul26i;
            angle += _sub27r;
        }
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x1r, x1i); _abs28i = 0;
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x2r, x2i); _abs29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs28r + _abs29r; _add30i = _abs28i + _abs29i;
        double _add31r = 0, _add31i = 0;
        _add31r = _add30r + j; _add31i = _add30i + 0;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        mag *= _log32r;
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

static void poly_523_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 6.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double mag_part1 = _mul14r; /* +_mul14ii */
        double _pow15r = 0, _pow15i = 0;
        c_powr(r2, 0, j, &_pow15r, &_pow15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = n - j; _sub16i = 0 - 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(i1, 0, _sub16r, &_pow17r, &_pow17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _pow15r + _pow17r; _add18i = _pow15i + _pow17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 4.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_add18r, _add18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double mag_part2 = _mul23r; /* +_mul23ii */
        double _add24r = 0, _add24i = 0;
        _add24r = mag_part1 + mag_part2; _add24i = 0 + 0;
        double mag = _add24r; /* +_add24ii */
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
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul29r + _mul34r; _add35i = _mul29i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 7.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _add35r + _sin39r; _add40i = _add35i + _sin39i;
        double angle = _add40r; /* +_add40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag, 0, _exp43r, _exp43i, &_mul44r, &_mul44i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x2r; _conj45i = -(x2i);
        double _c46r = 0, _c46i = 0;
        _c46r = 4.0; _c46i = 0;
        double _mod47r = 0, _mod47i = 0;
        _mod47r = fmod(j, _c46r); _mod47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_conj45r, _conj45i, _mod47r, _mod47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul44r + _mul48r; _add49i = _mul44i + _mul48i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_524_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add9r = 0, _add9i = 0;
        _add9r = _sin5r + _cos8r; _add9i = _sin5i + _cos8i;
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
        double _add15r = 0, _add15i = 0;
        _add15r = _add9r + _log14r; _add15i = _add9i + _log14i;
        double phase = _add15r; /* +_add15ii */
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_mul(j, 0, j, 0, &_pow17r, &_pow17i);
        double _sqrt18r = 0, _sqrt18i = 0;
        c_powr(j, 0, 0.5, &_sqrt18r, &_sqrt18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _pow17r + _sqrt18r; _add19i = _pow17i + _sqrt18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(j, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_div21r, _div21i, &_sin22r, &_sin22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_sin22r, _sin22i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_add19r, _add19i, _abs23r, _abs23i, &_mul24r, &_mul24i);
        double _neg25r = 0, _neg25i = 0;
        _neg25r = -(j); _neg25i = -(0);
        double _c26r = 0, _c26i = 0;
        _c26r = 10.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_neg25r, _neg25i, _c26r, _c26i, &_div27r, &_div27i);
        double _exp28r = 0, _exp28i = 0;
        c_exp2(_div27r, _div27i, &_exp28r, &_exp28i);
        double _add29r = 0, _add29i = 0;
        _add29r = x1r + x2r; _add29i = x1i + x2i;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(_add29r, _add29i); _abs30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_exp28r, _exp28i, _abs30r, _abs30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul24r + _mul31r; _add32i = _mul24i + _mul31i;
        double magnitude = _add32r; /* +_add32ii */
        double _cos33r = 0, _cos33i = 0;
        c_cos(phase, 0, &_cos33r, &_cos33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _sin35r = 0, _sin35i = 0;
        c_sin(phase, 0, &_sin35r, &_sin35i);
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

static void poly_525_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2r; _attr7i = 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_attr7r, _attr7i); _abs8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_powr(j, 0, _abs8r, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log6r, _log6i, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double _sqrt11r = 0, _sqrt11i = 0;
        c_powr(j, 0, 0.5, &_sqrt11r, &_sqrt11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(0, 0, _sqrt11r, _sqrt11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul10r + _mul12r; _add13i = _mul10i + _mul12i;
        double mag_part = _add13r; /* +_add13ii */
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
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1i; _attr21i = 0;
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
        _add28r = _add20r + _mul27r; _add28i = _add20i + _mul27i;
        double angle_part = _add28r; /* +_add28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, angle_part, 0, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag_part, 0, _exp31r, _exp31i, &_mul32r, &_mul32i);
        double coeff = _mul32r; /* +_mul32ii */
        for (int k = 1; k < 4; k++) {
            double _attr33r = 0, _attr33i = 0;
            _attr33r = x1r; _attr33i = 0;
            double _pow34r = 0, _pow34i = 0;
            c_powr(_attr33r, _attr33i, k, &_pow34r, &_pow34i);
            double _attr35r = 0, _attr35i = 0;
            _attr35r = x2i; _attr35i = 0;
            double _pow36r = 0, _pow36i = 0;
            c_powr(_attr35r, _attr35i, k, &_pow36r, &_pow36i);
            double _mul37r = 0, _mul37i = 0;
            c_mul(_pow34r, _pow34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
            double _mul38r = 0, _mul38i = 0;
            c_mul(k, 0, j, 0, &_mul38r, &_mul38i);
            double _sin39r = 0, _sin39i = 0;
            c_sin(_mul38r, _mul38i, &_sin39r, &_sin39i);
            double _mul40r = 0, _mul40i = 0;
            c_mul(_mul37r, _mul37i, _sin39r, _sin39i, &_mul40r, &_mul40i);
            double _c41r = 0, _c41i = 0;
            _c41r = 1.0; _c41i = 0;
            double _add42r = 0, _add42i = 0;
            _add42r = k + _c41r; _add42i = 0 + _c41i;
            double _div43r = 0, _div43i = 0;
            c_div(_mul40r, _mul40i, _add42r, _add42i, &_div43r, &_div43i);
            coeff += _div43r;
        }
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x2r; _conj44i = -(x2i);
        double _attr45r = 0, _attr45i = 0;
        _attr45r = x1r; _attr45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 5.0; _c46i = 0;
        double _mod47r = 0, _mod47i = 0;
        _mod47r = fmod(j, _c46r); _mod47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 1.0; _c48i = 0;
        double _add49r = 0, _add49i = 0;
        _add49r = _mod47r + _c48r; _add49i = _mod47i + _c48i;
        double _pow50r = 0, _pow50i = 0;
        c_powr(_attr45r, _attr45i, _add49r, &_pow50r, &_pow50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_conj44r, _conj44i, _pow50r, _pow50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = coeff + _mul51r; _add52i = 0 + _mul51i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_526_c(double x1r, double x1i, double x2r, double x2i,
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
        double im = _sub10r; /* +_sub10ii */
        double _c11r = 0, _c11i = 0;
        _c11r = 0.0; _c11i = 1.0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(im, 0, _c11r, _c11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = r + _mul12r; _add13i = 0 + _mul12i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(_add13r, _add13i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _div19r = 0, _div19i = 0;
        c_div(_mul18r, _mul18i, n, 0, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log17r, _log17i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 5.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul21r + _cos25r; _add26i = _mul21i + _cos25i;
        double mag = _add26r; /* +_add26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(r, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 7.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang27r, _ang27i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(r, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 4.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(_mul34r, _mul34i, _c35r, _c35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang33r, _ang33i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul32r + _mul38r; _add39i = _mul32i + _mul38i;
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

static void poly_527_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul4r = 0, _mul4i = 0;
        c_mul(_ang3r, _ang3i, j, 0, &_mul4r, &_mul4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _div8r = 0, _div8i = 0;
        c_div(_ang5r, _ang5i, _add7r, _add7i, &_div8r, &_div8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul4r + _div8r; _add9i = _mul4i + _div8i;
        double angle = _add9r; /* +_add9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_powr(_abs10r, _abs10i, j, &_pow11r, &_pow11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = n - j; _sub13i = 0 - 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs12r, _abs12i, _sub13r, &_pow14r, &_pow14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _pow11r + _pow14r; _add15i = _pow11i + _pow14i;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _abs17r; _add18i = _abs16i + _abs17i;
        double _add19r = 0, _add19i = 0;
        _add19r = _add18r + j; _add19i = _add18i + 0;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add15r + _log20r; _add21i = _add15i + _log20i;
        double magnitude = _add21r; /* +_add21ii */
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x1r; _attr22i = 0;
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
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin24r, _sin24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1i; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_attr29r, _attr29i, j, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul28r + _sin33r; _add34i = _mul28i + _sin33i;
        double phase = _add34r; /* +_add34ii */
        double _add35r = 0, _add35i = 0;
        _add35r = angle + phase; _add35i = 0 + 0;
        double _cos36r = 0, _cos36i = 0;
        c_cos(_add35r, _add35i, &_cos36r, &_cos36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _sub38r = 0, _sub38i = 0;
        _sub38r = angle - phase; _sub38i = 0 - 0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(_sub38r, _sub38i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c37r, _c37i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _cos36r + _mul40r; _add41i = _cos36i + _mul40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(magnitude, 0, _add41r, _add41i, &_mul42r, &_mul42i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
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
        double _cf47r = 0, _cf47i = 0;
        { int _idx = ((int)(n) - k); if (_idx >= 0 && _idx < 36) { _cf47r = cRe[_idx]; _cf47i = cIm[_idx]; } }
        double _conj48r = 0, _conj48i = 0;
        _conj48r = _cf47r; _conj48i = -(_cf47i);
        { double _tr = cRe[(k - 1)]*_conj48r - cIm[(k - 1)]*_conj48i; cIm[(k - 1)] = cRe[(k - 1)]*_conj48i + cIm[(k - 1)]*_conj48r; cRe[(k - 1)] = _tr; }
    }
    double _c49r = 0, _c49i = 0;
    _c49r = 1.0; _c49i = 0;
    double _add50r = 0, _add50i = 0;
    _add50r = n + _c49r; _add50i = 0 + _c49i;
    for (int r = 1; r < (int)(_add50r); r++) {
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _attr52r = 0, _attr52i = 0;
        _attr52r = x1r; _attr52i = 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_attr52r, _attr52i, r, 0, &_mul53r, &_mul53i);
        double _attr54r = 0, _attr54i = 0;
        _attr54r = x2i; _attr54i = 0;
        double _c55r = 0, _c55i = 0;
        _c55r = 1.0; _c55i = 0;
        double _add56r = 0, _add56i = 0;
        _add56r = r + _c55r; _add56i = 0 + _c55i;
        double _div57r = 0, _div57i = 0;
        c_div(_attr54r, _attr54i, _add56r, _add56i, &_div57r, &_div57i);
        double _sub58r = 0, _sub58i = 0;
        _sub58r = _mul53r - _div57r; _sub58i = _mul53i - _div57i;
        double _mul59r = 0, _mul59i = 0;
        c_mul(_c51r, _c51i, _sub58r, _sub58i, &_mul59r, &_mul59i);
        double _exp60r = 0, _exp60i = 0;
        c_exp2(_mul59r, _mul59i, &_exp60r, &_exp60i);
        cRe[(r - 1)] += _exp60r; cIm[(r - 1)] += _exp60i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_528_c(double x1r, double x1i, double x2r, double x2i,
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
        _c3r = 1.5; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(j, 0, 1.5, &_pow4r, &_pow4i);
        double k = _pow4r; /* +_pow4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = k + _c6r; _add7i = 0 + _c6i;
        double _log8r = 0, _log8i = 0;
        c_log(_add7r, _add7i, &_log8r, &_log8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_attr5r, _attr5i, _log8r, _log8i, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2r; _attr10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = k + _c11r; _add12i = 0 + _c11i;
        double _sqrt13r = 0, _sqrt13i = 0;
        c_powr(_add12r, _add12i, 0.5, &_sqrt13r, &_sqrt13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr10r, _attr10i, _sqrt13r, _sqrt13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul9r + _mul14r; _add15i = _mul9i + _mul14i;
        double r = _add15r; /* +_add15ii */
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1i; _attr16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(k, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_attr16r, _attr16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _cos20r = 0, _cos20i = 0;
        c_cos(k, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_attr19r, _attr19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
        double im_part = _add22r; /* +_add22ii */
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x1r, x1i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_abs23r, _abs23i, k, 0, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = k + _c26r; _add27i = 0 + _c26i;
        double _div28r = 0, _div28i = 0;
        c_div(_abs25r, _abs25i, _add27r, _add27i, &_div28r, &_div28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul24r + _div28r; _add29i = _mul24i + _div28i;
        double magnitude = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 10.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(k, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 10.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(k, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang35r, _ang35i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul34r + _mul39r; _add40i = _mul34i + _mul39i;
        double angle = _add40r; /* +_add40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, im_part, 0, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = r + _mul42r; _add43i = 0 + _mul42i;
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = _c44r + magnitude; _add45i = _c44i + 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_add43r, _add43i, _add45r, _add45i, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c47r, _c47i, angle, 0, &_mul48r, &_mul48i);
        double _exp49r = 0, _exp49i = 0;
        c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_mul46r, _mul46i, _exp49r, _exp49i, &_mul50r, &_mul50i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int k = 1; k < 6; k++) {
        for (int r = 1; r < 8; r++) {
            double _c51r = 0, _c51i = 0;
            _c51r = 1.0; _c51i = 0;
            double _sub52r = 0, _sub52i = 0;
            _sub52r = k - _c51r; _sub52i = 0 - _c51i;
            double _c53r = 0, _c53i = 0;
            _c53r = 7.0; _c53i = 0;
            double _mul54r = 0, _mul54i = 0;
            c_mul(_sub52r, _sub52i, _c53r, _c53i, &_mul54r, &_mul54i);
            double _add55r = 0, _add55i = 0;
            _add55r = _mul54r + r; _add55i = _mul54i + 0;
            double index = _add55r; /* +_add55ii */
            if (index <= n) {
                double _c56r = 0, _c56i = 0;
                _c56r = 1.0; _c56i = 0;
                double _c57r = 0, _c57i = 0;
                _c57r = 0.05; _c57i = 0;
                double _mul58r = 0, _mul58i = 0;
                c_mul(k, 0, r, 0, &_mul58r, &_mul58i);
                double _sin59r = 0, _sin59i = 0;
                c_sin(_mul58r, _mul58i, &_sin59r, &_sin59i);
                double _mul60r = 0, _mul60i = 0;
                c_mul(_c57r, _c57i, _sin59r, _sin59i, &_mul60r, &_mul60i);
                double _add61r = 0, _add61i = 0;
                _add61r = _c56r + _mul60r; _add61i = _c56i + _mul60i;
                double _c62r = 0, _c62i = 0;
                _c62r = 0.02; _c62i = 0;
                double _add63r = 0, _add63i = 0;
                _add63r = k + r; _add63i = 0 + 0;
                double _cos64r = 0, _cos64i = 0;
                c_cos(_add63r, _add63i, &_cos64r, &_cos64i);
                double _mul65r = 0, _mul65i = 0;
                c_mul(_c62r, _c62i, _cos64r, _cos64i, &_mul65r, &_mul65i);
                double _add66r = 0, _add66i = 0;
                _add66r = _add61r + _mul65r; _add66i = _add61i + _mul65i;
                { double _tr = cRe[((int)(index) - 1)]*_add66r - cIm[((int)(index) - 1)]*_add66i; cIm[((int)(index) - 1)] = cRe[((int)(index) - 1)]*_add66i + cIm[((int)(index) - 1)]*_add66r; cRe[((int)(index) - 1)] = _tr; }
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_529_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _add7r = 0, _add7i = 0;
        _add7r = r1 + j; _add7i = 0 + 0;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(_add7r, _add7i); _abs8i = 0;
        double _log9r = 0, _log9i = 0;
        c_log(_abs8r, _abs8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_mul10r, _mul10i, i2, 0, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log9r, _log9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _sqrt14r = 0, _sqrt14i = 0;
        c_powr(j, 0, 0.5, &_sqrt14r, &_sqrt14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_mul15r, _mul15i, r2, 0, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sqrt14r, _sqrt14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul13r + _mul18r; _add19i = _mul13i + _mul18i;
        double mag = _add19r; /* +_add19ii */
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
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, r1, 0, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang25r, _ang25i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul24r + _mul28r; _add29i = _mul24i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, i2, 0, &_mul30r, &_mul30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _add29r + _cos31r; _add32i = _add29i + _cos31i;
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
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x1r; _conj39i = -(x1i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(j, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj39r, _conj39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
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

static void poly_530_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr5r = 0, _arr5i = 0;
        { int _idx = j; _arr5r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr5i = 0; }
        double r = _arr5r; /* +_arr5ii */
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr6i = 0; }
        double m = _arr6r; /* +_arr6ii */
        double _mul7r = 0, _mul7i = 0;
        c_mul(r, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        double _div10r = 0, _div10i = 0;
        c_div(_mul7r, _mul7i, _add9r, _add9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(m, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 3.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _div15r = 0, _div15i = 0;
        c_div(_mul12r, _mul12i, _add14r, _add14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_sin11r, _sin11i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double term1 = _mul17r; /* +_mul17ii */
        double _add18r = 0, _add18i = 0;
        _add18r = r + m; _add18i = 0 + 0;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(_add18r, _add18i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_attr23r, _attr23i, _add25r, &_pow26r, &_pow26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log22r, _log22i, _pow26r, _pow26i, &_mul27r, &_mul27i);
        double term2 = _mul27r; /* +_mul27ii */
        double _prod28r = 0, _prod28i = 0;
        c_mul(r, 0, m, 0, &_prod28r, &_prod28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = j + _c29r; _add30i = 0 + _c29i;
        double _prod31r = 0, _prod31i = 0;
        c_mul(_prod28r, _prod28i, _add30r, _add30i, &_prod31r, &_prod31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _div35r = 0, _div35i = 0;
        c_div(_c32r, _c32i, _add34r, _add34i, &_div35r, &_div35i);
        double _pow36r = 0, _pow36i = 0;
        c_powr(_prod31r, _prod31i, _div35r, &_pow36r, &_pow36i);
        double term3 = _pow36r; /* +_pow36ii */
        double _add37r = 0, _add37i = 0;
        _add37r = term1 + term2; _add37i = 0 + 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _add37r + term3; _add38i = _add37i + 0;
        double mag = _add38r; /* +_add38ii */
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(m, 0, M_PI, 0, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 4.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = j + _c41r; _add42i = 0 + _c41i;
        double _div43r = 0, _div43i = 0;
        c_div(_mul40r, _mul40i, _add42r, _add42i, &_div43r, &_div43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_div43r, _div43i, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_ang39r, _ang39i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x2r, x2i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(r, 0, M_PI, 0, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 5.0; _c48i = 0;
        double _add49r = 0, _add49i = 0;
        _add49r = j + _c48r; _add49i = 0 + _c48i;
        double _div50r = 0, _div50i = 0;
        c_div(_mul47r, _mul47i, _add49r, _add49i, &_div50r, &_div50i);
        double _cos51r = 0, _cos51i = 0;
        c_cos(_div50r, _div50i, &_cos51r, &_cos51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_ang46r, _ang46i, _cos51r, _cos51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul45r + _mul52r; _add53i = _mul45i + _mul52i;
        double _c54r = 0, _c54i = 0;
        _c54r = 2.0; _c54i = 0;
        double _add55r = 0, _add55i = 0;
        _add55r = j + _c54r; _add55i = 0 + _c54i;
        double _log56r = 0, _log56i = 0;
        c_log(_add55r, _add55i, &_log56r, &_log56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _add53r + _log56r; _add57i = _add53i + _log56i;
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
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul63r; cIm[_idx] = _mul63i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_531_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
    for (int r = 1; r < (int)(_add6r); r++) {
        double ang = 0;
        double mag = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 3.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(r, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        if (_mod8r == _c9r) {
            double _add10r = 0, _add10i = 0;
            _add10r = x1r + r; _add10i = x1i + 0;
            double _abs11r = 0, _abs11i = 0;
            _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
            double _c12r = 0, _c12i = 0;
            _c12r = 1.0; _c12i = 0;
            double _add13r = 0, _add13i = 0;
            _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
            double _log14r = 0, _log14i = 0;
            c_log(_add13r, _add13i, &_log14r, &_log14i);
            double _div15r = 0, _div15i = 0;
            c_div(r, 0, n, 0, &_div15r, &_div15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_div15r, _div15i, M_PI, 0, &_mul16r, &_mul16i);
            double _sin17r = 0, _sin17i = 0;
            c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(_log14r, _log14i, _sin17r, _sin17i, &_mul18r, &_mul18i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(r, 0, M_PI, 0, &_mul19r, &_mul19i);
            double _c20r = 0, _c20i = 0;
            _c20r = 4.0; _c20i = 0;
            double _div21r = 0, _div21i = 0;
            c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
            double _cos22r = 0, _cos22i = 0;
            c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
            double _add23r = 0, _add23i = 0;
            _add23r = _mul18r + _cos22r; _add23i = _mul18i + _cos22i;
            mag = _add23r;
            double _ang24r = 0, _ang24i = 0;
            _ang24r = c_arg(x1r, x1i); _ang24i = 0;
            double _mul25r = 0, _mul25i = 0;
            c_mul(r, 0, M_PI, 0, &_mul25r, &_mul25i);
            double _c26r = 0, _c26i = 0;
            _c26r = 6.0; _c26i = 0;
            double _div27r = 0, _div27i = 0;
            c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
            double _sin28r = 0, _sin28i = 0;
            c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
            double _ang29r = 0, _ang29i = 0;
            _ang29r = c_arg(x2r, x2i); _ang29i = 0;
            double _mul30r = 0, _mul30i = 0;
            c_mul(_sin28r, _sin28i, _ang29r, _ang29i, &_mul30r, &_mul30i);
            double _add31r = 0, _add31i = 0;
            _add31r = _ang24r + _mul30r; _add31i = _ang24i + _mul30i;
            ang = _add31r;
        } else {
            double _c32r = 0, _c32i = 0;
            _c32r = 3.0; _c32i = 0;
            double _mod33r = 0, _mod33i = 0;
            _mod33r = fmod(r, _c32r); _mod33i = 0;
            double _c34r = 0, _c34i = 0;
            _c34r = 2.0; _c34i = 0;
            if (_mod33r == _c34r) {
                double _add35r = 0, _add35i = 0;
                _add35r = x2r + r; _add35i = x2i + 0;
                double _abs36r = 0, _abs36i = 0;
                _abs36r = c_abs(_add35r, _add35i); _abs36i = 0;
                double _c37r = 0, _c37i = 0;
                _c37r = 1.0; _c37i = 0;
                double _add38r = 0, _add38i = 0;
                _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
                double _log39r = 0, _log39i = 0;
                c_log(_add38r, _add38i, &_log39r, &_log39i);
                double _div40r = 0, _div40i = 0;
                c_div(r, 0, n, 0, &_div40r, &_div40i);
                double _mul41r = 0, _mul41i = 0;
                c_mul(_div40r, _div40i, M_PI, 0, &_mul41r, &_mul41i);
                double _cos42r = 0, _cos42i = 0;
                c_cos(_mul41r, _mul41i, &_cos42r, &_cos42i);
                double _mul43r = 0, _mul43i = 0;
                c_mul(_log39r, _log39i, _cos42r, _cos42i, &_mul43r, &_mul43i);
                double _mul44r = 0, _mul44i = 0;
                c_mul(r, 0, M_PI, 0, &_mul44r, &_mul44i);
                double _c45r = 0, _c45i = 0;
                _c45r = 3.0; _c45i = 0;
                double _div46r = 0, _div46i = 0;
                c_div(_mul44r, _mul44i, _c45r, _c45i, &_div46r, &_div46i);
                double _sin47r = 0, _sin47i = 0;
                c_sin(_div46r, _div46i, &_sin47r, &_sin47i);
                double _add48r = 0, _add48i = 0;
                _add48r = _mul43r + _sin47r; _add48i = _mul43i + _sin47i;
                mag = _add48r;
                double _ang49r = 0, _ang49i = 0;
                _ang49r = c_arg(x2r, x2i); _ang49i = 0;
                double _mul50r = 0, _mul50i = 0;
                c_mul(r, 0, M_PI, 0, &_mul50r, &_mul50i);
                double _c51r = 0, _c51i = 0;
                _c51r = 5.0; _c51i = 0;
                double _div52r = 0, _div52i = 0;
                c_div(_mul50r, _mul50i, _c51r, _c51i, &_div52r, &_div52i);
                double _cos53r = 0, _cos53i = 0;
                c_cos(_div52r, _div52i, &_cos53r, &_cos53i);
                double _ang54r = 0, _ang54i = 0;
                _ang54r = c_arg(x1r, x1i); _ang54i = 0;
                double _mul55r = 0, _mul55i = 0;
                c_mul(_cos53r, _cos53i, _ang54r, _ang54i, &_mul55r, &_mul55i);
                double _add56r = 0, _add56i = 0;
                _add56r = _ang49r + _mul55r; _add56i = _ang49i + _mul55i;
                ang = _add56r;
            } else {
                double _mul57r = 0, _mul57i = 0;
                c_mul(x1r, x1i, x2r, x2i, &_mul57r, &_mul57i);
                double _add58r = 0, _add58i = 0;
                _add58r = _mul57r + r; _add58i = _mul57i + 0;
                double _abs59r = 0, _abs59i = 0;
                _abs59r = c_abs(_add58r, _add58i); _abs59i = 0;
                double _c60r = 0, _c60i = 0;
                _c60r = 1.0; _c60i = 0;
                double _add61r = 0, _add61i = 0;
                _add61r = _abs59r + _c60r; _add61i = _abs59i + _c60i;
                double _log62r = 0, _log62i = 0;
                c_log(_add61r, _add61i, &_log62r, &_log62i);
                double _c63r = 0, _c63i = 0;
                _c63r = 2.0; _c63i = 0;
                double _mul64r = 0, _mul64i = 0;
                c_mul(_c63r, _c63i, n, 0, &_mul64r, &_mul64i);
                double _div65r = 0, _div65i = 0;
                c_div(r, 0, _mul64r, _mul64i, &_div65r, &_div65i);
                double _mul66r = 0, _mul66i = 0;
                c_mul(_div65r, _div65i, M_PI, 0, &_mul66r, &_mul66i);
                double _sin67r = 0, _sin67i = 0;
                c_sin(_mul66r, _mul66i, &_sin67r, &_sin67i);
                double _mul68r = 0, _mul68i = 0;
                c_mul(_log62r, _log62i, _sin67r, _sin67i, &_mul68r, &_mul68i);
                double _mul69r = 0, _mul69i = 0;
                c_mul(r, 0, M_PI, 0, &_mul69r, &_mul69i);
                double _c70r = 0, _c70i = 0;
                _c70r = 2.0; _c70i = 0;
                double _div71r = 0, _div71i = 0;
                c_div(_mul69r, _mul69i, _c70r, _c70i, &_div71r, &_div71i);
                double _cos72r = 0, _cos72i = 0;
                c_cos(_div71r, _div71i, &_cos72r, &_cos72i);
                double _add73r = 0, _add73i = 0;
                _add73r = _mul68r + _cos72r; _add73i = _mul68i + _cos72i;
                mag = _add73r;
                double _mul74r = 0, _mul74i = 0;
                c_mul(x1r, x1i, x2r, x2i, &_mul74r, &_mul74i);
                double _ang75r = 0, _ang75i = 0;
                _ang75r = c_arg(_mul74r, _mul74i); _ang75i = 0;
                double _mul76r = 0, _mul76i = 0;
                c_mul(r, 0, M_PI, 0, &_mul76r, &_mul76i);
                double _c77r = 0, _c77i = 0;
                _c77r = 4.0; _c77i = 0;
                double _div78r = 0, _div78i = 0;
                c_div(_mul76r, _mul76i, _c77r, _c77i, &_div78r, &_div78i);
                double _sin79r = 0, _sin79i = 0;
                c_sin(_div78r, _div78i, &_sin79r, &_sin79i);
                double _mul80r = 0, _mul80i = 0;
                c_mul(r, 0, M_PI, 0, &_mul80r, &_mul80i);
                double _c81r = 0, _c81i = 0;
                _c81r = 3.0; _c81i = 0;
                double _div82r = 0, _div82i = 0;
                c_div(_mul80r, _mul80i, _c81r, _c81i, &_div82r, &_div82i);
                double _cos83r = 0, _cos83i = 0;
                c_cos(_div82r, _div82i, &_cos83r, &_cos83i);
                double _mul84r = 0, _mul84i = 0;
                c_mul(_sin79r, _sin79i, _cos83r, _cos83i, &_mul84r, &_mul84i);
                double _add85r = 0, _add85i = 0;
                _add85r = _ang75r + _mul84r; _add85i = _ang75i + _mul84i;
                ang = _add85r;
            }
        }
        double _c86r = 0, _c86i = 0;
        _c86r = 0.0; _c86i = 1.0;
        double _mul87r = 0, _mul87i = 0;
        c_mul(_c86r, _c86i, ang, 0, &_mul87r, &_mul87i);
        double _exp88r = 0, _exp88i = 0;
        c_exp2(_mul87r, _mul87i, &_exp88r, &_exp88i);
        double _mul89r = 0, _mul89i = 0;
        c_mul(mag, 0, _exp88r, _exp88i, &_mul89r, &_mul89i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul89r; cIm[_idx] = _mul89i; } }
    }
    for (int k = 0; k < (int)(n); k++) {
        double _c90r = 0, _c90i = 0;
        _c90r = 3.0; _c90i = 0;
        double _div91r = 0, _div91i = 0;
        c_div(n, 0, _c90r, _c90i, &_div91r, &_div91i);
        if (k < _div91r) {
            double _cf92r = 0, _cf92i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf92r = cRe[_idx]; _cf92i = cIm[_idx]; } }
            double _c93r = 0, _c93i = 0;
            _c93r = 1.0; _c93i = 0;
            double _add94r = 0, _add94i = 0;
            _add94r = k + _c93r; _add94i = 0 + _c93i;
            double _mul95r = 0, _mul95i = 0;
            c_mul(_cf92r, _cf92i, _add94r, _add94i, &_mul95r, &_mul95i);
            { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul95r; cIm[_idx] = _mul95i; } }
        } else {
            double _c96r = 0, _c96i = 0;
            _c96r = 2.0; _c96i = 0;
            double _mul97r = 0, _mul97i = 0;
            c_mul(_c96r, _c96i, n, 0, &_mul97r, &_mul97i);
            double _c98r = 0, _c98i = 0;
            _c98r = 3.0; _c98i = 0;
            double _div99r = 0, _div99i = 0;
            c_div(_mul97r, _mul97i, _c98r, _c98i, &_div99r, &_div99i);
            if (k < _div99r) {
                double _cf100r = 0, _cf100i = 0;
                { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf100r = cRe[_idx]; _cf100i = cIm[_idx]; } }
                double _c101r = 0, _c101i = 0;
                _c101r = 1.0; _c101i = 0;
                double _add102r = 0, _add102i = 0;
                _add102r = k + _c101r; _add102i = 0 + _c101i;
                double _neg103r = 0, _neg103i = 0;
                _neg103r = -(_add102r); _neg103i = -(_add102i);
                double _mul104r = 0, _mul104i = 0;
                c_mul(_cf100r, _cf100i, _neg103r, _neg103i, &_mul104r, &_mul104i);
                { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul104r; cIm[_idx] = _mul104i; } }
            } else {
                double _cf105r = 0, _cf105i = 0;
                { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf105r = cRe[_idx]; _cf105i = cIm[_idx]; } }
                double _c106r = 0, _c106i = 0;
                _c106r = 1.0; _c106i = 0;
                double _c107r = 0, _c107i = 0;
                _c107r = 1.0; _c107i = 0;
                double _add108r = 0, _add108i = 0;
                _add108r = k + _c107r; _add108i = 0 + _c107i;
                double _div109r = 0, _div109i = 0;
                c_div(_c106r, _c106i, _add108r, _add108i, &_div109r, &_div109i);
                double _mul110r = 0, _mul110i = 0;
                c_mul(_cf105r, _cf105i, _div109r, _div109i, &_mul110r, &_mul110i);
                { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul110r; cIm[_idx] = _mul110i; } }
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_532_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int r = 1; r < (int)(_add2r); r++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 7.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(r, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double j = _add6r; /* +_add6ii */
        double _c7r = 0, _c7i = 0;
        _c7r = 5.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(r, 0, _c7r, _c7i, &_div8r, &_div8i);
        double _flr9r = 0, _flr9i = 0;
        _flr9r = floor(_div8r); _flr9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _flr9r + _c10r; _add11i = _flr9i + _c10i;
        double k = _add11r; /* +_add11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(r, 0, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log15r, _log15i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(r, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log21r, _log21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul17r + _mul23r; _add24i = _mul17i + _mul23i;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 10.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(r, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _c25r + _div27r; _add28i = _c25i + _div27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_add24r, _add24i, _add28r, _add28i, &_mul29r, &_mul29i);
        double magnitude = _mul29r; /* +_mul29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(r, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(r, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang35r, _ang35i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _sub40r = 0, _sub40i = 0;
        _sub40r = _mul34r - _mul39r; _sub40i = _mul34i - _mul39i;
        double _sin41r = 0, _sin41i = 0;
        c_sin(r, 0, &_sin41r, &_sin41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 4.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(r, 0, _c42r, _c42i, &_div43r, &_div43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_div43r, _div43i, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_sin41r, _sin41i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _sub40r + _mul45r; _add46i = _sub40i + _mul45i;
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
        c_mul(magnitude, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_533_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(k, 0, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr8r, _attr8i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _mul10r; _add11i = _mul7i + _mul10i;
        double r = _add11r; /* +_add11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1i; _attr12i = 0;
        double _cos13r = 0, _cos13i = 0;
        c_cos(k, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr12r, _attr12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x2i; _attr15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = k + _c16r; _add17i = 0 + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_attr15r, _attr15i, _log18r, _log18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul14r + _mul19r; _add20i = _mul14i + _mul19i;
        double im = _add20r; /* +_add20ii */
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(k, 0, k, 0, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _abs21r + _pow23r; _add24i = _abs21i + _pow23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(k, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _c26r + _sin29r; _add30i = _c26i + _sin29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log25r, _log25i, _add30r, _add30i, &_mul31r, &_mul31i);
        double mag = _mul31r; /* +_mul31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 4.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(k, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 5.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(k, 0, _c38r, _c38i, &_div39r, &_div39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang37r, _ang37i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul36r + _mul41r; _add42i = _mul36i + _mul41i;
        double ang = _add42r; /* +_add42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_c43r, _c43i, im, 0, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = r + _mul44r; _add45i = 0 + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_add45r, _add45i, mag, 0, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c47r, _c47i, ang, 0, &_mul48r, &_mul48i);
        double _exp49r = 0, _exp49i = 0;
        c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_mul46r, _mul46i, _exp49r, _exp49i, &_mul50r, &_mul50i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_534_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double r1 = _attr3r; /* +_attr3ii */
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x2r; _attr4i = 0;
        double r2 = _attr4r; /* +_attr4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1i; _attr5i = 0;
        double im1 = _attr5r; /* +_attr5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double im2 = _attr6r; /* +_attr6ii */
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
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, r1, 0, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c12r + _sin14r; _add15i = _c12i + _sin14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, im2, 0, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _add15r + _cos17r; _add18i = _add15i + _cos17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log11r, _log11i, _add18r, _add18i, &_mul19r, &_mul19i);
        double mag_part1 = _mul19r; /* +_mul19ii */
        double _c20r = 0, _c20i = 0;
        _c20r = 1.3; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(j, 0, 1.3, &_pow21r, &_pow21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(r2, 0, _pow21r, _pow21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _sqrt24r = 0, _sqrt24i = 0;
        c_powr(j, 0, 0.5, &_sqrt24r, &_sqrt24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(im1, 0, _sqrt24r, _sqrt24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_sin23r, _sin23i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double mag_part2 = _mul27r; /* +_mul27ii */
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = j + _c29r; _add30i = 0 + _c29i;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul28r + _log31r; _add32i = _mul28i + _log31i;
        double magnitude = _add32r; /* +_add32ii */
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
        c_mul(magnitude, 0, _add48r, _add48i, &_mul49r, &_mul49i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_535_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
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
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 5.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log15r, _log15i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = j + _c22r; _add23i = 0 + _c22i;
        double _div24r = 0, _div24i = 0;
        c_div(_mul21r, _mul21i, _add23r, _add23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul20r + _cos25r; _add26i = _mul20i + _cos25i;
        double mag = _add26r; /* +_add26ii */
        double _arr27r = 0, _arr27i = 0;
        { int _idx = (j - 1); _arr27r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr27i = 0; }
        double _arr28r = 0, _arr28i = 0;
        { int _idx = (j - 1); _arr28r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr28i = 0; }
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_arr28r, _arr28i, _c29r, _c29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _arr27r + _mul30r; _add31i = _arr27i + _mul30i;
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(_add31r, _add31i); _ang32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, n, 0, &_div33r, &_div33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_div33r, _div33i, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 4.0; _c35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_mul34r, _mul34i, _c35r, _c35i, &_mul36r, &_mul36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _ang32r + _sin37r; _add38i = _ang32i + _sin37i;
        double _div39r = 0, _div39i = 0;
        c_div(j, 0, n, 0, &_div39r, &_div39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_div39r, _div39i, M_PI, 0, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 3.0; _c41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_mul40r, _mul40i, _c41r, _c41i, &_mul42r, &_mul42i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(_mul42r, _mul42i, &_cos43r, &_cos43i);
        double _sub44r = 0, _sub44i = 0;
        _sub44r = _add38r - _cos43r; _sub44i = _add38i - _cos43i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_536_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 3.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(j, 0, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr8r, _attr8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _mul12r; _add13i = _mul7i + _mul12i;
        double r = _add13r; /* +_add13ii */
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 4.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(j, 0, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_attr14r, _attr14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr19r, _attr19i, _log22r, _log22i, &_mul23r, &_mul23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _mul18r - _mul23r; _sub24i = _mul18i - _mul23i;
        double im = _sub24r; /* +_sub24ii */
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.2; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(j, 0, 1.2, &_pow27r, &_pow27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _abs25r + _pow27r; _add28i = _abs25i + _pow27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.5; _c31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 6.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c31r, _c31i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _c30r + _mul36r; _add37i = _c30i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_log29r, _log29i, _add37r, _add37i, &_mul38r, &_mul38i);
        double magnitude = _mul38r; /* +_mul38ii */
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 5.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(j, 0, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang39r, _ang39i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 7.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(j, 0, _c45r, _c45i, &_div46r, &_div46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(_div46r, _div46i, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang44r, _ang44i, _sin47r, _sin47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul43r + _mul48r; _add49i = _mul43i + _mul48i;
        double _abs50r = 0, _abs50i = 0;
        _abs50r = c_abs(x2r, x2i); _abs50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 1.0; _c51i = 0;
        double _add52r = 0, _add52i = 0;
        _add52r = _abs50r + _c51r; _add52i = _abs50i + _c51i;
        double _log53r = 0, _log53i = 0;
        c_log(_add52r, _add52i, &_log53r, &_log53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _add49r + _log53r; _add54i = _add49i + _log53i;
        double angle = _add54r; /* +_add54ii */
        double _cos55r = 0, _cos55i = 0;
        c_cos(angle, 0, &_cos55r, &_cos55i);
        double _sin56r = 0, _sin56i = 0;
        c_sin(angle, 0, &_sin56r, &_sin56i);
        double _c57r = 0, _c57i = 0;
        _c57r = 0.0; _c57i = 1.0;
        double _mul58r = 0, _mul58i = 0;
        c_mul(_sin56r, _sin56i, _c57r, _c57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _cos55r + _mul58r; _add59i = _cos55i + _mul58i;
        double _mul60r = 0, _mul60i = 0;
        c_mul(magnitude, 0, _add59r, _add59i, &_mul60r, &_mul60i);
        double _add61r = 0, _add61i = 0;
        _add61r = r + im; _add61i = 0 + 0;
        double _c62r = 0, _c62i = 0;
        _c62r = 0.0; _c62i = 1.0;
        double _mul63r = 0, _mul63i = 0;
        c_mul(_add61r, _add61i, _c62r, _c62i, &_mul63r, &_mul63i);
        double _add64r = 0, _add64i = 0;
        _add64r = _mul60r + _mul63r; _add64i = _mul60i + _mul63i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_537_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (k - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr8i = 0; }
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
        double _mul14r = 0, _mul14i = 0;
        c_mul(k, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 7.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_div16r, _div16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(k, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _sin17r + _cos21r; _add22i = _sin17i + _cos21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log13r, _log13i, _add22r, _add22i, &_mul23r, &_mul23i);
        double magnitude = _mul23r; /* +_mul23ii */
        double _mul24r = 0, _mul24i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(_mul24r, _mul24i); _ang25i = 0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(k, 0, &_sin26r, &_sin26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _ang25r + _sin26r; _add27i = _ang25i + _sin26i;
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(k, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = _add27r - _cos30r; _sub31i = _add27i - _cos30i;
        double angle = _sub31r; /* +_sub31ii */
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_538_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
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
        double _sqrt17r = 0, _sqrt17i = 0;
        c_powr(j, 0, 0.5, &_sqrt17r, &_sqrt17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sqrt17r, _sqrt17i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul16r + _mul22r; _add23i = _mul16i + _mul22i;
        double magnitude = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(j, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
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
        double _arr33r = 0, _arr33i = 0;
        { int _idx = (j - 1); _arr33r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr33i = 0; }
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _arr33r, _arr33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add32r + _sin35r; _add36i = _add32i + _sin35i;
        double _arr37r = 0, _arr37i = 0;
        { int _idx = (j - 1); _arr37r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr37i = 0; }
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, _arr37r, _arr37i, &_mul38r, &_mul38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_mul38r, _mul38i, &_cos39r, &_cos39i);
        double _sub40r = 0, _sub40i = 0;
        _sub40r = _add36r - _cos39r; _sub40i = _add36i - _cos39i;
        double angle = _sub40r; /* +_sub40ii */
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
        c_mul(magnitude, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_539_c(double x1r, double x1i, double x2r, double x2i,
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
        _c3r = 2.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(j, 0, j, 0, &_pow4r, &_pow4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _flr6r = 0, _flr6i = 0;
        _flr6r = floor(_attr5r); _flr6i = 0;
        double _int7r = 0, _int7i = 0;
        _int7r = (int)(_flr6r); _int7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _pow4r + _int7r; _add8i = _pow4i + _int7i;
        double k = _add8r; /* +_add8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs9r + j; _add10i = _abs9i + 0;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c12r + _sin15r; _add16i = _c12i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log11r, _log11i, _add16r, _add16i, &_mul17r, &_mul17i);
        double r = _mul17r; /* +_mul17ii */
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x2r; _attr18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, _attr18r, _attr18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1i; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_cos20r, _cos20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul24r + _cos27r; _add28i = _mul24i + _cos27i;
        double s = _add28r; /* +_add28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x2r, x2i); _abs29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs29r + j; _add30i = _abs29i + 0;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _add32r = 0, _add32i = 0;
        _add32r = r + _log31r; _add32i = 0 + _log31i;
        double magnitude = _add32r; /* +_add32ii */
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x1r; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x1r, x1i); _ang36i = 0;
        double _cos37r = 0, _cos37i = 0;
        c_cos(_ang36r, _ang36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_sin35r, _sin35i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = s + _mul38r; _add39i = 0 + _mul38i;
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
        c_mul(magnitude, 0, _add44r, _add44i, &_mul45r, &_mul45i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_540_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
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
        double angle = 0;
        double mag = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 10.0; _c5i = 0;
        if (j <= _c5r) {
            double _abs6r = 0, _abs6i = 0;
            _abs6r = c_abs(x1r, x1i); _abs6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 1.0; _c7i = 0;
            double _add8r = 0, _add8i = 0;
            _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
            double _log9r = 0, _log9i = 0;
            c_log(_add8r, _add8i, &_log9r, &_log9i);
            double _mul10r = 0, _mul10i = 0;
            c_mul(j, 0, r2, 0, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_log9r, _log9i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            double _abs13r = 0, _abs13i = 0;
            _abs13r = c_abs(x2r, x2i); _abs13i = 0;
            double _pow14r = 0, _pow14i = 0;
            c_powr(_abs13r, _abs13i, j, &_pow14r, &_pow14i);
            double _c15r = 0, _c15i = 0;
            _c15r = 1.0; _c15i = 0;
            double _add16r = 0, _add16i = 0;
            _add16r = j + _c15r; _add16i = 0 + _c15i;
            double _div17r = 0, _div17i = 0;
            c_div(_pow14r, _pow14i, _add16r, _add16i, &_div17r, &_div17i);
            double _add18r = 0, _add18i = 0;
            _add18r = _mul12r + _div17r; _add18i = _mul12i + _div17i;
            mag = _add18r;
            double _ang19r = 0, _ang19i = 0;
            _ang19r = c_arg(x1r, x1i); _ang19i = 0;
            double _ang20r = 0, _ang20i = 0;
            _ang20r = c_arg(x2r, x2i); _ang20i = 0;
            double _mul21r = 0, _mul21i = 0;
            c_mul(j, 0, _ang20r, _ang20i, &_mul21r, &_mul21i);
            double _add22r = 0, _add22i = 0;
            _add22r = _ang19r + _mul21r; _add22i = _ang19i + _mul21i;
            angle = _add22r;
        } else {
            double _c23r = 0, _c23i = 0;
            _c23r = 25.0; _c23i = 0;
            if (j <= _c23r) {
                double _mul24r = 0, _mul24i = 0;
                c_mul(j, 0, i1, 0, &_mul24r, &_mul24i);
                double _cos25r = 0, _cos25i = 0;
                c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
                double _abs26r = 0, _abs26i = 0;
                _abs26r = c_abs(x2r, x2i); _abs26i = 0;
                double _c27r = 0, _c27i = 0;
                _c27r = 1.0; _c27i = 0;
                double _add28r = 0, _add28i = 0;
                _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
                double _log29r = 0, _log29i = 0;
                c_log(_add28r, _add28i, &_log29r, &_log29i);
                double _mul30r = 0, _mul30i = 0;
                c_mul(_cos25r, _cos25i, _log29r, _log29i, &_mul30r, &_mul30i);
                double _conj31r = 0, _conj31i = 0;
                _conj31r = x1r; _conj31i = -(x1i);
                double _re32r = 0, _re32i = 0;
                _re32r = _conj31r; _re32i = 0;
                double _c33r = 0, _c33i = 0;
                _c33r = 2.0; _c33i = 0;
                double _add34r = 0, _add34i = 0;
                _add34r = j + _c33r; _add34i = 0 + _c33i;
                double _div35r = 0, _div35i = 0;
                c_div(_re32r, _re32i, _add34r, _add34i, &_div35r, &_div35i);
                double _add36r = 0, _add36i = 0;
                _add36r = _mul30r + _div35r; _add36i = _mul30i + _div35i;
                mag = _add36r;
                double _ang37r = 0, _ang37i = 0;
                _ang37r = c_arg(x1r, x1i); _ang37i = 0;
                double _mul38r = 0, _mul38i = 0;
                c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
                double _c39r = 0, _c39i = 0;
                _c39r = 5.0; _c39i = 0;
                double _div40r = 0, _div40i = 0;
                c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
                double _sin41r = 0, _sin41i = 0;
                c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
                double _mul42r = 0, _mul42i = 0;
                c_mul(_ang37r, _ang37i, _sin41r, _sin41i, &_mul42r, &_mul42i);
                double _ang43r = 0, _ang43i = 0;
                _ang43r = c_arg(x2r, x2i); _ang43i = 0;
                double _mul44r = 0, _mul44i = 0;
                c_mul(j, 0, M_PI, 0, &_mul44r, &_mul44i);
                double _c45r = 0, _c45i = 0;
                _c45r = 7.0; _c45i = 0;
                double _div46r = 0, _div46i = 0;
                c_div(_mul44r, _mul44i, _c45r, _c45i, &_div46r, &_div46i);
                double _cos47r = 0, _cos47i = 0;
                c_cos(_div46r, _div46i, &_cos47r, &_cos47i);
                double _mul48r = 0, _mul48i = 0;
                c_mul(_ang43r, _ang43i, _cos47r, _cos47i, &_mul48r, &_mul48i);
                double _add49r = 0, _add49i = 0;
                _add49r = _mul42r + _mul48r; _add49i = _mul42i + _mul48i;
                angle = _add49r;
            } else {
                double _mul50r = 0, _mul50i = 0;
                c_mul(j, 0, r1, 0, &_mul50r, &_mul50i);
                double _mul51r = 0, _mul51i = 0;
                c_mul(j, 0, i2, 0, &_mul51r, &_mul51i);
                double _cos52r = 0, _cos52i = 0;
                c_cos(_mul51r, _mul51i, &_cos52r, &_cos52i);
                double _add53r = 0, _add53i = 0;
                _add53r = _mul50r + _cos52r; _add53i = _mul50i + _cos52i;
                double _sin54r = 0, _sin54i = 0;
                c_sin(_add53r, _add53i, &_sin54r, &_sin54i);
                double _abs55r = 0, _abs55i = 0;
                _abs55r = c_abs(x1r, x1i); _abs55i = 0;
                double _abs56r = 0, _abs56i = 0;
                _abs56r = c_abs(x2r, x2i); _abs56i = 0;
                double _add57r = 0, _add57i = 0;
                _add57r = _abs55r + _abs56r; _add57i = _abs55i + _abs56i;
                double _c58r = 0, _c58i = 0;
                _c58r = 1.0; _c58i = 0;
                double _add59r = 0, _add59i = 0;
                _add59r = _add57r + _c58r; _add59i = _add57i + _c58i;
                double _log60r = 0, _log60i = 0;
                c_log(_add59r, _add59i, &_log60r, &_log60i);
                double _mul61r = 0, _mul61i = 0;
                c_mul(_sin54r, _sin54i, _log60r, _log60i, &_mul61r, &_mul61i);
                mag = _mul61r;
                double _ang62r = 0, _ang62i = 0;
                _ang62r = c_arg(x1r, x1i); _ang62i = 0;
                double _c63r = 0, _c63i = 0;
                _c63r = 2.0; _c63i = 0;
                double _pow64r = 0, _pow64i = 0;
                c_mul(_ang62r, _ang62i, _ang62r, _ang62i, &_pow64r, &_pow64i);
                double _c65r = 0, _c65i = 0;
                _c65r = 3.0; _c65i = 0;
                double _add66r = 0, _add66i = 0;
                _add66r = j + _c65r; _add66i = 0 + _c65i;
                double _div67r = 0, _div67i = 0;
                c_div(_pow64r, _pow64i, _add66r, _add66i, &_div67r, &_div67i);
                double _ang68r = 0, _ang68i = 0;
                _ang68r = c_arg(x2r, x2i); _ang68i = 0;
                double _c69r = 0, _c69i = 0;
                _c69r = 2.0; _c69i = 0;
                double _pow70r = 0, _pow70i = 0;
                c_mul(_ang68r, _ang68i, _ang68r, _ang68i, &_pow70r, &_pow70i);
                double _c71r = 0, _c71i = 0;
                _c71r = 4.0; _c71i = 0;
                double _add72r = 0, _add72i = 0;
                _add72r = j + _c71r; _add72i = 0 + _c71i;
                double _div73r = 0, _div73i = 0;
                c_div(_pow70r, _pow70i, _add72r, _add72i, &_div73r, &_div73i);
                double _add74r = 0, _add74i = 0;
                _add74r = _div67r + _div73r; _add74i = _div67i + _div73i;
                angle = _add74r;
            }
        }
        double _cos75r = 0, _cos75i = 0;
        c_cos(angle, 0, &_cos75r, &_cos75i);
        double _c76r = 0, _c76i = 0;
        _c76r = 0.0; _c76i = 1.0;
        double _sin77r = 0, _sin77i = 0;
        c_sin(angle, 0, &_sin77r, &_sin77i);
        double _mul78r = 0, _mul78i = 0;
        c_mul(_c76r, _c76i, _sin77r, _sin77i, &_mul78r, &_mul78i);
        double _add79r = 0, _add79i = 0;
        _add79r = _cos75r + _mul78r; _add79i = _cos75i + _mul78i;
        double _mul80r = 0, _mul80i = 0;
        c_mul(mag, 0, _add79r, _add79i, &_mul80r, &_mul80i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul80r; cIm[_idx] = _mul80i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_541_c(double x1r, double x1i, double x2r, double x2i,
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
        _c6r = 0.3; _c6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c6r, _c6i, j, 0, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_mul7r, _mul7i, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log5r, _log5i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double mag_part1 = _mul11r; /* +_mul11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs12r + j; _add13i = _abs12i + 0;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 0.2; _c15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c15r, _c15i, j, 0, &_mul16r, &_mul16i);
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x1i; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul16r, _mul16i, _attr17r, _attr17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log14r, _log14i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double mag_part2 = _mul20r; /* +_mul20ii */
        double _add21r = 0, _add21i = 0;
        _add21r = mag_part1 + mag_part2; _add21i = 0 + 0;
        double mag = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.1; _c23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _c23r, _c23i, &_mul24r, &_mul24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul24r, _mul24i, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 5.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_mul25r, _mul25i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _ang22r + _mul29r; _add30i = _ang22i + _mul29i;
        double angle_part1 = _add30r; /* +_add30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.1; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, _c32r, _c32i, &_mul33r, &_mul33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_mul33r, _mul33i, M_PI, 0, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 3.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, _c35r, _c35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_mul34r, _mul34i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _ang31r + _mul38r; _add39i = _ang31i + _mul38i;
        double angle_part2 = _add39r; /* +_add39ii */
        double _add40r = 0, _add40i = 0;
        _add40r = angle_part1 + angle_part2; _add40i = 0 + 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_542_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 0;
        double mag_sum = _c7r; /* +_c7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        for (int k = 1; k < (int)(_add9r); k++) {
            double _attr10r = 0, _attr10i = 0;
            _attr10r = x1r; _attr10i = 0;
            double _mul11r = 0, _mul11i = 0;
            c_mul(k, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
            double _sin12r = 0, _sin12i = 0;
            c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
            double _attr13r = 0, _attr13i = 0;
            _attr13r = x2i; _attr13i = 0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(k, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
            double _cos15r = 0, _cos15i = 0;
            c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_sin12r, _sin12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
            mag_sum += _mul16r;
        }
        double _arr17r = 0, _arr17i = 0;
        { int _idx = (j - 1); _arr17r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr17i = 0; }
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr18i = 0; }
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
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log23r, _log23i, mag_sum, 0, &_mul24r, &_mul24i);
        double magnitude = _mul24r; /* +_mul24ii */
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 0;
        double angle = _c25r; /* +_c25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = j + _c26r; _add27i = 0 + _c26i;
        for (int r = 1; r < (int)(_add27r); r++) {
            double _ang28r = 0, _ang28i = 0;
            _ang28r = c_arg(x1r, x1i); _ang28i = 0;
            double _sin29r = 0, _sin29i = 0;
            c_sin(r, 0, &_sin29r, &_sin29i);
            double _mul30r = 0, _mul30i = 0;
            c_mul(_ang28r, _ang28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
            double _ang31r = 0, _ang31i = 0;
            _ang31r = c_arg(x2r, x2i); _ang31i = 0;
            double _cos32r = 0, _cos32i = 0;
            c_cos(r, 0, &_cos32r, &_cos32i);
            double _mul33r = 0, _mul33i = 0;
            c_mul(_ang31r, _ang31i, _cos32r, _cos32i, &_mul33r, &_mul33i);
            double _add34r = 0, _add34i = 0;
            _add34r = _mul30r + _mul33r; _add34i = _mul30i + _mul33i;
            angle += _add34r;
        }
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
        c_mul(magnitude, 0, _add39r, _add39i, &_mul40r, &_mul40i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_543_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs3r, _abs3i, j, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double mag_part1 = _log7r; /* +_log7ii */
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _sqrt9r = 0, _sqrt9i = 0;
        c_powr(j, 0, 0.5, &_sqrt9r, &_sqrt9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _sqrt9r; _add10i = _abs8i + _sqrt9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double mag_part2 = _log11r; /* +_log11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1r; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(mag_part1, 0, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2i; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(mag_part2, 0, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul15r + _mul19r; _add20i = _mul15i + _mul19i;
        double mag_variation = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.3; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(j, 0, 1.3, &_pow23r, &_pow23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang21r, _ang21i, _pow23r, _pow23i, &_mul24r, &_mul24i);
        double angle_part1 = _mul24r; /* +_mul24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = j + _c26r; _add27i = 0 + _c26i;
        double _div28r = 0, _div28i = 0;
        c_div(_ang25r, _ang25i, _add27r, _add27i, &_div28r, &_div28i);
        double angle_part2 = _div28r; /* +_div28ii */
        double _sub29r = 0, _sub29i = 0;
        _sub29r = angle_part1 - angle_part2; _sub29i = 0 - 0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(j, 0, &_sin30r, &_sin30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin30r, _sin30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _sub29r + _mul34r; _add35i = _sub29i + _mul34i;
        double angle_variation = _add35r; /* +_add35ii */
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(mag_variation, 0); _abs36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, angle_variation, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_add38r, _add38i, _exp41r, _exp41i, &_mul42r, &_mul42i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_544_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _abs6r + j; _add7i = _abs6i + 0;
        double _sqrt8r = 0, _sqrt8i = 0;
        c_powr(_add7r, _add7i, 0.5, &_sqrt8r, &_sqrt8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sqrt8r, _sqrt8i, &_mul9r, &_mul9i);
        double mag_part1 = _mul9r; /* +_mul9ii */
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
        double mag_part2 = _mul16r; /* +_mul16ii */
        double _c17r = 0, _c17i = 0;
        _c17r = 5.0; _c17i = 0;
        double _mod18r = 0, _mod18i = 0;
        _mod18r = fmod(j, _c17r); _mod18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _mod18r + _c19r; _add20i = _mod18i + _c19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(mag_part2, 0, _add20r, _add20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = mag_part1 + _mul21r; _add22i = 0 + _mul21i;
        double magnitude = _add22r; /* +_add22ii */
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
        double angle_part1 = _mul27r; /* +_mul27ii */
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
        double angle_part2 = _mul32r; /* +_mul32ii */
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(x1r, x1i); _abs33i = 0;
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(x2r, x2i); _abs34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _abs33r + _abs34r; _add35i = _abs33i + _abs34i;
        double _add36r = 0, _add36i = 0;
        _add36r = _add35r + j; _add36i = _add35i + 0;
        double _log37r = 0, _log37i = 0;
        c_log(_add36r, _add36i, &_log37r, &_log37i);
        double angle_part3 = _log37r; /* +_log37ii */
        double _add38r = 0, _add38i = 0;
        _add38r = angle_part1 + angle_part2; _add38i = 0 + 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _add38r + angle_part3; _add39i = _add38i + 0;
        double angle = _add39r; /* +_add39ii */
        double _cos40r = 0, _cos40i = 0;
        c_cos(angle, 0, &_cos40r, &_cos40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(angle, 0, &_sin41r, &_sin41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_sin41r, _sin41i, _c42r, _c42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _cos40r + _mul43r; _add44i = _cos40i + _mul43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(magnitude, 0, _add44r, _add44i, &_mul45r, &_mul45i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    double _c46r = 0, _c46i = 0;
    _c46r = 1.0; _c46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = n + _c46r; _add47i = 0 + _c46i;
    for (int k = 1; k < (int)(_add47r); k++) {
        double _cf48r = 0, _cf48i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
        double _conj49r = 0, _conj49i = 0;
        _conj49r = x1r; _conj49i = -(x1i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_cf48r, _cf48i, _conj49r, _conj49i, &_mul50r, &_mul50i);
        double _abs51r = 0, _abs51i = 0;
        _abs51r = c_abs(x2r, x2i); _abs51i = 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 1.0; _c52i = 0;
        double _add53r = 0, _add53i = 0;
        _add53r = _abs51r + _c52r; _add53i = _abs51i + _c52i;
        double _div54r = 0, _div54i = 0;
        c_div(_mul50r, _mul50i, _add53r, _add53i, &_div54r, &_div54i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div54r; cIm[_idx] = _div54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_545_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr7i = 0; }
        double _c8r = 0, _c8i = 0;
        _c8r = 3.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(_arr7r, _arr7i, _arr7r, _arr7i, &_pow9r, &_pow9i);
        c_mul(_pow9r, _pow9i, _arr7r, _arr7i, &_pow9r, &_pow9i);
        double _arr10r = 0, _arr10i = 0;
        { int _idx = (j - 1); _arr10r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr10i = 0; }
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(_arr10r, _arr10i, _arr10r, _arr10i, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _pow9r + _pow12r; _add13i = _pow9i + _pow12i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_add13r, _add13i, _log17r, _log17i, &_mul18r, &_mul18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 4.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_div21r, _div21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_sin22r, _sin22i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul18r + _mul27r; _add28i = _mul18i + _mul27i;
        double mag = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang30r, _ang30i, j, 0, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _ang29r + _mul31r; _add32i = _ang29i + _mul31i;
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x1r; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _attr35r = 0, _attr35i = 0;
        _attr35r = x2i; _attr35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_mul34r, _mul34i, _attr35r, _attr35i, &_mul36r, &_mul36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add32r + _sin37r; _add38i = _add32i + _sin37i;
        double ang = _add38r; /* +_add38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, ang, 0, &_mul40r, &_mul40i);
        double _exp41r = 0, _exp41i = 0;
        c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(mag, 0, _exp41r, _exp41i, &_mul42r, &_mul42i);
        double _conj43r = 0, _conj43i = 0;
        _conj43r = x1r; _conj43i = -(x1i);
        double _c44r = 0, _c44i = 0;
        _c44r = 5.0; _c44i = 0;
        double _mod45r = 0, _mod45i = 0;
        _mod45r = fmod(j, _c44r); _mod45i = 0;
        double _pow46r = 0, _pow46i = 0;
        c_powr(_conj43r, _conj43i, _mod45r, &_pow46r, &_pow46i);
        double _arr47r = 0, _arr47i = 0;
        { int _idx = (j - 1); _arr47r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr47i = 0; }
        double _mul48r = 0, _mul48i = 0;
        c_mul(j, 0, _arr47r, _arr47i, &_mul48r, &_mul48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_mul48r, _mul48i, &_cos49r, &_cos49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_pow46r, _pow46i, _cos49r, _cos49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul42r + _mul50r; _add51i = _mul42i + _mul50i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    double _c52r = 0, _c52i = 0;
    _c52r = 1.0; _c52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = n + _c52r; _add53i = 0 + _c52i;
    for (int k = 1; k < (int)(_add53r); k++) {
        double _unk54r = 0, _unk54i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='k', ctx=Load())) */
        double _arr55r = 0, _arr55i = 0;
        { int _idx = (int)(_unk54r); _arr55r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr55i = 0; }
        double _unk56r = 0, _unk56i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='k', ctx=Load())) */
        double _arr57r = 0, _arr57i = 0;
        { int _idx = (int)(_unk56r); _arr57r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr57i = 0; }
        double _add58r = 0, _add58i = 0;
        _add58r = _arr55r + _arr57r; _add58i = _arr55i + _arr57i;
        double _ang59r = 0, _ang59i = 0;
        _ang59r = c_arg(x2r, x2i); _ang59i = 0;
        double _mul60r = 0, _mul60i = 0;
        c_mul(k, 0, _ang59r, _ang59i, &_mul60r, &_mul60i);
        double _sin61r = 0, _sin61i = 0;
        c_sin(_mul60r, _mul60i, &_sin61r, &_sin61i);
        double _mul62r = 0, _mul62i = 0;
        c_mul(_add58r, _add58i, _sin61r, _sin61i, &_mul62r, &_mul62i);
        cRe[(k - 1)] += _mul62r; cIm[(k - 1)] += _mul62i;
    }
    double _c63r = 0, _c63i = 0;
    _c63r = 1.0; _c63i = 0;
    double _add64r = 0, _add64i = 0;
    _add64r = n + _c63r; _add64i = 0 + _c63i;
    for (int r = 1; r < (int)(_add64r); r++) {
        double _cf65r = 0, _cf65i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf65r = cRe[_idx]; _cf65i = cIm[_idx]; } }
        double _c66r = 0, _c66i = 0;
        _c66r = 1.0; _c66i = 0;
        double _sub67r = 0, _sub67i = 0;
        _sub67r = x1r - x2r; _sub67i = x1i - x2i;
        double _abs68r = 0, _abs68i = 0;
        _abs68r = c_abs(_sub67r, _sub67i); _abs68i = 0;
        double _c69r = 0, _c69i = 0;
        _c69r = 1.0; _c69i = 0;
        double _add70r = 0, _add70i = 0;
        _add70r = r + _c69r; _add70i = 0 + _c69i;
        double _div71r = 0, _div71i = 0;
        c_div(_abs68r, _abs68i, _add70r, _add70i, &_div71r, &_div71i);
        double _add72r = 0, _add72i = 0;
        _add72r = _c66r + _div71r; _add72i = _c66i + _div71i;
        double _mul73r = 0, _mul73i = 0;
        c_mul(_cf65r, _cf65i, _add72r, _add72i, &_mul73r, &_mul73i);
        double _add74r = 0, _add74i = 0;
        _add74r = x1r + x2r; _add74i = x1i + x2i;
        double _abs75r = 0, _abs75i = 0;
        _abs75r = c_abs(_add74r, _add74i); _abs75i = 0;
        double _c76r = 0, _c76i = 0;
        _c76r = 1.0; _c76i = 0;
        double _add77r = 0, _add77i = 0;
        _add77r = _abs75r + _c76r; _add77i = _abs75i + _c76i;
        double _log78r = 0, _log78i = 0;
        c_log(_add77r, _add77i, &_log78r, &_log78i);
        double _ang79r = 0, _ang79i = 0;
        _ang79r = c_arg(x1r, x1i); _ang79i = 0;
        double _mul80r = 0, _mul80i = 0;
        c_mul(r, 0, _ang79r, _ang79i, &_mul80r, &_mul80i);
        double _cos81r = 0, _cos81i = 0;
        c_cos(_mul80r, _mul80i, &_cos81r, &_cos81i);
        double _mul82r = 0, _mul82i = 0;
        c_mul(_log78r, _log78i, _cos81r, _cos81i, &_mul82r, &_mul82i);
        double _add83r = 0, _add83i = 0;
        _add83r = _mul73r + _mul82r; _add83i = _mul73i + _mul82i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add83r; cIm[_idx] = _add83i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_546_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double _add7r = 0, _add7i = 0;
        _add7r = rec1 + rec2; _add7i = 0 + 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_add7r, _add7i, _c8r, _c8i, &_div9r, &_div9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 5.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sin13r, _sin13i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _div9r + _mul18r; _add19i = _div9i + _mul18i;
        double r = _add19r; /* +_add19ii */
        double _sub20r = 0, _sub20i = 0;
        _sub20r = imc1 - imc2; _sub20i = 0 - 0;
        double _div21r = 0, _div21i = 0;
        c_div(_sub20r, _sub20i, n, 0, &_div21r, &_div21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_div21r, _div21i, j, 0, &_mul22r, &_mul22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 3.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul22r + _sin26r; _add27i = _mul22i + _sin26i;
        double theta = _add27r; /* +_add27ii */
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x1r, x1i); _abs28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_abs28r, _abs28i, j, 0, &_mul29r, &_mul29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _sub31r = 0, _sub31i = 0;
        _sub31r = n - j; _sub31i = 0 - 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _sub31r + _c32r; _add33i = _sub31i + _c32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_abs30r, _abs30i, _add33r, _add33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul29r + _mul34r; _add35i = _mul29i + _mul34i;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _sqrt37r = 0, _sqrt37i = 0;
        c_powr(j, 0, 0.5, &_sqrt37r, &_sqrt37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _log36r + _sqrt37r; _add38i = _log36i + _sqrt37i;
        double magnitude = _add38r; /* +_add38ii */
        double _cos39r = 0, _cos39i = 0;
        c_cos(theta, 0, &_cos39r, &_cos39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _sin41r = 0, _sin41i = 0;
        c_sin(theta, 0, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c40r, _c40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
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

static void poly_547_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
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
        double mag = _add17r; /* +_add17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_mul(j, 0, j, 0, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang18r, _ang18i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _sqrt23r = 0, _sqrt23i = 0;
        c_powr(j, 0, 0.5, &_sqrt23r, &_sqrt23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _sqrt23r, _sqrt23i, &_mul24r, &_mul24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _mul21r - _mul24r; _sub25i = _mul21i - _mul24i;
        double angle = _sub25r; /* +_sub25ii */
        double _cos26r = 0, _cos26i = 0;
        c_cos(angle, 0, &_cos26r, &_cos26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(angle, 0, &_sin27r, &_sin27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin27r, _sin27i, _c28r, _c28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _cos26r + _mul29r; _add30i = _cos26i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag, 0, _add30r, _add30i, &_mul31r, &_mul31i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    double _c32r = 0, _c32i = 0;
    _c32r = 1.0; _c32i = 0;
    double _add33r = 0, _add33i = 0;
    _add33r = n + _c32r; _add33i = 0 + _c32i;
    for (int k = 1; k < (int)(_add33r); k++) {
        double _attr34r = 0, _attr34i = 0;
        _attr34r = x1r; _attr34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = k + _attr34r; _add35i = 0 + _attr34i;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _attr37r = 0, _attr37i = 0;
        _attr37r = x2r; _attr37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(k, 0, _attr37r, _attr37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _c36r + _mul38r; _add39i = _c36i + _mul38i;
        double _div40r = 0, _div40i = 0;
        c_div(_add35r, _add35i, _add39r, _add39i, &_div40r, &_div40i);
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x1r, x1i); _ang41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(k, 0, _ang41r, _ang41i, &_mul42r, &_mul42i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(_mul42r, _mul42i, &_cos43r, &_cos43i);
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(k, 0, _ang44r, _ang44i, &_mul45r, &_mul45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_mul45r, _mul45i, &_sin46r, &_sin46i);
        double _sum47r = 0, _sum47i = 0;
        _sum47r = _cos43r + _sin46r; _sum47i = _cos43i + _sin46i;
        double _add48r = 0, _add48i = 0;
        _add48r = _div40r + _sum47r; _add48i = _div40i + _sum47i;
        { double _tr = cRe[(k - 1)]*_add48r - cIm[(k - 1)]*_add48i; cIm[(k - 1)] = cRe[(k - 1)]*_add48i + cIm[(k - 1)]*_add48r; cRe[(k - 1)] = _tr; }
    }
    double _c49r = 0, _c49i = 0;
    _c49r = 1.0; _c49i = 0;
    double _add50r = 0, _add50i = 0;
    _add50r = n + _c49r; _add50i = 0 + _c49i;
    for (int r = 1; r < (int)(_add50r); r++) {
        double _cf51r = 0, _cf51i = 0;
        { int _idx = ((int)(n) - r); if (_idx >= 0 && _idx < 36) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
        double _conj52r = 0, _conj52i = 0;
        _conj52r = _cf51r; _conj52i = -(_cf51i);
        double _abs53r = 0, _abs53i = 0;
        _abs53r = c_abs(x1r, x1i); _abs53i = 0;
        double _c54r = 0, _c54i = 0;
        _c54r = 1.0; _c54i = 0;
        double _div55r = 0, _div55i = 0;
        c_div(_c54r, _c54i, r, 0, &_div55r, &_div55i);
        double _pow56r = 0, _pow56i = 0;
        c_powr(_abs53r, _abs53i, _div55r, &_pow56r, &_pow56i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(_conj52r, _conj52i, _pow56r, _pow56i, &_mul57r, &_mul57i);
        cRe[(r - 1)] += _mul57r; cIm[(r - 1)] += _mul57i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_548_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _sqrt9r = 0, _sqrt9i = 0;
        c_powr(j, 0, 0.5, &_sqrt9r, &_sqrt9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr8r, _attr8i, _sqrt9r, _sqrt9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _mul10r; _add11i = _mul7i + _mul10i;
        double r = _add11r; /* +_add11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1i; _attr12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(j, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr12r, _attr12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x2i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 4.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_attr15r, _attr15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul14r + _mul20r; _add21i = _mul14i + _mul20i;
        double im = _add21r; /* +_add21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 5.0; _c23i = 0;
        double _mod24r = 0, _mod24i = 0;
        _mod24r = fmod(j, _c23r); _mod24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _mod24r + _c25r; _add26i = _mod24i + _c25i;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_abs22r, _abs22i, _add26r, &_pow27r, &_pow27i);
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x2r, x2i); _abs28i = 0;
        double _sub29r = 0, _sub29i = 0;
        _sub29r = n - j; _sub29i = 0 - 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _sub29r + _c30r; _add31i = _sub29i + _c30i;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_abs28r, _abs28i, _add31r, &_pow32r, &_pow32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _pow27r + _pow32r; _add33i = _pow27i + _pow32i;
        double mag = _add33r; /* +_add33ii */
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
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag, 0, _exp43r, _exp43i, &_mul44r, &_mul44i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x1r; _conj45i = -(x1i);
        double _conj46r = 0, _conj46i = 0;
        _conj46r = x2r; _conj46i = -(x2i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_conj45r, _conj45i, _conj46r, _conj46i, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 1.0; _c48i = 0;
        double _add49r = 0, _add49i = 0;
        _add49r = j + _c48r; _add49i = 0 + _c48i;
        double _div50r = 0, _div50i = 0;
        c_div(_mul47r, _mul47i, _add49r, _add49i, &_div50r, &_div50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul44r + _div50r; _add51i = _mul44i + _div50i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_549_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs3r, _abs3i, j, 0, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
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
        _add13r = _log7r + _pow12r; _add13i = _log7i + _pow12i;
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _add13r + _cos16r; _add17i = _add13i + _cos16i;
        double mag = _add17r; /* +_add17ii */
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
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _add24r + _sin27r; _add28i = _add24i + _sin27i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_550_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 0;
        double coeff = _c7r; /* +_c7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        for (int k = 1; k < (int)(_add9r); k++) {
            double _arr10r = 0, _arr10i = 0;
            { int _idx = (k - 1); _arr10r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr10i = 0; }
            double _c11r = 0, _c11i = 0;
            _c11r = 2.0; _c11i = 0;
            double _pow12r = 0, _pow12i = 0;
            c_mul(_arr10r, _arr10i, _arr10r, _arr10i, &_pow12r, &_pow12i);
            double _arr13r = 0, _arr13i = 0;
            { int _idx = (k - 1); _arr13r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr13i = 0; }
            double _c14r = 0, _c14i = 0;
            _c14r = 3.0; _c14i = 0;
            double _pow15r = 0, _pow15i = 0;
            c_mul(_arr13r, _arr13i, _arr13r, _arr13i, &_pow15r, &_pow15i);
            c_mul(_pow15r, _pow15i, _arr13r, _arr13i, &_pow15r, &_pow15i);
            double _sub16r = 0, _sub16i = 0;
            _sub16r = _pow12r - _pow15r; _sub16i = _pow12i - _pow15i;
            double _c17r = 0, _c17i = 0;
            _c17r = 0.0; _c17i = 1.0;
            double _mul18r = 0, _mul18i = 0;
            c_mul(k, 0, M_PI, 0, &_mul18r, &_mul18i);
            double _div19r = 0, _div19i = 0;
            c_div(_mul18r, _mul18i, n, 0, &_div19r, &_div19i);
            double _sin20r = 0, _sin20i = 0;
            c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(_c17r, _c17i, _sin20r, _sin20i, &_mul21r, &_mul21i);
            double _exp22r = 0, _exp22i = 0;
            c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
            double _mul23r = 0, _mul23i = 0;
            c_mul(_sub16r, _sub16i, _exp22r, _exp22i, &_mul23r, &_mul23i);
            coeff += _mul23r;
            double _conj24r = 0, _conj24i = 0;
            _conj24r = x1r; _conj24i = -(x1i);
            double _mul25r = 0, _mul25i = 0;
            c_mul(k, 0, M_PI, 0, &_mul25r, &_mul25i);
            double _c26r = 0, _c26i = 0;
            _c26r = 1.0; _c26i = 0;
            double _add27r = 0, _add27i = 0;
            _add27r = j + _c26r; _add27i = 0 + _c26i;
            double _div28r = 0, _div28i = 0;
            c_div(_mul25r, _mul25i, _add27r, _add27i, &_div28r, &_div28i);
            double _cos29r = 0, _cos29i = 0;
            c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
            double _mul30r = 0, _mul30i = 0;
            c_mul(_conj24r, _conj24i, _cos29r, _cos29i, &_mul30r, &_mul30i);
            coeff += _mul30r;
        }
        double _sub31r = 0, _sub31i = 0;
        _sub31r = n - j; _sub31i = 0 - 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _sub31r + _c32r; _add33i = _sub31i + _c32i;
        for (int r = 1; r < (int)(_add33r); r++) {
            double _arr34r = 0, _arr34i = 0;
            { int _idx = (j - 1); _arr34r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr34i = 0; }
            double _arr35r = 0, _arr35i = 0;
            { int _idx = (j - 1); _arr35r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr35i = 0; }
            double _add36r = 0, _add36i = 0;
            _add36r = _arr34r + _arr35r; _add36i = _arr34i + _arr35i;
            double _abs37r = 0, _abs37i = 0;
            _abs37r = c_abs(_add36r, _add36i); _abs37i = 0;
            double _c38r = 0, _c38i = 0;
            _c38r = 1.0; _c38i = 0;
            double _add39r = 0, _add39i = 0;
            _add39r = _abs37r + _c38r; _add39i = _abs37i + _c38i;
            double _log40r = 0, _log40i = 0;
            c_log(_add39r, _add39i, &_log40r, &_log40i);
            double _mul41r = 0, _mul41i = 0;
            c_mul(_log40r, _log40i, r, 0, &_mul41r, &_mul41i);
            double _mul42r = 0, _mul42i = 0;
            c_mul(r, 0, M_PI, 0, &_mul42r, &_mul42i);
            double _div43r = 0, _div43i = 0;
            c_div(_mul42r, _mul42i, n, 0, &_div43r, &_div43i);
            double _sin44r = 0, _sin44i = 0;
            c_sin(_div43r, _div43i, &_sin44r, &_sin44i);
            double _mul45r = 0, _mul45i = 0;
            c_mul(_mul41r, _mul41i, _sin44r, _sin44i, &_mul45r, &_mul45i);
            coeff += _mul45r;
            double _abs46r = 0, _abs46i = 0;
            _abs46r = c_abs(x2r, x2i); _abs46i = 0;
            double _pow47r = 0, _pow47i = 0;
            c_powr(_abs46r, _abs46i, r, &_pow47r, &_pow47i);
            double _add48r = 0, _add48i = 0;
            _add48r = x1r + x2r; _add48i = x1i + x2i;
            double _ang49r = 0, _ang49i = 0;
            _ang49r = c_arg(_add48r, _add48i); _ang49i = 0;
            double _mul50r = 0, _mul50i = 0;
            c_mul(r, 0, _ang49r, _ang49i, &_mul50r, &_mul50i);
            double _cos51r = 0, _cos51i = 0;
            c_cos(_mul50r, _mul50i, &_cos51r, &_cos51i);
            double _mul52r = 0, _mul52i = 0;
            c_mul(_pow47r, _pow47i, _cos51r, _cos51i, &_mul52r, &_mul52i);
            coeff += _mul52r;
        }
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = coeff; cIm[_idx] = 0; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_551_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
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
        double rec_t1 = _attr3r; /* +_attr3ii */
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1i; _attr4i = 0;
        double imc_t1 = _attr4r; /* +_attr4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2r; _attr5i = 0;
        double rec_t2 = _attr5r; /* +_attr5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double imc_t2 = _attr6r; /* +_attr6ii */
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _div12r = 0, _div12i = 0;
        c_div(_mul11r, _mul11i, n, 0, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log10r, _log10i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double mag_part1 = _mul14r; /* +_mul14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(n, 0, _c20r, _c20i, &_div21r, &_div21i);
        double _div22r = 0, _div22i = 0;
        c_div(_mul19r, _mul19i, _div21r, _div21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log18r, _log18i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double mag_part2 = _mul24r; /* +_mul24ii */
        double _add25r = 0, _add25i = 0;
        _add25r = mag_part1 + mag_part2; _add25i = 0 + 0;
        double _prod26r = 0, _prod26i = 0;
        c_mul(rec_t1, 0, imc_t2, 0, &_prod26r, &_prod26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_c27r, _c27i, j, 0, &_div28r, &_div28i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(_prod26r, _prod26i, _div28r, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add25r + _pow29r; _add30i = _add25i + _pow29i;
        double mag_variation = _add30r; /* +_add30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double angle_part1 = _mul35r; /* +_mul35ii */
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 3.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(j, 0, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang36r, _ang36i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double angle_part2 = _mul40r; /* +_mul40ii */
        double _add41r = 0, _add41i = 0;
        _add41r = angle_part1 + angle_part2; _add41i = 0 + 0;
        double _sin42r = 0, _sin42i = 0;
        c_sin(j, 0, &_sin42r, &_sin42i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(j, 0, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_sin42r, _sin42i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _add41r + _mul44r; _add45i = _add41i + _mul44i;
        double angle_variation = _add45r; /* +_add45ii */
        double _cos46r = 0, _cos46i = 0;
        c_cos(angle_variation, 0, &_cos46r, &_cos46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _sin48r = 0, _sin48i = 0;
        c_sin(angle_variation, 0, &_sin48r, &_sin48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c47r, _c47i, _sin48r, _sin48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _cos46r + _mul49r; _add50i = _cos46i + _mul49i;
        double complex_component = _add50r; /* +_add50ii */
        double _mul51r = 0, _mul51i = 0;
        c_mul(mag_variation, 0, complex_component, 0, &_mul51r, &_mul51i);
        double _conj52r = 0, _conj52i = 0;
        _conj52r = x1r; _conj52i = -(x1i);
        double _sin53r = 0, _sin53i = 0;
        c_sin(j, 0, &_sin53r, &_sin53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_conj52r, _conj52i, _sin53r, _sin53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _mul51r + _mul54r; _add55i = _mul51i + _mul54i;
        double _conj56r = 0, _conj56i = 0;
        _conj56r = x2r; _conj56i = -(x2i);
        double _cos57r = 0, _cos57i = 0;
        c_cos(j, 0, &_cos57r, &_cos57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_conj56r, _conj56i, _cos57r, _cos57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _add55r + _mul58r; _add59i = _add55i + _mul58i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_552_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 11.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log14r, _log14i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul10r + _mul19r; _add20i = _mul10i + _mul19i;
        double mag_part = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(j, 0, _c22r, _c22i, &_div23r, &_div23i);
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
        double angle_part = _add31r; /* +_add31ii */
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 0;
        double intricate_sum = _c32r; /* +_c32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        for (int k = 1; k < (int)(_add34r); k++) {
            double _attr35r = 0, _attr35i = 0;
            _attr35r = x1r; _attr35i = 0;
            double _pow36r = 0, _pow36i = 0;
            c_powr(_attr35r, _attr35i, k, &_pow36r, &_pow36i);
            double _attr37r = 0, _attr37i = 0;
            _attr37r = x2i; _attr37i = 0;
            double _pow38r = 0, _pow38i = 0;
            c_powr(_attr37r, _attr37i, k, &_pow38r, &_pow38i);
            double _sub39r = 0, _sub39i = 0;
            _sub39r = _pow36r - _pow38r; _sub39i = _pow36i - _pow38i;
            double _mul40r = 0, _mul40i = 0;
            c_mul(k, 0, M_PI, 0, &_mul40r, &_mul40i);
            double _c41r = 0, _c41i = 0;
            _c41r = 1.0; _c41i = 0;
            double _add42r = 0, _add42i = 0;
            _add42r = j + _c41r; _add42i = 0 + _c41i;
            double _div43r = 0, _div43i = 0;
            c_div(_mul40r, _mul40i, _add42r, _add42i, &_div43r, &_div43i);
            double _sin44r = 0, _sin44i = 0;
            c_sin(_div43r, _div43i, &_sin44r, &_sin44i);
            double _mul45r = 0, _mul45i = 0;
            c_mul(_sub39r, _sub39i, _sin44r, _sin44i, &_mul45r, &_mul45i);
            intricate_sum += _mul45r;
        }
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c46r, _c46i, angle_part, 0, &_mul47r, &_mul47i);
        double _exp48r = 0, _exp48i = 0;
        c_exp2(_mul47r, _mul47i, &_exp48r, &_exp48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(mag_part, 0, _exp48r, _exp48i, &_mul49r, &_mul49i);
        double _conj50r = 0, _conj50i = 0;
        _conj50r = x1r; _conj50i = -(x1i);
        double _c51r = 0, _c51i = 0;
        _c51r = 5.0; _c51i = 0;
        double _mod52r = 0, _mod52i = 0;
        _mod52r = fmod(j, _c51r); _mod52i = 0;
        double _pow53r = 0, _pow53i = 0;
        c_powr(x2r, x2i, _mod52r, &_pow53r, &_pow53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_conj50r, _conj50i, _pow53r, _pow53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _mul49r + _mul54r; _add55i = _mul49i + _mul54i;
        double _add56r = 0, _add56i = 0;
        _add56r = _add55r + intricate_sum; _add56i = _add55i + 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_553_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_sum = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double ang_sum = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        for (int k = 1; k < (int)(_add6r); k++) {
            double _abs7r = 0, _abs7i = 0;
            _abs7r = c_abs(x1r, x1i); _abs7i = 0;
            double _c8r = 0, _c8i = 0;
            _c8r = 2.0; _c8i = 0;
            double _pow9r = 0, _pow9i = 0;
            c_mul(k, 0, k, 0, &_pow9r, &_pow9i);
            double _add10r = 0, _add10i = 0;
            _add10r = _abs7r + _pow9r; _add10i = _abs7i + _pow9i;
            double _log11r = 0, _log11i = 0;
            c_log(_add10r, _add10i, &_log11r, &_log11i);
            double _attr12r = 0, _attr12i = 0;
            _attr12r = x2r; _attr12i = 0;
            double _mul13r = 0, _mul13i = 0;
            c_mul(k, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
            double _sin14r = 0, _sin14i = 0;
            c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
            double _mul15r = 0, _mul15i = 0;
            c_mul(_log11r, _log11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
            double _attr16r = 0, _attr16i = 0;
            _attr16r = x1i; _attr16i = 0;
            double _mul17r = 0, _mul17i = 0;
            c_mul(k, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
            double _cos18r = 0, _cos18i = 0;
            c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
            double _add19r = 0, _add19i = 0;
            _add19r = _mul15r + _cos18r; _add19i = _mul15i + _cos18i;
            double term_mag = _add19r; /* +_add19ii */
            double _ang20r = 0, _ang20i = 0;
            _ang20r = c_arg(x2r, x2i); _ang20i = 0;
            double _sqrt21r = 0, _sqrt21i = 0;
            c_powr(k, 0, 0.5, &_sqrt21r, &_sqrt21i);
            double _mul22r = 0, _mul22i = 0;
            c_mul(_ang20r, _ang20i, _sqrt21r, _sqrt21i, &_mul22r, &_mul22i);
            double _c23r = 0, _c23i = 0;
            _c23r = 2.0; _c23i = 0;
            double _div24r = 0, _div24i = 0;
            c_div(k, 0, _c23r, _c23i, &_div24r, &_div24i);
            double _sin25r = 0, _sin25i = 0;
            c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
            double _add26r = 0, _add26i = 0;
            _add26r = _mul22r + _sin25r; _add26i = _mul22i + _sin25i;
            double term_ang = _add26r; /* +_add26ii */
            mag_sum += term_mag;
            ang_sum += term_ang;
        }
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, ang_sum, 0, &_mul28r, &_mul28i);
        double _exp29r = 0, _exp29i = 0;
        c_exp2(_mul28r, _mul28i, &_exp29r, &_exp29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag_sum, 0, _exp29r, _exp29i, &_mul30r, &_mul30i);
        double _conj31r = 0, _conj31i = 0;
        _conj31r = x1r; _conj31i = -(x1i);
        double _pow32r = 0, _pow32i = 0;
        c_powr(x2r, x2i, j, &_pow32r, &_pow32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_conj31r, _conj31i, _pow32r, _pow32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul30r + _mul33r; _add34i = _mul30i + _mul33i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add34r; cIm[_idx] = _add34i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_554_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
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
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _pow7r = 0, _pow7i = 0;
        c_powr(rec1, 0, j, &_pow7r, &_pow7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 4.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_mul8r, _mul8i, _c9r, _c9i, &_div10r, &_div10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_div10r, _div10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_pow7r, _pow7i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double term1 = _mul12r; /* +_mul12ii */
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 3.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(imc2, 0, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double term2 = _mul17r; /* +_mul17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x2r; _attr22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 5.0; _c23i = 0;
        double _mod24r = 0, _mod24i = 0;
        _mod24r = fmod(j, _c23r); _mod24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _mod24r + _c25r; _add26i = _mod24i + _c25i;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_attr22r, _attr22i, _add26r, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log21r, _log21i, _pow27r, _pow27i, &_mul28r, &_mul28i);
        double term3 = _mul28r; /* +_mul28ii */
        double _add29r = 0, _add29i = 0;
        _add29r = x1r + x2r; _add29i = x1i + x2i;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(_add29r, _add29i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.5; _c31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, j, 0, &_mul32r, &_mul32i);
        double _pow33r = 0, _pow33i = 0;
        c_powr(_abs30r, _abs30i, _mul32r, &_pow33r, &_pow33i);
        double term4 = _pow33r; /* +_pow33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang35r, _ang35i, j, 0, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _ang34r + _mul36r; _add37i = _ang34i + _mul36i;
        double angle = _add37r; /* +_add37ii */
        double _add38r = 0, _add38i = 0;
        _add38r = term1 + term2; _add38i = 0 + 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _add38r + term3; _add39i = _add38i + 0;
        double _abs40r = 0, _abs40i = 0;
        _abs40r = c_abs(_add39r, _add39i); _abs40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _abs40r + term4; _add41i = _abs40i + 0;
        double magnitude = _add41r; /* +_add41ii */
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
        c_mul(magnitude, 0, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    double _c48r = 0, _c48i = 0;
    _c48r = 1.0; _c48i = 0;
    double _add49r = 0, _add49i = 0;
    _add49r = n + _c48r; _add49i = 0 + _c48i;
    for (int k = 1; k < (int)(_add49r); k++) {
        double _cf50r = 0, _cf50i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _sin52r = 0, _sin52i = 0;
        c_sin(k, 0, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c51r, _c51i, _sin52r, _sin52i, &_mul53r, &_mul53i);
        double _exp54r = 0, _exp54i = 0;
        c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_cf50r, _cf50i, _exp54r, _exp54i, &_mul55r, &_mul55i);
        double _cf56r = 0, _cf56i = 0;
        { int _idx = (k % (int)(n)); if (_idx >= 0 && _idx < 36) { _cf56r = cRe[_idx]; _cf56i = cIm[_idx]; } }
        double _conj57r = 0, _conj57i = 0;
        _conj57r = _cf56r; _conj57i = -(_cf56i);
        double _add58r = 0, _add58i = 0;
        _add58r = _mul55r + _conj57r; _add58i = _mul55i + _conj57i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_555_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_attr3r, _attr3i, j, &_pow4r, &_pow4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2r; _attr5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 3.0; _c6i = 0;
        double _mod7r = 0, _mod7i = 0;
        _mod7r = fmod(j, _c6r); _mod7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _mod7r + _c8r; _add9i = _mod7i + _c8i;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_attr5r, _attr5i, _add9r, &_pow10r, &_pow10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _pow4r + _pow10r; _add11i = _pow4i + _pow10i;
        double real_part = _add11r; /* +_add11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1i; _attr12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 4.0; _c13i = 0;
        double _mod14r = 0, _mod14i = 0;
        _mod14r = fmod(j, _c13r); _mod14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _mod14r + _c15r; _add16i = _mod14i + _c15i;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_attr12r, _attr12i, _add16r, &_pow17r, &_pow17i);
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x2i; _attr18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _mod20r = 0, _mod20i = 0;
        _mod20r = fmod(j, _c19r); _mod20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _mod20r + _c21r; _add22i = _mod20i + _c21i;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_attr18r, _attr18i, _add22r, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _pow17r + _pow23r; _add24i = _pow17i + _pow23i;
        double imag_part = _add24r; /* +_add24ii */
        double _add25r = 0, _add25i = 0;
        _add25r = real_part + imag_part; _add25i = 0 + 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul26r, &_mul26i);
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_mul26r, _mul26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = _abs27r + _c28r; _add29i = _abs27i + _c28i;
        double _log30r = 0, _log30i = 0;
        c_log(_add29r, _add29i, &_log30r, &_log30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add25r + _log30r; _add31i = _add25i + _log30i;
        double magnitude = _add31r; /* +_add31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, _ang32r, _ang32i, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x2r, x2i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, _ang35r, _ang35i, &_mul36r, &_mul36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_mul36r, _mul36i, &_cos37r, &_cos37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sin34r + _cos37r; _add38i = _sin34i + _cos37i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_556_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr1r + (_attr2r - _attr1r) * _li / 34.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr3r + (_attr4r - _attr3r) * _li / 34.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = n + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _arr7r, _arr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _arr10r = 0, _arr10i = 0;
        { int _idx = (j - 1); _arr10r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr10i = 0; }
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _arr10r, _arr10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _sin9r + _cos12r; _add13i = _sin9i + _cos12i;
        double _add14r = 0, _add14i = 0;
        _add14r = x1r + x2r; _add14i = x1i + x2i;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(_add14r, _add14i); _ang15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _add13r + _ang15r; _add16i = _add13i + _ang15i;
        double angle = _add16r; /* +_add16ii */
        double _arr17r = 0, _arr17i = 0;
        { int _idx = (j - 1); _arr17r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr17i = 0; }
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_mul(_arr17r, _arr17i, _arr17r, _arr17i, &_pow19r, &_pow19i);
        double _arr20r = 0, _arr20i = 0;
        { int _idx = (j - 1); _arr20r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr20i = 0; }
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_mul(_arr20r, _arr20i, _arr20r, _arr20i, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _pow19r + _pow22r; _add23i = _pow19i + _pow22i;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_add23r, _add23i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.5; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_powr(j, 0, 1.5, &_pow29r, &_pow29i);
        double _unk30r = 0, _unk30i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='j', ctx=Load())) */
        double _arr31r = 0, _arr31i = 0;
        { int _idx = (int)(_unk30r); _arr31r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr31i = 0; }
        double _unk32r = 0, _unk32i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='j', ctx=Load())) */
        double _arr33r = 0, _arr33i = 0;
        { int _idx = (int)(_unk32r); _arr33r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr33i = 0; }
        double _add34r = 0, _add34i = 0;
        _add34r = _arr31r + _arr33r; _add34i = _arr31i + _arr33i;
        double _add35r = 0, _add35i = 0;
        _add35r = _pow29r + _add34r; _add35i = _pow29i + _add34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_log27r, _log27i, _add35r, _add35i, &_mul36r, &_mul36i);
        double magnitude = _mul36r; /* +_mul36ii */
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

static void poly_557_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double mag_sum = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        for (int j = 1; j < (int)(_add5r); j++) {
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
            double _mul15r = 0, _mul15i = 0;
            c_mul(x1r, x1i, j, 0, &_mul15r, &_mul15i);
            double _c16r = 0, _c16i = 0;
            _c16r = 1.0; _c16i = 0;
            double _add17r = 0, _add17i = 0;
            _add17r = j + _c16r; _add17i = 0 + _c16i;
            double _div18r = 0, _div18i = 0;
            c_div(x2r, x2i, _add17r, _add17i, &_div18r, &_div18i);
            double _add19r = 0, _add19i = 0;
            _add19r = _mul15r + _div18r; _add19i = _mul15i + _div18i;
            double _abs20r = 0, _abs20i = 0;
            _abs20r = c_abs(_add19r, _add19i); _abs20i = 0;
            double _c21r = 0, _c21i = 0;
            _c21r = 1.0; _c21i = 0;
            double _add22r = 0, _add22i = 0;
            _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
            double _log23r = 0, _log23i = 0;
            c_log(_add22r, _add22i, &_log23r, &_log23i);
            double _add24r = 0, _add24i = 0;
            _add24r = _mul14r + _log23r; _add24i = _mul14i + _log23i;
            mag_sum += _add24r;
        }
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _sub26r = 0, _sub26i = 0;
        _sub26r = x1r - x2r; _sub26i = x1i - x2i;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_sub26r, _sub26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 10.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_abs27r, _abs27i, _c28r, _c28i, &_div29r, &_div29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _c25r + _div29r; _add30i = _c25i + _div29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag_sum, 0, _add30r, _add30i, &_mul31r, &_mul31i);
        double magnitude = _mul31r; /* +_mul31ii */
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 0;
        double angle_sum = _c32r; /* +_c32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = k + _c33r; _add34i = 0 + _c33i;
        for (int j = 1; j < (int)(_add34r); j++) {
            double _mul35r = 0, _mul35i = 0;
            c_mul(j, 0, x2r, x2i, &_mul35r, &_mul35i);
            double _add36r = 0, _add36i = 0;
            _add36r = x1r + _mul35r; _add36i = x1i + _mul35i;
            double _ang37r = 0, _ang37i = 0;
            _ang37r = c_arg(_add36r, _add36i); _ang37i = 0;
            double _mul38r = 0, _mul38i = 0;
            c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
            double _c39r = 0, _c39i = 0;
            _c39r = 9.0; _c39i = 0;
            double _div40r = 0, _div40i = 0;
            c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
            double _sin41r = 0, _sin41i = 0;
            c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
            double _mul42r = 0, _mul42i = 0;
            c_mul(_ang37r, _ang37i, _sin41r, _sin41i, &_mul42r, &_mul42i);
            double _ang43r = 0, _ang43i = 0;
            _ang43r = c_arg(x2r, x2i); _ang43i = 0;
            double _mul44r = 0, _mul44i = 0;
            c_mul(j, 0, M_PI, 0, &_mul44r, &_mul44i);
            double _c45r = 0, _c45i = 0;
            _c45r = 11.0; _c45i = 0;
            double _div46r = 0, _div46i = 0;
            c_div(_mul44r, _mul44i, _c45r, _c45i, &_div46r, &_div46i);
            double _cos47r = 0, _cos47i = 0;
            c_cos(_div46r, _div46i, &_cos47r, &_cos47i);
            double _mul48r = 0, _mul48i = 0;
            c_mul(_ang43r, _ang43i, _cos47r, _cos47i, &_mul48r, &_mul48i);
            double _sub49r = 0, _sub49i = 0;
            _sub49r = _mul42r - _mul48r; _sub49i = _mul42i - _mul48i;
            angle_sum += _sub49r;
        }
        double _div50r = 0, _div50i = 0;
        c_div(angle_sum, 0, k, 0, &_div50r, &_div50i);
        double angle = _div50r; /* +_div50ii */
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
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_558_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _sqrt9r = 0, _sqrt9i = 0;
        c_powr(j, 0, 0.5, &_sqrt9r, &_sqrt9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr8r, _attr8i, _sqrt9r, _sqrt9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _mul10r; _add11i = _mul7i + _mul10i;
        double r = _add11r; /* +_add11ii */
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x1i; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _attr12r, _attr12i, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x2i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _sin14r + _cos17r; _add18i = _sin14i + _cos17i;
        double _add19r = 0, _add19i = 0;
        _add19r = x1r + x2r; _add19i = x1i + x2i;
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(_add19r, _add19i); _ang20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _add18r + _ang20r; _add21i = _add18i + _ang20i;
        double theta = _add21r; /* +_add21ii */
        for (int k = 1; k < 4; k++) {
            double _attr22r = 0, _attr22i = 0;
            _attr22r = x1r; _attr22i = 0;
            double _mul23r = 0, _mul23i = 0;
            c_mul(_attr22r, _attr22i, k, 0, &_mul23r, &_mul23i);
            double _c24r = 0, _c24i = 0;
            _c24r = 1.0; _c24i = 0;
            double _add25r = 0, _add25i = 0;
            _add25r = j + _c24r; _add25i = 0 + _c24i;
            double _div26r = 0, _div26i = 0;
            c_div(_mul23r, _mul23i, _add25r, _add25i, &_div26r, &_div26i);
            r += _div26r;
            double _mul27r = 0, _mul27i = 0;
            c_mul(k, 0, M_PI, 0, &_mul27r, &_mul27i);
            double _div28r = 0, _div28i = 0;
            c_div(_mul27r, _mul27i, j, 0, &_div28r, &_div28i);
            double _sin29r = 0, _sin29i = 0;
            c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
            theta += _sin29r;
        }
        double _cos30r = 0, _cos30i = 0;
        c_cos(theta, 0, &_cos30r, &_cos30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(theta, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c31r, _c31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _cos30r + _mul33r; _add34i = _cos30i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(r, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_559_c(double x1r, double x1i, double x2r, double x2i,
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
        double temp_real = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double temp_imag = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        for (int k = 1; k < (int)(_add6r); k++) {
            double _attr7r = 0, _attr7i = 0;
            _attr7r = x1r; _attr7i = 0;
            double _pow8r = 0, _pow8i = 0;
            c_powr(_attr7r, _attr7i, k, &_pow8r, &_pow8i);
            double _attr9r = 0, _attr9i = 0;
            _attr9r = x2r; _attr9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(k, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
            double _sin11r = 0, _sin11i = 0;
            c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_pow8r, _pow8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
            double _c13r = 0, _c13i = 0;
            _c13r = 1.0; _c13i = 0;
            double _add14r = 0, _add14i = 0;
            _add14r = k + _c13r; _add14i = 0 + _c13i;
            double _div15r = 0, _div15i = 0;
            c_div(_mul12r, _mul12i, _add14r, _add14i, &_div15r, &_div15i);
            temp_real += _div15r;
            double _attr16r = 0, _attr16i = 0;
            _attr16r = x2i; _attr16i = 0;
            double _sub17r = 0, _sub17i = 0;
            _sub17r = j - k; _sub17i = 0 - 0;
            double _c18r = 0, _c18i = 0;
            _c18r = 1.0; _c18i = 0;
            double _add19r = 0, _add19i = 0;
            _add19r = _sub17r + _c18r; _add19i = _sub17i + _c18i;
            double _pow20r = 0, _pow20i = 0;
            c_powr(_attr16r, _attr16i, _add19r, &_pow20r, &_pow20i);
            double _sub21r = 0, _sub21i = 0;
            _sub21r = j - k; _sub21i = 0 - 0;
            double _c22r = 0, _c22i = 0;
            _c22r = 1.0; _c22i = 0;
            double _add23r = 0, _add23i = 0;
            _add23r = _sub21r + _c22r; _add23i = _sub21i + _c22i;
            double _attr24r = 0, _attr24i = 0;
            _attr24r = x1i; _attr24i = 0;
            double _mul25r = 0, _mul25i = 0;
            c_mul(_add23r, _add23i, _attr24r, _attr24i, &_mul25r, &_mul25i);
            double _cos26r = 0, _cos26i = 0;
            c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
            double _mul27r = 0, _mul27i = 0;
            c_mul(_pow20r, _pow20i, _cos26r, _cos26i, &_mul27r, &_mul27i);
            double _sub28r = 0, _sub28i = 0;
            _sub28r = j - k; _sub28i = 0 - 0;
            double _c29r = 0, _c29i = 0;
            _c29r = 2.0; _c29i = 0;
            double _add30r = 0, _add30i = 0;
            _add30r = _sub28r + _c29r; _add30i = _sub28i + _c29i;
            double _div31r = 0, _div31i = 0;
            c_div(_mul27r, _mul27i, _add30r, _add30i, &_div31r, &_div31i);
            temp_imag += _div31r;
        }
        double _add32r = 0, _add32i = 0;
        _add32r = temp_real + temp_imag; _add32i = 0 + 0;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_add32r, _add32i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _abs33r + _c34r; _add35i = _abs33i + _c34i;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_log36r, _log36i, j, 0, &_mul37r, &_mul37i);
        double magnitude = _mul37r; /* +_mul37ii */
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x1r; _attr38i = 0;
        double _attr39r = 0, _attr39i = 0;
        _attr39r = x2i; _attr39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_attr38r, _attr38i, _attr39r, _attr39i, &_mul40r, &_mul40i);
        double _div41r = 0, _div41i = 0;
        c_div(_mul40r, _mul40i, j, 0, &_div41r, &_div41i);
        double _add42r = 0, _add42i = 0;
        _add42r = x1r + x2r; _add42i = x1i + x2i;
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(_add42r, _add42i); _ang43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(j, 0, _ang43r, _ang43i, &_mul44r, &_mul44i);
        double _sin45r = 0, _sin45i = 0;
        c_sin(_mul44r, _mul44i, &_sin45r, &_sin45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _div41r + _sin45r; _add46i = _div41i + _sin45i;
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
        c_mul(magnitude, 0, _add51r, _add51i, &_mul52r, &_mul52i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_560_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double re1 = _attr1r; /* +_attr1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double im1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double re2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double im2 = _attr4r; /* +_attr4ii */
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
        double _div11r = 0, _div11i = 0;
        c_div(_mul10r, _mul10i, n, 0, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log9r, _log9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _sqrt15r = 0, _sqrt15i = 0;
        c_powr(j, 0, 0.5, &_sqrt15r, &_sqrt15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _sqrt15r; _add16i = _abs14i + _sqrt15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = n + _c19r; _add20i = 0 + _c19i;
        double _div21r = 0, _div21i = 0;
        c_div(_mul18r, _mul18i, _add20r, _add20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log17r, _log17i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul13r + _mul23r; _add24i = _mul13i + _mul23i;
        double magnitude = _add24r; /* +_add24ii */
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
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang30r, _ang30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul29r + _mul34r; _add35i = _mul29i + _mul34i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_561_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr8r, _attr8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _mul12r; _add13i = _mul7i + _mul12i;
        double r = _add13r; /* +_add13ii */
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
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _mul16r - _mul19r; _sub20i = _mul16i - _mul19i;
        double theta = _sub20r; /* +_sub20ii */
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_abs21r, _abs21i, j, &_pow22r, &_pow22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _c23r + j; _add24i = _c23i + 0;
        double _div25r = 0, _div25i = 0;
        c_div(_pow22r, _pow22i, _add24r, _add24i, &_div25r, &_div25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x2r, x2i); _abs26i = 0;
        double _sqrt27r = 0, _sqrt27i = 0;
        c_powr(j, 0, 0.5, &_sqrt27r, &_sqrt27i);
        double _pow28r = 0, _pow28i = 0;
        c_powr(_abs26r, _abs26i, _sqrt27r, &_pow28r, &_pow28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _div25r + _pow28r; _add29i = _div25i + _pow28i;
        double mag_variation = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, theta, 0, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = r + _mul31r; _add32i = 0 + _mul31i;
        double _sin33r = 0, _sin33i = 0;
        c_sin(j, 0, &_sin33r, &_sin33i);
        double _add34r = 0, _add34i = 0;
        _add34r = mag_variation + _sin33r; _add34i = 0 + _sin33i;
        double _cos35r = 0, _cos35i = 0;
        c_cos(j, 0, &_cos35r, &_cos35i);
        double _sub36r = 0, _sub36i = 0;
        _sub36r = _add34r - _cos35r; _sub36i = _add34i - _cos35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_add32r, _add32i, _sub36r, _sub36i, &_mul37r, &_mul37i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_562_c(double x1r, double x1i, double x2r, double x2i,
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
        _abs17r = c_abs(rec, 0); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_mul(j, 0, j, 0, &_pow22r, &_pow22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = n - j; _sub23i = 0 - 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _sub23r + _c24r; _add25i = _sub23i + _c24i;
        double _sqrt26r = 0, _sqrt26i = 0;
        c_powr(_add25r, _add25i, 0.5, &_sqrt26r, &_sqrt26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _pow22r + _sqrt26r; _add27i = _pow22i + _sqrt26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log20r, _log20i, _add27r, _add27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(j, 0, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul28r, _mul28i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul30r + 0; _add31i = _mul30i + 0;
        double mag = _add31r; /* +_add31ii */
        double _mul32r = 0, _mul32i = 0;
        c_mul(rec, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 7.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(imc, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 5.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _sin35r + _cos39r; _add40i = _sin35i + _cos39i;
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x1r, x1i); _ang41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _add40r + _ang41r; _add42i = _add40i + _ang41i;
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x2r, x2i); _ang43i = 0;
        double _sub44r = 0, _sub44i = 0;
        _sub44r = _add42r - _ang43r; _sub44i = _add42i - _ang43i;
        double angle = _sub44r; /* +_sub44ii */
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
        c_mul(mag, 0, _add49r, _add49i, &_mul50r, &_mul50i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_563_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr6r, _attr6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _pow5r + _mul11r; _add12i = _pow5i + _mul11i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log3r, _log3i, _add12r, _add12i, &_mul13r, &_mul13i);
        double mag_part = _mul13r; /* +_mul13ii */
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 7.0; _c16i = 0;
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
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul19r - _mul25r; _sub26i = _mul19i - _mul25i;
        double angle_part = _sub26r; /* +_sub26ii */
        double _cos27r = 0, _cos27i = 0;
        c_cos(angle_part, 0, &_cos27r, &_cos27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(angle_part, 0, &_sin28r, &_sin28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_sin28r, _sin28i, _c29r, _c29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _cos27r + _mul30r; _add31i = _cos27i + _mul30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag_part, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 26) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_564_c(double x1r, double x1i, double x2r, double x2i,
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
        _attr4r = x2r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log3r, _log3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double mag_part1 = _mul7r; /* +_mul7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = x2r + j; _add11i = x2i + 0;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_cos10r, _cos10i, _abs12r, _abs12i, &_mul13r, &_mul13i);
        double mag_part2 = _mul13r; /* +_mul13ii */
        double _add14r = 0, _add14i = 0;
        _add14r = mag_part1 + mag_part2; _add14i = 0 + 0;
        double mag = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.5; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(j, 0, 0.5, &_pow17r, &_pow17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang15r, _ang15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
        double angle_part1 = _mul18r; /* +_mul18ii */
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 7.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_div21r, _div21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 11.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _sin22r + _cos26r; _add27i = _sin22i + _cos26i;
        double angle_part2 = _add27r; /* +_add27ii */
        double _add28r = 0, _add28i = 0;
        _add28r = angle_part1 + angle_part2; _add28i = 0 + 0;
        double angle = _add28r; /* +_add28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, angle, 0, &_mul30r, &_mul30i);
        double _exp31r = 0, _exp31i = 0;
        c_exp2(_mul30r, _mul30i, &_exp31r, &_exp31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag, 0, _exp31r, _exp31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 26) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_565_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 27; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _sin2r = 0, _sin2i = 0;
        c_sin(j, 0, &_sin2r, &_sin2i);
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2r; _attr3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_sin2r, _sin2i, _attr3r, _attr3i, &_mul4r, &_mul4i);
        double _add5r = 0, _add5i = 0;
        _add5r = _attr1r + _mul4r; _add5i = _attr1i + _mul4i;
        double a = _add5r; /* +_add5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1i; _attr6i = 0;
        double _cos7r = 0, _cos7i = 0;
        c_cos(j, 0, &_cos7r, &_cos7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_cos7r, _cos7i, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _attr6r + _mul9r; _add10i = _attr6i + _mul9i;
        double b = _add10r; /* +_add10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _abs12r; _add13i = _abs11i + _abs12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _add13r + _c14r; _add15i = _add13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double c = _log16r; /* +_log16ii */
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang17r, _ang17i, j, 0, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_ang19r, _ang19i, _add21r, _add21i, &_div22r, &_div22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul18r + _div22r; _add23i = _mul18i + _div22i;
        double d = _add23r; /* +_add23ii */
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, b, 0, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = a + _mul25r; _add26i = 0 + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_add26r, _add26i, c, 0, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(d, 0, &_cos28r, &_cos28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 0.0; _c29i = 1.0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(d, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c29r, _c29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _cos28r + _mul31r; _add32i = _cos28i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_mul27r, _mul27i, _add32r, _add32i, &_mul33r, &_mul33i);
        double _conj34r = 0, _conj34i = 0;
        _conj34r = x1r; _conj34i = -(x1i);
        double _pow35r = 0, _pow35i = 0;
        c_powr(_conj34r, _conj34i, j, &_pow35r, &_pow35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(j, 0, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_pow35r, _pow35i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul33r + _mul37r; _add38i = _mul33i + _mul37i;
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x2r; _conj39i = -(x2i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(j, 0, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj39r, _conj39i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _sub42r = 0, _sub42i = 0;
        _sub42r = _add38r - _mul41r; _sub42i = _add38i - _mul41i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub42r; cIm[_idx] = _sub42i; } }
    }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_566_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double mag_part1 = _log5r; /* +_log5ii */
        double mag_part2 = 0; /* +0i */
        double _sqrt6r = 0, _sqrt6i = 0;
        c_powr(j, 0, 0.5, &_sqrt6r, &_sqrt6i);
        double _add7r = 0, _add7i = 0;
        _add7r = mag_part2 + _sqrt6r; _add7i = 0 + _sqrt6i;
        double _mul8r = 0, _mul8i = 0;
        c_mul(mag_part1, 0, _add7r, _add7i, &_mul8r, &_mul8i);
        double magnitude = _mul8r; /* +_mul8ii */
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double angle_part1 = _sin11r; /* +_sin11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _div15r = 0, _div15i = 0;
        c_div(_ang12r, _ang12i, _add14r, _add14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double angle_part2 = _cos16r; /* +_cos16ii */
        double _add17r = 0, _add17i = 0;
        _add17r = angle_part1 + angle_part2; _add17i = 0 + 0;
        double angle = _add17r; /* +_add17ii */
        double _cos18r = 0, _cos18i = 0;
        c_cos(angle, 0, &_cos18r, &_cos18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(angle, 0, &_sin19r, &_sin19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_sin19r, _sin19i, _c20r, _c20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _cos18r + _mul21r; _add22i = _cos18i + _mul21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(magnitude, 0, _add22r, _add22i, &_mul23r, &_mul23i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1i; _attr24i = 0;
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _attr24r - _attr25r; _sub26i = _attr24i - _attr25i;
        double _sin27r = 0, _sin27i = 0;
        c_sin(j, 0, &_sin27r, &_sin27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 2.0; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_mul(_sin27r, _sin27i, _sin27r, _sin27i, &_pow29r, &_pow29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_sub26r, _sub26i, _pow29r, _pow29i, &_mul30r, &_mul30i);
        cRe[(j - 1)] += _mul30r; cIm[(j - 1)] += _mul30i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_567_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 26; j++) {
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
        double _add10r = 0, _add10i = 0;
        _add10r = _c6r + _sin9r; _add10i = _c6i + _sin9i;
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2i; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add10r + _cos13r; _add14i = _add10i + _cos13i;
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1r; _attr15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.5; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_attr15r, _attr15i, 0.5, &_pow17r, &_pow17i);
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x2i; _attr18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.3; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_attr18r, _attr18i, 0.3, &_pow20r, &_pow20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_pow17r, _pow17i, _pow20r, _pow20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _add14r + _mul21r; _add22i = _add14i + _mul21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log5r, _log5i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang24r, _ang24i, j, 0, &_mul25r, &_mul25i);
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
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
        double _c32r = 0, _c32i = 0;
        _c32r = 5.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add31r + _cos34r; _add35i = _add31i + _cos34i;
        double angle = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, angle, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_568_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int r = 1; r < (int)(_add6r); r++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (r - 1); _arr7r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(x1r, x1i, _arr7r, _arr7i, &_mul8r, &_mul8i);
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (r - 1); _arr9r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr9i = 0; }
        double _mul10r = 0, _mul10i = 0;
        c_mul(x2r, x2i, _arr9r, _arr9i, &_mul10r, &_mul10i);
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
        double _mul17r = 0, _mul17i = 0;
        c_mul(r, 0, M_PI, 0, &_mul17r, &_mul17i);
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
        double _mul23r = 0, _mul23i = 0;
        c_mul(r, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 5.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul22r + _cos26r; _add27i = _mul22i + _cos26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _arr29r = 0, _arr29i = 0;
        { int _idx = (r - 1); _arr29r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr29i = 0; }
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _arr29r, _arr29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _arr32r = 0, _arr32i = 0;
        { int _idx = (r - 1); _arr32r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr32i = 0; }
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _arr32r, _arr32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul30r + _mul33r; _add34i = _mul30i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(r, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 6.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add34r + _sin38r; _add39i = _add34i + _sin38i;
        double ang = _add39r; /* +_add39ii */
        double _cos40r = 0, _cos40i = 0;
        c_cos(ang, 0, &_cos40r, &_cos40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(ang, 0, &_sin41r, &_sin41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_sin41r, _sin41i, _c42r, _c42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _cos40r + _mul43r; _add44i = _cos40i + _mul43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag, 0, _add44r, _add44i, &_mul45r, &_mul45i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_569_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _arr10r = 0, _arr10i = 0;
        { int _idx = (j - 1); _arr10r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr10i = 0; }
        double _mul11r = 0, _mul11i = 0;
        c_mul(_arr10r, _arr10i, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _div14r = 0, _div14i = 0;
        c_div(_mul11r, _mul11i, _add13r, _add13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _arr16r = 0, _arr16i = 0;
        { int _idx = (j - 1); _arr16r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr16i = 0; }
        double _mul17r = 0, _mul17i = 0;
        c_mul(_arr16r, _arr16i, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _div20r = 0, _div20i = 0;
        c_div(_mul17r, _mul17i, _add19r, _add19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin15r, _sin15i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _log9r + _mul22r; _add23i = _log9i + _mul22i;
        double mag_part = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang24r, _ang24i, j, 0, &_mul25r, &_mul25i);
        double _arr26r = 0, _arr26i = 0;
        { int _idx = (j - 1); _arr26r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr26i = 0; }
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_arr26r, _arr26i, _add28r, _add28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _sin30r; _add31i = _mul25i + _sin30i;
        double _arr32r = 0, _arr32i = 0;
        { int _idx = (j - 1); _arr32r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr32i = 0; }
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _div35r = 0, _div35i = 0;
        c_div(_arr32r, _arr32i, _add34r, _add34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _add31r - _cos36r; _sub37i = _add31i - _cos36i;
        double angle_part = _sub37r; /* +_sub37ii */
        double _cos38r = 0, _cos38i = 0;
        c_cos(angle_part, 0, &_cos38r, &_cos38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(angle_part, 0, &_sin39r, &_sin39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_sin39r, _sin39i, _c40r, _c40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos38r + _mul41r; _add42i = _cos38i + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(mag_part, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_570_c(double x1r, double x1i, double x2r, double x2i,
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
        _c3r = 0.0; _c3i = 0;
        double sum_mag = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double sum_ang = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        for (int k = 1; k < (int)(_add6r); k++) {
            double _mul7r = 0, _mul7i = 0;
            c_mul(x1r, x1i, k, 0, &_mul7r, &_mul7i);
            double _div8r = 0, _div8i = 0;
            c_div(x2r, x2i, k, 0, &_div8r, &_div8i);
            double _add9r = 0, _add9i = 0;
            _add9r = _mul7r + _div8r; _add9i = _mul7i + _div8i;
            double term = _add9r; /* +_add9ii */
            double _abs10r = 0, _abs10i = 0;
            _abs10r = c_abs(term, 0); _abs10i = 0;
            double _c11r = 0, _c11i = 0;
            _c11r = 1.0; _c11i = 0;
            double _add12r = 0, _add12i = 0;
            _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
            double _log13r = 0, _log13i = 0;
            c_log(_add12r, _add12i, &_log13r, &_log13i);
            sum_mag += _log13r;
            double _attr14r = 0, _attr14i = 0;
            _attr14r = x2r; _attr14i = 0;
            double _mul15r = 0, _mul15i = 0;
            c_mul(k, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
            double _sin16r = 0, _sin16i = 0;
            c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(x1r, x1i, _sin16r, _sin16i, &_mul17r, &_mul17i);
            double _attr18r = 0, _attr18i = 0;
            _attr18r = x1i; _attr18i = 0;
            double _mul19r = 0, _mul19i = 0;
            c_mul(k, 0, _attr18r, _attr18i, &_mul19r, &_mul19i);
            double _cos20r = 0, _cos20i = 0;
            c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(x2r, x2i, _cos20r, _cos20i, &_mul21r, &_mul21i);
            double _add22r = 0, _add22i = 0;
            _add22r = _mul17r + _mul21r; _add22i = _mul17i + _mul21i;
            double angle_term = _add22r; /* +_add22ii */
            double _ang23r = 0, _ang23i = 0;
            _ang23r = c_arg(angle_term, 0); _ang23i = 0;
            sum_ang += _ang23r;
        }
        double _cos24r = 0, _cos24i = 0;
        c_cos(sum_ang, 0, &_cos24r, &_cos24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(sum_ang, 0, &_sin25r, &_sin25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_sin25r, _sin25i, _c26r, _c26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _cos24r + _mul27r; _add28i = _cos24i + _mul27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(sum_mag, 0, _add28r, _add28i, &_mul29r, &_mul29i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_571_c(double x1r, double x1i, double x2r, double x2i,
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
        _c3r = 7.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(j, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double k = _add6r; /* +_add6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1r; _attr7i = 0;
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(k, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr7r, _attr7i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2r; _attr12i = 0;
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(k, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_attr12r, _attr12i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul11r + _mul16r; _add17i = _mul11i + _mul16i;
        double r = _add17r; /* +_add17ii */
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x1i; _attr18i = 0;
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2r; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(k, 0, _attr19r, _attr19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_attr18r, _attr18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x2i; _attr23i = 0;
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1i; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(k, 0, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(_mul25r, _mul25i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_attr23r, _attr23i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul22r - _mul27r; _sub28i = _mul22i - _mul27i;
        double i_part = _sub28r; /* +_sub28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs29r + j; _add30i = _abs29i + 0;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.5; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_powr(j, 0, 1.5, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_log31r, _log31i, _pow33r, _pow33i, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = j + _c36r; _add37i = 0 + _c36i;
        double _log38r = 0, _log38i = 0;
        c_log(_add37r, _add37i, &_log38r, &_log38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _c35r + _log38r; _add39i = _c35i + _log38i;
        double _div40r = 0, _div40i = 0;
        c_div(_mul34r, _mul34i, _add39r, _add39i, &_div40r, &_div40i);
        double magnitude = _div40r; /* +_div40ii */
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x1r, x1i); _ang41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang41r, _ang41i, j, 0, &_mul42r, &_mul42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = j + _c43r; _add44i = 0 + _c43i;
        double _log45r = 0, _log45i = 0;
        c_log(_add44r, _add44i, &_log45r, &_log45i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x2r, x2i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_log45r, _log45i, _ang46r, _ang46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul42r + _mul47r; _add48i = _mul42i + _mul47i;
        double angle = _add48r; /* +_add48ii */
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c49r, _c49i, angle, 0, &_mul50r, &_mul50i);
        double _exp51r = 0, _exp51i = 0;
        c_exp2(_mul50r, _mul50i, &_exp51r, &_exp51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(magnitude, 0, _exp51r, _exp51i, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 0.0; _c53i = 1.0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c53r, _c53i, i_part, 0, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = r + _mul54r; _add55i = 0 + _mul54i;
        double _mul56r = 0, _mul56i = 0;
        c_mul(_mul52r, _mul52i, _add55r, _add55i, &_mul56r, &_mul56i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_572_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 26; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double summation_mag = _c1r; /* +_c1ii */
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double summation_ang = _c2r; /* +_c2ii */
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        for (int k = 1; k < (int)(_add4r); k++) {
            double _mul5r = 0, _mul5i = 0;
            c_mul(x1r, x1i, k, 0, &_mul5r, &_mul5i);
            double _add6r = 0, _add6i = 0;
            _add6r = _mul5r + x2r; _add6i = _mul5i + x2i;
            double _abs7r = 0, _abs7i = 0;
            _abs7r = c_abs(_add6r, _add6i); _abs7i = 0;
            double _c8r = 0, _c8i = 0;
            _c8r = 1.0; _c8i = 0;
            double _add9r = 0, _add9i = 0;
            _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
            double _log10r = 0, _log10i = 0;
            c_log(_add9r, _add9i, &_log10r, &_log10i);
            double _attr11r = 0, _attr11i = 0;
            _attr11r = x1r; _attr11i = 0;
            double _mul12r = 0, _mul12i = 0;
            c_mul(k, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
            double _sin13r = 0, _sin13i = 0;
            c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(_log10r, _log10i, _sin13r, _sin13i, &_mul14r, &_mul14i);
            summation_mag += _mul14r;
            double _mul15r = 0, _mul15i = 0;
            c_mul(x1r, x1i, k, 0, &_mul15r, &_mul15i);
            double _sub16r = 0, _sub16i = 0;
            _sub16r = _mul15r - x2r; _sub16i = _mul15i - x2i;
            double _ang17r = 0, _ang17i = 0;
            _ang17r = c_arg(_sub16r, _sub16i); _ang17i = 0;
            double _attr18r = 0, _attr18i = 0;
            _attr18r = x2i; _attr18i = 0;
            double _mul19r = 0, _mul19i = 0;
            c_mul(k, 0, _attr18r, _attr18i, &_mul19r, &_mul19i);
            double _cos20r = 0, _cos20i = 0;
            c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
            double _add21r = 0, _add21i = 0;
            _add21r = _ang17r + _cos20r; _add21i = _ang17i + _cos20i;
            summation_ang += _add21r;
        }
        double _cos22r = 0, _cos22i = 0;
        c_cos(summation_ang, 0, &_cos22r, &_cos22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(summation_ang, 0, &_sin23r, &_sin23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin23r, _sin23i, _c24r, _c24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _cos22r + _mul25r; _add26i = _cos22i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(summation_mag, 0, _add26r, _add26i, &_mul27r, &_mul27i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_573_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x2r, x2i); _ang4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _ang4r, _ang4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr8r, _attr8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _mul12r; _add13i = _mul7i + _mul12i;
        double part1 = _add13r; /* +_add13ii */
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x2r; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_attr14r, _attr14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr19r, _attr19i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _mul18r - _mul23r; _sub24i = _mul18i - _mul23i;
        double part2 = _sub24r; /* +_sub24ii */
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs25r + j; _add26i = _abs25i + 0;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _attr28r = 0, _attr28i = 0;
        _attr28r = x1r; _attr28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(0, 0, _attr28r, _attr28i, &_mul29r, &_mul29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_mul29r, _mul29i, &_sin30r, &_sin30i);
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x2i; _attr31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(0, 0, _attr31r, _attr31i, &_mul32r, &_mul32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_mul32r, _mul32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin30r, _sin30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _log27r + _mul34r; _add35i = _log27i + _mul34i;
        double magnitude = _add35r; /* +_add35ii */
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x1r, x1i); _ang36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang36r, _ang36i, j, 0, &_mul37r, &_mul37i);
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 0.5; _c39i = 0;
        double _pow40r = 0, _pow40i = 0;
        c_powr(j, 0, 0.5, &_pow40r, &_pow40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang38r, _ang38i, _pow40r, _pow40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul37r + _mul41r; _add42i = _mul37i + _mul41i;
        double angle = _add42r; /* +_add42ii */
        double _cos43r = 0, _cos43i = 0;
        c_cos(angle, 0, &_cos43r, &_cos43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(angle, 0, &_sin44r, &_sin44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_sin44r, _sin44i, _c45r, _c45i, &_mul46r, &_mul46i);
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

static void poly_574_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double deg = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = deg + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_powr(_attr3r, _attr3i, j, &_pow4r, &_pow4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_pow4r, _pow4i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = deg + _c10r; _add11i = 0 + _c10i;
        double _sub12r = 0, _sub12i = 0;
        _sub12r = _add11r - j; _sub12i = _add11i - 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_attr9r, _attr9i, _sub12r, &_pow13r, &_pow13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_pow13r, _pow13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul8r + _mul17r; _add18i = _mul8i + _mul17i;
        double r_part = _add18r; /* +_add18ii */
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x1i; _attr19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_attr19r, _attr19i, j, &_pow20r, &_pow20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _ang21r, _ang21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_pow20r, _pow20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = deg + _c26r; _add27i = 0 + _c26i;
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _add27r - j; _sub28i = _add27i - 0;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_attr25r, _attr25i, _sub28r, &_pow29r, &_pow29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _ang30r, _ang30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_pow29r, _pow29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul24r - _mul33r; _sub34i = _mul24i - _mul33i;
        double im_part = _sub34r; /* +_sub34ii */
        double _add35r = 0, _add35i = 0;
        _add35r = r_part + im_part; _add35i = 0 + 0;
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(_add35r, _add35i); _abs36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
        double _log39r = 0, _log39i = 0;
        c_log(_add38r, _add38i, &_log39r, &_log39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 1.5; _c40i = 0;
        double _pow41r = 0, _pow41i = 0;
        c_powr(j, 0, 1.5, &_pow41r, &_pow41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_log39r, _log39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
        double magnitude = _mul42r; /* +_mul42ii */
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x1r, x1i); _ang43i = 0;
        double _sin44r = 0, _sin44i = 0;
        c_sin(j, 0, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_ang43r, _ang43i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x2r, x2i); _ang46i = 0;
        double _cos47r = 0, _cos47i = 0;
        c_cos(j, 0, &_cos47r, &_cos47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang46r, _ang46i, _cos47r, _cos47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul45r + _mul48r; _add49i = _mul45i + _mul48i;
        double angle = _add49r; /* +_add49ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    }
    double _c56r = 0, _c56i = 0;
    _c56r = 2.0; _c56i = 0;
    double _add57r = 0, _add57i = 0;
    _add57r = deg + _c56r; _add57i = 0 + _c56i;
    for (int k = 1; k < (int)(_add57r); k++) {
        double _c58r = 0, _c58i = 0;
        _c58r = 3.0; _c58i = 0;
        double _mod59r = 0, _mod59i = 0;
        _mod59r = fmod(k, _c58r); _mod59i = 0;
        double _c60r = 0, _c60i = 0;
        _c60r = 1.0; _c60i = 0;
        double _add61r = 0, _add61i = 0;
        _add61r = _mod59r + _c60r; _add61i = _mod59i + _c60i;
        double factor = _add61r; /* +_add61ii */
        double _attr62r = 0, _attr62i = 0;
        _attr62r = x1r; _attr62i = 0;
        double _c63r = 0, _c63i = 0;
        _c63r = 5.0; _c63i = 0;
        double _mod64r = 0, _mod64i = 0;
        _mod64r = fmod(k, _c63r); _mod64i = 0;
        double _pow65r = 0, _pow65i = 0;
        c_powr(_attr62r, _attr62i, _mod64r, &_pow65r, &_pow65i);
        double _attr66r = 0, _attr66i = 0;
        _attr66r = x2i; _attr66i = 0;
        double _c67r = 0, _c67i = 0;
        _c67r = 4.0; _c67i = 0;
        double _mod68r = 0, _mod68i = 0;
        _mod68r = fmod(k, _c67r); _mod68i = 0;
        double _pow69r = 0, _pow69i = 0;
        c_powr(_attr66r, _attr66i, _mod68r, &_pow69r, &_pow69i);
        double _add70r = 0, _add70i = 0;
        _add70r = _pow65r + _pow69r; _add70i = _pow65i + _pow69i;
        double _cf71r = 0, _cf71i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf71r = cRe[_idx]; _cf71i = cIm[_idx]; } }
        double _ang72r = 0, _ang72i = 0;
        _ang72r = c_arg(_cf71r, _cf71i); _ang72i = 0;
        double _mul73r = 0, _mul73i = 0;
        c_mul(k, 0, _ang72r, _ang72i, &_mul73r, &_mul73i);
        double _sin74r = 0, _sin74i = 0;
        c_sin(_mul73r, _mul73i, &_sin74r, &_sin74i);
        double _mul75r = 0, _mul75i = 0;
        c_mul(_add70r, _add70i, _sin74r, _sin74i, &_mul75r, &_mul75i);
        { double _tr = cRe[(k - 1)]*_mul75r - cIm[(k - 1)]*_mul75i; cIm[(k - 1)] = cRe[(k - 1)]*_mul75i + cIm[(k - 1)]*_mul75r; cRe[(k - 1)] = _tr; }
    }
    for (int r = 2; r < (int)(deg); r++) {
        double _cf76r = 0, _cf76i = 0;
        { int _idx = (r - 2); if (_idx >= 0 && _idx < 36) { _cf76r = cRe[_idx]; _cf76i = cIm[_idx]; } }
        double _cf77r = 0, _cf77i = 0;
        { int _idx = r; if (_idx >= 0 && _idx < 36) { _cf77r = cRe[_idx]; _cf77i = cIm[_idx]; } }
        double _mul78r = 0, _mul78i = 0;
        c_mul(_cf76r, _cf76i, _cf77r, _cf77i, &_mul78r, &_mul78i);
        double _c79r = 0, _c79i = 0;
        _c79r = 1.0; _c79i = 0;
        double _cf80r = 0, _cf80i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf80r = cRe[_idx]; _cf80i = cIm[_idx]; } }
        double _abs81r = 0, _abs81i = 0;
        _abs81r = c_abs(_cf80r, _cf80i); _abs81i = 0;
        double _add82r = 0, _add82i = 0;
        _add82r = _c79r + _abs81r; _add82i = _c79i + _abs81i;
        double _div83r = 0, _div83i = 0;
        c_div(_mul78r, _mul78i, _add82r, _add82i, &_div83r, &_div83i);
        cRe[(r - 1)] += _div83r; cIm[(r - 1)] += _div83i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_575_c(double x1r, double x1i, double x2r, double x2i,
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
        _c3r = 3.0; _c3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _c3r, _c3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 7.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 10.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(_add6r, _c7r); _mod8i = 0;
        double k = _mod8r; /* +_mod8ii */
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1r; _attr9i = 0;
        double _cos10r = 0, _cos10i = 0;
        c_cos(j, 0, &_cos10r, &_cos10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr9r, _attr9i, _cos10r, _cos10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2r; _attr12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(k, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr12r, _attr12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul11r + _mul14r; _add15i = _mul11i + _mul14i;
        double r = _add15r; /* +_add15ii */
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1i; _attr16i = 0;
        double _sin17r = 0, _sin17i = 0;
        c_sin(j, 0, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_attr16r, _attr16i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _cos20r = 0, _cos20i = 0;
        c_cos(k, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_attr19r, _attr19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
        double im = _add22r; /* +_add22ii */
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x1r, x1i); _abs23i = 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _abs24r; _add25i = _abs23i + _abs24i;
        double _add26r = 0, _add26i = 0;
        _add26r = _add25r + j; _add26i = _add25i + 0;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 5.0; _c29i = 0;
        double _mod30r = 0, _mod30i = 0;
        _mod30r = fmod(j, _c29r); _mod30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _c28r + _mod30r; _add31i = _c28i + _mod30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_log27r, _log27i, _add31r, _add31i, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _div35r = 0, _div35i = 0;
        c_div(_mul32r, _mul32i, _add34r, _add34i, &_div35r, &_div35i);
        double magnitude = _div35r; /* +_div35ii */
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x1r, x1i); _ang36i = 0;
        double _sin37r = 0, _sin37i = 0;
        c_sin(k, 0, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang36r, _ang36i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x2r, x2i); _ang39i = 0;
        double _cos40r = 0, _cos40i = 0;
        c_cos(j, 0, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang39r, _ang39i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul38r + _mul41r; _add42i = _mul38i + _mul41i;
        double angle = _add42r; /* +_add42ii */
        double _cos43r = 0, _cos43i = 0;
        c_cos(angle, 0, &_cos43r, &_cos43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(angle, 0, &_sin44r, &_sin44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_sin44r, _sin44i, _c45r, _c45i, &_mul46r, &_mul46i);
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

static void poly_576_c(double x1r, double x1i, double x2r, double x2i,
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
        double _div3r = 0, _div3i = 0;
        c_div(j, 0, degree, 0, &_div3r, &_div3i);
        double r = _div3r; /* +_div3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 3.0; _c6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c6r, _c6i, j, 0, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow5r + _mul7r; _add8i = _pow5i + _mul7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _add8r + _c9r; _add10i = _add8i + _c9i;
        double k = _add10r; /* +_add10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _abs12r; _add13i = _abs11i + _abs12i;
        double _mul14r = 0, _mul14i = 0;
        c_mul(r, 0, k, 0, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _add13r + _mul14r; _add15i = _add13i + _mul14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _sin18r = 0, _sin18i = 0;
        c_sin(j, 0, &_sin18r, &_sin18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(k, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sin18r, _sin18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _c17r + _mul20r; _add21i = _c17i + _mul20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log16r, _log16i, _add21r, _add21i, &_mul22r, &_mul22i);
        double mag = _mul22r; /* +_mul22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _sin24r = 0, _sin24i = 0;
        c_sin(k, 0, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(r, 0, j, 0, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang26r, _ang26i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul25r + _mul29r; _add30i = _mul25i + _mul29i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_577_c(double x1r, double x1i, double x2r, double x2i,
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
    double rec_seq[26];
    for (int _li = 0; _li < 26; _li++) {
        rec_seq[_li] = _attr1r + (_attr2r - _attr1r) * _li / 25.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_seq[26];
    for (int _li = 0; _li < 26; _li++) {
        imc_seq[_li] = _attr3r + (_attr4r - _attr3r) * _li / 25.0;
    }
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
    for (int j = 1; j < (int)(_add6r); j++) {
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 26) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double r = _arr7r; /* +_arr7ii */
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 26) ? imc_seq[_idx] : 0.0; _arr8i = 0; }
        double im = _arr8r; /* +_arr8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(r, 0); _abs9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(im, 0); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _abs10r; _add11i = _abs9i + _abs10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _add11r + _c12r; _add13i = _add11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 2.0; _c15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c15r, _c15i, M_PI, 0, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul16r, _mul16i, r, 0, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log14r, _log14i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c20r, _c20i, M_PI, 0, &_mul21r, &_mul21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul21r, _mul21i, im, 0, &_mul22r, &_mul22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul19r + _cos23r; _add24i = _mul19i + _cos23i;
        double mag = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang25r, _ang25i, j, 0, &_mul26r, &_mul26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(im, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul26r + _sin28r; _add29i = _mul26i + _sin28i;
        double ang = _add29r; /* +_add29ii */
        double _cos30r = 0, _cos30i = 0;
        c_cos(ang, 0, &_cos30r, &_cos30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(ang, 0, &_sin31r, &_sin31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_sin31r, _sin31i, _c32r, _c32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _cos30r + _mul33r; _add34i = _cos30i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(mag, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        double _conj36r = 0, _conj36i = 0;
        _conj36r = x2r; _conj36i = -(x2i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, M_PI, 0, &_mul37r, &_mul37i);
        double _div38r = 0, _div38i = 0;
        c_div(_mul37r, _mul37i, degree, 0, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_conj36r, _conj36i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul35r + _mul40r; _add41i = _mul35i + _mul40i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_578_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 26; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_attr1r, _attr1i, j, 0, &_mul2r, &_mul2i);
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _div6r = 0, _div6i = 0;
        c_div(_attr3r, _attr3i, _add5r, _add5i, &_div6r, &_div6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _mul2r + _div6r; _add7i = _mul2i + _div6i;
        double r = _add7r; /* +_add7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr8r, _attr8i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2i; _attr11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(j, 0, _c12r, _c12i, &_div13r, &_div13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_div13r, _div13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr11r, _attr11i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul10r + _mul15r; _add16i = _mul10i + _mul15i;
        double im = _add16r; /* +_add16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs17r + j; _add18i = _abs17i + 0;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1i; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_sin22r, _sin22i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _log19r + _mul26r; _add27i = _log19i + _mul26i;
        double magnitude = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang28r, _ang28i, j, 0, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.5; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = j + _c31r; _add32i = 0 + _c31i;
        double _div33r = 0, _div33i = 0;
        c_div(_ang30r, _ang30i, _add32r, _add32i, &_div33r, &_div33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul29r - _div33r; _sub34i = _mul29i - _div33i;
        double angle = _sub34r; /* +_sub34ii */
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
        double _c40r = 0, _c40i = 0;
        _c40r = 7.0; _c40i = 0;
        double _mod41r = 0, _mod41i = 0;
        _mod41r = fmod(j, _c40r); _mod41i = 0;
        double _pow42r = 0, _pow42i = 0;
        c_powr(x2r, x2i, _mod41r, &_pow42r, &_pow42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_conj39r, _conj39i, _pow42r, _pow42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul38r + _mul43r; _add44i = _mul38i + _mul43i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add44r; cIm[_idx] = _add44i; } }
    }
    for (int k = 1; k < 26; k++) {
        double _cf45r = 0, _cf45i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf45r = cRe[_idx]; _cf45i = cIm[_idx]; } }
        double _c46r = 0, _c46i = 0;
        _c46r = 1.0; _c46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 0.05; _c47i = 0;
        double _attr48r = 0, _attr48i = 0;
        _attr48r = x1r; _attr48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(k, 0, _attr48r, _attr48i, &_mul49r, &_mul49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(_mul49r, _mul49i, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c47r, _c47i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _c46r + _mul51r; _add52i = _c46i + _mul51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_cf45r, _cf45i, _add52r, _add52i, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 0.05;
        double _attr55r = 0, _attr55i = 0;
        _attr55r = x2i; _attr55i = 0;
        double _mul56r = 0, _mul56i = 0;
        c_mul(k, 0, _attr55r, _attr55i, &_mul56r, &_mul56i);
        double _cos57r = 0, _cos57i = 0;
        c_cos(_mul56r, _mul56i, &_cos57r, &_cos57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_c54r, _c54i, _cos57r, _cos57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _mul53r + _mul58r; _add59i = _mul53i + _mul58i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_579_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = degree + _c11r; _add12i = 0 + _c11i;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _add12r - j; _sub13i = _add12i - 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_abs10r, _abs10i, _sub13r, &_pow14r, &_pow14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(j, 0, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_pow14r, _pow14i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul9r + _mul16r; _add17i = _mul9i + _mul16i;
        double mag_part = _add17r; /* +_add17ii */
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
        double _conj31r = 0, _conj31i = 0;
        _conj31r = x1r; _conj31i = -(x1i);
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2r; _attr32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_conj31r, _conj31i, _attr32r, _attr32i, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = k + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_mul33r, _mul33i, _add35r, _add35i, &_div36r, &_div36i);
        cRe[(k - 1)] += _div36r; cIm[(k - 1)] += _div36i;
        double _cf37r = 0, _cf37i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(k, 0, M_PI, 0, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 12.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_mul39r, _mul39i, _c40r, _c40i, &_div41r, &_div41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_div41r, _div41i, &_sin42r, &_sin42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _c38r + _sin42r; _add43i = _c38i + _sin42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_cf37r, _cf37i, _add43r, _add43i, &_mul44r, &_mul44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(k, 0, M_PI, 0, &_mul45r, &_mul45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 18.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(_mul45r, _mul45i, _c46r, _c46i, &_div47r, &_div47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_div47r, _div47i, &_cos48r, &_cos48i);
        double _attr49r = 0, _attr49i = 0;
        _attr49r = x1i; _attr49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_cos48r, _cos48i, _attr49r, _attr49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul44r + _mul50r; _add51i = _mul44i + _mul50i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    double _c52r = 0, _c52i = 0;
    _c52r = 2.0; _c52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = degree + _c52r; _add53i = 0 + _c52i;
    for (int r = 1; r < (int)(_add53r); r++) {
        double _abs54r = 0, _abs54i = 0;
        _abs54r = c_abs(x1r, x1i); _abs54i = 0;
        double _abs55r = 0, _abs55i = 0;
        _abs55r = c_abs(x2r, x2i); _abs55i = 0;
        double _prod56r = 0, _prod56i = 0;
        c_mul(_abs54r, _abs54i, _abs55r, _abs55i, &_prod56r, &_prod56i);
        double _c57r = 0, _c57i = 0;
        _c57r = 2.0; _c57i = 0;
        double _add58r = 0, _add58i = 0;
        _add58r = r + _c57r; _add58i = 0 + _c57i;
        double _div59r = 0, _div59i = 0;
        c_div(_prod56r, _prod56i, _add58r, _add58i, &_div59r, &_div59i);
        cRe[(r - 1)] += _div59r; cIm[(r - 1)] += _div59i;
        double _cf60r = 0, _cf60i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
        double _cf61r = 0, _cf61i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf61r = cRe[_idx]; _cf61i = cIm[_idx]; } }
        double _abs62r = 0, _abs62i = 0;
        _abs62r = c_abs(_cf61r, _cf61i); _abs62i = 0;
        double _c63r = 0, _c63i = 0;
        _c63r = 1.0; _c63i = 0;
        double _add64r = 0, _add64i = 0;
        _add64r = _abs62r + _c63r; _add64i = _abs62i + _c63i;
        double _log65r = 0, _log65i = 0;
        c_log(_add64r, _add64i, &_log65r, &_log65i);
        double _mul66r = 0, _mul66i = 0;
        c_mul(_cf60r, _cf60i, _log65r, _log65i, &_mul66r, &_mul66i);
        double _c67r = 0, _c67i = 0;
        _c67r = 0.0; _c67i = 1.0;
        double _sin68r = 0, _sin68i = 0;
        c_sin(r, 0, &_sin68r, &_sin68i);
        double _mul69r = 0, _mul69i = 0;
        c_mul(_c67r, _c67i, _sin68r, _sin68i, &_mul69r, &_mul69i);
        double _exp70r = 0, _exp70i = 0;
        c_exp2(_mul69r, _mul69i, &_exp70r, &_exp70i);
        double _add71r = 0, _add71i = 0;
        _add71r = _mul66r + _exp70r; _add71i = _mul66i + _exp70i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add71r; cIm[_idx] = _add71i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_580_c(double x1r, double x1i, double x2r, double x2i,
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

static void poly_581_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _ang4r = 0, _ang4i = 0;
        _ang4r = c_arg(x2r, x2i); _ang4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _ang4r, _ang4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1i; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr8r, _attr8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _mul12r; _add13i = _mul7i + _mul12i;
        double term1 = _add13r; /* +_add13ii */
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x2r; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_attr14r, _attr14i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _ang20r, _ang20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr19r, _attr19i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _mul18r - _mul23r; _sub24i = _mul18i - _mul23i;
        double term2 = _sub24r; /* +_sub24ii */
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x1r, x1i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs25r + j; _add26i = _abs25i + 0;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x2r, x2i); _abs28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_abs28r, _abs28i, j, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _log27r + _pow29r; _add30i = _log27i + _pow29i;
        double magnitude = _add30r; /* +_add30ii */
        double _add31r = 0, _add31i = 0;
        _add31r = term1 + term2; _add31i = 0 + 0;
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
        c_mul(magnitude, 0, _add36r, _add36i, &_mul37r, &_mul37i);
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x1r; _conj38i = -(x1i);
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x2r; _conj39i = -(x2i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_conj38r, _conj38i, _conj39r, _conj39i, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = j + _c41r; _add42i = 0 + _c41i;
        double _div43r = 0, _div43i = 0;
        c_div(_mul40r, _mul40i, _add42r, _add42i, &_div43r, &_div43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul37r + _div43r; _add44i = _mul37i + _div43i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add44r; cIm[_idx] = _add44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_582_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 27; j++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 5.0; _c2i = 0;
        double _mod3r = 0, _mod3i = 0;
        _mod3r = fmod(j, _c2r); _mod3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _mod3r + _c4r; _add5i = _mod3i + _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powr(_attr1r, _attr1i, _add5r, &_pow6r, &_pow6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2r; _attr7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(j, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _mod9r + _c10r; _add11i = _mod9i + _c10i;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_attr7r, _attr7i, _add11r, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _pow6r + _pow12r; _add13i = _pow6i + _pow12i;
        double r = _add13r; /* +_add13ii */
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _mod16r = 0, _mod16i = 0;
        _mod16r = fmod(j, _c15r); _mod16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _mod16r + _c17r; _add18i = _mod16i + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_attr14r, _attr14i, _add18r, &_pow19r, &_pow19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _mod22r = 0, _mod22i = 0;
        _mod22r = fmod(j, _c21r); _mod22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _mod22r + _c23r; _add24i = _mod22i + _c23i;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_attr20r, _attr20i, _add24r, &_pow25r, &_pow25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _pow19r - _pow25r; _sub26i = _pow19i - _pow25i;
        double imc = _sub26r; /* +_sub26ii */
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x1r, x1i); _abs27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs27r + j; _add28i = _abs27i + 0;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(r, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_log29r, _log29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(imc, 0, &_cos32r, &_cos32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul31r + _cos32r; _add33i = _mul31i + _cos32i;
        double magnitude = _add33r; /* +_add33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _cos35r = 0, _cos35i = 0;
        c_cos(j, 0, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang34r, _ang34i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(imc, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang37r, _ang37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul36r + _mul39r; _add40i = _mul36i + _mul39i;
        double angle = _add40r; /* +_add40ii */
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, angle, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(magnitude, 0, _exp43r, _exp43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 26) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_583_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_attr3r, _attr3i, _pow5r, _pow5i, &_mul6r, &_mul6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2i; _attr7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        double _div10r = 0, _div10i = 0;
        c_div(_attr7r, _attr7i, _add9r, _add9i, &_div10r, &_div10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul6r + _div10r; _add11i = _mul6i + _div10i;
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
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(j, 0, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang30r, _ang30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 2.0; _c34i = 0;
        double _pow35r = 0, _pow35i = 0;
        c_mul(j, 0, j, 0, &_pow35r, &_pow35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_pow35r, _pow35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang33r, _ang33i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul32r + _mul37r; _add38i = _mul32i + _mul37i;
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = j + _c39r; _add40i = 0 + _c39i;
        double _log41r = 0, _log41i = 0;
        c_log(_add40r, _add40i, &_log41r, &_log41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _add38r + _log41r; _add42i = _add38i + _log41i;
        double angle = _add42r; /* +_add42ii */
        double _cos43r = 0, _cos43i = 0;
        c_cos(angle, 0, &_cos43r, &_cos43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(angle, 0, &_sin44r, &_sin44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_sin44r, _sin44i, _c45r, _c45i, &_mul46r, &_mul46i);
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

static void poly_584_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _sin4r = 0, _sin4i = 0;
        c_sin(j, 0, &_sin4r, &_sin4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr3r, _attr3i, _sin4r, _sin4i, &_mul5r, &_mul5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _cos7r = 0, _cos7i = 0;
        c_cos(j, 0, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr6r, _attr6i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _mul8r; _add9i = _mul5i + _mul8i;
        double r = _add9r; /* +_add9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _cos11r = 0, _cos11i = 0;
        c_cos(j, 0, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr10r, _attr10i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2i; _attr13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(j, 0, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr13r, _attr13i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = _mul12r - _mul15r; _sub16i = _mul12i - _mul15i;
        double k = _sub16r; /* +_sub16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(r, 0); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 5.0; _c22i = 0;
        double _mod23r = 0, _mod23i = 0;
        _mod23r = fmod(j, _c22r); _mod23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _c21r + _mod23r; _add24i = _c21i + _mod23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log20r, _log20i, _add24r, _add24i, &_mul25r, &_mul25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(k, 0); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.5; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_powr(_abs26r, _abs26i, 1.5, &_pow28r, &_pow28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _pow28r; _add29i = _mul25i + _pow28i;
        double magnitude = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang30r, _ang30i, j, 0, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _sqrt33r = 0, _sqrt33i = 0;
        c_powr(j, 0, 0.5, &_sqrt33r, &_sqrt33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang32r, _ang32i, _sqrt33r, _sqrt33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
        double angle = _add35r; /* +_add35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, angle, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x1r; _conj40i = -(x1i);
        double _conj41r = 0, _conj41i = 0;
        _conj41r = x2r; _conj41i = -(x2i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj40r, _conj40i, _conj41r, _conj41i, &_mul42r, &_mul42i);
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

static void poly_585_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(j, 0, j, 0, &_pow4r, &_pow4i);
        double k = _pow4r; /* +_pow4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(k, 0, M_PI, 0, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 7.0; _c7i = 0;
        double _div8r = 0, _div8i = 0;
        c_div(_mul6r, _mul6i, _c7r, _c7i, &_div8r, &_div8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_div8r, _div8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr5r, _attr5i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 5.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_mul12r, _mul12i, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_attr11r, _attr11i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul10r + _mul16r; _add17i = _mul10i + _mul16i;
        double r = _add17r; /* +_add17ii */
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x1i; _attr18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(k, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr18r, _attr18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(k, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 4.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_attr24r, _attr24i, _sin28r, _sin28i, &_mul29r, &_mul29i);
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
        _add34r = _add33r + k; _add34i = _add33i + 0;
        double _log35r = 0, _log35i = 0;
        c_log(_add34r, _add34i, &_log35r, &_log35i);
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(r, 0); _abs36i = 0;
        double _abs37r = 0, _abs37i = 0;
        _abs37r = c_abs(s, 0); _abs37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _abs36r + _abs37r; _add38i = _abs36i + _abs37i;
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = _add38r + _c39r; _add40i = _add38i + _c39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_log35r, _log35i, _add40r, _add40i, &_mul41r, &_mul41i);
        double mag = _mul41r; /* +_mul41ii */
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x1r, x1i); _ang42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = k + _c43r; _add44i = 0 + _c43i;
        double _log45r = 0, _log45i = 0;
        c_log(_add44r, _add44i, &_log45r, &_log45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang42r, _ang42i, _log45r, _log45i, &_mul46r, &_mul46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(r, 0, &_sin47r, &_sin47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul46r + _sin47r; _add48i = _mul46i + _sin47i;
        double _cos49r = 0, _cos49i = 0;
        c_cos(s, 0, &_cos49r, &_cos49i);
        double _sub50r = 0, _sub50i = 0;
        _sub50r = _add48r - _cos49r; _sub50i = _add48i - _cos49i;
        double angle = _sub50r; /* +_sub50ii */
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
        c_mul(mag, 0, _add55r, _add55i, &_mul56r, &_mul56i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_586_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
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
        double mag_part1 = _mul10r; /* +_mul10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs11r + j; _add12i = _abs11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 5.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log13r, _log13i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double mag_part2 = _mul18r; /* +_mul18ii */
        double _add19r = 0, _add19i = 0;
        _add19r = mag_part1 + mag_part2; _add19i = 0 + 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _add19r + j; _add20i = _add19i + 0;
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
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang27r, _ang27i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double angle_part2 = _mul32r; /* +_mul32ii */
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
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = n + _c40r; _add41i = 0 + _c40i;
    for (int k = 1; k < (int)(_add41r); k++) {
        double _cf42r = 0, _cf42i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 0.05; _c44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 2.0; _c45i = 0;
        double _pow46r = 0, _pow46i = 0;
        c_mul(k, 0, k, 0, &_pow46r, &_pow46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c44r, _c44i, _pow46r, _pow46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _c43r + _mul47r; _add48i = _c43i + _mul47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_cf42r, _cf42i, _add48r, _add48i, &_mul49r, &_mul49i);
        double _neg50r = 0, _neg50i = 0;
        _neg50r = -(k); _neg50i = -(0);
        double _div51r = 0, _div51i = 0;
        c_div(_neg50r, _neg50i, n, 0, &_div51r, &_div51i);
        double _exp52r = 0, _exp52i = 0;
        c_exp2(_div51r, _div51i, &_exp52r, &_exp52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_mul49r, _mul49i, _exp52r, _exp52i, &_mul53r, &_mul53i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_587_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
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
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _attr3r + _mul5r; _add6i = _attr3i + _mul5i;
        double r1 = _add6r; /* +_add6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x1i; _attr7i = 0;
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sub10r = 0, _sub10i = 0;
        _sub10r = _attr7r - _mul9r; _sub10i = _attr7i - _mul9i;
        double i1 = _sub10r; /* +_sub10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs11r + j; _add12i = _abs11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x2r, x2i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log13r, _log13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 12.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul17r + _cos21r; _add22i = _mul17i + _cos21i;
        double magnitude = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _cos24r = 0, _cos24i = 0;
        c_cos(j, 0, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _ang26r, _ang26i, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _sin28r; _add29i = _mul25i + _sin28i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_588_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double degree = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = degree + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.3; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_powr(j, 0, 1.3, &_pow5r, &_pow5i);
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
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_sin11r, _sin11i); _abs12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log7r, _log7i, _abs12r, _abs12i, &_mul13r, &_mul13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 6.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_abs14r, _abs14i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul13r + _mul19r; _add20i = _mul13i + _mul19i;
        double mag = _add20r; /* +_add20ii */
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
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 7.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add31r + _sin35r; _add36i = _add31i + _sin35i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    double _c43r = 0, _c43i = 0;
    _c43r = 1.0; _c43i = 0;
    double _add44r = 0, _add44i = 0;
    _add44r = degree + _c43r; _add44i = 0 + _c43i;
    for (int k = (int)(_add44r); k < 36; k++) {
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = k + _c45r; _add46i = 0 + _c45i;
        double _log47r = 0, _log47i = 0;
        c_log(_add46r, _add46i, &_log47r, &_log47i);
        double _ang48r = 0, _ang48i = 0;
        _ang48r = c_arg(x1r, x1i); _ang48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(k, 0, _ang48r, _ang48i, &_mul49r, &_mul49i);
        double _sin50r = 0, _sin50i = 0;
        c_sin(_mul49r, _mul49i, &_sin50r, &_sin50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _c52r = 0, _c52i = 0;
        _c52r = 2.0; _c52i = 0;
        double _div53r = 0, _div53i = 0;
        c_div(k, 0, _c52r, _c52i, &_div53r, &_div53i);
        double _cos54r = 0, _cos54i = 0;
        c_cos(_div53r, _div53i, &_cos54r, &_cos54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c51r, _c51i, _cos54r, _cos54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _sin50r + _mul55r; _add56i = _sin50i + _mul55i;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_log47r, _log47i, _add56r, _add56i, &_mul57r, &_mul57i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_589_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 26; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double k = _add2r; /* +_add2ii */
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs4r + _c5r; _add6i = _abs4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr3r, _attr3i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _div11r = 0, _div11i = 0;
        c_div(_mul8r, _mul8i, _add10r, _add10i, &_div11r, &_div11i);
        double r = _div11r; /* +_div11ii */
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _sin13r = 0, _sin13i = 0;
        c_sin(j, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 8.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang15r, _ang15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul14r + _mul20r; _add21i = _mul14i + _mul20i;
        double theta = _add21r; /* +_add21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs22r, _abs22i, j, &_pow23r, &_pow23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 25.0; _c25i = 0;
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _c25r - j; _sub26i = _c25i - 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_abs24r, _abs24i, _sub26r, &_pow27r, &_pow27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _pow23r + _pow27r; _add28i = _pow23i + _pow27i;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_div32r, _div32i, &_sin33r, &_sin33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _c29r + _sin33r; _add34i = _c29i + _sin33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_add28r, _add28i, _add34r, _add34i, &_mul35r, &_mul35i);
        double mag = _mul35r; /* +_mul35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c36r, _c36i, theta, 0, &_mul37r, &_mul37i);
        double _exp38r = 0, _exp38i = 0;
        c_exp2(_mul37r, _mul37i, &_exp38r, &_exp38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(mag, 0, _exp38r, _exp38i, &_mul39r, &_mul39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x1r; _conj40i = -(x1i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, M_PI, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 7.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(_mul41r, _mul41i, _c42r, _c42i, &_div43r, &_div43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_div43r, _div43i, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_conj40r, _conj40i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul39r + _mul45r; _add46i = _mul39i + _mul45i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    }
    for (int k = 1; k < 26; k++) {
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 0.05; _c48i = 0;
        double _attr49r = 0, _attr49i = 0;
        _attr49r = x1r; _attr49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(k, 0, _attr49r, _attr49i, &_mul50r, &_mul50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(_mul50r, _mul50i, &_sin51r, &_sin51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c48r, _c48i, _sin51r, _sin51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _c47r + _mul52r; _add53i = _c47i + _mul52i;
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 0.05;
        double _attr55r = 0, _attr55i = 0;
        _attr55r = x2i; _attr55i = 0;
        double _mul56r = 0, _mul56i = 0;
        c_mul(k, 0, _attr55r, _attr55i, &_mul56r, &_mul56i);
        double _cos57r = 0, _cos57i = 0;
        c_cos(_mul56r, _mul56i, &_cos57r, &_cos57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_c54r, _c54i, _cos57r, _cos57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _add53r + _mul58r; _add59i = _add53i + _mul58i;
        cRe[(k - 1)] += _add59r; cIm[(k - 1)] += _add59i;
    }
    for (int r = 1; r < 26; r++) {
        double _abs60r = 0, _abs60i = 0;
        _abs60r = c_abs(x1r, x1i); _abs60i = 0;
        double _abs61r = 0, _abs61i = 0;
        _abs61r = c_abs(x2r, x2i); _abs61i = 0;
        double _prod62r = 0, _prod62i = 0;
        c_mul(_abs60r, _abs60i, _abs61r, _abs61i, &_prod62r, &_prod62i);
        double _c63r = 0, _c63i = 0;
        _c63r = 2.0; _c63i = 0;
        double _add64r = 0, _add64i = 0;
        _add64r = r + _c63r; _add64i = 0 + _c63i;
        double _div65r = 0, _div65i = 0;
        c_div(_prod62r, _prod62i, _add64r, _add64i, &_div65r, &_div65i);
        cRe[(r - 1)] += _div65r; cIm[(r - 1)] += _div65i;
        double _cf66r = 0, _cf66i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 25) { _cf66r = cRe[_idx]; _cf66i = cIm[_idx]; } }
        double _cf67r = 0, _cf67i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 25) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
        double _abs68r = 0, _abs68i = 0;
        _abs68r = c_abs(_cf67r, _cf67i); _abs68i = 0;
        double _c69r = 0, _c69i = 0;
        _c69r = 1.0; _c69i = 0;
        double _add70r = 0, _add70i = 0;
        _add70r = _abs68r + _c69r; _add70i = _abs68i + _c69i;
        double _log71r = 0, _log71i = 0;
        c_log(_add70r, _add70i, &_log71r, &_log71i);
        double _mul72r = 0, _mul72i = 0;
        c_mul(_cf66r, _cf66i, _log71r, _log71i, &_mul72r, &_mul72i);
        double _c73r = 0, _c73i = 0;
        _c73r = 0.0; _c73i = 1.0;
        double _sin74r = 0, _sin74i = 0;
        c_sin(r, 0, &_sin74r, &_sin74i);
        double _mul75r = 0, _mul75i = 0;
        c_mul(_c73r, _c73i, _sin74r, _sin74i, &_mul75r, &_mul75i);
        double _exp76r = 0, _exp76i = 0;
        c_exp2(_mul75r, _mul75i, &_exp76r, &_exp76i);
        double _add77r = 0, _add77i = 0;
        _add77r = _mul72r + _exp76r; _add77i = _mul72i + _exp76i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add77r; cIm[_idx] = _add77i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_590_c(double x1r, double x1i, double x2r, double x2i,
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
        c_mul(_attr3r, _attr3i, j, 0, &_mul4r, &_mul4i);
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
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr10r, _attr10i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2i; _attr13i = 0;
        double _cos14r = 0, _cos14i = 0;
        c_cos(j, 0, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr13r, _attr13i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _mul15r; _add16i = _mul12i + _mul15i;
        double i_part = _add16r; /* +_add16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(r_part, 0); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_abs21r, _abs21i, _abs22r, _abs22i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _div26r = 0, _div26i = 0;
        c_div(_mul23r, _mul23i, _add25r, _add25i, &_div26r, &_div26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _log20r + _div26r; _add27i = _log20i + _div26i;
        double magnitude = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang28r, _ang28i, j, 0, &_mul29r, &_mul29i);
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x2r, x2i); _ang30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = j + _c31r; _add32i = 0 + _c31i;
        double _div33r = 0, _div33i = 0;
        c_div(_ang30r, _ang30i, _add32r, _add32i, &_div33r, &_div33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul29r - _div33r; _sub34i = _mul29i - _div33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 5.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_div37r, _div37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _sub34r + _sin38r; _add39i = _sub34i + _sin38i;
        double angle = _add39r; /* +_add39ii */
        double _cos40r = 0, _cos40i = 0;
        c_cos(angle, 0, &_cos40r, &_cos40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(angle, 0, &_sin41r, &_sin41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_sin41r, _sin41i, _c42r, _c42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _cos40r + _mul43r; _add44i = _cos40i + _mul43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(magnitude, 0, _add44r, _add44i, &_mul45r, &_mul45i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    double _c46r = 0, _c46i = 0;
    _c46r = 2.0; _c46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = degree + _c46r; _add47i = 0 + _c46i;
    for (int k = 1; k < (int)(_add47r); k++) {
        double _c48r = 0, _c48i = 0;
        _c48r = 1.0; _c48i = 0;
        double _c49r = 0, _c49i = 0;
        _c49r = 0.05; _c49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 2.0; _c50i = 0;
        double _pow51r = 0, _pow51i = 0;
        c_mul(k, 0, k, 0, &_pow51r, &_pow51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c49r, _c49i, _pow51r, _pow51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _c48r + _mul52r; _add53i = _c48i + _mul52i;
        { double _tr = cRe[(k - 1)]*_add53r - cIm[(k - 1)]*_add53i; cIm[(k - 1)] = cRe[(k - 1)]*_add53i + cIm[(k - 1)]*_add53r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_591_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 3.0; _c3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _c3r, _c3i, &_mul4r, &_mul4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _attr5r; _add6i = _mul4i + _attr5i;
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2i; _attr7i = 0;
        double _sub8r = 0, _sub8i = 0;
        _sub8r = _add6r - _attr7r; _sub8i = _add6i - _attr7i;
        double k = _sub8r; /* +_sub8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _abs10r; _add11i = _abs9i + _abs10i;
        double _add12r = 0, _add12i = 0;
        _add12r = _add11r + j; _add12i = _add11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x2r, x2i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _sin16r + _cos19r; _add20i = _sin16i + _cos19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log13r, _log13i, _add20r, _add20i, &_mul21r, &_mul21i);
        double r = _mul21r; /* +_mul21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(k, 0, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(k, 0, _ang25r, _ang25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sin24r + _cos27r; _add28i = _sin24i + _cos27i;
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
        c_mul(r, 0, _add33r, _add33i, &_mul34r, &_mul34i);
        double _conj35r = 0, _conj35i = 0;
        _conj35r = x1r; _conj35i = -(x1i);
        double _pow36r = 0, _pow36i = 0;
        c_powr(_conj35r, _conj35i, k, &_pow36r, &_pow36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(j, 0, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_pow36r, _pow36i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul34r + _mul38r; _add39i = _mul34i + _mul38i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_592_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        double sum_val = _c3r; /* +_c3ii */
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double prod_val = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        for (int k = 1; k < (int)(_add6r); k++) {
            double _attr7r = 0, _attr7i = 0;
            _attr7r = x1r; _attr7i = 0;
            double _mul8r = 0, _mul8i = 0;
            c_mul(k, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
            double _sin9r = 0, _sin9i = 0;
            c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
            double _attr10r = 0, _attr10i = 0;
            _attr10r = x2i; _attr10i = 0;
            double _mul11r = 0, _mul11i = 0;
            c_mul(k, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
            double _cos12r = 0, _cos12i = 0;
            c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_sin9r, _sin9i, _cos12r, _cos12i, &_mul13r, &_mul13i);
            sum_val += _mul13r;
            double _attr14r = 0, _attr14i = 0;
            _attr14r = x1r; _attr14i = 0;
            double _attr15r = 0, _attr15i = 0;
            _attr15r = x2i; _attr15i = 0;
            double _mul16r = 0, _mul16i = 0;
            c_mul(_attr15r, _attr15i, k, 0, &_mul16r, &_mul16i);
            double _add17r = 0, _add17i = 0;
            _add17r = _attr14r + _mul16r; _add17i = _attr14i + _mul16i;
            prod_val *= _add17r;
        }
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs18r + j; _add19i = _abs18i + 0;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log20r, _log20i, sum_val, 0, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul21r + prod_val; _add22i = _mul21i + 0;
        double magnitude = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang23r, _ang23i, j, 0, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_mul(j, 0, j, 0, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang25r, _ang25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul24r - _mul28r; _sub29i = _mul24i - _mul28i;
        double angle = _sub29r; /* +_sub29ii */
        double _cos30r = 0, _cos30i = 0;
        c_cos(angle, 0, &_cos30r, &_cos30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(angle, 0, &_sin31r, &_sin31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_sin31r, _sin31i, _c32r, _c32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _cos30r + _mul33r; _add34i = _cos30i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(magnitude, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        double _conj36r = 0, _conj36i = 0;
        _conj36r = x1r; _conj36i = -(x1i);
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x2r; _conj37i = -(x2i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_conj36r, _conj36i, _conj37r, _conj37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul35r + _mul38r; _add39i = _mul35i + _mul38i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_593_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double deg = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = deg + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2r; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _c6r + _sin9r; _add10i = _c6i + _sin9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log5r, _log5i, _add10r, _add10i, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_mul14r, _mul14i, &_cos15r, &_cos15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c12r + _cos15r; _add16i = _c12i + _cos15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul11r, _mul11i, _add16r, _add16i, &_mul17r, &_mul17i);
        double mag = _mul17r; /* +_mul17ii */
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang18r, _ang18i, j, 0, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _sqrt21r = 0, _sqrt21i = 0;
        c_powr(j, 0, 0.5, &_sqrt21r, &_sqrt21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang20r, _ang20i, _sqrt21r, _sqrt21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul19r + _mul22r; _add23i = _mul19i + _mul22i;
        double ang = _add23r; /* +_add23ii */
        double _cos24r = 0, _cos24i = 0;
        c_cos(ang, 0, &_cos24r, &_cos24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(ang, 0, &_sin25r, &_sin25i);
        double _cplx26r = 0, _cplx26i = 0;
        _cplx26r = _cos24r; _cplx26i = _sin25r;
        double _mul27r = 0, _mul27i = 0;
        c_mul(mag, 0, _cplx26r, _cplx26i, &_mul27r, &_mul27i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_594_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(j, 0, j, 0, &_pow5r, &_pow5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(j, 0, &_sin6r, &_sin6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow5r + _sin6r; _add7i = _pow5i + _sin6i;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr3r, _attr3i, _add7r, _add7i, &_mul8r, &_mul8i);
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
        double r = _add14r; /* +_add14ii */
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(j, 0, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _cos18r + j; _add19i = _cos18i + 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_attr15r, _attr15i, _add19r, _add19i, &_mul20r, &_mul20i);
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2i; _attr21i = 0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(j, 0, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr21r, _attr21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
        double im = _add24r; /* +_add24ii */
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(im, 0, _c25r, _c25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = r + _mul26r; _add27i = 0 + _mul26i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_add27r, _add27i); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _c29r + _cos33r; _add34i = _c29i + _cos33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_abs28r, _abs28i, _add34r, _add34i, &_mul35r, &_mul35i);
        double mag = _mul35r; /* +_mul35ii */
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(im, 0, _c36r, _c36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = r + _mul37r; _add38i = 0 + _mul37i;
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(_add38r, _add38i); _ang39i = 0;
        double angle = _ang39r; /* +_ang39ii */
        double _cos40r = 0, _cos40i = 0;
        c_cos(angle, 0, &_cos40r, &_cos40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(angle, 0, &_sin41r, &_sin41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_sin41r, _sin41i, _c42r, _c42i, &_mul43r, &_mul43i);
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

static void poly_595_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add4r = 0, _add4i = 0;
        _add4r = _abs3r + j; _add4i = _abs3i + 0;
        double _log5r = 0, _log5i = 0;
        c_log(_add4r, _add4i, &_log5r, &_log5i);
        double _sqrt6r = 0, _sqrt6i = 0;
        c_powr(j, 0, 0.5, &_sqrt6r, &_sqrt6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_log5r, _log5i, _sqrt6r, _sqrt6i, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _sin9r = 0, _sin9i = 0;
        c_sin(j, 0, &_sin9r, &_sin9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _c8r + _sin9r; _add10i = _c8i + _sin9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_mul7r, _mul7i, _add10r, _add10i, &_mul11r, &_mul11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _div15r = 0, _div15i = 0;
        c_div(_abs12r, _abs12i, _add14r, _add14i, &_div15r, &_div15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul11r + _div15r; _add16i = _mul11i + _div15i;
        double magnitude = _add16r; /* +_add16ii */
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
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _add23r + _sin27r; _add28i = _add23i + _sin27i;
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
        c_mul(magnitude, 0, _add33r, _add33i, &_mul34r, &_mul34i);
        double _conj35r = 0, _conj35i = 0;
        _conj35r = x1r; _conj35i = -(x1i);
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, n, 0, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_conj35r, _conj35i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul34r + _mul38r; _add39i = _mul34i + _mul38i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_596_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 3.0; _c3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _c3r, _c3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 7.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 10.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(_add6r, _c7r); _mod8i = 0;
        double k = _mod8r; /* +_mod8ii */
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1r; _attr9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr9r, _attr9i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2i; _attr12i = 0;
        double _cos13r = 0, _cos13i = 0;
        c_cos(k, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr12r, _attr12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul11r + _mul14r; _add15i = _mul11i + _mul14i;
        double r = _add15r; /* +_add15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_mul(j, 0, j, 0, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _abs16r + _pow18r; _add19i = _abs16i + _pow18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 4.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log20r, _log20i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(r, 0, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul25r + _cos26r; _add27i = _mul25i + _cos26i;
        double mag = _add27r; /* +_add27ii */
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x1r, x1i); _ang28i = 0;
        double _cos29r = 0, _cos29i = 0;
        c_cos(j, 0, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang28r, _ang28i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(k, 0, _ang31r, _ang31i, &_mul32r, &_mul32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_mul32r, _mul32i, &_sin33r, &_sin33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul30r + _sin33r; _add34i = _mul30i + _sin33i;
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
        double _conj41r = 0, _conj41i = 0;
        _conj41r = x2r; _conj41i = -(x2i);
        double _pow42r = 0, _pow42i = 0;
        c_powr(_conj41r, _conj41i, k, &_pow42r, &_pow42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul40r + _pow42r; _add43i = _mul40i + _pow42i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 26) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    }
    double _unk44r = 0, _unk44i = 0;
    /* WARNING: unhandled node Slice(upper=Name(id='n', ctx=Load())) */
    double _cf45r = 0, _cf45i = 0;
    { int _idx = (int)(_unk44r); if (_idx >= 0 && _idx < 26) { _cf45r = cRe[_idx]; _cf45i = cIm[_idx]; } }
    double _abs46r = 0, _abs46i = 0;
    _abs46r = c_abs(_cf45r, _cf45i); _abs46i = 0;
    double _mul47r = 0, _mul47i = 0;
    c_mul(0, 0, M_PI, 0, &_mul47r, &_mul47i);
    double _c48r = 0, _c48i = 0;
    _c48r = 6.0; _c48i = 0;
    double _div49r = 0, _div49i = 0;
    c_div(_mul47r, _mul47i, _c48r, _c48i, &_div49r, &_div49i);
    double _cos50r = 0, _cos50i = 0;
    c_cos(_div49r, _div49i, &_cos50r, &_cos50i);
    double _mul51r = 0, _mul51i = 0;
    c_mul(_abs46r, _abs46i, _cos50r, _cos50i, &_mul51r, &_mul51i);
    double _abs52r = 0, _abs52i = 0;
    _abs52r = c_abs(x1r, x1i); _abs52i = 0;
    double _abs53r = 0, _abs53i = 0;
    _abs53r = c_abs(x2r, x2i); _abs53i = 0;
    double _prod54r = 0, _prod54i = 0;
    c_mul(_abs52r, _abs52i, _abs53r, _abs53i, &_prod54r, &_prod54i);
    double _add55r = 0, _add55i = 0;
    _add55r = _mul51r + _prod54r; _add55i = _mul51i + _prod54i;
    { int _idx = 25; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_597_c(double x1r, double x1i, double x2r, double x2i,
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
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = degree + _c5r; _add6i = 0 + _c5i;
    for (int j = 0; j < (int)(_add6r); j++) {
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 0;
        double mag = _c7r; /* +_c7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        double angle = _c8r; /* +_c8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        for (int k = 1; k < (int)(_add10r); k++) {
            double _abs11r = 0, _abs11i = 0;
            _abs11r = c_abs(x1r, x1i); _abs11i = 0;
            double _mul12r = 0, _mul12i = 0;
            c_mul(_abs11r, _abs11i, k, 0, &_mul12r, &_mul12i);
            double _c13r = 0, _c13i = 0;
            _c13r = 1.0; _c13i = 0;
            double _add14r = 0, _add14i = 0;
            _add14r = _mul12r + _c13r; _add14i = _mul12i + _c13i;
            double _log15r = 0, _log15i = 0;
            c_log(_add14r, _add14i, &_log15r, &_log15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(k, 0, M_PI, 0, &_mul16r, &_mul16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_mul16r, _mul16i, r1, 0, &_mul17r, &_mul17i);
            double _sin18r = 0, _sin18i = 0;
            c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(_log15r, _log15i, _sin18r, _sin18i, &_mul19r, &_mul19i);
            double _mul20r = 0, _mul20i = 0;
            c_mul(k, 0, M_PI, 0, &_mul20r, &_mul20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(_mul20r, _mul20i, i2, 0, &_mul21r, &_mul21i);
            double _cos22r = 0, _cos22i = 0;
            c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
            double _add23r = 0, _add23i = 0;
            _add23r = _mul19r + _cos22r; _add23i = _mul19i + _cos22i;
            double term_mag = _add23r; /* +_add23ii */
            double _ang24r = 0, _ang24i = 0;
            _ang24r = c_arg(x1r, x1i); _ang24i = 0;
            double _c25r = 0, _c25i = 0;
            _c25r = 2.0; _c25i = 0;
            double _pow26r = 0, _pow26i = 0;
            c_mul(k, 0, k, 0, &_pow26r, &_pow26i);
            double _mul27r = 0, _mul27i = 0;
            c_mul(_ang24r, _ang24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
            double _ang28r = 0, _ang28i = 0;
            _ang28r = c_arg(x2r, x2i); _ang28i = 0;
            double _sqrt29r = 0, _sqrt29i = 0;
            c_powr(k, 0, 0.5, &_sqrt29r, &_sqrt29i);
            double _mul30r = 0, _mul30i = 0;
            c_mul(_ang28r, _ang28i, _sqrt29r, _sqrt29i, &_mul30r, &_mul30i);
            double _sub31r = 0, _sub31i = 0;
            _sub31r = _mul27r - _mul30r; _sub31i = _mul27i - _mul30i;
            double term_angle = _sub31r; /* +_sub31ii */
            double _c32r = 0, _c32i = 0;
            _c32r = 0.0; _c32i = 1.0;
            double _mul33r = 0, _mul33i = 0;
            c_mul(_c32r, _c32i, term_angle, 0, &_mul33r, &_mul33i);
            double _exp34r = 0, _exp34i = 0;
            c_exp2(_mul33r, _mul33i, &_exp34r, &_exp34i);
            double _mul35r = 0, _mul35i = 0;
            c_mul(term_mag, 0, _exp34r, _exp34i, &_mul35r, &_mul35i);
            mag += _mul35r;
        }
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(degree, 0, _c36r, _c36i, &_div37r, &_div37i);
        if (j < _div37r) {
            double _c38r = 0, _c38i = 0;
            _c38r = 1.0; _c38i = 0;
            double _add39r = 0, _add39i = 0;
            _add39r = j + _c38r; _add39i = 0 + _c38i;
            mag *= _add39r;
        } else {
            double _c40r = 0, _c40i = 0;
            _c40r = 2.0; _c40i = 0;
            double _mul41r = 0, _mul41i = 0;
            c_mul(_c40r, _c40i, degree, 0, &_mul41r, &_mul41i);
            double _c42r = 0, _c42i = 0;
            _c42r = 3.0; _c42i = 0;
            double _div43r = 0, _div43i = 0;
            c_div(_mul41r, _mul41i, _c42r, _c42i, &_div43r, &_div43i);
            if (j < _div43r) {
                double _c44r = 0, _c44i = 0;
                _c44r = 1.0; _c44i = 0;
                double _add45r = 0, _add45i = 0;
                _add45r = j + _c44r; _add45i = 0 + _c44i;
            } else {
                double _c46r = 0, _c46i = 0;
                _c46r = 1.0; _c46i = 0;
                double _add47r = 0, _add47i = 0;
                _add47r = j + _c46r; _add47i = 0 + _c46i;
                double _c48r = 0, _c48i = 0;
                _c48r = 2.0; _c48i = 0;
                double _pow49r = 0, _pow49i = 0;
                c_mul(_add47r, _add47i, _add47r, _add47i, &_pow49r, &_pow49i);
                mag *= _pow49r;
            }
        }
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = mag; cIm[_idx] = 0; } }
    }
    double _attr50r = 0, _attr50i = 0;
    _attr50r = x1r; _attr50i = 0;
    double _attr51r = 0, _attr51i = 0;
    _attr51r = x2r; _attr51i = 0;
    double _mul52r = 0, _mul52i = 0;
    c_mul(_attr50r, _attr50i, _attr51r, _attr51i, &_mul52r, &_mul52i);
    double _c53r = 0, _c53i = 0;
    _c53r = 0.0; _c53i = 1.0;
    double _attr54r = 0, _attr54i = 0;
    _attr54r = x1i; _attr54i = 0;
    double _attr55r = 0, _attr55i = 0;
    _attr55r = x2i; _attr55i = 0;
    double _sub56r = 0, _sub56i = 0;
    _sub56r = _attr54r - _attr55r; _sub56i = _attr54i - _attr55i;
    double _mul57r = 0, _mul57i = 0;
    c_mul(_c53r, _c53i, _sub56r, _sub56i, &_mul57r, &_mul57i);
    double _add58r = 0, _add58i = 0;
    _add58r = _mul52r + _mul57r; _add58i = _mul52i + _mul57i;
    double _attr59r = 0, _attr59i = 0;
    _attr59r = x1r; _attr59i = 0;
    double _sin60r = 0, _sin60i = 0;
    c_sin(_attr59r, _attr59i, &_sin60r, &_sin60i);
    double _attr61r = 0, _attr61i = 0;
    _attr61r = x2i; _attr61i = 0;
    double _cos62r = 0, _cos62i = 0;
    c_cos(_attr61r, _attr61i, &_cos62r, &_cos62i);
    double _mul63r = 0, _mul63i = 0;
    c_mul(_sin60r, _sin60i, _cos62r, _cos62i, &_mul63r, &_mul63i);
    double _add64r = 0, _add64i = 0;
    _add64r = _add58r + _mul63r; _add64i = _add58i + _mul63i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_598_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x1r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = j + _c4r; _add5i = 0 + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr3r, _attr3i, _log6r, _log6i, &_mul7r, &_mul7i);
        double rec1 = _mul7r; /* +_mul7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 7.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr8r, _attr8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double rec2 = _mul13r; /* +_mul13ii */
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_attr14r, _attr14i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double imc1 = _mul19r; /* +_mul19ii */
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 3.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_div23r, _div23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_attr20r, _attr20i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double imc2 = _mul25r; /* +_mul25ii */
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs26r + j; _add27i = _abs26i + 0;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _mod31r = 0, _mod31i = 0;
        _mod31r = fmod(j, _c30r); _mod31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _c29r + _mod31r; _add32i = _c29i + _mod31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_log28r, _log28i, _add32r, _add32i, &_mul33r, &_mul33i);
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(x2r, x2i); _abs34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, _c35r, _c35i, &_div36r, &_div36i);
        double _pow37r = 0, _pow37i = 0;
        c_powr(_abs34r, _abs34i, _div36r, &_pow37r, &_pow37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul33r + _pow37r; _add38i = _mul33i + _pow37i;
        double mag = _add38r; /* +_add38ii */
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x1r, x1i); _ang39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 4.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(j, 0, _c40r, _c40i, &_div41r, &_div41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_div41r, _div41i, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang39r, _ang39i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 6.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(j, 0, _c45r, _c45i, &_div46r, &_div46i);
        double _cos47r = 0, _cos47i = 0;
        c_cos(_div46r, _div46i, &_cos47r, &_cos47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_ang44r, _ang44i, _cos47r, _cos47i, &_mul48r, &_mul48i);
        double _sub49r = 0, _sub49i = 0;
        _sub49r = _mul43r - _mul48r; _sub49i = _mul43i - _mul48i;
        double ang = _sub49r; /* +_sub49ii */
        double _add50r = 0, _add50i = 0;
        _add50r = rec1 + rec2; _add50i = 0 + 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _add52r = 0, _add52i = 0;
        _add52r = imc1 + imc2; _add52i = 0 + 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c51r, _c51i, _add52r, _add52i, &_mul53r, &_mul53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _add50r + _mul53r; _add54i = _add50i + _mul53i;
        double _c55r = 0, _c55i = 0;
        _c55r = 0.0; _c55i = 1.0;
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c55r, _c55i, ang, 0, &_mul56r, &_mul56i);
        double _exp57r = 0, _exp57i = 0;
        c_exp2(_mul56r, _mul56i, &_exp57r, &_exp57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(mag, 0, _exp57r, _exp57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _add54r + _mul58r; _add59i = _add54i + _mul58i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_599_c(double x1r, double x1i, double x2r, double x2i,
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
        c_mul(j, 0, _attr3r, _attr3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_sin5r, _sin5i, _cos8r, _cos8i, &_mul9r, &_mul9i);
        double part1 = _mul9r; /* +_mul9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs10r + j; _add11i = _abs10i + 0;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs13r + j; _add14i = _abs13i + 0;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _log12r + _log15r; _add16i = _log12i + _log15i;
        double part2 = _add16r; /* +_add16ii */
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x1r; _attr17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_attr17r, _attr17i, j, &_pow18r, &_pow18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_attr19r, _attr19i, j, &_pow20r, &_pow20i);
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _pow18r - _pow20r; _sub21i = _pow18i - _pow20i;
        double part3 = _sub21r; /* +_sub21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, j, 0, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2r; _attr24i = 0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(j, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_attr24r, _attr24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _mul23r + _mul26r; _add27i = _mul23i + _mul26i;
        double angle = _add27r; /* +_add27ii */
        double _mul28r = 0, _mul28i = 0;
        c_mul(part1, 0, part2, 0, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul28r + part3; _add29i = _mul28i + 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, angle, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_add29r, _add29i, _exp32r, _exp32i, &_mul33r, &_mul33i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_600_c(double x1r, double x1i, double x2r, double x2i,
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
    double rec_step[26];
    for (int _li = 0; _li < 26; _li++) {
        rec_step[_li] = _attr1r + (_attr2r - _attr1r) * _li / 25.0;
    }
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double imc_step[26];
    for (int _li = 0; _li < 26; _li++) {
        imc_step[_li] = _attr3r + (_attr4r - _attr3r) * _li / 25.0;
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
        _c11r = 12.0; _c11i = 0;
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
        _c16r = 8.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 26) ? rec_step[_idx] : 0.0; _arr19i = 0; }
        double _arr20r = 0, _arr20i = 0;
        { int _idx = (j - 1); _arr20r = (_idx >= 0 && _idx < 26) ? imc_step[_idx] : 0.0; _arr20i = 0; }
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _arr19r - _arr20r; _sub21i = _arr19i - _arr20i;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_sub21r, _sub21i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_cos18r, _cos18i, _log25r, _log25i, &_mul26r, &_mul26i);
        double mag_part2 = _mul26r; /* +_mul26ii */
        double _add27r = 0, _add27i = 0;
        _add27r = mag_part1 + mag_part2; _add27i = 0 + 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 0.8; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_powr(j, 0, 0.8, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add27r + _pow29r; _add30i = _add27i + _pow29i;
        double magnitude = _add30r; /* +_add30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 10.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang31r, _ang31i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double angle_part1 = _mul36r; /* +_mul36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 14.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
        double _cos41r = 0, _cos41i = 0;
        c_cos(_div40r, _div40i, &_cos41r, &_cos41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang37r, _ang37i, _cos41r, _cos41i, &_mul42r, &_mul42i);
        double angle_part2 = _mul42r; /* +_mul42ii */
        double _mul43r = 0, _mul43i = 0;
        c_mul(j, 0, M_PI, 0, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 6.0; _c44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(_mul43r, _mul43i, _c44r, _c44i, &_div45r, &_div45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_div45r, _div45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(j, 0, M_PI, 0, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 9.0; _c48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(_mul47r, _mul47i, _c48r, _c48i, &_div49r, &_div49i);
        double _cos50r = 0, _cos50i = 0;
        c_cos(_div49r, _div49i, &_cos50r, &_cos50i);
        double _sub51r = 0, _sub51i = 0;
        _sub51r = _sin46r - _cos50r; _sub51i = _sin46i - _cos50i;
        double angle_part3 = _sub51r; /* +_sub51ii */
        double _add52r = 0, _add52i = 0;
        _add52r = angle_part1 + angle_part2; _add52i = 0 + 0;
        double _add53r = 0, _add53i = 0;
        _add53r = _add52r + angle_part3; _add53i = _add52i + 0;
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
        c_mul(magnitude, 0, _add58r, _add58i, &_mul59r, &_mul59i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
