/* AUTO-GENERATED from poly600.py — do not edit manually */
/* 100 coefficient functions */

static void poly_501_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 5.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(j, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double _pow9r = 0, _pow9i = 0;
        c_powr(_abs4r, _abs4i, _add8r, &_pow9r, &_pow9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs10r + j; _add11i = _abs10i + 0;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_pow9r, _pow9i, _log12r, _log12i, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 0.5; _c14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _c14r, _c14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul13r, _mul13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.3; _c19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _c19r, _c19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang18r, _ang18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul17r + _mul22r; _add23i = _mul17i + _mul22i;
        double mag = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 0.5; _c25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, _c25r, _c25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 0.3; _c30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _c30r, _c30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _mul28r + _mul33r; _add34i = _mul28i + _mul33i;
        double angle = _add34r; /* +_add34ii */
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, angle, 0, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(mag, 0, _exp37r, _exp37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _mod40r = 0, _mod40i = 0;
        _mod40r = fmod(j, _c39r); _mod40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        if (_mod40r == _c41r) {
            double _conj42r = 0, _conj42i = 0;
            _conj42r = x1r; _conj42i = -(x1i);
            double _attr43r = 0, _attr43i = 0;
            _attr43r = x2r; _attr43i = 0;
            double _mul44r = 0, _mul44i = 0;
            c_mul(j, 0, _attr43r, _attr43i, &_mul44r, &_mul44i);
            double _sin45r = 0, _sin45i = 0;
            c_sin(_mul44r, _mul44i, &_sin45r, &_sin45i);
            double _mul46r = 0, _mul46i = 0;
            c_mul(_conj42r, _conj42i, _sin45r, _sin45i, &_mul46r, &_mul46i);
            double _conj47r = 0, _conj47i = 0;
            _conj47r = x2r; _conj47i = -(x2i);
            double _attr48r = 0, _attr48i = 0;
            _attr48r = x1i; _attr48i = 0;
            double _mul49r = 0, _mul49i = 0;
            c_mul(j, 0, _attr48r, _attr48i, &_mul49r, &_mul49i);
            double _cos50r = 0, _cos50i = 0;
            c_cos(_mul49r, _mul49i, &_cos50r, &_cos50i);
            double _mul51r = 0, _mul51i = 0;
            c_mul(_conj47r, _conj47i, _cos50r, _cos50i, &_mul51r, &_mul51i);
            double _sub52r = 0, _sub52i = 0;
            _sub52r = _mul46r - _mul51r; _sub52i = _mul46i - _mul51i;
            cRe[(j - 1)] += _sub52r; cIm[(j - 1)] += _sub52i;
        } else {
            double _c53r = 0, _c53i = 0;
            _c53r = 3.0; _c53i = 0;
            double _mod54r = 0, _mod54i = 0;
            _mod54r = fmod(j, _c53r); _mod54i = 0;
            double _c55r = 0, _c55i = 0;
            _c55r = 2.0; _c55i = 0;
            if (_mod54r == _c55r) {
                double _attr56r = 0, _attr56i = 0;
                _attr56r = x1r; _attr56i = 0;
                double _attr57r = 0, _attr57i = 0;
                _attr57r = x2i; _attr57i = 0;
                double _mul58r = 0, _mul58i = 0;
                c_mul(_attr56r, _attr56i, _attr57r, _attr57i, &_mul58r, &_mul58i);
                double _sin59r = 0, _sin59i = 0;
                c_sin(j, 0, &_sin59r, &_sin59i);
                double _mul60r = 0, _mul60i = 0;
                c_mul(_mul58r, _mul58i, _sin59r, _sin59i, &_mul60r, &_mul60i);
                cRe[(j - 1)] += _mul60r; cIm[(j - 1)] += _mul60i;
            } else {
                double _attr61r = 0, _attr61i = 0;
                _attr61r = x2r; _attr61i = 0;
                double _attr62r = 0, _attr62i = 0;
                _attr62r = x1i; _attr62i = 0;
                double _mul63r = 0, _mul63i = 0;
                c_mul(_attr61r, _attr61i, _attr62r, _attr62i, &_mul63r, &_mul63i);
                double _cos64r = 0, _cos64i = 0;
                c_cos(j, 0, &_cos64r, &_cos64i);
                double _mul65r = 0, _mul65i = 0;
                c_mul(_mul63r, _mul63i, _cos64r, _cos64i, &_mul65r, &_mul65i);
                cRe[(j - 1)] += _mul65r; cIm[(j - 1)] += _mul65i;
            }
        }
    }
    double _c66r = 0, _c66i = 0;
    _c66r = 1.0; _c66i = 0;
    double _add67r = 0, _add67i = 0;
    _add67r = n + _c66r; _add67i = 0 + _c66i;
    for (int k = 1; k < (int)(_add67r); k++) {
        double _cf68r = 0, _cf68i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf68r = cRe[_idx]; _cf68i = cIm[_idx]; } }
        double _c69r = 0, _c69i = 0;
        _c69r = 1.0; _c69i = 0;
        double _c70r = 0, _c70i = 0;
        _c70r = 0.05; _c70i = 0;
        double _mul71r = 0, _mul71i = 0;
        c_mul(_c70r, _c70i, k, 0, &_mul71r, &_mul71i);
        double _add72r = 0, _add72i = 0;
        _add72r = _c69r + _mul71r; _add72i = _c69i + _mul71i;
        double _mul73r = 0, _mul73i = 0;
        c_mul(_cf68r, _cf68i, _add72r, _add72i, &_mul73r, &_mul73i);
        double _c74r = 0, _c74i = 0;
        _c74r = 0.02; _c74i = 0;
        double _ang75r = 0, _ang75i = 0;
        _ang75r = c_arg(x1r, x1i); _ang75i = 0;
        double _mul76r = 0, _mul76i = 0;
        c_mul(k, 0, _ang75r, _ang75i, &_mul76r, &_mul76i);
        double _sin77r = 0, _sin77i = 0;
        c_sin(_mul76r, _mul76i, &_sin77r, &_sin77i);
        double _mul78r = 0, _mul78i = 0;
        c_mul(_c74r, _c74i, _sin77r, _sin77i, &_mul78r, &_mul78i);
        double _ang79r = 0, _ang79i = 0;
        _ang79r = c_arg(x2r, x2i); _ang79i = 0;
        double _mul80r = 0, _mul80i = 0;
        c_mul(k, 0, _ang79r, _ang79i, &_mul80r, &_mul80i);
        double _cos81r = 0, _cos81i = 0;
        c_cos(_mul80r, _mul80i, &_cos81r, &_cos81i);
        double _mul82r = 0, _mul82i = 0;
        c_mul(_mul78r, _mul78i, _cos81r, _cos81i, &_mul82r, &_mul82i);
        double _add83r = 0, _add83i = 0;
        _add83r = _mul73r + _mul82r; _add83i = _mul73i + _mul82i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add83r; cIm[_idx] = _add83i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_502_c(double x1r, double x1i, double x2r, double x2i,
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
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2r; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _sin6r + _cos9r; _add10i = _sin6i + _cos9i;
        double phase_r = _add10r; /* +_add10ii */
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1i; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x2i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _cos13r - _sin16r; _sub17i = _cos13i - _sin16i;
        double phase_i = _sub17r; /* +_sub17ii */
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
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _add23r + _c24r; _add25i = _add23i + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double magnitude = _log26r; /* +_log26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang27r, _ang27i, j, 0, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _sub30r = 0, _sub30i = 0;
        _sub30r = n - j; _sub30i = 0 - 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _sub30r, _sub30i, &_mul31r, &_mul31i);
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _mul28r - _mul31r; _sub32i = _mul28i - _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(phase_r, 0, phase_i, 0, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _sub32r + _mul33r; _add34i = _sub32i + _mul33i;
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

static void poly_503_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(j, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin12r, _sin12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c11r + _mul14r; _add15i = _c11i + _mul14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log10r, _log10i, _add15r, _add15i, &_mul16r, &_mul16i);
        double mag_part1 = _mul16r; /* +_mul16ii */
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
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, rec1, 0, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_mul24r, _mul24i, &_sin25r, &_sin25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _add23r + _sin25r; _add26i = _add23i + _sin25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, imc2, 0, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _add26r - _cos28r; _sub29i = _add26i - _cos28i;
        double mag_part2 = _sub29r; /* +_sub29ii */
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul30r, &_mul30i);
        double magnitude = _mul30r; /* +_mul30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang31r, _ang31i, j, 0, &_mul32r, &_mul32i);
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
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, rec1, 0, &_mul38r, &_mul38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_mul38r, _mul38i, &_sin39r, &_sin39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _add37r + _sin39r; _add40i = _add37i + _sin39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, imc2, 0, &_mul41r, &_mul41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_mul41r, _mul41i, &_cos42r, &_cos42i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = _add40r - _cos42r; _sub43i = _add40i - _cos42i;
        double angle = _sub43r; /* +_sub43ii */
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, angle, 0, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(magnitude, 0, _exp46r, _exp46i, &_mul47r, &_mul47i);
        double _conj48r = 0, _conj48i = 0;
        _conj48r = x1r; _conj48i = -(x1i);
        double _sin49r = 0, _sin49i = 0;
        c_sin(j, 0, &_sin49r, &_sin49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_conj48r, _conj48i, _sin49r, _sin49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul47r + _mul50r; _add51i = _mul47i + _mul50i;
        double _conj52r = 0, _conj52i = 0;
        _conj52r = x2r; _conj52i = -(x2i);
        double _cos53r = 0, _cos53i = 0;
        c_cos(j, 0, &_cos53r, &_cos53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_conj52r, _conj52i, _cos53r, _cos53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _add51r + _mul54r; _add55i = _add51i + _mul54i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_504_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
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
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_powr(_abs15r, _abs15i, j, &_pow16r, &_pow16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x2r, x2i); _abs17i = 0;
        double _sub18r = 0, _sub18i = 0;
        _sub18r = n - j; _sub18i = 0 - 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_abs17r, _abs17i, _sub18r, &_pow19r, &_pow19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _pow16r + _pow19r; _add20i = _pow16i + _pow19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log14r, _log14i, _add20r, _add20i, &_mul21r, &_mul21i);
        double magnitude = _mul21r; /* +_mul21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(j, 0, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    double _c37r = 0, _c37i = 0;
    _c37r = 2.0; _c37i = 0;
    double _div38r = 0, _div38i = 0;
    c_div(n, 0, _c37r, _c37i, &_div38r, &_div38i);
    double _flr39r = 0, _flr39i = 0;
    _flr39r = floor(_div38r); _flr39i = 0;
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = _flr39r + _c40r; _add41i = _flr39i + _c40i;
    for (int k = 1; k < (int)(_add41r); k++) {
        if (k <= n) {
            double _prod42r = 0, _prod42i = 0;
            /* WARNING: unhandled np.prod */
            double _conj43r = 0, _conj43i = 0;
            _conj43r = x2r; _conj43i = -(x2i);
            double _pow44r = 0, _pow44i = 0;
            c_powr(_conj43r, _conj43i, k, &_pow44r, &_pow44i);
            double _mul45r = 0, _mul45i = 0;
            c_mul(_prod42r, _prod42i, _pow44r, _pow44i, &_mul45r, &_mul45i);
            cRe[(k - 1)] += _mul45r; cIm[(k - 1)] += _mul45i;
            double _sum46r = 0, _sum46i = 0;
            /* WARNING: unhandled np.sum */
            double _abs47r = 0, _abs47i = 0;
            _abs47r = c_abs(x1r, x1i); _abs47i = 0;
            double _mul48r = 0, _mul48i = 0;
            c_mul(_abs47r, _abs47i, k, 0, &_mul48r, &_mul48i);
            double _sin49r = 0, _sin49i = 0;
            c_sin(_mul48r, _mul48i, &_sin49r, &_sin49i);
            double _mul50r = 0, _mul50i = 0;
            c_mul(_sum46r, _sum46i, _sin49r, _sin49i, &_mul50r, &_mul50i);
            double _abs51r = 0, _abs51i = 0;
            _abs51r = c_abs(x2r, x2i); _abs51i = 0;
            double _c52r = 0, _c52i = 0;
            _c52r = 1.0; _c52i = 0;
            double _add53r = 0, _add53i = 0;
            _add53r = k + _c52r; _add53i = 0 + _c52i;
            double _div54r = 0, _div54i = 0;
            c_div(_abs51r, _abs51i, _add53r, _add53i, &_div54r, &_div54i);
            double _cos55r = 0, _cos55i = 0;
            c_cos(_div54r, _div54i, &_cos55r, &_cos55i);
            double _mul56r = 0, _mul56i = 0;
            c_mul(_mul50r, _mul50i, _cos55r, _cos55i, &_mul56r, &_mul56i);
            cRe[((int)(n) - k)] += _mul56r; cIm[((int)(n) - k)] += _mul56i;
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_505_c(double x1r, double x1i, double x2r, double x2i,
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

static void poly_506_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _attr5r, _attr5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _attr4r + _mul6r; _add7i = _attr4i + _mul6i;
        double r = _add7r; /* +_add7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sub11r = 0, _sub11i = 0;
        _sub11r = _attr8r - _mul10r; _sub11i = _attr8i - _mul10i;
        double im = _sub11r; /* +_sub11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs12r + j; _add13i = _abs12i + 0;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
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
        double _add21r = 0, _add21i = 0;
        _add21r = _sin17r + _cos20r; _add21i = _sin17i + _cos20i;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_add21r, _add21i); _abs22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log14r, _log14i, _abs22r, _abs22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang24r, _ang24i, j, 0, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _sub27r = 0, _sub27i = 0;
        _sub27r = n - j; _sub27i = 0 - 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang26r, _ang26i, _sub27r, _sub27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _mul28r; _add29i = _mul25i + _mul28i;
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
        c_mul(mag, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    double _c36r = 0, _c36i = 0;
    _c36r = 1.0; _c36i = 0;
    double _add37r = 0, _add37i = 0;
    _add37r = n + _c36r; _add37i = 0 + _c36i;
    for (int k = 1; k < (int)(_add37r); k++) {
        double _cf38r = 0, _cf38i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
        double _conj39r = 0, _conj39i = 0;
        _conj39r = _cf38r; _conj39i = -(_cf38i);
        double _attr40r = 0, _attr40i = 0;
        _attr40r = x1r; _attr40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(k, 0, _attr40r, _attr40i, &_mul41r, &_mul41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_mul41r, _mul41i, &_sin42r, &_sin42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_conj39r, _conj39i, _sin42r, _sin42i, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = _c44r + k; _add45i = _c44i + 0;
        double _div46r = 0, _div46i = 0;
        c_div(_mul43r, _mul43i, _add45r, _add45i, &_div46r, &_div46i);
        cRe[(k - 1)] += _div46r; cIm[(k - 1)] += _div46i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_507_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
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
        double _mul10r = 0, _mul10i = 0;
        c_mul(_arr8r, _arr8i, _arr9r, _arr9i, &_mul10r, &_mul10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_mul10r, _mul10i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 4.0; _c15i = 0;
        double _mod16r = 0, _mod16i = 0;
        _mod16r = fmod(j, _c15r); _mod16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _mod16r + _c17r; _add18i = _mod16i + _c17i;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log14r, _log14i, _add18r, _add18i, &_mul19r, &_mul19i);
        double mag = _mul19r; /* +_mul19ii */
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1r; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul20r, _mul20i, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul24r, _mul24i, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sin23r + _cos27r; _add28i = _sin23i + _cos27i;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1r; _attr29i = 0;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(_attr29r, _attr29i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.5; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_abs30r, _abs30i, 0.5, &_pow32r, &_pow32i);
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x2i; _attr33i = 0;
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(_attr33r, _attr33i); _abs34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 0.3; _c35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_powr(_abs34r, _abs34i, 0.3, &_pow36r, &_pow36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_pow32r, _pow32i, _pow36r, _pow36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add28r + _mul37r; _add38i = _add28i + _mul37i;
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
    double _c45r = 0, _c45i = 0;
    _c45r = 1.0; _c45i = 0;
    double _add46r = 0, _add46i = 0;
    _add46r = n + _c45r; _add46i = 0 + _c45i;
    for (int k = 1; k < (int)(_add46r); k++) {
        double _cf47r = 0, _cf47i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf47r = cRe[_idx]; _cf47i = cIm[_idx]; } }
        double _c48r = 0, _c48i = 0;
        _c48r = 1.0; _c48i = 0;
        double _c49r = 0, _c49i = 0;
        _c49r = 0.5; _c49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 3.0; _c51i = 0;
        double _sub52r = 0, _sub52i = 0;
        _sub52r = k - _c51r; _sub52i = 0 - _c51i;
        double _max53r = 0, _max53i = 0;
        _max53r = fmax(_c50r, _sub52r); _max53i = 0;
        double _cf54r = 0, _cf54i = 0;
        { int _idx = (int)(_max53r); if (_idx >= 0 && _idx < 36) { _cf54r = cRe[_idx]; _cf54i = cIm[_idx]; } }
        double _conj55r = 0, _conj55i = 0;
        _conj55r = _cf54r; _conj55i = -(_cf54i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c49r, _c49i, _conj55r, _conj55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _c48r + _mul56r; _add57i = _c48i + _mul56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(_cf47r, _cf47i, _add57r, _add57i, &_mul58r, &_mul58i);
        double _c59r = 0, _c59i = 0;
        _c59r = 0.3; _c59i = 0;
        double _sin60r = 0, _sin60i = 0;
        c_sin(k, 0, &_sin60r, &_sin60i);
        double _mul61r = 0, _mul61i = 0;
        c_mul(_c59r, _c59i, _sin60r, _sin60i, &_mul61r, &_mul61i);
        double _cos62r = 0, _cos62i = 0;
        c_cos(k, 0, &_cos62r, &_cos62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(_mul61r, _mul61i, _cos62r, _cos62i, &_mul63r, &_mul63i);
        double _add64r = 0, _add64i = 0;
        _add64r = _mul58r + _mul63r; _add64i = _mul58i + _mul63i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
    }
    double _c65r = 0, _c65i = 0;
    _c65r = 1.0; _c65i = 0;
    double _add66r = 0, _add66i = 0;
    _add66r = n + _c65r; _add66i = 0 + _c65i;
    for (int r = 1; r < (int)(_add66r); r++) {
        double _cf67r = 0, _cf67i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
        double _c68r = 0, _c68i = 0;
        _c68r = 1.0; _c68i = 0;
        double _c69r = 0, _c69i = 0;
        _c69r = 0.5; _c69i = 0;
        double _c70r = 0, _c70i = 0;
        _c70r = 0.0; _c70i = 0;
        double _c71r = 0, _c71i = 0;
        _c71r = 2.0; _c71i = 0;
        double _sub72r = 0, _sub72i = 0;
        _sub72r = r - _c71r; _sub72i = 0 - _c71i;
        double _max73r = 0, _max73i = 0;
        _max73r = fmax(_c70r, _sub72r); _max73i = 0;
        double _cf74r = 0, _cf74i = 0;
        { int _idx = (int)(_max73r); if (_idx >= 0 && _idx < 36) { _cf74r = cRe[_idx]; _cf74i = cIm[_idx]; } }
        double _conj75r = 0, _conj75i = 0;
        _conj75r = _cf74r; _conj75i = -(_cf74i);
        double _mul76r = 0, _mul76i = 0;
        c_mul(_c69r, _c69i, _conj75r, _conj75i, &_mul76r, &_mul76i);
        double _add77r = 0, _add77i = 0;
        _add77r = _c68r + _mul76r; _add77i = _c68i + _mul76i;
        double _mul78r = 0, _mul78i = 0;
        c_mul(_cf67r, _cf67i, _add77r, _add77i, &_mul78r, &_mul78i);
        double _c79r = 0, _c79i = 0;
        _c79r = 0.3; _c79i = 0;
        double _sin80r = 0, _sin80i = 0;
        c_sin(r, 0, &_sin80r, &_sin80i);
        double _mul81r = 0, _mul81i = 0;
        c_mul(_c79r, _c79i, _sin80r, _sin80i, &_mul81r, &_mul81i);
        double _cos82r = 0, _cos82i = 0;
        c_cos(r, 0, &_cos82r, &_cos82i);
        double _mul83r = 0, _mul83i = 0;
        c_mul(_mul81r, _mul81i, _cos82r, _cos82i, &_mul83r, &_mul83i);
        double _add84r = 0, _add84i = 0;
        _add84r = _mul78r + _mul83r; _add84i = _mul78i + _mul83i;
        cRe[(r - 1)] += _add84r; cIm[(r - 1)] += _add84i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_508_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x1r, x1i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 4.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_ang6r, _ang6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 6.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang12r, _ang12i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul11r + _mul17r; _add18i = _mul11i + _mul17i;
        double angle_part = _add18r; /* +_add18ii */
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
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(j, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _c24r + _sin27r; _add28i = _c24i + _sin27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log23r, _log23i, _add28r, _add28i, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(j, 0, _c31r, _c31i, &_div32r, &_div32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _c30r + _cos33r; _add34i = _c30i + _cos33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_mul29r, _mul29i, _add34r, _add34i, &_mul35r, &_mul35i);
        double magnitude_part = _mul35r; /* +_mul35ii */
        double _conj36r = 0, _conj36i = 0;
        _conj36r = x1r; _conj36i = -(x1i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.5; _c37i = 0;
        double _pow38r = 0, _pow38i = 0;
        c_powr(_conj36r, _conj36i, 0.5, &_pow38r, &_pow38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(j, 0, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_pow38r, _pow38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _conj41r = 0, _conj41i = 0;
        _conj41r = x2r; _conj41i = -(x2i);
        double _c42r = 0, _c42i = 0;
        _c42r = 0.3; _c42i = 0;
        double _pow43r = 0, _pow43i = 0;
        c_powr(_conj41r, _conj41i, 0.3, &_pow43r, &_pow43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(j, 0, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_pow43r, _pow43i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _mul40r + _mul45r; _add46i = _mul40i + _mul45i;
        double perturbation = _add46r; /* +_add46ii */
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c47r, _c47i, angle_part, 0, &_mul48r, &_mul48i);
        double _exp49r = 0, _exp49i = 0;
        c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(magnitude_part, 0, _exp49r, _exp49i, &_mul50r, &_mul50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 0.1; _c51i = 0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(_c51r, _c51i, perturbation, 0, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul50r + _mul52r; _add53i = _mul50i + _mul52i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add53r; cIm[_idx] = _add53i; } }
    }
    for (int k = 1; k < 6; k++) {
        for (int r = 1; r < 8; r++) {
            double _c54r = 0, _c54i = 0;
            _c54r = 1.0; _c54i = 0;
            double _sub55r = 0, _sub55i = 0;
            _sub55r = k - _c54r; _sub55i = 0 - _c54i;
            double _c56r = 0, _c56i = 0;
            _c56r = 7.0; _c56i = 0;
            double _mul57r = 0, _mul57i = 0;
            c_mul(_sub55r, _sub55i, _c56r, _c56i, &_mul57r, &_mul57i);
            double _add58r = 0, _add58i = 0;
            _add58r = _mul57r + r; _add58i = _mul57i + 0;
            double index = _add58r; /* +_add58ii */
            if (index <= n) {
                double _cf59r = 0, _cf59i = 0;
                { int _idx = ((int)(index) - 1); if (_idx >= 0 && _idx < 36) { _cf59r = cRe[_idx]; _cf59i = cIm[_idx]; } }
                double _c60r = 0, _c60i = 0;
                _c60r = 1.0; _c60i = 0;
                double _c61r = 0, _c61i = 0;
                _c61r = 0.05; _c61i = 0;
                double _mul62r = 0, _mul62i = 0;
                c_mul(k, 0, r, 0, &_mul62r, &_mul62i);
                double _sin63r = 0, _sin63i = 0;
                c_sin(_mul62r, _mul62i, &_sin63r, &_sin63i);
                double _mul64r = 0, _mul64i = 0;
                c_mul(_c61r, _c61i, _sin63r, _sin63i, &_mul64r, &_mul64i);
                double _add65r = 0, _add65i = 0;
                _add65r = _c60r + _mul64r; _add65i = _c60i + _mul64i;
                double _mul66r = 0, _mul66i = 0;
                c_mul(_cf59r, _cf59i, _add65r, _add65i, &_mul66r, &_mul66i);
                double _c67r = 0, _c67i = 0;
                _c67r = 0.02; _c67i = 0;
                double _add68r = 0, _add68i = 0;
                _add68r = k + r; _add68i = 0 + 0;
                double _cos69r = 0, _cos69i = 0;
                c_cos(_add68r, _add68i, &_cos69r, &_cos69i);
                double _mul70r = 0, _mul70i = 0;
                c_mul(_c67r, _c67i, _cos69r, _cos69i, &_mul70r, &_mul70i);
                double _add71r = 0, _add71i = 0;
                _add71r = _mul66r + _mul70r; _add71i = _mul66i + _mul70i;
                { int _idx = ((int)(index) - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add71r; cIm[_idx] = _add71i; } }
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_509_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    for (int j = 1; j < 36; j++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x2r, x2i); _abs3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _abs2r + _abs3r; _add4i = _abs2i + _abs3i;
        double _add5r = 0, _add5i = 0;
        _add5r = _add4r + j; _add5i = _add4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 4.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(j, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 6.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(j, 0, _c11r, _c11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_sin10r, _sin10i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _c7r + _mul14r; _add15i = _c7i + _mul14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log6r, _log6i, _add15r, _add15i, &_mul16r, &_mul16i);
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
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _mul19r - _mul24r; _sub25i = _mul19i - _mul24i;
        double angle_part = _sub25r; /* +_sub25ii */
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
        double _conj32r = 0, _conj32i = 0;
        _conj32r = x1r; _conj32i = -(x1i);
        double _pow33r = 0, _pow33i = 0;
        c_powr(x2r, x2i, j, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_conj32r, _conj32i, _pow33r, _pow33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_510_c(double x1r, double x1i, double x2r, double x2i,
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
    double im1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2r; _attr4i = 0;
    double r2 = _attr4r; /* +_attr4ii */
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double im2 = _attr5r; /* +_attr5ii */
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
        double _sqrt13r = 0, _sqrt13i = 0;
        c_powr(j, 0, 0.5, &_sqrt13r, &_sqrt13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log12r, _log12i, _sqrt13r, _sqrt13i, &_mul14r, &_mul14i);
        double mag_part1 = _mul14r; /* +_mul14ii */
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(j, 0, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_abs15r, _abs15i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double mag_part2 = _mul19r; /* +_mul19ii */
        double _add20r = 0, _add20i = 0;
        _add20r = mag_part1 + mag_part2; _add20i = 0 + 0;
        double magnitude = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 5.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _ang21r + _cos25r; _add26i = _ang21i + _cos25i;
        double ang_part1 = _add26r; /* +_add26ii */
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, M_PI, 0, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 7.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_mul27r, _mul27i, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_sin30r, _sin30i, _ang31r, _ang31i, &_mul32r, &_mul32i);
        double ang_part2 = _mul32r; /* +_mul32ii */
        double _add33r = 0, _add33i = 0;
        _add33r = ang_part1 + ang_part2; _add33i = 0 + 0;
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

static void poly_511_c(double x1r, double x1i, double x2r, double x2i,
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
        double _sin4r = 0, _sin4i = 0;
        c_sin(j, 0, &_sin4r, &_sin4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1r; _attr5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_sin4r, _sin4i, _attr5r, _attr5i, &_mul6r, &_mul6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _c7r, _c7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2i; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_cos9r, _cos9i, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul6r + _mul11r; _add12i = _mul6i + _mul11i;
        double term1 = _add12r; /* +_add12ii */
        double _c13r = 0, _c13i = 0;
        _c13r = 3.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(j, 0, _c13r, _c13i, &_div14r, &_div14i);
        double _cos15r = 0, _cos15i = 0;
        c_cos(_div14r, _div14i, &_cos15r, &_cos15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_cos15r, _cos15i, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(j, 0, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1i; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin20r, _sin20i, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _mul17r - _mul22r; _sub23i = _mul17i - _mul22i;
        double term2 = _sub23r; /* +_sub23ii */
        double _add24r = 0, _add24i = 0;
        _add24r = term1 + term2; _add24i = 0 + 0;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_add24r, _add24i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log28r, _log28i, j, 0, &_mul29r, &_mul29i);
        double magnitude = _mul29r; /* +_mul29ii */
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
        double _add40r = 0, _add40i = 0;
        _add40r = _mul34r + _mul39r; _add40i = _mul34i + _mul39i;
        double _sin41r = 0, _sin41i = 0;
        c_sin(j, 0, &_sin41r, &_sin41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 2.0; _c42i = 0;
        double _pow43r = 0, _pow43i = 0;
        c_mul(_sin41r, _sin41i, _sin41r, _sin41i, &_pow43r, &_pow43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _add40r + _pow43r; _add44i = _add40i + _pow43i;
        double angle = _add44r; /* +_add44ii */
        double _c45r = 0, _c45i = 0;
        _c45r = 0.0; _c45i = 1.0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c45r, _c45i, angle, 0, &_mul46r, &_mul46i);
        double _exp47r = 0, _exp47i = 0;
        c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(magnitude, 0, _exp47r, _exp47i, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 0.3; _c49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _c51r = 0, _c51i = 0;
        _c51r = 2.0; _c51i = 0;
        double _div52r = 0, _div52i = 0;
        c_div(angle, 0, _c51r, _c51i, &_div52r, &_div52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c50r, _c50i, _div52r, _div52i, &_mul53r, &_mul53i);
        double _exp54r = 0, _exp54i = 0;
        c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c49r, _c49i, _exp54r, _exp54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _mul48r + _mul55r; _add56i = _mul48i + _mul55i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_512_c(double x1r, double x1i, double x2r, double x2i,
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
        double _sub10r = 0, _sub10i = 0;
        _sub10r = x1r - x2r; _sub10i = x1i - x2i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_sub10r, _sub10i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _abs11r; _add12i = 0 + _abs11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _add9r + _log13r; _add14i = _add9i + _log13i;
        double mag = _add14r; /* +_add14ii */
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
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _mul16r - _mul19r; _sub20i = _mul16i - _mul19i;
        double _sin21r = 0, _sin21i = 0;
        c_sin(j, 0, &_sin21r, &_sin21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(j, 0, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin21r, _sin21i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _sub20r + _mul23r; _add24i = _sub20i + _mul23i;
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
    for (int k = 1; k < 6; k++) {
        double _conj31r = 0, _conj31i = 0;
        _conj31r = x1r; _conj31i = -(x1i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(k, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_conj31r, _conj31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        { double _tr = cRe[(k - 1)]*_mul33r - cIm[(k - 1)]*_mul33i; cIm[(k - 1)] = cRe[(k - 1)]*_mul33i + cIm[(k - 1)]*_mul33r; cRe[(k - 1)] = _tr; }
    }
    for (int r = 1; r < 6; r++) {
        double _conj34r = 0, _conj34i = 0;
        _conj34r = x2r; _conj34i = -(x2i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(r, 0, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_conj34r, _conj34i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        { double _tr = cRe[((int)(n) - r)]*_mul36r - cIm[((int)(n) - r)]*_mul36i; cIm[((int)(n) - r)] = cRe[((int)(n) - r)]*_mul36i + cIm[((int)(n) - r)]*_mul36r; cRe[((int)(n) - r)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_513_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double real_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        real_seq[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imag_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imag_seq[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _add8r = 0, _add8i = 0;
        _add8r = x1r + j; _add8i = x1i + 0;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 7.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log12r, _log12i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 5.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_cos21r, _cos21i, 0, 0, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul17r + _mul22r; _add23i = _mul17i + _mul22i;
        double magnitude = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 3.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 4.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_sin28r, _sin28i, _cos32r, _cos32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _ang24r + _mul33r; _add34i = _ang24i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 6.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _tsin38r = 0, _tsin38i = 0;
        double _tcos39r = 0, _tcos39i = 0;
        double _tan40r = 0, _tan40i = 0;
        c_sin(_div37r, _div37i, &_tsin38r, &_tsin38i);
        c_cos(_div37r, _div37i, &_tcos39r, &_tcos39i);
        c_div(_tsin38r, _tsin38i, _tcos39r, _tcos39i, &_tan40r, &_tan40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _add34r + _tan40r; _add41i = _add34i + _tan40i;
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
    _c48r = 2.0; _c48i = 0;
    double _fdiv49r = 0, _fdiv49i = 0;
    c_div(n, 0, _c48r, _c48i, &_fdiv49r, &_fdiv49i);
    _fdiv49r = floor(_fdiv49r); _fdiv49i = 0;
    double _c50r = 0, _c50i = 0;
    _c50r = 1.0; _c50i = 0;
    double _add51r = 0, _add51i = 0;
    _add51r = _fdiv49r + _c50r; _add51i = _fdiv49i + _c50i;
    for (int k = 1; k < (int)(_add51r); k++) {
        double _c52r = 0, _c52i = 0;
        _c52r = 2.0; _c52i = 0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(k, 0, _c52r, _c52i, &_mul53r, &_mul53i);
        double idx = _mul53r; /* +_mul53ii */
        if (idx <= n) {
            double _c54r = 0, _c54i = 0;
            _c54r = 0.0; _c54i = 1.0;
            double _arr55r = 0, _arr55i = 0;
            { int _idx = (k - 1); _arr55r = (_idx >= 0 && _idx < 35) ? real_seq[_idx] : 0.0; _arr55i = 0; }
            double _mul56r = 0, _mul56i = 0;
            c_mul(_c54r, _c54i, _arr55r, _arr55i, &_mul56r, &_mul56i);
            double _arr57r = 0, _arr57i = 0;
            { int _idx = (k - 1); _arr57r = (_idx >= 0 && _idx < 35) ? imag_seq[_idx] : 0.0; _arr57i = 0; }
            double _abs58r = 0, _abs58i = 0;
            _abs58r = c_abs(_arr57r, _arr57i); _abs58i = 0;
            double _c59r = 0, _c59i = 0;
            _c59r = 1.0; _c59i = 0;
            double _add60r = 0, _add60i = 0;
            _add60r = _abs58r + _c59r; _add60i = _abs58i + _c59i;
            double _div61r = 0, _div61i = 0;
            c_div(_mul56r, _mul56i, _add60r, _add60i, &_div61r, &_div61i);
            double _exp62r = 0, _exp62i = 0;
            c_exp2(_div61r, _div61i, &_exp62r, &_exp62i);
            { double _tr = cRe[((int)(idx) - 1)]*_exp62r - cIm[((int)(idx) - 1)]*_exp62i; cIm[((int)(idx) - 1)] = cRe[((int)(idx) - 1)]*_exp62i + cIm[((int)(idx) - 1)]*_exp62r; cRe[((int)(idx) - 1)] = _tr; }
        }
    }
    double _c63r = 0, _c63i = 0;
    _c63r = 3.0; _c63i = 0;
    double _mod64r = 0, _mod64i = 0;
    _mod64r = fmod(n, _c63r); _mod64i = 0;
    double _c65r = 0, _c65i = 0;
    _c65r = 2.0; _c65i = 0;
    double _add66r = 0, _add66i = 0;
    _add66r = _mod64r + _c65r; _add66i = _mod64i + _c65i;
    for (int r = 1; r < (int)(_add66r); r++) {
        double _cf67r = 0, _cf67i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
        double _c68r = 0, _c68i = 0;
        _c68r = 2.0; _c68i = 0;
        double _pow69r = 0, _pow69i = 0;
        c_mul(_cf67r, _cf67i, _cf67r, _cf67i, &_pow69r, &_pow69i);
        double _c70r = 0, _c70i = 0;
        _c70r = 1.0; _c70i = 0;
        double _cf71r = 0, _cf71i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf71r = cRe[_idx]; _cf71i = cIm[_idx]; } }
        double _abs72r = 0, _abs72i = 0;
        _abs72r = c_abs(_cf71r, _cf71i); _abs72i = 0;
        double _add73r = 0, _add73i = 0;
        _add73r = _c70r + _abs72r; _add73i = _c70i + _abs72i;
        double _div74r = 0, _div74i = 0;
        c_div(_pow69r, _pow69i, _add73r, _add73i, &_div74r, &_div74i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div74r; cIm[_idx] = _div74i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_514_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int r = 1; r < (int)(_add7r); r++) {
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _abs9r; _add10i = _abs8i + _abs9i;
        double _add11r = 0, _add11i = 0;
        _add11r = _add10r + r; _add11i = _add10i + 0;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _arr13r = 0, _arr13i = 0;
        { int _idx = (r - 1); _arr13r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr13i = 0; }
        double _mul14r = 0, _mul14i = 0;
        c_mul(_arr13r, _arr13i, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = r + _c15r; _add16i = 0 + _c15i;
        double _div17r = 0, _div17i = 0;
        c_div(_mul14r, _mul14i, _add16r, _add16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (r - 1); _arr19r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr19i = 0; }
        double _mul20r = 0, _mul20i = 0;
        c_mul(_arr19r, _arr19i, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = r + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_mul20r, _mul20i, _add22r, _add22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _sin18r + _cos24r; _add25i = _sin18i + _cos24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log12r, _log12i, _add25r, _add25i, &_mul26r, &_mul26i);
        double mag_part = _mul26r; /* +_mul26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 5.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(r, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang27r, _ang27i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 7.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(r, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul31r + _mul36r; _add37i = _mul31i + _mul36i;
        double angle_part = _add37r; /* +_add37ii */
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 0;
        double intricate_sum = _c38r; /* +_c38ii */
        for (int j = 1; j < 4; j++) {
            double _attr39r = 0, _attr39i = 0;
            _attr39r = x1r; _attr39i = 0;
            double _pow40r = 0, _pow40i = 0;
            c_powr(_attr39r, _attr39i, j, &_pow40r, &_pow40i);
            double _attr41r = 0, _attr41i = 0;
            _attr41r = x2i; _attr41i = 0;
            double _pow42r = 0, _pow42i = 0;
            c_powr(_attr41r, _attr41i, j, &_pow42r, &_pow42i);
            double _sub43r = 0, _sub43i = 0;
            _sub43r = _pow40r - _pow42r; _sub43i = _pow40i - _pow42i;
            double _arr44r = 0, _arr44i = 0;
            { int _idx = (r - 1); _arr44r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr44i = 0; }
            double _mul45r = 0, _mul45i = 0;
            c_mul(j, 0, _arr44r, _arr44i, &_mul45r, &_mul45i);
            double _sin46r = 0, _sin46i = 0;
            c_sin(_mul45r, _mul45i, &_sin46r, &_sin46i);
            double _mul47r = 0, _mul47i = 0;
            c_mul(_sub43r, _sub43i, _sin46r, _sin46i, &_mul47r, &_mul47i);
            double _arr48r = 0, _arr48i = 0;
            { int _idx = (r - 1); _arr48r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr48i = 0; }
            double _mul49r = 0, _mul49i = 0;
            c_mul(j, 0, _arr48r, _arr48i, &_mul49r, &_mul49i);
            double _cos50r = 0, _cos50i = 0;
            c_cos(_mul49r, _mul49i, &_cos50r, &_cos50i);
            double _mul51r = 0, _mul51i = 0;
            c_mul(_mul47r, _mul47i, _cos50r, _cos50i, &_mul51r, &_mul51i);
            intricate_sum += _mul51r;
        }
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c52r, _c52i, angle_part, 0, &_mul53r, &_mul53i);
        double _exp54r = 0, _exp54i = 0;
        c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(mag_part, 0, _exp54r, _exp54i, &_mul55r, &_mul55i);
        double _conj56r = 0, _conj56i = 0;
        _conj56r = x1r; _conj56i = -(x1i);
        double _mul57r = 0, _mul57i = 0;
        c_mul(intricate_sum, 0, _conj56r, _conj56i, &_mul57r, &_mul57i);
        double _pow58r = 0, _pow58i = 0;
        c_powr(x2r, x2i, r, &_pow58r, &_pow58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(_mul57r, _mul57i, _pow58r, _pow58i, &_mul59r, &_mul59i);
        double _add60r = 0, _add60i = 0;
        _add60r = _mul55r + _mul59r; _add60i = _mul55i + _mul59i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add60r; cIm[_idx] = _add60i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_515_c(double x1r, double x1i, double x2r, double x2i,
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
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _attr6r, _attr6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _cos8r; _add9i = _mul5i + _cos8i;
        double _sin10r = 0, _sin10i = 0;
        c_sin(_add9r, _add9i, &_sin10r, &_sin10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2r; _attr11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = _mul12r - _sin15r; _sub16i = _mul12i - _sin15i;
        double _cos17r = 0, _cos17i = 0;
        c_cos(_sub16r, _sub16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _sin10r + _cos17r; _add18i = _sin10i + _cos17i;
        double phase = _add18r; /* +_add18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _sqrt23r = 0, _sqrt23i = 0;
        c_powr(j, 0, 0.5, &_sqrt23r, &_sqrt23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2r; _attr24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sqrt23r, _sqrt23i, _attr24r, _attr24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _log22r + _mul25r; _add26i = _log22i + _mul25i;
        double _attr27r = 0, _attr27i = 0;
        _attr27r = x1i; _attr27i = 0;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_attr27r, _attr27i); _abs28i = 0;
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _add26r - _abs28r; _sub29i = _add26i - _abs28i;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x1r; _attr30i = 0;
        double _attr31r = 0, _attr31i = 0;
        _attr31r = x2r; _attr31i = 0;
        double _prod32r = 0, _prod32i = 0;
        c_mul(_attr30r, _attr30i, _attr31r, _attr31i, &_prod32r, &_prod32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _div35r = 0, _div35i = 0;
        c_div(_prod32r, _prod32i, _add34r, _add34i, &_div35r, &_div35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _sub29r + _div35r; _add36i = _sub29i + _div35i;
        double magnitude = _add36r; /* +_add36ii */
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

static void poly_516_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double rec = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2i; _attr3i = 0;
    double imc = _attr3r; /* +_attr3ii */
    double _c4r = 0, _c4i = 0;
    _c4r = 1.0; _c4i = 0;
    double _add5r = 0, _add5i = 0;
    _add5r = n + _c4r; _add5i = 0 + _c4i;
    for (int j = 1; j < (int)(_add5r); j++) {
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
        double mag_part = _add19r; /* +_add19ii */
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 0.5; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(j, 0, 0.5, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang20r, _ang20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang24r, _ang24i, _log27r, _log27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul23r + _mul28r; _add29i = _mul23i + _mul28i;
        double angle_part = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, angle_part, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(mag_part, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        double _conj34r = 0, _conj34i = 0;
        _conj34r = x1r; _conj34i = -(x1i);
        double _pow35r = 0, _pow35i = 0;
        c_powr(_conj34r, _conj34i, j, &_pow35r, &_pow35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(j, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 5.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_pow35r, _pow35i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul33r + _mul40r; _add41i = _mul33i + _mul40i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_517_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int r = 1; r < (int)(_add3r); r++) {
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
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(r, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _c11r + r; _add12i = _c11i + 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul10r, _mul10i, _add12r, _add12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log8r, _log8i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1i; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(r, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _c18r + r; _add19i = _c18i + 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul17r, _mul17i, _add19r, _add19i, &_div20r, &_div20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_div20r, _div20i, &_cos21r, &_cos21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul15r + _cos21r; _add22i = _mul15i + _cos21i;
        double mag_part = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang23r, _ang23i, r, 0, &_mul24r, &_mul24i);
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x2r, x2i); _ang25i = 0;
        double _sub26r = 0, _sub26i = 0;
        _sub26r = n - r; _sub26i = 0 - 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _sub26r, _sub26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul24r + _mul27r; _add28i = _mul24i + _mul27i;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1r; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(r, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_mul30r, _mul30i, &_sin31r, &_sin31i);
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2i; _attr32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(r, 0, _attr32r, _attr32i, &_mul33r, &_mul33i);
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
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _neg42r = 0, _neg42i = 0;
        _neg42r = -(_c41r); _neg42i = -(_c41i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_neg42r, _neg42i, angle_part, 0, &_mul43r, &_mul43i);
        double _exp44r = 0, _exp44i = 0;
        c_exp2(_mul43r, _mul43i, &_exp44r, &_exp44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(mag_part, 0, _exp44r, _exp44i, &_mul45r, &_mul45i);
        double _conj46r = 0, _conj46i = 0;
        _conj46r = _mul45r; _conj46i = -(_mul45i);
        double _add47r = 0, _add47i = 0;
        _add47r = _mul40r + _conj46r; _add47i = _mul40i + _conj46i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_518_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(j, 0, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin12r + _cos13r; _add14i = _sin12i + _cos13i;
        double _pow15r = 0, _pow15i = 0;
        c_powr(j, 0, _add14r, &_pow15r, &_pow15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log11r, _log11i, _pow15r, _pow15i, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 7.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(_div19r, _div19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, M_PI, 0, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 5.0; _c22i = 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul21r, _mul21i, _c22r, _c22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin20r, _sin20i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul16r + _mul25r; _add26i = _mul16i + _mul25i;
        double mag = _add26r; /* +_add26ii */
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 3.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(j, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang27r, _ang27i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 4.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(j, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _cos35r = 0, _cos35i = 0;
        c_cos(_div34r, _div34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul31r + _mul36r; _add37i = _mul31i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 6.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(_mul38r, _mul38i, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _add37r + _sin41r; _add42i = _add37i + _sin41i;
        double angle = _add42r; /* +_add42ii */
        double _c43r = 0, _c43i = 0;
        _c43r = 5.0; _c43i = 0;
        double _min44r = 0, _min44i = 0;
        _min44r = fmin(j, _c43r); _min44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = _min44r + _c45r; _add46i = _min44i + _c45i;
        for (int k = 1; k < (int)(_add46r); k++) {
            double _pow47r = 0, _pow47i = 0;
            c_powr(r1, 0, k, &_pow47r, &_pow47i);
            double _sub48r = 0, _sub48i = 0;
            _sub48r = j - k; _sub48i = 0 - 0;
            double _pow49r = 0, _pow49i = 0;
            c_powr(i2, 0, _sub48r, &_pow49r, &_pow49i);
            double _mul50r = 0, _mul50i = 0;
            c_mul(_pow47r, _pow47i, _pow49r, _pow49i, &_mul50r, &_mul50i);
            double _c51r = 0, _c51i = 0;
            _c51r = 1.0; _c51i = 0;
            double _add52r = 0, _add52i = 0;
            _add52r = k + _c51r; _add52i = 0 + _c51i;
            double _log53r = 0, _log53i = 0;
            c_log(_add52r, _add52i, &_log53r, &_log53i);
            double _mul54r = 0, _mul54i = 0;
            c_mul(_mul50r, _mul50i, _log53r, _log53i, &_mul54r, &_mul54i);
            mag += _mul54r;
            double _ang55r = 0, _ang55i = 0;
            _ang55r = c_arg(x1r, x1i); _ang55i = 0;
            double _pow56r = 0, _pow56i = 0;
            c_powr(_ang55r, _ang55i, k, &_pow56r, &_pow56i);
            double _ang57r = 0, _ang57i = 0;
            _ang57r = c_arg(x2r, x2i); _ang57i = 0;
            double _sub58r = 0, _sub58i = 0;
            _sub58r = j - k; _sub58i = 0 - 0;
            double _pow59r = 0, _pow59i = 0;
            c_powr(_ang57r, _ang57i, _sub58r, &_pow59r, &_pow59i);
            double _mul60r = 0, _mul60i = 0;
            c_mul(k, 0, M_PI, 0, &_mul60r, &_mul60i);
            double _c61r = 0, _c61i = 0;
            _c61r = 8.0; _c61i = 0;
            double _div62r = 0, _div62i = 0;
            c_div(_mul60r, _mul60i, _c61r, _c61i, &_div62r, &_div62i);
            double _cos63r = 0, _cos63i = 0;
            c_cos(_div62r, _div62i, &_cos63r, &_cos63i);
            double _mul64r = 0, _mul64i = 0;
            c_mul(_pow59r, _pow59i, _cos63r, _cos63i, &_mul64r, &_mul64i);
            double _sub65r = 0, _sub65i = 0;
            _sub65r = _pow56r - _mul64r; _sub65i = _pow56i - _mul64i;
            angle += _sub65r;
        }
        double _c66r = 0, _c66i = 0;
        _c66r = 0.0; _c66i = 1.0;
        double _mul67r = 0, _mul67i = 0;
        c_mul(_c66r, _c66i, angle, 0, &_mul67r, &_mul67i);
        double _exp68r = 0, _exp68i = 0;
        c_exp2(_mul67r, _mul67i, &_exp68r, &_exp68i);
        double _mul69r = 0, _mul69i = 0;
        c_mul(mag, 0, _exp68r, _exp68i, &_mul69r, &_mul69i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul69r; cIm[_idx] = _mul69i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_519_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_sin10r, _sin10i); _abs11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log6r, _log6i, _abs11r, _abs11i, &_mul12r, &_mul12i);
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
        double mag = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _sin20r = 0, _sin20i = 0;
        c_sin(j, 0, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _sin20r, _sin20i, &_mul21r, &_mul21i);
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_520_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
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
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 5.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_div18r, _div18i, &_sin19r, &_sin19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _c15r + _sin19r; _add20i = _c15i + _sin19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log14r, _log14i, _add20r, _add20i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 7.0; _c24i = 0;
        double _div25r = 0, _div25i = 0;
        c_div(_mul23r, _mul23i, _c24r, _c24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _c22r + _cos26r; _add27i = _c22i + _cos26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul21r, _mul21i, _add27r, _add27i, &_mul28r, &_mul28i);
        double magnitude = _mul28r; /* +_mul28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 3.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(j, 0, _c30r, _c30i, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 4.0; _c35i = 0;
        double _div36r = 0, _div36i = 0;
        c_div(j, 0, _c35r, _c35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang34r, _ang34i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul33r + _mul38r; _add39i = _mul33i + _mul38i;
        double _attr40r = 0, _attr40i = 0;
        _attr40r = x1r; _attr40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(j, 0, _attr40r, _attr40i, &_mul41r, &_mul41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_mul41r, _mul41i, &_sin42r, &_sin42i);
        double _attr43r = 0, _attr43i = 0;
        _attr43r = x2i; _attr43i = 0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(j, 0, _attr43r, _attr43i, &_mul44r, &_mul44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_mul44r, _mul44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_sin42r, _sin42i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _add39r + _mul46r; _add47i = _add39i + _mul46i;
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
    double _c54r = 0, _c54i = 0;
    _c54r = 1.0; _c54i = 0;
    double _add55r = 0, _add55i = 0;
    _add55r = n + _c54r; _add55i = 0 + _c54i;
    for (int k = 1; k < (int)(_add55r); k++) {
        double _c56r = 0, _c56i = 0;
        _c56r = 1.0; _c56i = 0;
        double _c57r = 0, _c57i = 0;
        _c57r = 0.5; _c57i = 0;
        double _c58r = 0, _c58i = 0;
        _c58r = 0.0; _c58i = 0;
        double _c59r = 0, _c59i = 0;
        _c59r = 2.0; _c59i = 0;
        double _sub60r = 0, _sub60i = 0;
        _sub60r = k - _c59r; _sub60i = 0 - _c59i;
        double _max61r = 0, _max61i = 0;
        _max61r = fmax(_c58r, _sub60r); _max61i = 0;
        double _cf62r = 0, _cf62i = 0;
        { int _idx = (int)(_max61r); if (_idx >= 0 && _idx < 36) { _cf62r = cRe[_idx]; _cf62i = cIm[_idx]; } }
        double _conj63r = 0, _conj63i = 0;
        _conj63r = _cf62r; _conj63i = -(_cf62i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(_c57r, _c57i, _conj63r, _conj63i, &_mul64r, &_mul64i);
        double _add65r = 0, _add65i = 0;
        _add65r = _c56r + _mul64r; _add65i = _c56i + _mul64i;
        double _c66r = 0, _c66i = 0;
        _c66r = 0.3; _c66i = 0;
        double _sin67r = 0, _sin67i = 0;
        c_sin(k, 0, &_sin67r, &_sin67i);
        double _mul68r = 0, _mul68i = 0;
        c_mul(_c66r, _c66i, _sin67r, _sin67i, &_mul68r, &_mul68i);
        double _cos69r = 0, _cos69i = 0;
        c_cos(k, 0, &_cos69r, &_cos69i);
        double _mul70r = 0, _mul70i = 0;
        c_mul(_mul68r, _mul68i, _cos69r, _cos69i, &_mul70r, &_mul70i);
        double _add71r = 0, _add71i = 0;
        _add71r = _add65r + _mul70r; _add71i = _add65i + _mul70i;
        { double _tr = cRe[(k - 1)]*_add71r - cIm[(k - 1)]*_add71i; cIm[(k - 1)] = cRe[(k - 1)]*_add71i + cIm[(k - 1)]*_add71r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_521_c(double x1r, double x1i, double x2r, double x2i,
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
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(j, 0, _c10r, _c10i, &_div11r, &_div11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_div11r, _div11i, _ang12r, _ang12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _sin9r + _cos14r; _add15i = _sin9i + _cos14i;
        double mag_part2 = _add15r; /* +_add15ii */
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_mul(mag_part2, 0, mag_part2, 0, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _c16r + _pow18r; _add19i = _c16i + _pow18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(mag_part1, 0, _add19r, _add19i, &_mul20r, &_mul20i);
        double magnitude = _mul20r; /* +_mul20ii */
        double _add21r = 0, _add21i = 0;
        _add21r = x1r + j; _add21i = x1i + 0;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(_add21r, _add21i); _ang22i = 0;
        double angle_part1 = _ang22r; /* +_ang22ii */
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x2i; _attr23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _attr23r, _attr23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double angle_part2 = _cos25r; /* +_cos25ii */
        double _add26r = 0, _add26i = 0;
        _add26r = angle_part1 + angle_part2; _add26i = 0 + 0;
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
        c_mul(magnitude, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_522_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        double angle = _c5r; /* +_c5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        for (int k = 1; k < (int)(_add7r); k++) {
            double _attr8r = 0, _attr8i = 0;
            _attr8r = x1r; _attr8i = 0;
            double _pow9r = 0, _pow9i = 0;
            c_powr(_attr8r, _attr8i, k, &_pow9r, &_pow9i);
            double _attr10r = 0, _attr10i = 0;
            _attr10r = x2r; _attr10i = 0;
            double _sub11r = 0, _sub11i = 0;
            _sub11r = j - k; _sub11i = 0 - 0;
            double _pow12r = 0, _pow12i = 0;
            c_powr(_attr10r, _attr10i, _sub11r, &_pow12r, &_pow12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_pow9r, _pow9i, _pow12r, _pow12i, &_mul13r, &_mul13i);
            double _attr14r = 0, _attr14i = 0;
            _attr14r = x1i; _attr14i = 0;
            double _pow15r = 0, _pow15i = 0;
            c_powr(_attr14r, _attr14i, k, &_pow15r, &_pow15i);
            double _attr16r = 0, _attr16i = 0;
            _attr16r = x2i; _attr16i = 0;
            double _sub17r = 0, _sub17i = 0;
            _sub17r = j - k; _sub17i = 0 - 0;
            double _pow18r = 0, _pow18i = 0;
            c_powr(_attr16r, _attr16i, _sub17r, &_pow18r, &_pow18i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(_pow15r, _pow15i, _pow18r, _pow18i, &_mul19r, &_mul19i);
            double _add20r = 0, _add20i = 0;
            _add20r = _mul13r + _mul19r; _add20i = _mul13i + _mul19i;
            mag += _add20r;
            double _ang21r = 0, _ang21i = 0;
            _ang21r = c_arg(x1r, x1i); _ang21i = 0;
            double _sin22r = 0, _sin22i = 0;
            c_sin(k, 0, &_sin22r, &_sin22i);
            double _mul23r = 0, _mul23i = 0;
            c_mul(_ang21r, _ang21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
            double _ang24r = 0, _ang24i = 0;
            _ang24r = c_arg(x2r, x2i); _ang24i = 0;
            double _sub25r = 0, _sub25i = 0;
            _sub25r = j - k; _sub25i = 0 - 0;
            double _cos26r = 0, _cos26i = 0;
            c_cos(_sub25r, _sub25i, &_cos26r, &_cos26i);
            double _mul27r = 0, _mul27i = 0;
            c_mul(_ang24r, _ang24i, _cos26r, _cos26i, &_mul27r, &_mul27i);
            double _sub28r = 0, _sub28i = 0;
            _sub28r = _mul23r - _mul27r; _sub28i = _mul23i - _mul27i;
            angle += _sub28r;
        }
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
        mag *= _log33r;
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

static void poly_523_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 6.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log10r, _log10i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double mag_part1 = _mul15r; /* +_mul15ii */
        double _pow16r = 0, _pow16i = 0;
        c_powr(r2, 0, j, &_pow16r, &_pow16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = n - j; _sub17i = 0 - 0;
        double _pow18r = 0, _pow18i = 0;
        c_powr(i1, 0, _sub17r, &_pow18r, &_pow18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _pow16r + _pow18r; _add19i = _pow16i + _pow18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_add19r, _add19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double mag_part2 = _mul24r; /* +_mul24ii */
        double _add25r = 0, _add25i = 0;
        _add25r = mag_part1 + mag_part2; _add25i = 0 + 0;
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
        _c32r = 5.0; _c32i = 0;
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
        _c38r = 7.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(_div39r, _div39i, &_sin40r, &_sin40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _add36r + _sin40r; _add41i = _add36i + _sin40i;
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
        _conj46r = x2r; _conj46i = -(x2i);
        double _c47r = 0, _c47i = 0;
        _c47r = 4.0; _c47i = 0;
        double _mod48r = 0, _mod48i = 0;
        _mod48r = fmod(j, _c47r); _mod48i = 0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_conj46r, _conj46i, _mod48r, _mod48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul45r + _mul49r; _add50i = _mul45i + _mul49i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_524_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add10r = 0, _add10i = 0;
        _add10r = _sin6r + _cos9r; _add10i = _sin6i + _cos9i;
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
        double _add16r = 0, _add16i = 0;
        _add16r = _add10r + _log15r; _add16i = _add10i + _log15i;
        double phase = _add16r; /* +_add16ii */
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_mul(j, 0, j, 0, &_pow18r, &_pow18i);
        double _sqrt19r = 0, _sqrt19i = 0;
        c_powr(j, 0, 0.5, &_sqrt19r, &_sqrt19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _pow18r + _sqrt19r; _add20i = _pow18i + _sqrt19i;
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_sin23r, _sin23i); _abs24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_add20r, _add20i, _abs24r, _abs24i, &_mul25r, &_mul25i);
        double _neg26r = 0, _neg26i = 0;
        _neg26r = -(j); _neg26i = -(0);
        double _c27r = 0, _c27i = 0;
        _c27r = 10.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_neg26r, _neg26i, _c27r, _c27i, &_div28r, &_div28i);
        double _exp29r = 0, _exp29i = 0;
        c_exp2(_div28r, _div28i, &_exp29r, &_exp29i);
        double _add30r = 0, _add30i = 0;
        _add30r = x1r + x2r; _add30i = x1i + x2i;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(_add30r, _add30i); _abs31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_exp29r, _exp29i, _abs31r, _abs31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul25r + _mul32r; _add33i = _mul25i + _mul32i;
        double magnitude = _add33r; /* +_add33ii */
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

static void poly_525_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _abs4r + _c5r; _add6i = _abs4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_attr8r, _attr8i); _abs9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(j, 0, _abs9r, &_pow10r, &_pow10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log7r, _log7i, _pow10r, _pow10i, &_mul11r, &_mul11i);
        double _sqrt12r = 0, _sqrt12i = 0;
        c_powr(j, 0, 0.5, &_sqrt12r, &_sqrt12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(0, 0, _sqrt12r, _sqrt12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul11r + _mul13r; _add14i = _mul11i + _mul13i;
        double mag_part = _add14r; /* +_add14ii */
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _sin16r = 0, _sin16i = 0;
        c_sin(j, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_ang15r, _ang15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _cos19r = 0, _cos19i = 0;
        c_cos(j, 0, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang18r, _ang18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
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
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin24r, _sin24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add21r + _mul28r; _add29i = _add21i + _mul28i;
        double angle_part = _add29r; /* +_add29ii */
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, angle_part, 0, &_mul31r, &_mul31i);
        double _exp32r = 0, _exp32i = 0;
        c_exp2(_mul31r, _mul31i, &_exp32r, &_exp32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(mag_part, 0, _exp32r, _exp32i, &_mul33r, &_mul33i);
        double coeff = _mul33r; /* +_mul33ii */
        for (int k = 1; k < 4; k++) {
            double _attr34r = 0, _attr34i = 0;
            _attr34r = x1r; _attr34i = 0;
            double _pow35r = 0, _pow35i = 0;
            c_powr(_attr34r, _attr34i, k, &_pow35r, &_pow35i);
            double _attr36r = 0, _attr36i = 0;
            _attr36r = x2i; _attr36i = 0;
            double _pow37r = 0, _pow37i = 0;
            c_powr(_attr36r, _attr36i, k, &_pow37r, &_pow37i);
            double _mul38r = 0, _mul38i = 0;
            c_mul(_pow35r, _pow35i, _pow37r, _pow37i, &_mul38r, &_mul38i);
            double _mul39r = 0, _mul39i = 0;
            c_mul(k, 0, j, 0, &_mul39r, &_mul39i);
            double _sin40r = 0, _sin40i = 0;
            c_sin(_mul39r, _mul39i, &_sin40r, &_sin40i);
            double _mul41r = 0, _mul41i = 0;
            c_mul(_mul38r, _mul38i, _sin40r, _sin40i, &_mul41r, &_mul41i);
            double _c42r = 0, _c42i = 0;
            _c42r = 1.0; _c42i = 0;
            double _add43r = 0, _add43i = 0;
            _add43r = k + _c42r; _add43i = 0 + _c42i;
            double _div44r = 0, _div44i = 0;
            c_div(_mul41r, _mul41i, _add43r, _add43i, &_div44r, &_div44i);
            coeff += _div44r;
        }
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x2r; _conj45i = -(x2i);
        double _attr46r = 0, _attr46i = 0;
        _attr46r = x1r; _attr46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 5.0; _c47i = 0;
        double _mod48r = 0, _mod48i = 0;
        _mod48r = fmod(j, _c47r); _mod48i = 0;
        double _c49r = 0, _c49i = 0;
        _c49r = 1.0; _c49i = 0;
        double _add50r = 0, _add50i = 0;
        _add50r = _mod48r + _c49r; _add50i = _mod48i + _c49i;
        double _pow51r = 0, _pow51i = 0;
        c_powr(_attr46r, _attr46i, _add50r, &_pow51r, &_pow51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_conj45r, _conj45i, _pow51r, _pow51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = coeff + _mul52r; _add53i = 0 + _mul52i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add53r; cIm[_idx] = _add53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_526_c(double x1r, double x1i, double x2r, double x2i,
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
        double im = _sub11r; /* +_sub11ii */
        double _c12r = 0, _c12i = 0;
        _c12r = 0.0; _c12i = 1.0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(im, 0, _c12r, _c12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = r + _mul13r; _add14i = 0 + _mul13i;
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
        double _div20r = 0, _div20i = 0;
        c_div(_mul19r, _mul19i, n, 0, &_div20r, &_div20i);
        double _sin21r = 0, _sin21i = 0;
        c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log18r, _log18i, _sin21r, _sin21i, &_mul22r, &_mul22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, M_PI, 0, &_mul23r, &_mul23i);
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
        double _mul29r = 0, _mul29i = 0;
        c_mul(r, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 7.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang28r, _ang28i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(r, 0, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 4.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(_mul35r, _mul35i, _c36r, _c36i, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_ang34r, _ang34i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul33r + _mul39r; _add40i = _mul33i + _mul39i;
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

static void poly_527_c(double x1r, double x1i, double x2r, double x2i,
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
        double angle = _add10r; /* +_add10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_abs11r, _abs11i, j, &_pow12r, &_pow12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _sub14r = 0, _sub14i = 0;
        _sub14r = n - j; _sub14i = 0 - 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs13r, _abs13i, _sub14r, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _pow12r + _pow15r; _add16i = _pow12i + _pow15i;
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
        double _add22r = 0, _add22i = 0;
        _add22r = _add16r + _log21r; _add22i = _add16i + _log21i;
        double magnitude = _add22r; /* +_add22ii */
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x1r; _attr23i = 0;
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
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin25r, _sin25i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x1i; _attr30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_attr30r, _attr30i, j, 0, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_mul31r, _mul31i, _c32r, _c32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul29r + _sin34r; _add35i = _mul29i + _sin34i;
        double phase = _add35r; /* +_add35ii */
        double _add36r = 0, _add36i = 0;
        _add36r = angle + phase; _add36i = 0 + 0;
        double _cos37r = 0, _cos37i = 0;
        c_cos(_add36r, _add36i, &_cos37r, &_cos37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _sub39r = 0, _sub39i = 0;
        _sub39r = angle - phase; _sub39i = 0 - 0;
        double _sin40r = 0, _sin40i = 0;
        c_sin(_sub39r, _sub39i, &_sin40r, &_sin40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c38r, _c38i, _sin40r, _sin40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos37r + _mul41r; _add42i = _cos37i + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude, 0, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
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
        double _cf48r = 0, _cf48i = 0;
        { int _idx = ((int)(n) - k); if (_idx >= 0 && _idx < 36) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
        double _conj49r = 0, _conj49i = 0;
        _conj49r = _cf48r; _conj49i = -(_cf48i);
        { double _tr = cRe[(k - 1)]*_conj49r - cIm[(k - 1)]*_conj49i; cIm[(k - 1)] = cRe[(k - 1)]*_conj49i + cIm[(k - 1)]*_conj49r; cRe[(k - 1)] = _tr; }
    }
    double _c50r = 0, _c50i = 0;
    _c50r = 1.0; _c50i = 0;
    double _add51r = 0, _add51i = 0;
    _add51r = n + _c50r; _add51i = 0 + _c50i;
    for (int r = 1; r < (int)(_add51r); r++) {
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _attr53r = 0, _attr53i = 0;
        _attr53r = x1r; _attr53i = 0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_attr53r, _attr53i, r, 0, &_mul54r, &_mul54i);
        double _attr55r = 0, _attr55i = 0;
        _attr55r = x2i; _attr55i = 0;
        double _c56r = 0, _c56i = 0;
        _c56r = 1.0; _c56i = 0;
        double _add57r = 0, _add57i = 0;
        _add57r = r + _c56r; _add57i = 0 + _c56i;
        double _div58r = 0, _div58i = 0;
        c_div(_attr55r, _attr55i, _add57r, _add57i, &_div58r, &_div58i);
        double _sub59r = 0, _sub59i = 0;
        _sub59r = _mul54r - _div58r; _sub59i = _mul54i - _div58i;
        double _mul60r = 0, _mul60i = 0;
        c_mul(_c52r, _c52i, _sub59r, _sub59i, &_mul60r, &_mul60i);
        double _exp61r = 0, _exp61i = 0;
        c_exp2(_mul60r, _mul60i, &_exp61r, &_exp61i);
        cRe[(r - 1)] += _exp61r; cIm[(r - 1)] += _exp61i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_528_c(double x1r, double x1i, double x2r, double x2i,
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
        _c4r = 1.5; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_powr(j, 0, 1.5, &_pow5r, &_pow5i);
        double k = _pow5r; /* +_pow5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = k + _c7r; _add8i = 0 + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_attr6r, _attr6i, _log9r, _log9i, &_mul10r, &_mul10i);
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x2r; _attr11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = k + _c12r; _add13i = 0 + _c12i;
        double _sqrt14r = 0, _sqrt14i = 0;
        c_powr(_add13r, _add13i, 0.5, &_sqrt14r, &_sqrt14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr11r, _attr11i, _sqrt14r, _sqrt14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul10r + _mul15r; _add16i = _mul10i + _mul15i;
        double r = _add16r; /* +_add16ii */
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x1i; _attr17i = 0;
        double _sin18r = 0, _sin18i = 0;
        c_sin(k, 0, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_attr17r, _attr17i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _cos21r = 0, _cos21i = 0;
        c_cos(k, 0, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_attr20r, _attr20i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul19r + _mul22r; _add23i = _mul19i + _mul22i;
        double im_part = _add23r; /* +_add23ii */
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_abs24r, _abs24i, k, 0, &_mul25r, &_mul25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x2r, x2i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = k + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_abs26r, _abs26i, _add28r, _add28i, &_div29r, &_div29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul25r + _div29r; _add30i = _mul25i + _div29i;
        double magnitude = _add30r; /* +_add30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 10.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(k, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 10.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(k, 0, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang36r, _ang36i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul35r + _mul40r; _add41i = _mul35i + _mul40i;
        double angle = _add41r; /* +_add41ii */
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, im_part, 0, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = r + _mul43r; _add44i = 0 + _mul43i;
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = _c45r + magnitude; _add46i = _c45i + 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_add44r, _add44i, _add46r, _add46i, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c48r, _c48i, angle, 0, &_mul49r, &_mul49i);
        double _exp50r = 0, _exp50i = 0;
        c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_mul47r, _mul47i, _exp50r, _exp50i, &_mul51r, &_mul51i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
    }
    for (int k = 1; k < 6; k++) {
        for (int r = 1; r < 8; r++) {
            double _c52r = 0, _c52i = 0;
            _c52r = 1.0; _c52i = 0;
            double _sub53r = 0, _sub53i = 0;
            _sub53r = k - _c52r; _sub53i = 0 - _c52i;
            double _c54r = 0, _c54i = 0;
            _c54r = 7.0; _c54i = 0;
            double _mul55r = 0, _mul55i = 0;
            c_mul(_sub53r, _sub53i, _c54r, _c54i, &_mul55r, &_mul55i);
            double _add56r = 0, _add56i = 0;
            _add56r = _mul55r + r; _add56i = _mul55i + 0;
            double index = _add56r; /* +_add56ii */
            if (index <= n) {
                double _c57r = 0, _c57i = 0;
                _c57r = 1.0; _c57i = 0;
                double _c58r = 0, _c58i = 0;
                _c58r = 0.05; _c58i = 0;
                double _mul59r = 0, _mul59i = 0;
                c_mul(k, 0, r, 0, &_mul59r, &_mul59i);
                double _sin60r = 0, _sin60i = 0;
                c_sin(_mul59r, _mul59i, &_sin60r, &_sin60i);
                double _mul61r = 0, _mul61i = 0;
                c_mul(_c58r, _c58i, _sin60r, _sin60i, &_mul61r, &_mul61i);
                double _add62r = 0, _add62i = 0;
                _add62r = _c57r + _mul61r; _add62i = _c57i + _mul61i;
                double _c63r = 0, _c63i = 0;
                _c63r = 0.02; _c63i = 0;
                double _add64r = 0, _add64i = 0;
                _add64r = k + r; _add64i = 0 + 0;
                double _cos65r = 0, _cos65i = 0;
                c_cos(_add64r, _add64i, &_cos65r, &_cos65i);
                double _mul66r = 0, _mul66i = 0;
                c_mul(_c63r, _c63i, _cos65r, _cos65i, &_mul66r, &_mul66i);
                double _add67r = 0, _add67i = 0;
                _add67r = _add62r + _mul66r; _add67i = _add62i + _mul66i;
                { double _tr = cRe[((int)(index) - 1)]*_add67r - cIm[((int)(index) - 1)]*_add67i; cIm[((int)(index) - 1)] = cRe[((int)(index) - 1)]*_add67i + cIm[((int)(index) - 1)]*_add67r; cRe[((int)(index) - 1)] = _tr; }
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_529_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add8r = 0, _add8i = 0;
        _add8r = r1 + j; _add8i = 0 + 0;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
        double _log10r = 0, _log10i = 0;
        c_log(_abs9r, _abs9i, &_log10r, &_log10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_mul11r, _mul11i, i2, 0, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log10r, _log10i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _sqrt15r = 0, _sqrt15i = 0;
        c_powr(j, 0, 0.5, &_sqrt15r, &_sqrt15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_mul16r, _mul16i, r2, 0, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_sqrt15r, _sqrt15i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul14r + _mul19r; _add20i = _mul14i + _mul19i;
        double mag = _add20r; /* +_add20ii */
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = j + _c22r; _add23i = 0 + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang21r, _ang21i, _log24r, _log24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, r1, 0, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang26r, _ang26i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul25r + _mul29r; _add30i = _mul25i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, i2, 0, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _add30r + _cos32r; _add33i = _add30i + _cos32i;
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
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x1r; _conj40i = -(x1i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(j, 0, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj40r, _conj40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
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

static void poly_530_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
    }
    for (int j = 0; j < (int)(n); j++) {
        double _arr6r = 0, _arr6i = 0;
        { int _idx = j; _arr6r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr6i = 0; }
        double r = _arr6r; /* +_arr6ii */
        double _arr7r = 0, _arr7i = 0;
        { int _idx = j; _arr7r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr7i = 0; }
        double m = _arr7r; /* +_arr7ii */
        double _mul8r = 0, _mul8i = 0;
        c_mul(r, 0, M_PI, 0, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _div11r = 0, _div11i = 0;
        c_div(_mul8r, _mul8i, _add10r, _add10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(m, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 3.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _div16r = 0, _div16i = 0;
        c_div(_mul13r, _mul13i, _add15r, _add15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sin12r, _sin12i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double term1 = _mul18r; /* +_mul18ii */
        double _add19r = 0, _add19i = 0;
        _add19r = r + m; _add19i = 0 + 0;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_add19r, _add19i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x1r; _attr24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_attr24r, _attr24i, _add26r, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_log23r, _log23i, _pow27r, _pow27i, &_mul28r, &_mul28i);
        double term2 = _mul28r; /* +_mul28ii */
        double _prod29r = 0, _prod29i = 0;
        c_mul(r, 0, m, 0, &_prod29r, &_prod29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = j + _c30r; _add31i = 0 + _c30i;
        double _prod32r = 0, _prod32i = 0;
        c_mul(_prod29r, _prod29i, _add31r, _add31i, &_prod32r, &_prod32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_c33r, _c33i, _add35r, _add35i, &_div36r, &_div36i);
        double _pow37r = 0, _pow37i = 0;
        c_powr(_prod32r, _prod32i, _div36r, &_pow37r, &_pow37i);
        double term3 = _pow37r; /* +_pow37ii */
        double _add38r = 0, _add38i = 0;
        _add38r = term1 + term2; _add38i = 0 + 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _add38r + term3; _add39i = _add38i + 0;
        double mag = _add39r; /* +_add39ii */
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(x1r, x1i); _ang40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(m, 0, M_PI, 0, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 4.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = j + _c42r; _add43i = 0 + _c42i;
        double _div44r = 0, _div44i = 0;
        c_div(_mul41r, _mul41i, _add43r, _add43i, &_div44r, &_div44i);
        double _sin45r = 0, _sin45i = 0;
        c_sin(_div44r, _div44i, &_sin45r, &_sin45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang40r, _ang40i, _sin45r, _sin45i, &_mul46r, &_mul46i);
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x2r, x2i); _ang47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(r, 0, M_PI, 0, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 5.0; _c49i = 0;
        double _add50r = 0, _add50i = 0;
        _add50r = j + _c49r; _add50i = 0 + _c49i;
        double _div51r = 0, _div51i = 0;
        c_div(_mul48r, _mul48i, _add50r, _add50i, &_div51r, &_div51i);
        double _cos52r = 0, _cos52i = 0;
        c_cos(_div51r, _div51i, &_cos52r, &_cos52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_ang47r, _ang47i, _cos52r, _cos52i, &_mul53r, &_mul53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _mul46r + _mul53r; _add54i = _mul46i + _mul53i;
        double _c55r = 0, _c55i = 0;
        _c55r = 2.0; _c55i = 0;
        double _add56r = 0, _add56i = 0;
        _add56r = j + _c55r; _add56i = 0 + _c55i;
        double _log57r = 0, _log57i = 0;
        c_log(_add56r, _add56i, &_log57r, &_log57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _add54r + _log57r; _add58i = _add54i + _log57i;
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
        c_mul(mag, 0, _add63r, _add63i, &_mul64r, &_mul64i);
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul64r; cIm[_idx] = _mul64i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_531_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int r = 1; r < (int)(_add7r); r++) {
        double ang = 0;
        double mag = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 3.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(r, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        if (_mod9r == _c10r) {
            double _add11r = 0, _add11i = 0;
            _add11r = x1r + r; _add11i = x1i + 0;
            double _abs12r = 0, _abs12i = 0;
            _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
            double _c13r = 0, _c13i = 0;
            _c13r = 1.0; _c13i = 0;
            double _add14r = 0, _add14i = 0;
            _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
            double _log15r = 0, _log15i = 0;
            c_log(_add14r, _add14i, &_log15r, &_log15i);
            double _div16r = 0, _div16i = 0;
            c_div(r, 0, n, 0, &_div16r, &_div16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_div16r, _div16i, M_PI, 0, &_mul17r, &_mul17i);
            double _sin18r = 0, _sin18i = 0;
            c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(_log15r, _log15i, _sin18r, _sin18i, &_mul19r, &_mul19i);
            double _mul20r = 0, _mul20i = 0;
            c_mul(r, 0, M_PI, 0, &_mul20r, &_mul20i);
            double _c21r = 0, _c21i = 0;
            _c21r = 4.0; _c21i = 0;
            double _div22r = 0, _div22i = 0;
            c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
            double _cos23r = 0, _cos23i = 0;
            c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
            double _add24r = 0, _add24i = 0;
            _add24r = _mul19r + _cos23r; _add24i = _mul19i + _cos23i;
            mag = _add24r;
            double _ang25r = 0, _ang25i = 0;
            _ang25r = c_arg(x1r, x1i); _ang25i = 0;
            double _mul26r = 0, _mul26i = 0;
            c_mul(r, 0, M_PI, 0, &_mul26r, &_mul26i);
            double _c27r = 0, _c27i = 0;
            _c27r = 6.0; _c27i = 0;
            double _div28r = 0, _div28i = 0;
            c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
            double _sin29r = 0, _sin29i = 0;
            c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
            double _ang30r = 0, _ang30i = 0;
            _ang30r = c_arg(x2r, x2i); _ang30i = 0;
            double _mul31r = 0, _mul31i = 0;
            c_mul(_sin29r, _sin29i, _ang30r, _ang30i, &_mul31r, &_mul31i);
            double _add32r = 0, _add32i = 0;
            _add32r = _ang25r + _mul31r; _add32i = _ang25i + _mul31i;
            ang = _add32r;
        } else {
            double _c33r = 0, _c33i = 0;
            _c33r = 3.0; _c33i = 0;
            double _mod34r = 0, _mod34i = 0;
            _mod34r = fmod(r, _c33r); _mod34i = 0;
            double _c35r = 0, _c35i = 0;
            _c35r = 2.0; _c35i = 0;
            if (_mod34r == _c35r) {
                double _add36r = 0, _add36i = 0;
                _add36r = x2r + r; _add36i = x2i + 0;
                double _abs37r = 0, _abs37i = 0;
                _abs37r = c_abs(_add36r, _add36i); _abs37i = 0;
                double _c38r = 0, _c38i = 0;
                _c38r = 1.0; _c38i = 0;
                double _add39r = 0, _add39i = 0;
                _add39r = _abs37r + _c38r; _add39i = _abs37i + _c38i;
                double _log40r = 0, _log40i = 0;
                c_log(_add39r, _add39i, &_log40r, &_log40i);
                double _div41r = 0, _div41i = 0;
                c_div(r, 0, n, 0, &_div41r, &_div41i);
                double _mul42r = 0, _mul42i = 0;
                c_mul(_div41r, _div41i, M_PI, 0, &_mul42r, &_mul42i);
                double _cos43r = 0, _cos43i = 0;
                c_cos(_mul42r, _mul42i, &_cos43r, &_cos43i);
                double _mul44r = 0, _mul44i = 0;
                c_mul(_log40r, _log40i, _cos43r, _cos43i, &_mul44r, &_mul44i);
                double _mul45r = 0, _mul45i = 0;
                c_mul(r, 0, M_PI, 0, &_mul45r, &_mul45i);
                double _c46r = 0, _c46i = 0;
                _c46r = 3.0; _c46i = 0;
                double _div47r = 0, _div47i = 0;
                c_div(_mul45r, _mul45i, _c46r, _c46i, &_div47r, &_div47i);
                double _sin48r = 0, _sin48i = 0;
                c_sin(_div47r, _div47i, &_sin48r, &_sin48i);
                double _add49r = 0, _add49i = 0;
                _add49r = _mul44r + _sin48r; _add49i = _mul44i + _sin48i;
                mag = _add49r;
                double _ang50r = 0, _ang50i = 0;
                _ang50r = c_arg(x2r, x2i); _ang50i = 0;
                double _mul51r = 0, _mul51i = 0;
                c_mul(r, 0, M_PI, 0, &_mul51r, &_mul51i);
                double _c52r = 0, _c52i = 0;
                _c52r = 5.0; _c52i = 0;
                double _div53r = 0, _div53i = 0;
                c_div(_mul51r, _mul51i, _c52r, _c52i, &_div53r, &_div53i);
                double _cos54r = 0, _cos54i = 0;
                c_cos(_div53r, _div53i, &_cos54r, &_cos54i);
                double _ang55r = 0, _ang55i = 0;
                _ang55r = c_arg(x1r, x1i); _ang55i = 0;
                double _mul56r = 0, _mul56i = 0;
                c_mul(_cos54r, _cos54i, _ang55r, _ang55i, &_mul56r, &_mul56i);
                double _add57r = 0, _add57i = 0;
                _add57r = _ang50r + _mul56r; _add57i = _ang50i + _mul56i;
                ang = _add57r;
            } else {
                double _mul58r = 0, _mul58i = 0;
                c_mul(x1r, x1i, x2r, x2i, &_mul58r, &_mul58i);
                double _add59r = 0, _add59i = 0;
                _add59r = _mul58r + r; _add59i = _mul58i + 0;
                double _abs60r = 0, _abs60i = 0;
                _abs60r = c_abs(_add59r, _add59i); _abs60i = 0;
                double _c61r = 0, _c61i = 0;
                _c61r = 1.0; _c61i = 0;
                double _add62r = 0, _add62i = 0;
                _add62r = _abs60r + _c61r; _add62i = _abs60i + _c61i;
                double _log63r = 0, _log63i = 0;
                c_log(_add62r, _add62i, &_log63r, &_log63i);
                double _c64r = 0, _c64i = 0;
                _c64r = 2.0; _c64i = 0;
                double _mul65r = 0, _mul65i = 0;
                c_mul(_c64r, _c64i, n, 0, &_mul65r, &_mul65i);
                double _div66r = 0, _div66i = 0;
                c_div(r, 0, _mul65r, _mul65i, &_div66r, &_div66i);
                double _mul67r = 0, _mul67i = 0;
                c_mul(_div66r, _div66i, M_PI, 0, &_mul67r, &_mul67i);
                double _sin68r = 0, _sin68i = 0;
                c_sin(_mul67r, _mul67i, &_sin68r, &_sin68i);
                double _mul69r = 0, _mul69i = 0;
                c_mul(_log63r, _log63i, _sin68r, _sin68i, &_mul69r, &_mul69i);
                double _mul70r = 0, _mul70i = 0;
                c_mul(r, 0, M_PI, 0, &_mul70r, &_mul70i);
                double _c71r = 0, _c71i = 0;
                _c71r = 2.0; _c71i = 0;
                double _div72r = 0, _div72i = 0;
                c_div(_mul70r, _mul70i, _c71r, _c71i, &_div72r, &_div72i);
                double _cos73r = 0, _cos73i = 0;
                c_cos(_div72r, _div72i, &_cos73r, &_cos73i);
                double _add74r = 0, _add74i = 0;
                _add74r = _mul69r + _cos73r; _add74i = _mul69i + _cos73i;
                mag = _add74r;
                double _mul75r = 0, _mul75i = 0;
                c_mul(x1r, x1i, x2r, x2i, &_mul75r, &_mul75i);
                double _ang76r = 0, _ang76i = 0;
                _ang76r = c_arg(_mul75r, _mul75i); _ang76i = 0;
                double _mul77r = 0, _mul77i = 0;
                c_mul(r, 0, M_PI, 0, &_mul77r, &_mul77i);
                double _c78r = 0, _c78i = 0;
                _c78r = 4.0; _c78i = 0;
                double _div79r = 0, _div79i = 0;
                c_div(_mul77r, _mul77i, _c78r, _c78i, &_div79r, &_div79i);
                double _sin80r = 0, _sin80i = 0;
                c_sin(_div79r, _div79i, &_sin80r, &_sin80i);
                double _mul81r = 0, _mul81i = 0;
                c_mul(r, 0, M_PI, 0, &_mul81r, &_mul81i);
                double _c82r = 0, _c82i = 0;
                _c82r = 3.0; _c82i = 0;
                double _div83r = 0, _div83i = 0;
                c_div(_mul81r, _mul81i, _c82r, _c82i, &_div83r, &_div83i);
                double _cos84r = 0, _cos84i = 0;
                c_cos(_div83r, _div83i, &_cos84r, &_cos84i);
                double _mul85r = 0, _mul85i = 0;
                c_mul(_sin80r, _sin80i, _cos84r, _cos84i, &_mul85r, &_mul85i);
                double _add86r = 0, _add86i = 0;
                _add86r = _ang76r + _mul85r; _add86i = _ang76i + _mul85i;
                ang = _add86r;
            }
        }
        double _c87r = 0, _c87i = 0;
        _c87r = 0.0; _c87i = 1.0;
        double _mul88r = 0, _mul88i = 0;
        c_mul(_c87r, _c87i, ang, 0, &_mul88r, &_mul88i);
        double _exp89r = 0, _exp89i = 0;
        c_exp2(_mul88r, _mul88i, &_exp89r, &_exp89i);
        double _mul90r = 0, _mul90i = 0;
        c_mul(mag, 0, _exp89r, _exp89i, &_mul90r, &_mul90i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul90r; cIm[_idx] = _mul90i; } }
    }
    for (int k = 0; k < (int)(n); k++) {
        double _c91r = 0, _c91i = 0;
        _c91r = 3.0; _c91i = 0;
        double _div92r = 0, _div92i = 0;
        c_div(n, 0, _c91r, _c91i, &_div92r, &_div92i);
        if (k < _div92r) {
            double _cf93r = 0, _cf93i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf93r = cRe[_idx]; _cf93i = cIm[_idx]; } }
            double _c94r = 0, _c94i = 0;
            _c94r = 1.0; _c94i = 0;
            double _add95r = 0, _add95i = 0;
            _add95r = k + _c94r; _add95i = 0 + _c94i;
            double _mul96r = 0, _mul96i = 0;
            c_mul(_cf93r, _cf93i, _add95r, _add95i, &_mul96r, &_mul96i);
            { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul96r; cIm[_idx] = _mul96i; } }
        } else {
            double _c97r = 0, _c97i = 0;
            _c97r = 2.0; _c97i = 0;
            double _mul98r = 0, _mul98i = 0;
            c_mul(_c97r, _c97i, n, 0, &_mul98r, &_mul98i);
            double _c99r = 0, _c99i = 0;
            _c99r = 3.0; _c99i = 0;
            double _div100r = 0, _div100i = 0;
            c_div(_mul98r, _mul98i, _c99r, _c99i, &_div100r, &_div100i);
            if (k < _div100r) {
                double _cf101r = 0, _cf101i = 0;
                { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf101r = cRe[_idx]; _cf101i = cIm[_idx]; } }
                double _c102r = 0, _c102i = 0;
                _c102r = 1.0; _c102i = 0;
                double _add103r = 0, _add103i = 0;
                _add103r = k + _c102r; _add103i = 0 + _c102i;
                double _neg104r = 0, _neg104i = 0;
                _neg104r = -(_add103r); _neg104i = -(_add103i);
                double _mul105r = 0, _mul105i = 0;
                c_mul(_cf101r, _cf101i, _neg104r, _neg104i, &_mul105r, &_mul105i);
                { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul105r; cIm[_idx] = _mul105i; } }
            } else {
                double _cf106r = 0, _cf106i = 0;
                { int _idx = k; if (_idx >= 0 && _idx < 36) { _cf106r = cRe[_idx]; _cf106i = cIm[_idx]; } }
                double _c107r = 0, _c107i = 0;
                _c107r = 1.0; _c107i = 0;
                double _c108r = 0, _c108i = 0;
                _c108r = 1.0; _c108i = 0;
                double _add109r = 0, _add109i = 0;
                _add109r = k + _c108r; _add109i = 0 + _c108i;
                double _div110r = 0, _div110i = 0;
                c_div(_c107r, _c107i, _add109r, _add109i, &_div110r, &_div110i);
                double _mul111r = 0, _mul111i = 0;
                c_mul(_cf106r, _cf106i, _div110r, _div110i, &_mul111r, &_mul111i);
                { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul111r; cIm[_idx] = _mul111i; } }
            }
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_532_c(double x1r, double x1i, double x2r, double x2i,
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
    for (int r = 1; r < (int)(_add3r); r++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 7.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(r, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double j = _add7r; /* +_add7ii */
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(r, 0, _c8r, _c8i, &_div9r, &_div9i);
        double _flr10r = 0, _flr10i = 0;
        _flr10r = floor(_div9r); _flr10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _flr10r + _c11r; _add12i = _flr10i + _c11i;
        double k = _add12r; /* +_add12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x1r, x1i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(r, 0, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log16r, _log16i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(r, 0, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log22r, _log22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul18r + _mul24r; _add25i = _mul18i + _mul24i;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 10.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(r, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _c26r + _div28r; _add29i = _c26i + _div28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_add25r, _add25i, _add29r, _add29i, &_mul30r, &_mul30i);
        double magnitude = _mul30r; /* +_mul30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(r, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_div33r, _div33i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 3.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(r, 0, _c37r, _c37i, &_div38r, &_div38i);
        double _cos39r = 0, _cos39i = 0;
        c_cos(_div38r, _div38i, &_cos39r, &_cos39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang36r, _ang36i, _cos39r, _cos39i, &_mul40r, &_mul40i);
        double _sub41r = 0, _sub41i = 0;
        _sub41r = _mul35r - _mul40r; _sub41i = _mul35i - _mul40i;
        double _sin42r = 0, _sin42i = 0;
        c_sin(r, 0, &_sin42r, &_sin42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 4.0; _c43i = 0;
        double _div44r = 0, _div44i = 0;
        c_div(r, 0, _c43r, _c43i, &_div44r, &_div44i);
        double _cos45r = 0, _cos45i = 0;
        c_cos(_div44r, _div44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_sin42r, _sin42i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _sub41r + _mul46r; _add47i = _sub41i + _mul46i;
        double angle = _add47r; /* +_add47ii */
        double _cos48r = 0, _cos48i = 0;
        c_cos(angle, 0, &_cos48r, &_cos48i);
        double _sin49r = 0, _sin49i = 0;
        c_sin(angle, 0, &_sin49r, &_sin49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_sin49r, _sin49i, _c50r, _c50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _cos48r + _mul51r; _add52i = _cos48i + _mul51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(magnitude, 0, _add52r, _add52i, &_mul53r, &_mul53i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_533_c(double x1r, double x1i, double x2r, double x2i,
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
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = k + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr4r, _attr4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(k, 0, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr9r, _attr9i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _mul11r; _add12i = _mul8i + _mul11i;
        double r = _add12r; /* +_add12ii */
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _cos14r = 0, _cos14i = 0;
        c_cos(k, 0, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr13r, _attr13i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2i; _attr16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = k + _c17r; _add18i = 0 + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_attr16r, _attr16i, _log19r, _log19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul15r + _mul20r; _add21i = _mul15i + _mul20i;
        double im = _add21r; /* +_add21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(k, 0, k, 0, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _abs22r + _pow24r; _add25i = _abs22i + _pow24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 3.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(k, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _c27r + _sin30r; _add31i = _c27i + _sin30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_log26r, _log26i, _add31r, _add31i, &_mul32r, &_mul32i);
        double mag = _mul32r; /* +_mul32ii */
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x1r, x1i); _ang33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 4.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(k, 0, _c34r, _c34i, &_div35r, &_div35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_div35r, _div35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang33r, _ang33i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 5.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(k, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang38r, _ang38i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul37r + _mul42r; _add43i = _mul37i + _mul42i;
        double ang = _add43r; /* +_add43ii */
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, im, 0, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = r + _mul45r; _add46i = 0 + _mul45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_add46r, _add46i, mag, 0, &_mul47r, &_mul47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c48r, _c48i, ang, 0, &_mul49r, &_mul49i);
        double _exp50r = 0, _exp50i = 0;
        c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_mul47r, _mul47i, _exp50r, _exp50i, &_mul51r, &_mul51i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_534_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double r1 = _attr4r; /* +_attr4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2r; _attr5i = 0;
        double r2 = _attr5r; /* +_attr5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1i; _attr6i = 0;
        double im1 = _attr6r; /* +_attr6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2i; _attr7i = 0;
        double im2 = _attr7r; /* +_attr7ii */
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
        _c13r = 1.0; _c13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, r1, 0, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _c13r + _sin15r; _add16i = _c13i + _sin15i;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, im2, 0, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _add16r + _cos18r; _add19i = _add16i + _cos18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log12r, _log12i, _add19r, _add19i, &_mul20r, &_mul20i);
        double mag_part1 = _mul20r; /* +_mul20ii */
        double _c21r = 0, _c21i = 0;
        _c21r = 1.3; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(j, 0, 1.3, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(r2, 0, _pow22r, _pow22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _sqrt25r = 0, _sqrt25i = 0;
        c_powr(j, 0, 0.5, &_sqrt25r, &_sqrt25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(im1, 0, _sqrt25r, _sqrt25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin24r, _sin24i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double mag_part2 = _mul28r; /* +_mul28ii */
        double _mul29r = 0, _mul29i = 0;
        c_mul(mag_part1, 0, mag_part2, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = j + _c30r; _add31i = 0 + _c30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul29r + _log32r; _add33i = _mul29i + _log32i;
        double magnitude = _add33r; /* +_add33ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_535: too complex for auto-transpile, stubbed */
static void poly_535_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_536_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr4r, _attr4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 3.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(j, 0, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr9r, _attr9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _mul13r; _add14i = _mul8i + _mul13i;
        double r = _add14r; /* +_add14ii */
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 4.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(j, 0, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_attr15r, _attr15i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_attr20r, _attr20i, _log23r, _log23i, &_mul24r, &_mul24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _mul19r - _mul24r; _sub25i = _mul19i - _mul24i;
        double im = _sub25r; /* +_sub25ii */
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.2; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_powr(j, 0, 1.2, &_pow28r, &_pow28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _abs26r + _pow28r; _add29i = _abs26i + _pow28i;
        double _log30r = 0, _log30i = 0;
        c_log(_add29r, _add29i, &_log30r, &_log30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.5; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 6.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c32r, _c32i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _c31r + _mul37r; _add38i = _c31i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_log30r, _log30i, _add38r, _add38i, &_mul39r, &_mul39i);
        double magnitude = _mul39r; /* +_mul39ii */
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(x1r, x1i); _ang40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 5.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(j, 0, _c41r, _c41i, &_div42r, &_div42i);
        double _cos43r = 0, _cos43i = 0;
        c_cos(_div42r, _div42i, &_cos43r, &_cos43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_ang40r, _ang40i, _cos43r, _cos43i, &_mul44r, &_mul44i);
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x2r, x2i); _ang45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 7.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(j, 0, _c46r, _c46i, &_div47r, &_div47i);
        double _sin48r = 0, _sin48i = 0;
        c_sin(_div47r, _div47i, &_sin48r, &_sin48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_ang45r, _ang45i, _sin48r, _sin48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul44r + _mul49r; _add50i = _mul44i + _mul49i;
        double _abs51r = 0, _abs51i = 0;
        _abs51r = c_abs(x2r, x2i); _abs51i = 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 1.0; _c52i = 0;
        double _add53r = 0, _add53i = 0;
        _add53r = _abs51r + _c52r; _add53i = _abs51i + _c52i;
        double _log54r = 0, _log54i = 0;
        c_log(_add53r, _add53i, &_log54r, &_log54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _add50r + _log54r; _add55i = _add50i + _log54i;
        double angle = _add55r; /* +_add55ii */
        double _cos56r = 0, _cos56i = 0;
        c_cos(angle, 0, &_cos56r, &_cos56i);
        double _sin57r = 0, _sin57i = 0;
        c_sin(angle, 0, &_sin57r, &_sin57i);
        double _c58r = 0, _c58i = 0;
        _c58r = 0.0; _c58i = 1.0;
        double _mul59r = 0, _mul59i = 0;
        c_mul(_sin57r, _sin57i, _c58r, _c58i, &_mul59r, &_mul59i);
        double _add60r = 0, _add60i = 0;
        _add60r = _cos56r + _mul59r; _add60i = _cos56i + _mul59i;
        double _mul61r = 0, _mul61i = 0;
        c_mul(magnitude, 0, _add60r, _add60i, &_mul61r, &_mul61i);
        double _add62r = 0, _add62i = 0;
        _add62r = r + im; _add62i = 0 + 0;
        double _c63r = 0, _c63i = 0;
        _c63r = 0.0; _c63i = 1.0;
        double _mul64r = 0, _mul64i = 0;
        c_mul(_add62r, _add62i, _c63r, _c63i, &_mul64r, &_mul64i);
        double _add65r = 0, _add65i = 0;
        _add65r = _mul61r + _mul64r; _add65i = _mul61i + _mul64i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add65r; cIm[_idx] = _add65i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_537_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        rec_seq[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc_seq[35];
    for (int _li = 0; _li < 35; _li++) {
        imc_seq[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int k = 1; k < (int)(_add7r); k++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (k - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr8i = 0; }
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (k - 1); _arr9r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr9i = 0; }
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
        double _mul15r = 0, _mul15i = 0;
        c_mul(k, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 7.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(k, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _sin18r + _cos22r; _add23i = _sin18i + _cos22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log14r, _log14i, _add23r, _add23i, &_mul24r, &_mul24i);
        double magnitude = _mul24r; /* +_mul24ii */
        double _mul25r = 0, _mul25i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(_mul25r, _mul25i); _ang26i = 0;
        double _sin27r = 0, _sin27i = 0;
        c_sin(k, 0, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _ang26r + _sin27r; _add28i = _ang26i + _sin27i;
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(k, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
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
        c_mul(magnitude, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_538_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
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
        double _sqrt18r = 0, _sqrt18i = 0;
        c_powr(j, 0, 0.5, &_sqrt18r, &_sqrt18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sqrt18r, _sqrt18i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul17r + _mul23r; _add24i = _mul17i + _mul23i;
        double magnitude = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(j, 0, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang25r, _ang25i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(j, 0, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul27r + _mul32r; _add33i = _mul27i + _mul32i;
        double _arr34r = 0, _arr34i = 0;
        { int _idx = (j - 1); _arr34r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr34i = 0; }
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, _arr34r, _arr34i, &_mul35r, &_mul35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_mul35r, _mul35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add33r + _sin36r; _add37i = _add33i + _sin36i;
        double _arr38r = 0, _arr38i = 0;
        { int _idx = (j - 1); _arr38r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr38i = 0; }
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, _arr38r, _arr38i, &_mul39r, &_mul39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_mul39r, _mul39i, &_cos40r, &_cos40i);
        double _sub41r = 0, _sub41i = 0;
        _sub41r = _add37r - _cos40r; _sub41i = _add37i - _cos40i;
        double angle = _sub41r; /* +_sub41ii */
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
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_539_c(double x1r, double x1i, double x2r, double x2i,
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
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _flr7r = 0, _flr7i = 0;
        _flr7r = floor(_attr6r); _flr7i = 0;
        double _int8r = 0, _int8i = 0;
        _int8r = (int)(_flr7r); _int8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _pow5r + _int8r; _add9i = _pow5i + _int8i;
        double k = _add9r; /* +_add9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs10r + j; _add11i = _abs10i + 0;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x2r, x2i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _c13r + _sin16r; _add17i = _c13i + _sin16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log12r, _log12i, _add17r, _add17i, &_mul18r, &_mul18i);
        double r = _mul18r; /* +_mul18ii */
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2r; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, _attr19r, _attr19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x1i; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _attr22r, _attr22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_cos21r, _cos21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x2i; _attr26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(j, 0, _attr26r, _attr26i, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul25r + _cos28r; _add29i = _mul25i + _cos28i;
        double s = _add29r; /* +_add29ii */
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs30r + j; _add31i = _abs30i + 0;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _add33r = 0, _add33i = 0;
        _add33r = r + _log32r; _add33i = 0 + _log32i;
        double magnitude = _add33r; /* +_add33ii */
        double _attr34r = 0, _attr34i = 0;
        _attr34r = x1r; _attr34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, _attr34r, _attr34i, &_mul35r, &_mul35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_mul35r, _mul35i, &_sin36r, &_sin36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x1r, x1i); _ang37i = 0;
        double _cos38r = 0, _cos38i = 0;
        c_cos(_ang37r, _ang37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_sin36r, _sin36i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = s + _mul39r; _add40i = 0 + _mul39i;
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

static void poly_540_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
        _c7r = 0.3; _c7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c7r, _c7i, j, 0, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_mul8r, _mul8i, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log6r, _log6i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double mag_part1 = _mul12r; /* +_mul12ii */
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs13r + j; _add14i = _abs13i + 0;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 0.2; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, j, 0, &_mul17r, &_mul17i);
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x1i; _attr18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_mul17r, _mul17i, _attr18r, _attr18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log15r, _log15i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double mag_part2 = _mul21r; /* +_mul21ii */
        double _add22r = 0, _add22i = 0;
        _add22r = mag_part1 + mag_part2; _add22i = 0 + 0;
        double mag = _add22r; /* +_add22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 0.1; _c24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, _c24r, _c24i, &_mul25r, &_mul25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_mul25r, _mul25i, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 5.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul26r, _mul26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _ang23r + _mul30r; _add31i = _ang23i + _mul30i;
        double angle_part1 = _add31r; /* +_add31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 0.1; _c33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _c33r, _c33i, &_mul34r, &_mul34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_mul34r, _mul34i, M_PI, 0, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_mul35r, _mul35i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _ang32r + _mul39r; _add40i = _ang32i + _mul39i;
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

static void poly_542_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        double mag_sum = _c8r; /* +_c8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        for (int k = 1; k < (int)(_add10r); k++) {
            double _attr11r = 0, _attr11i = 0;
            _attr11r = x1r; _attr11i = 0;
            double _mul12r = 0, _mul12i = 0;
            c_mul(k, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
            double _sin13r = 0, _sin13i = 0;
            c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
            double _attr14r = 0, _attr14i = 0;
            _attr14r = x2i; _attr14i = 0;
            double _mul15r = 0, _mul15i = 0;
            c_mul(k, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
            double _cos16r = 0, _cos16i = 0;
            c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(_sin13r, _sin13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
            mag_sum += _mul17r;
        }
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr18i = 0; }
        double _arr19r = 0, _arr19i = 0;
        { int _idx = (j - 1); _arr19r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr19i = 0; }
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
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log24r, _log24i, mag_sum, 0, &_mul25r, &_mul25i);
        double magnitude = _mul25r; /* +_mul25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        double angle = _c26r; /* +_c26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        for (int r = 1; r < (int)(_add28r); r++) {
            double _ang29r = 0, _ang29i = 0;
            _ang29r = c_arg(x1r, x1i); _ang29i = 0;
            double _sin30r = 0, _sin30i = 0;
            c_sin(r, 0, &_sin30r, &_sin30i);
            double _mul31r = 0, _mul31i = 0;
            c_mul(_ang29r, _ang29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
            double _ang32r = 0, _ang32i = 0;
            _ang32r = c_arg(x2r, x2i); _ang32i = 0;
            double _cos33r = 0, _cos33i = 0;
            c_cos(r, 0, &_cos33r, &_cos33i);
            double _mul34r = 0, _mul34i = 0;
            c_mul(_ang32r, _ang32i, _cos33r, _cos33i, &_mul34r, &_mul34i);
            double _add35r = 0, _add35i = 0;
            _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
            angle += _add35r;
        }
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

static void poly_543_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_part1 = _log8r; /* +_log8ii */
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x2r, x2i); _abs9i = 0;
        double _sqrt10r = 0, _sqrt10i = 0;
        c_powr(j, 0, 0.5, &_sqrt10r, &_sqrt10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _sqrt10r; _add11i = _abs9i + _sqrt10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double mag_part2 = _log12r; /* +_log12ii */
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1r; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(mag_part1, 0, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x2i; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _attr17r, _attr17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(mag_part2, 0, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul16r + _mul20r; _add21i = _mul16i + _mul20i;
        double mag_variation = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.3; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(j, 0, 1.3, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang22r, _ang22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double angle_part1 = _mul25r; /* +_mul25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_ang26r, _ang26i, _add28r, _add28i, &_div29r, &_div29i);
        double angle_part2 = _div29r; /* +_div29ii */
        double _sub30r = 0, _sub30i = 0;
        _sub30r = angle_part1 - angle_part2; _sub30i = 0 - 0;
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
        _add36r = _sub30r + _mul35r; _add36i = _sub30i + _mul35i;
        double angle_variation = _add36r; /* +_add36ii */
        double _abs37r = 0, _abs37i = 0;
        _abs37r = c_abs(mag_variation, 0); _abs37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _abs37r + _c38r; _add39i = _abs37i + _c38i;
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, angle_variation, 0, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_add39r, _add39i, _exp42r, _exp42i, &_mul43r, &_mul43i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_544_c(double x1r, double x1i, double x2r, double x2i,
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
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _sqrt9r = 0, _sqrt9i = 0;
        c_powr(_add8r, _add8i, 0.5, &_sqrt9r, &_sqrt9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_log6r, _log6i, _sqrt9r, _sqrt9i, &_mul10r, &_mul10i);
        double mag_part1 = _mul10r; /* +_mul10ii */
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
        double mag_part2 = _mul17r; /* +_mul17ii */
        double _c18r = 0, _c18i = 0;
        _c18r = 5.0; _c18i = 0;
        double _mod19r = 0, _mod19i = 0;
        _mod19r = fmod(j, _c18r); _mod19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _mod19r + _c20r; _add21i = _mod19i + _c20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(mag_part2, 0, _add21r, _add21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = mag_part1 + _mul22r; _add23i = 0 + _mul22i;
        double magnitude = _add23r; /* +_add23ii */
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
        double angle_part1 = _mul28r; /* +_mul28ii */
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
        double angle_part2 = _mul33r; /* +_mul33ii */
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(x1r, x1i); _abs34i = 0;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(x2r, x2i); _abs35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _abs34r + _abs35r; _add36i = _abs34i + _abs35i;
        double _add37r = 0, _add37i = 0;
        _add37r = _add36r + j; _add37i = _add36i + 0;
        double _log38r = 0, _log38i = 0;
        c_log(_add37r, _add37i, &_log38r, &_log38i);
        double angle_part3 = _log38r; /* +_log38ii */
        double _add39r = 0, _add39i = 0;
        _add39r = angle_part1 + angle_part2; _add39i = 0 + 0;
        double _add40r = 0, _add40i = 0;
        _add40r = _add39r + angle_part3; _add40i = _add39i + 0;
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
        c_mul(magnitude, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    double _c47r = 0, _c47i = 0;
    _c47r = 1.0; _c47i = 0;
    double _add48r = 0, _add48i = 0;
    _add48r = n + _c47r; _add48i = 0 + _c47i;
    for (int k = 1; k < (int)(_add48r); k++) {
        double _cf49r = 0, _cf49i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf49r = cRe[_idx]; _cf49i = cIm[_idx]; } }
        double _conj50r = 0, _conj50i = 0;
        _conj50r = x1r; _conj50i = -(x1i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_cf49r, _cf49i, _conj50r, _conj50i, &_mul51r, &_mul51i);
        double _abs52r = 0, _abs52i = 0;
        _abs52r = c_abs(x2r, x2i); _abs52i = 0;
        double _c53r = 0, _c53i = 0;
        _c53r = 1.0; _c53i = 0;
        double _add54r = 0, _add54i = 0;
        _add54r = _abs52r + _c53r; _add54i = _abs52i + _c53i;
        double _div55r = 0, _div55i = 0;
        c_div(_mul51r, _mul51i, _add54r, _add54i, &_div55r, &_div55i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div55r; cIm[_idx] = _div55i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_545_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr8i = 0; }
        double _c9r = 0, _c9i = 0;
        _c9r = 3.0; _c9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_mul(_arr8r, _arr8i, _arr8r, _arr8i, &_pow10r, &_pow10i);
        c_mul(_pow10r, _pow10i, _arr8r, _arr8i, &_pow10r, &_pow10i);
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr11i = 0; }
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_arr11r, _arr11i, _arr11r, _arr11i, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _pow10r + _pow13r; _add14i = _pow10i + _pow13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_add14r, _add14i, _log18r, _log18i, &_mul19r, &_mul19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin23r, _sin23i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul19r + _mul28r; _add29i = _mul19i + _mul28i;
        double mag = _add29r; /* +_add29ii */
        double _ang30r = 0, _ang30i = 0;
        _ang30r = c_arg(x1r, x1i); _ang30i = 0;
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang31r, _ang31i, j, 0, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _ang30r + _mul32r; _add33i = _ang30i + _mul32i;
        double _attr34r = 0, _attr34i = 0;
        _attr34r = x1r; _attr34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(j, 0, _attr34r, _attr34i, &_mul35r, &_mul35i);
        double _attr36r = 0, _attr36i = 0;
        _attr36r = x2i; _attr36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_mul35r, _mul35i, _attr36r, _attr36i, &_mul37r, &_mul37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(_mul37r, _mul37i, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add33r + _sin38r; _add39i = _add33i + _sin38i;
        double ang = _add39r; /* +_add39ii */
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, ang, 0, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(mag, 0, _exp42r, _exp42i, &_mul43r, &_mul43i);
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x1r; _conj44i = -(x1i);
        double _c45r = 0, _c45i = 0;
        _c45r = 5.0; _c45i = 0;
        double _mod46r = 0, _mod46i = 0;
        _mod46r = fmod(j, _c45r); _mod46i = 0;
        double _pow47r = 0, _pow47i = 0;
        c_powr(_conj44r, _conj44i, _mod46r, &_pow47r, &_pow47i);
        double _arr48r = 0, _arr48i = 0;
        { int _idx = (j - 1); _arr48r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr48i = 0; }
        double _mul49r = 0, _mul49i = 0;
        c_mul(j, 0, _arr48r, _arr48i, &_mul49r, &_mul49i);
        double _cos50r = 0, _cos50i = 0;
        c_cos(_mul49r, _mul49i, &_cos50r, &_cos50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_pow47r, _pow47i, _cos50r, _cos50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul43r + _mul51r; _add52i = _mul43i + _mul51i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    }
    double _c53r = 0, _c53i = 0;
    _c53r = 1.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = n + _c53r; _add54i = 0 + _c53i;
    for (int k = 1; k < (int)(_add54r); k++) {
        double _unk55r = 0, _unk55i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='k', ctx=Load())) */
        double _arr56r = 0, _arr56i = 0;
        { int _idx = (int)(_unk55r); _arr56r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr56i = 0; }
        double _unk57r = 0, _unk57i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='k', ctx=Load())) */
        double _arr58r = 0, _arr58i = 0;
        { int _idx = (int)(_unk57r); _arr58r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr58i = 0; }
        double _add59r = 0, _add59i = 0;
        _add59r = _arr56r + _arr58r; _add59i = _arr56i + _arr58i;
        double _ang60r = 0, _ang60i = 0;
        _ang60r = c_arg(x2r, x2i); _ang60i = 0;
        double _mul61r = 0, _mul61i = 0;
        c_mul(k, 0, _ang60r, _ang60i, &_mul61r, &_mul61i);
        double _sin62r = 0, _sin62i = 0;
        c_sin(_mul61r, _mul61i, &_sin62r, &_sin62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(_add59r, _add59i, _sin62r, _sin62i, &_mul63r, &_mul63i);
        cRe[(k - 1)] += _mul63r; cIm[(k - 1)] += _mul63i;
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
        double _sub68r = 0, _sub68i = 0;
        _sub68r = x1r - x2r; _sub68i = x1i - x2i;
        double _abs69r = 0, _abs69i = 0;
        _abs69r = c_abs(_sub68r, _sub68i); _abs69i = 0;
        double _c70r = 0, _c70i = 0;
        _c70r = 1.0; _c70i = 0;
        double _add71r = 0, _add71i = 0;
        _add71r = r + _c70r; _add71i = 0 + _c70i;
        double _div72r = 0, _div72i = 0;
        c_div(_abs69r, _abs69i, _add71r, _add71i, &_div72r, &_div72i);
        double _add73r = 0, _add73i = 0;
        _add73r = _c67r + _div72r; _add73i = _c67i + _div72i;
        double _mul74r = 0, _mul74i = 0;
        c_mul(_cf66r, _cf66i, _add73r, _add73i, &_mul74r, &_mul74i);
        double _add75r = 0, _add75i = 0;
        _add75r = x1r + x2r; _add75i = x1i + x2i;
        double _abs76r = 0, _abs76i = 0;
        _abs76r = c_abs(_add75r, _add75i); _abs76i = 0;
        double _c77r = 0, _c77i = 0;
        _c77r = 1.0; _c77i = 0;
        double _add78r = 0, _add78i = 0;
        _add78r = _abs76r + _c77r; _add78i = _abs76i + _c77i;
        double _log79r = 0, _log79i = 0;
        c_log(_add78r, _add78i, &_log79r, &_log79i);
        double _ang80r = 0, _ang80i = 0;
        _ang80r = c_arg(x1r, x1i); _ang80i = 0;
        double _mul81r = 0, _mul81i = 0;
        c_mul(r, 0, _ang80r, _ang80i, &_mul81r, &_mul81i);
        double _cos82r = 0, _cos82i = 0;
        c_cos(_mul81r, _mul81i, &_cos82r, &_cos82i);
        double _mul83r = 0, _mul83i = 0;
        c_mul(_log79r, _log79i, _cos82r, _cos82i, &_mul83r, &_mul83i);
        double _add84r = 0, _add84i = 0;
        _add84r = _mul74r + _mul83r; _add84i = _mul74i + _mul83i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add84r; cIm[_idx] = _add84i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_546_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add8r = 0, _add8i = 0;
        _add8r = rec1 + rec2; _add8i = 0 + 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _div10r = 0, _div10i = 0;
        c_div(_add8r, _add8i, _c9r, _c9i, &_div10r, &_div10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 7.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_sin14r, _sin14i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _div10r + _mul19r; _add20i = _div10i + _mul19i;
        double r = _add20r; /* +_add20ii */
        double _sub21r = 0, _sub21i = 0;
        _sub21r = imc1 - imc2; _sub21i = 0 - 0;
        double _div22r = 0, _div22i = 0;
        c_div(_sub21r, _sub21i, n, 0, &_div22r, &_div22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_div22r, _div22i, j, 0, &_mul23r, &_mul23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul23r + _sin27r; _add28i = _mul23i + _sin27i;
        double theta = _add28r; /* +_add28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_abs29r, _abs29i, j, 0, &_mul30r, &_mul30i);
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(x2r, x2i); _abs31i = 0;
        double _sub32r = 0, _sub32i = 0;
        _sub32r = n - j; _sub32i = 0 - 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _sub32r + _c33r; _add34i = _sub32i + _c33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_abs31r, _abs31i, _add34r, _add34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul30r + _mul35r; _add36i = _mul30i + _mul35i;
        double _log37r = 0, _log37i = 0;
        c_log(_add36r, _add36i, &_log37r, &_log37i);
        double _sqrt38r = 0, _sqrt38i = 0;
        c_powr(j, 0, 0.5, &_sqrt38r, &_sqrt38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _log37r + _sqrt38r; _add39i = _log37i + _sqrt38i;
        double magnitude = _add39r; /* +_add39ii */
        double _cos40r = 0, _cos40i = 0;
        c_cos(theta, 0, &_cos40r, &_cos40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _sin42r = 0, _sin42i = 0;
        c_sin(theta, 0, &_sin42r, &_sin42i);
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

static void poly_547_c(double x1r, double x1i, double x2r, double x2i,
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
        double _add18r = 0, _add18i = 0;
        _add18r = _mul10r + _mul17r; _add18i = _mul10i + _mul17i;
        double mag = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_mul(j, 0, j, 0, &_pow21r, &_pow21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang19r, _ang19i, _pow21r, _pow21i, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x2r, x2i); _ang23i = 0;
        double _sqrt24r = 0, _sqrt24i = 0;
        c_powr(j, 0, 0.5, &_sqrt24r, &_sqrt24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang23r, _ang23i, _sqrt24r, _sqrt24i, &_mul25r, &_mul25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul22r - _mul25r; _sub26i = _mul22i - _mul25i;
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
        c_mul(mag, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    double _c33r = 0, _c33i = 0;
    _c33r = 1.0; _c33i = 0;
    double _add34r = 0, _add34i = 0;
    _add34r = n + _c33r; _add34i = 0 + _c33i;
    for (int k = 1; k < (int)(_add34r); k++) {
        double _attr35r = 0, _attr35i = 0;
        _attr35r = x1r; _attr35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = k + _attr35r; _add36i = 0 + _attr35i;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _attr38r = 0, _attr38i = 0;
        _attr38r = x2r; _attr38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(k, 0, _attr38r, _attr38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _c37r + _mul39r; _add40i = _c37i + _mul39i;
        double _div41r = 0, _div41i = 0;
        c_div(_add36r, _add36i, _add40r, _add40i, &_div41r, &_div41i);
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x1r, x1i); _ang42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(k, 0, _ang42r, _ang42i, &_mul43r, &_mul43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_mul43r, _mul43i, &_cos44r, &_cos44i);
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x2r, x2i); _ang45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(k, 0, _ang45r, _ang45i, &_mul46r, &_mul46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(_mul46r, _mul46i, &_sin47r, &_sin47i);
        double _sum48r = 0, _sum48i = 0;
        _sum48r = _cos44r + _sin47r; _sum48i = _cos44i + _sin47i;
        double _add49r = 0, _add49i = 0;
        _add49r = _div41r + _sum48r; _add49i = _div41i + _sum48i;
        { double _tr = cRe[(k - 1)]*_add49r - cIm[(k - 1)]*_add49i; cIm[(k - 1)] = cRe[(k - 1)]*_add49i + cIm[(k - 1)]*_add49r; cRe[(k - 1)] = _tr; }
    }
    double _c50r = 0, _c50i = 0;
    _c50r = 1.0; _c50i = 0;
    double _add51r = 0, _add51i = 0;
    _add51r = n + _c50r; _add51i = 0 + _c50i;
    for (int r = 1; r < (int)(_add51r); r++) {
        double _cf52r = 0, _cf52i = 0;
        { int _idx = ((int)(n) - r); if (_idx >= 0 && _idx < 36) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
        double _conj53r = 0, _conj53i = 0;
        _conj53r = _cf52r; _conj53i = -(_cf52i);
        double _abs54r = 0, _abs54i = 0;
        _abs54r = c_abs(x1r, x1i); _abs54i = 0;
        double _c55r = 0, _c55i = 0;
        _c55r = 1.0; _c55i = 0;
        double _div56r = 0, _div56i = 0;
        c_div(_c55r, _c55i, r, 0, &_div56r, &_div56i);
        double _pow57r = 0, _pow57i = 0;
        c_powr(_abs54r, _abs54i, _div56r, &_pow57r, &_pow57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_conj53r, _conj53i, _pow57r, _pow57i, &_mul58r, &_mul58i);
        cRe[(r - 1)] += _mul58r; cIm[(r - 1)] += _mul58i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_548_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr4r, _attr4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _sqrt10r = 0, _sqrt10i = 0;
        c_powr(j, 0, 0.5, &_sqrt10r, &_sqrt10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr9r, _attr9i, _sqrt10r, _sqrt10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _mul11r; _add12i = _mul8i + _mul11i;
        double r = _add12r; /* +_add12ii */
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(j, 0, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr13r, _attr13i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2i; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_attr16r, _attr16i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul15r + _mul21r; _add22i = _mul15i + _mul21i;
        double im = _add22r; /* +_add22ii */
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x1r, x1i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 5.0; _c24i = 0;
        double _mod25r = 0, _mod25i = 0;
        _mod25r = fmod(j, _c24r); _mod25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _mod25r + _c26r; _add27i = _mod25i + _c26i;
        double _pow28r = 0, _pow28i = 0;
        c_powr(_abs23r, _abs23i, _add27r, &_pow28r, &_pow28i);
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x2r, x2i); _abs29i = 0;
        double _sub30r = 0, _sub30i = 0;
        _sub30r = n - j; _sub30i = 0 - 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _sub30r + _c31r; _add32i = _sub30i + _c31i;
        double _pow33r = 0, _pow33i = 0;
        c_powr(_abs29r, _abs29i, _add32r, &_pow33r, &_pow33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _pow28r + _pow33r; _add34i = _pow28i + _pow33i;
        double mag = _add34r; /* +_add34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x1r, x1i); _ang35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang35r, _ang35i, j, 0, &_mul36r, &_mul36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = j + _c38r; _add39i = 0 + _c38i;
        double _div40r = 0, _div40i = 0;
        c_div(_ang37r, _ang37i, _add39r, _add39i, &_div40r, &_div40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul36r + _div40r; _add41i = _mul36i + _div40i;
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
        double _conj47r = 0, _conj47i = 0;
        _conj47r = x2r; _conj47i = -(x2i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_conj46r, _conj46i, _conj47r, _conj47i, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 1.0; _c49i = 0;
        double _add50r = 0, _add50i = 0;
        _add50r = j + _c49r; _add50i = 0 + _c49i;
        double _div51r = 0, _div51i = 0;
        c_div(_mul48r, _mul48i, _add50r, _add50i, &_div51r, &_div51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul45r + _div51r; _add52i = _mul45i + _div51i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_549_c(double x1r, double x1i, double x2r, double x2i,
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
        _add14r = _log8r + _pow13r; _add14i = _log8i + _pow13i;
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_mul16r, _mul16i, &_cos17r, &_cos17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _add14r + _cos17r; _add18i = _add14i + _cos17i;
        double mag = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang19r, _ang19i, j, 0, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = j + _c22r; _add23i = 0 + _c22i;
        double _div24r = 0, _div24i = 0;
        c_div(_ang21r, _ang21i, _add23r, _add23i, &_div24r, &_div24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul20r + _div24r; _add25i = _mul20i + _div24i;
        double _c26r = 0, _c26i = 0;
        _c26r = 3.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(j, 0, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add25r + _sin28r; _add29i = _add25i + _sin28i;
        double angle = _add29r; /* +_add29ii */
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
        c_mul(mag, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_550_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        double coeff = _c8r; /* +_c8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        for (int k = 1; k < (int)(_add10r); k++) {
            double _arr11r = 0, _arr11i = 0;
            { int _idx = (k - 1); _arr11r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr11i = 0; }
            double _c12r = 0, _c12i = 0;
            _c12r = 2.0; _c12i = 0;
            double _pow13r = 0, _pow13i = 0;
            c_mul(_arr11r, _arr11i, _arr11r, _arr11i, &_pow13r, &_pow13i);
            double _arr14r = 0, _arr14i = 0;
            { int _idx = (k - 1); _arr14r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr14i = 0; }
            double _c15r = 0, _c15i = 0;
            _c15r = 3.0; _c15i = 0;
            double _pow16r = 0, _pow16i = 0;
            c_mul(_arr14r, _arr14i, _arr14r, _arr14i, &_pow16r, &_pow16i);
            c_mul(_pow16r, _pow16i, _arr14r, _arr14i, &_pow16r, &_pow16i);
            double _sub17r = 0, _sub17i = 0;
            _sub17r = _pow13r - _pow16r; _sub17i = _pow13i - _pow16i;
            double _c18r = 0, _c18i = 0;
            _c18r = 0.0; _c18i = 1.0;
            double _mul19r = 0, _mul19i = 0;
            c_mul(k, 0, M_PI, 0, &_mul19r, &_mul19i);
            double _div20r = 0, _div20i = 0;
            c_div(_mul19r, _mul19i, n, 0, &_div20r, &_div20i);
            double _sin21r = 0, _sin21i = 0;
            c_sin(_div20r, _div20i, &_sin21r, &_sin21i);
            double _mul22r = 0, _mul22i = 0;
            c_mul(_c18r, _c18i, _sin21r, _sin21i, &_mul22r, &_mul22i);
            double _exp23r = 0, _exp23i = 0;
            c_exp2(_mul22r, _mul22i, &_exp23r, &_exp23i);
            double _mul24r = 0, _mul24i = 0;
            c_mul(_sub17r, _sub17i, _exp23r, _exp23i, &_mul24r, &_mul24i);
            coeff += _mul24r;
            double _conj25r = 0, _conj25i = 0;
            _conj25r = x1r; _conj25i = -(x1i);
            double _mul26r = 0, _mul26i = 0;
            c_mul(k, 0, M_PI, 0, &_mul26r, &_mul26i);
            double _c27r = 0, _c27i = 0;
            _c27r = 1.0; _c27i = 0;
            double _add28r = 0, _add28i = 0;
            _add28r = j + _c27r; _add28i = 0 + _c27i;
            double _div29r = 0, _div29i = 0;
            c_div(_mul26r, _mul26i, _add28r, _add28i, &_div29r, &_div29i);
            double _cos30r = 0, _cos30i = 0;
            c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
            double _mul31r = 0, _mul31i = 0;
            c_mul(_conj25r, _conj25i, _cos30r, _cos30i, &_mul31r, &_mul31i);
            coeff += _mul31r;
        }
        double _sub32r = 0, _sub32i = 0;
        _sub32r = n - j; _sub32i = 0 - 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _sub32r + _c33r; _add34i = _sub32i + _c33i;
        for (int r = 1; r < (int)(_add34r); r++) {
            double _arr35r = 0, _arr35i = 0;
            { int _idx = (j - 1); _arr35r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr35i = 0; }
            double _arr36r = 0, _arr36i = 0;
            { int _idx = (j - 1); _arr36r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr36i = 0; }
            double _add37r = 0, _add37i = 0;
            _add37r = _arr35r + _arr36r; _add37i = _arr35i + _arr36i;
            double _abs38r = 0, _abs38i = 0;
            _abs38r = c_abs(_add37r, _add37i); _abs38i = 0;
            double _c39r = 0, _c39i = 0;
            _c39r = 1.0; _c39i = 0;
            double _add40r = 0, _add40i = 0;
            _add40r = _abs38r + _c39r; _add40i = _abs38i + _c39i;
            double _log41r = 0, _log41i = 0;
            c_log(_add40r, _add40i, &_log41r, &_log41i);
            double _mul42r = 0, _mul42i = 0;
            c_mul(_log41r, _log41i, r, 0, &_mul42r, &_mul42i);
            double _mul43r = 0, _mul43i = 0;
            c_mul(r, 0, M_PI, 0, &_mul43r, &_mul43i);
            double _div44r = 0, _div44i = 0;
            c_div(_mul43r, _mul43i, n, 0, &_div44r, &_div44i);
            double _sin45r = 0, _sin45i = 0;
            c_sin(_div44r, _div44i, &_sin45r, &_sin45i);
            double _mul46r = 0, _mul46i = 0;
            c_mul(_mul42r, _mul42i, _sin45r, _sin45i, &_mul46r, &_mul46i);
            coeff += _mul46r;
            double _abs47r = 0, _abs47i = 0;
            _abs47r = c_abs(x2r, x2i); _abs47i = 0;
            double _pow48r = 0, _pow48i = 0;
            c_powr(_abs47r, _abs47i, r, &_pow48r, &_pow48i);
            double _add49r = 0, _add49i = 0;
            _add49r = x1r + x2r; _add49i = x1i + x2i;
            double _ang50r = 0, _ang50i = 0;
            _ang50r = c_arg(_add49r, _add49i); _ang50i = 0;
            double _mul51r = 0, _mul51i = 0;
            c_mul(r, 0, _ang50r, _ang50i, &_mul51r, &_mul51i);
            double _cos52r = 0, _cos52i = 0;
            c_cos(_mul51r, _mul51i, &_cos52r, &_cos52i);
            double _mul53r = 0, _mul53i = 0;
            c_mul(_pow48r, _pow48i, _cos52r, _cos52i, &_mul53r, &_mul53i);
            coeff += _mul53r;
        }
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = coeff; cIm[_idx] = 0; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_551_c(double x1r, double x1i, double x2r, double x2i,
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
        double rec_t1 = _attr4r; /* +_attr4ii */
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x1i; _attr5i = 0;
        double imc_t1 = _attr5r; /* +_attr5ii */
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double rec_t2 = _attr6r; /* +_attr6ii */
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2i; _attr7i = 0;
        double imc_t2 = _attr7r; /* +_attr7ii */
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
        double _log11r = 0, _log11i = 0;
        c_log(_add10r, _add10i, &_log11r, &_log11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, M_PI, 0, &_mul12r, &_mul12i);
        double _div13r = 0, _div13i = 0;
        c_div(_mul12r, _mul12i, n, 0, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log11r, _log11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double mag_part1 = _mul15r; /* +_mul15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(j, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(n, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _div23r = 0, _div23i = 0;
        c_div(_mul20r, _mul20i, _div22r, _div22i, &_div23r, &_div23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_div23r, _div23i, &_cos24r, &_cos24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_log19r, _log19i, _cos24r, _cos24i, &_mul25r, &_mul25i);
        double mag_part2 = _mul25r; /* +_mul25ii */
        double _add26r = 0, _add26i = 0;
        _add26r = mag_part1 + mag_part2; _add26i = 0 + 0;
        double _prod27r = 0, _prod27i = 0;
        c_mul(rec_t1, 0, imc_t2, 0, &_prod27r, &_prod27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(_c28r, _c28i, j, 0, &_div29r, &_div29i);
        double _pow30r = 0, _pow30i = 0;
        c_powr(_prod27r, _prod27i, _div29r, &_pow30r, &_pow30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add26r + _pow30r; _add31i = _add26i + _pow30i;
        double mag_variation = _add31r; /* +_add31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(j, 0, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double angle_part1 = _mul36r; /* +_mul36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 3.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(j, 0, _c38r, _c38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_ang37r, _ang37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double angle_part2 = _mul41r; /* +_mul41ii */
        double _add42r = 0, _add42i = 0;
        _add42r = angle_part1 + angle_part2; _add42i = 0 + 0;
        double _sin43r = 0, _sin43i = 0;
        c_sin(j, 0, &_sin43r, &_sin43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(j, 0, &_cos44r, &_cos44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_sin43r, _sin43i, _cos44r, _cos44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _add42r + _mul45r; _add46i = _add42i + _mul45i;
        double angle_variation = _add46r; /* +_add46ii */
        double _cos47r = 0, _cos47i = 0;
        c_cos(angle_variation, 0, &_cos47r, &_cos47i);
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _sin49r = 0, _sin49i = 0;
        c_sin(angle_variation, 0, &_sin49r, &_sin49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c48r, _c48i, _sin49r, _sin49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _cos47r + _mul50r; _add51i = _cos47i + _mul50i;
        double complex_component = _add51r; /* +_add51ii */
        double _mul52r = 0, _mul52i = 0;
        c_mul(mag_variation, 0, complex_component, 0, &_mul52r, &_mul52i);
        double _conj53r = 0, _conj53i = 0;
        _conj53r = x1r; _conj53i = -(x1i);
        double _sin54r = 0, _sin54i = 0;
        c_sin(j, 0, &_sin54r, &_sin54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_conj53r, _conj53i, _sin54r, _sin54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _mul52r + _mul55r; _add56i = _mul52i + _mul55i;
        double _conj57r = 0, _conj57i = 0;
        _conj57r = x2r; _conj57i = -(x2i);
        double _cos58r = 0, _cos58i = 0;
        c_cos(j, 0, &_cos58r, &_cos58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(_conj57r, _conj57i, _cos58r, _cos58i, &_mul59r, &_mul59i);
        double _add60r = 0, _add60i = 0;
        _add60r = _add56r + _mul59r; _add60i = _add56i + _mul59i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add60r; cIm[_idx] = _add60i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_552_c(double x1r, double x1i, double x2r, double x2i,
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
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
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
        _c17r = 11.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log15r, _log15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul11r + _mul20r; _add21i = _mul11i + _mul20i;
        double mag_part = _add21r; /* +_add21ii */
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(j, 0, _c23r, _c23i, &_div24r, &_div24i);
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
        double angle_part = _add32r; /* +_add32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 0;
        double intricate_sum = _c33r; /* +_c33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        for (int k = 1; k < (int)(_add35r); k++) {
            double _attr36r = 0, _attr36i = 0;
            _attr36r = x1r; _attr36i = 0;
            double _pow37r = 0, _pow37i = 0;
            c_powr(_attr36r, _attr36i, k, &_pow37r, &_pow37i);
            double _attr38r = 0, _attr38i = 0;
            _attr38r = x2i; _attr38i = 0;
            double _pow39r = 0, _pow39i = 0;
            c_powr(_attr38r, _attr38i, k, &_pow39r, &_pow39i);
            double _sub40r = 0, _sub40i = 0;
            _sub40r = _pow37r - _pow39r; _sub40i = _pow37i - _pow39i;
            double _mul41r = 0, _mul41i = 0;
            c_mul(k, 0, M_PI, 0, &_mul41r, &_mul41i);
            double _c42r = 0, _c42i = 0;
            _c42r = 1.0; _c42i = 0;
            double _add43r = 0, _add43i = 0;
            _add43r = j + _c42r; _add43i = 0 + _c42i;
            double _div44r = 0, _div44i = 0;
            c_div(_mul41r, _mul41i, _add43r, _add43i, &_div44r, &_div44i);
            double _sin45r = 0, _sin45i = 0;
            c_sin(_div44r, _div44i, &_sin45r, &_sin45i);
            double _mul46r = 0, _mul46i = 0;
            c_mul(_sub40r, _sub40i, _sin45r, _sin45i, &_mul46r, &_mul46i);
            intricate_sum += _mul46r;
        }
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 1.0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c47r, _c47i, angle_part, 0, &_mul48r, &_mul48i);
        double _exp49r = 0, _exp49i = 0;
        c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(mag_part, 0, _exp49r, _exp49i, &_mul50r, &_mul50i);
        double _conj51r = 0, _conj51i = 0;
        _conj51r = x1r; _conj51i = -(x1i);
        double _c52r = 0, _c52i = 0;
        _c52r = 5.0; _c52i = 0;
        double _mod53r = 0, _mod53i = 0;
        _mod53r = fmod(j, _c52r); _mod53i = 0;
        double _pow54r = 0, _pow54i = 0;
        c_powr(x2r, x2i, _mod53r, &_pow54r, &_pow54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_conj51r, _conj51i, _pow54r, _pow54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _mul50r + _mul55r; _add56i = _mul50i + _mul55i;
        double _add57r = 0, _add57i = 0;
        _add57r = _add56r + intricate_sum; _add57i = _add56i + 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add57r; cIm[_idx] = _add57i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_553_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 0.0; _c5i = 0;
        double ang_sum = _c5r; /* +_c5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        for (int k = 1; k < (int)(_add7r); k++) {
            double _abs8r = 0, _abs8i = 0;
            _abs8r = c_abs(x1r, x1i); _abs8i = 0;
            double _c9r = 0, _c9i = 0;
            _c9r = 2.0; _c9i = 0;
            double _pow10r = 0, _pow10i = 0;
            c_mul(k, 0, k, 0, &_pow10r, &_pow10i);
            double _add11r = 0, _add11i = 0;
            _add11r = _abs8r + _pow10r; _add11i = _abs8i + _pow10i;
            double _log12r = 0, _log12i = 0;
            c_log(_add11r, _add11i, &_log12r, &_log12i);
            double _attr13r = 0, _attr13i = 0;
            _attr13r = x2r; _attr13i = 0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(k, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
            double _sin15r = 0, _sin15i = 0;
            c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_log12r, _log12i, _sin15r, _sin15i, &_mul16r, &_mul16i);
            double _attr17r = 0, _attr17i = 0;
            _attr17r = x1i; _attr17i = 0;
            double _mul18r = 0, _mul18i = 0;
            c_mul(k, 0, _attr17r, _attr17i, &_mul18r, &_mul18i);
            double _cos19r = 0, _cos19i = 0;
            c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
            double _add20r = 0, _add20i = 0;
            _add20r = _mul16r + _cos19r; _add20i = _mul16i + _cos19i;
            double term_mag = _add20r; /* +_add20ii */
            double _ang21r = 0, _ang21i = 0;
            _ang21r = c_arg(x2r, x2i); _ang21i = 0;
            double _sqrt22r = 0, _sqrt22i = 0;
            c_powr(k, 0, 0.5, &_sqrt22r, &_sqrt22i);
            double _mul23r = 0, _mul23i = 0;
            c_mul(_ang21r, _ang21i, _sqrt22r, _sqrt22i, &_mul23r, &_mul23i);
            double _c24r = 0, _c24i = 0;
            _c24r = 2.0; _c24i = 0;
            double _div25r = 0, _div25i = 0;
            c_div(k, 0, _c24r, _c24i, &_div25r, &_div25i);
            double _sin26r = 0, _sin26i = 0;
            c_sin(_div25r, _div25i, &_sin26r, &_sin26i);
            double _add27r = 0, _add27i = 0;
            _add27r = _mul23r + _sin26r; _add27i = _mul23i + _sin26i;
            double term_ang = _add27r; /* +_add27ii */
            mag_sum += term_mag;
            ang_sum += term_ang;
        }
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c28r, _c28i, ang_sum, 0, &_mul29r, &_mul29i);
        double _exp30r = 0, _exp30i = 0;
        c_exp2(_mul29r, _mul29i, &_exp30r, &_exp30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag_sum, 0, _exp30r, _exp30i, &_mul31r, &_mul31i);
        double _conj32r = 0, _conj32i = 0;
        _conj32r = x1r; _conj32i = -(x1i);
        double _pow33r = 0, _pow33i = 0;
        c_powr(x2r, x2i, j, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_conj32r, _conj32i, _pow33r, _pow33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_554_c(double x1r, double x1i, double x2r, double x2i,
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
    _attr3r = x2r; _attr3i = 0;
    double rec2 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double imc1 = _attr4r; /* +_attr4ii */
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc2 = _attr5r; /* +_attr5ii */
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _pow8r = 0, _pow8i = 0;
        c_powr(rec1, 0, j, &_pow8r, &_pow8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, M_PI, 0, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 4.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_mul9r, _mul9i, _c10r, _c10i, &_div11r, &_div11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_div11r, _div11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_pow8r, _pow8i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double term1 = _mul13r; /* +_mul13ii */
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, M_PI, 0, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _div16r = 0, _div16i = 0;
        c_div(_mul14r, _mul14i, _c15r, _c15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(imc2, 0, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double term2 = _mul18r; /* +_mul18ii */
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
        double _log22r = 0, _log22i = 0;
        c_log(_add21r, _add21i, &_log22r, &_log22i);
        double _attr23r = 0, _attr23i = 0;
        _attr23r = x2r; _attr23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 5.0; _c24i = 0;
        double _mod25r = 0, _mod25i = 0;
        _mod25r = fmod(j, _c24r); _mod25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _mod25r + _c26r; _add27i = _mod25i + _c26i;
        double _pow28r = 0, _pow28i = 0;
        c_powr(_attr23r, _attr23i, _add27r, &_pow28r, &_pow28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log22r, _log22i, _pow28r, _pow28i, &_mul29r, &_mul29i);
        double term3 = _mul29r; /* +_mul29ii */
        double _add30r = 0, _add30i = 0;
        _add30r = x1r + x2r; _add30i = x1i + x2i;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(_add30r, _add30i); _abs31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.5; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, j, 0, &_mul33r, &_mul33i);
        double _pow34r = 0, _pow34i = 0;
        c_powr(_abs31r, _abs31i, _mul33r, &_pow34r, &_pow34i);
        double term4 = _pow34r; /* +_pow34ii */
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(x1r, x1i); _ang35i = 0;
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang36r, _ang36i, j, 0, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _ang35r + _mul37r; _add38i = _ang35i + _mul37i;
        double angle = _add38r; /* +_add38ii */
        double _add39r = 0, _add39i = 0;
        _add39r = term1 + term2; _add39i = 0 + 0;
        double _add40r = 0, _add40i = 0;
        _add40r = _add39r + term3; _add40i = _add39i + 0;
        double _abs41r = 0, _abs41i = 0;
        _abs41r = c_abs(_add40r, _add40i); _abs41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = _abs41r + term4; _add42i = _abs41i + 0;
        double magnitude = _add42r; /* +_add42ii */
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
    double _c49r = 0, _c49i = 0;
    _c49r = 1.0; _c49i = 0;
    double _add50r = 0, _add50i = 0;
    _add50r = n + _c49r; _add50i = 0 + _c49i;
    for (int k = 1; k < (int)(_add50r); k++) {
        double _cf51r = 0, _cf51i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _sin53r = 0, _sin53i = 0;
        c_sin(k, 0, &_sin53r, &_sin53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c52r, _c52i, _sin53r, _sin53i, &_mul54r, &_mul54i);
        double _exp55r = 0, _exp55i = 0;
        c_exp2(_mul54r, _mul54i, &_exp55r, &_exp55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_cf51r, _cf51i, _exp55r, _exp55i, &_mul56r, &_mul56i);
        double _cf57r = 0, _cf57i = 0;
        { int _idx = (k % (int)(n)); if (_idx >= 0 && _idx < 36) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
        double _conj58r = 0, _conj58i = 0;
        _conj58r = _cf57r; _conj58i = -(_cf57i);
        double _add59r = 0, _add59i = 0;
        _add59r = _mul56r + _conj58r; _add59i = _mul56i + _conj58i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_555_c(double x1r, double x1i, double x2r, double x2i,
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
        double _pow5r = 0, _pow5i = 0;
        c_powr(_attr4r, _attr4i, j, &_pow5r, &_pow5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 3.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(j, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
        double _pow11r = 0, _pow11i = 0;
        c_powr(_attr6r, _attr6i, _add10r, &_pow11r, &_pow11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _pow5r + _pow11r; _add12i = _pow5i + _pow11i;
        double real_part = _add12r; /* +_add12ii */
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 4.0; _c14i = 0;
        double _mod15r = 0, _mod15i = 0;
        _mod15r = fmod(j, _c14r); _mod15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _mod15r + _c16r; _add17i = _mod15i + _c16i;
        double _pow18r = 0, _pow18i = 0;
        c_powr(_attr13r, _attr13i, _add17r, &_pow18r, &_pow18i);
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x2i; _attr19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _mod21r = 0, _mod21i = 0;
        _mod21r = fmod(j, _c20r); _mod21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _mod21r + _c22r; _add23i = _mod21i + _c22i;
        double _pow24r = 0, _pow24i = 0;
        c_powr(_attr19r, _attr19i, _add23r, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _pow18r + _pow24r; _add25i = _pow18i + _pow24i;
        double imag_part = _add25r; /* +_add25ii */
        double _add26r = 0, _add26i = 0;
        _add26r = real_part + imag_part; _add26i = 0 + 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul27r, &_mul27i);
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_mul27r, _mul27i); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs28r + _c29r; _add30i = _abs28i + _c29i;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _add26r + _log31r; _add32i = _add26i + _log31i;
        double magnitude = _add32r; /* +_add32ii */
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x1r, x1i); _ang33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, _ang33r, _ang33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(j, 0, _ang36r, _ang36i, &_mul37r, &_mul37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_mul37r, _mul37i, &_cos38r, &_cos38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _sin35r + _cos38r; _add39i = _sin35i + _cos38i;
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
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_556_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec[35];
    for (int _li = 0; _li < 35; _li++) {
        rec[_li] = _attr2r + (_attr3r - _attr2r) * _li / 34.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc[35];
    for (int _li = 0; _li < 35; _li++) {
        imc[_li] = _attr4r + (_attr5r - _attr4r) * _li / 34.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = n + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _arr8r, _arr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr11i = 0; }
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _arr11r, _arr11i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin10r + _cos13r; _add14i = _sin10i + _cos13i;
        double _add15r = 0, _add15i = 0;
        _add15r = x1r + x2r; _add15i = x1i + x2i;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(_add15r, _add15i); _ang16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _add14r + _ang16r; _add17i = _add14i + _ang16i;
        double angle = _add17r; /* +_add17ii */
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr18i = 0; }
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_mul(_arr18r, _arr18i, _arr18r, _arr18i, &_pow20r, &_pow20i);
        double _arr21r = 0, _arr21i = 0;
        { int _idx = (j - 1); _arr21r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr21i = 0; }
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(_arr21r, _arr21i, _arr21r, _arr21i, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _pow20r + _pow23r; _add24i = _pow20i + _pow23i;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_add24r, _add24i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.5; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(j, 0, 1.5, &_pow30r, &_pow30i);
        double _unk31r = 0, _unk31i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='j', ctx=Load())) */
        double _arr32r = 0, _arr32i = 0;
        { int _idx = (int)(_unk31r); _arr32r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr32i = 0; }
        double _unk33r = 0, _unk33i = 0;
        /* WARNING: unhandled node Slice(upper=Name(id='j', ctx=Load())) */
        double _arr34r = 0, _arr34i = 0;
        { int _idx = (int)(_unk33r); _arr34r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr34i = 0; }
        double _add35r = 0, _add35i = 0;
        _add35r = _arr32r + _arr34r; _add35i = _arr32i + _arr34i;
        double _add36r = 0, _add36i = 0;
        _add36r = _pow30r + _add35r; _add36i = _pow30i + _add35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_log28r, _log28i, _add36r, _add36i, &_mul37r, &_mul37i);
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

static void poly_557_c(double x1r, double x1i, double x2r, double x2i,
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
        double mag_sum = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = k + _c5r; _add6i = 0 + _c5i;
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
            double _mul16r = 0, _mul16i = 0;
            c_mul(x1r, x1i, j, 0, &_mul16r, &_mul16i);
            double _c17r = 0, _c17i = 0;
            _c17r = 1.0; _c17i = 0;
            double _add18r = 0, _add18i = 0;
            _add18r = j + _c17r; _add18i = 0 + _c17i;
            double _div19r = 0, _div19i = 0;
            c_div(x2r, x2i, _add18r, _add18i, &_div19r, &_div19i);
            double _add20r = 0, _add20i = 0;
            _add20r = _mul16r + _div19r; _add20i = _mul16i + _div19i;
            double _abs21r = 0, _abs21i = 0;
            _abs21r = c_abs(_add20r, _add20i); _abs21i = 0;
            double _c22r = 0, _c22i = 0;
            _c22r = 1.0; _c22i = 0;
            double _add23r = 0, _add23i = 0;
            _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
            double _log24r = 0, _log24i = 0;
            c_log(_add23r, _add23i, &_log24r, &_log24i);
            double _add25r = 0, _add25i = 0;
            _add25r = _mul15r + _log24r; _add25i = _mul15i + _log24i;
            mag_sum += _add25r;
        }
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _sub27r = 0, _sub27i = 0;
        _sub27r = x1r - x2r; _sub27i = x1i - x2i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_sub27r, _sub27i); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 10.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_abs28r, _abs28i, _c29r, _c29i, &_div30r, &_div30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _c26r + _div30r; _add31i = _c26i + _div30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(mag_sum, 0, _add31r, _add31i, &_mul32r, &_mul32i);
        double magnitude = _mul32r; /* +_mul32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 0;
        double angle_sum = _c33r; /* +_c33ii */
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = k + _c34r; _add35i = 0 + _c34i;
        for (int j = 1; j < (int)(_add35r); j++) {
            double _mul36r = 0, _mul36i = 0;
            c_mul(j, 0, x2r, x2i, &_mul36r, &_mul36i);
            double _add37r = 0, _add37i = 0;
            _add37r = x1r + _mul36r; _add37i = x1i + _mul36i;
            double _ang38r = 0, _ang38i = 0;
            _ang38r = c_arg(_add37r, _add37i); _ang38i = 0;
            double _mul39r = 0, _mul39i = 0;
            c_mul(j, 0, M_PI, 0, &_mul39r, &_mul39i);
            double _c40r = 0, _c40i = 0;
            _c40r = 9.0; _c40i = 0;
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
            _c46r = 11.0; _c46i = 0;
            double _div47r = 0, _div47i = 0;
            c_div(_mul45r, _mul45i, _c46r, _c46i, &_div47r, &_div47i);
            double _cos48r = 0, _cos48i = 0;
            c_cos(_div47r, _div47i, &_cos48r, &_cos48i);
            double _mul49r = 0, _mul49i = 0;
            c_mul(_ang44r, _ang44i, _cos48r, _cos48i, &_mul49r, &_mul49i);
            double _sub50r = 0, _sub50i = 0;
            _sub50r = _mul43r - _mul49r; _sub50i = _mul43i - _mul49i;
            angle_sum += _sub50r;
        }
        double _div51r = 0, _div51i = 0;
        c_div(angle_sum, 0, k, 0, &_div51r, &_div51i);
        double angle = _div51r; /* +_div51ii */
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
        c_mul(magnitude, 0, _add56r, _add56i, &_mul57r, &_mul57i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_558_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr4r, _attr4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _sqrt10r = 0, _sqrt10i = 0;
        c_powr(j, 0, 0.5, &_sqrt10r, &_sqrt10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_attr9r, _attr9i, _sqrt10r, _sqrt10i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _mul11r; _add12i = _mul8i + _mul11i;
        double r = _add12r; /* +_add12ii */
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x1i; _attr13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _attr13r, _attr13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2i; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _sin15r + _cos18r; _add19i = _sin15i + _cos18i;
        double _add20r = 0, _add20i = 0;
        _add20r = x1r + x2r; _add20i = x1i + x2i;
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(_add20r, _add20i); _ang21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _add19r + _ang21r; _add22i = _add19i + _ang21i;
        double theta = _add22r; /* +_add22ii */
        for (int k = 1; k < 4; k++) {
            double _attr23r = 0, _attr23i = 0;
            _attr23r = x1r; _attr23i = 0;
            double _mul24r = 0, _mul24i = 0;
            c_mul(_attr23r, _attr23i, k, 0, &_mul24r, &_mul24i);
            double _c25r = 0, _c25i = 0;
            _c25r = 1.0; _c25i = 0;
            double _add26r = 0, _add26i = 0;
            _add26r = j + _c25r; _add26i = 0 + _c25i;
            double _div27r = 0, _div27i = 0;
            c_div(_mul24r, _mul24i, _add26r, _add26i, &_div27r, &_div27i);
            r += _div27r;
            double _mul28r = 0, _mul28i = 0;
            c_mul(k, 0, M_PI, 0, &_mul28r, &_mul28i);
            double _div29r = 0, _div29i = 0;
            c_div(_mul28r, _mul28i, j, 0, &_div29r, &_div29i);
            double _sin30r = 0, _sin30i = 0;
            c_sin(_div29r, _div29i, &_sin30r, &_sin30i);
            theta += _sin30r;
        }
        double _cos31r = 0, _cos31i = 0;
        c_cos(theta, 0, &_cos31r, &_cos31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(theta, 0, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c32r, _c32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(r, 0, _add35r, _add35i, &_mul36r, &_mul36i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_559_c(double x1r, double x1i, double x2r, double x2i,
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
        double temp_real = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        double temp_imag = _c5r; /* +_c5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        for (int k = 1; k < (int)(_add7r); k++) {
            double _attr8r = 0, _attr8i = 0;
            _attr8r = x1r; _attr8i = 0;
            double _pow9r = 0, _pow9i = 0;
            c_powr(_attr8r, _attr8i, k, &_pow9r, &_pow9i);
            double _attr10r = 0, _attr10i = 0;
            _attr10r = x2r; _attr10i = 0;
            double _mul11r = 0, _mul11i = 0;
            c_mul(k, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
            double _sin12r = 0, _sin12i = 0;
            c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_pow9r, _pow9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
            double _c14r = 0, _c14i = 0;
            _c14r = 1.0; _c14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = k + _c14r; _add15i = 0 + _c14i;
            double _div16r = 0, _div16i = 0;
            c_div(_mul13r, _mul13i, _add15r, _add15i, &_div16r, &_div16i);
            temp_real += _div16r;
            double _attr17r = 0, _attr17i = 0;
            _attr17r = x2i; _attr17i = 0;
            double _sub18r = 0, _sub18i = 0;
            _sub18r = j - k; _sub18i = 0 - 0;
            double _c19r = 0, _c19i = 0;
            _c19r = 1.0; _c19i = 0;
            double _add20r = 0, _add20i = 0;
            _add20r = _sub18r + _c19r; _add20i = _sub18i + _c19i;
            double _pow21r = 0, _pow21i = 0;
            c_powr(_attr17r, _attr17i, _add20r, &_pow21r, &_pow21i);
            double _sub22r = 0, _sub22i = 0;
            _sub22r = j - k; _sub22i = 0 - 0;
            double _c23r = 0, _c23i = 0;
            _c23r = 1.0; _c23i = 0;
            double _add24r = 0, _add24i = 0;
            _add24r = _sub22r + _c23r; _add24i = _sub22i + _c23i;
            double _attr25r = 0, _attr25i = 0;
            _attr25r = x1i; _attr25i = 0;
            double _mul26r = 0, _mul26i = 0;
            c_mul(_add24r, _add24i, _attr25r, _attr25i, &_mul26r, &_mul26i);
            double _cos27r = 0, _cos27i = 0;
            c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
            double _mul28r = 0, _mul28i = 0;
            c_mul(_pow21r, _pow21i, _cos27r, _cos27i, &_mul28r, &_mul28i);
            double _sub29r = 0, _sub29i = 0;
            _sub29r = j - k; _sub29i = 0 - 0;
            double _c30r = 0, _c30i = 0;
            _c30r = 2.0; _c30i = 0;
            double _add31r = 0, _add31i = 0;
            _add31r = _sub29r + _c30r; _add31i = _sub29i + _c30i;
            double _div32r = 0, _div32i = 0;
            c_div(_mul28r, _mul28i, _add31r, _add31i, &_div32r, &_div32i);
            temp_imag += _div32r;
        }
        double _add33r = 0, _add33i = 0;
        _add33r = temp_real + temp_imag; _add33i = 0 + 0;
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(_add33r, _add33i); _abs34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _abs34r + _c35r; _add36i = _abs34i + _c35i;
        double _log37r = 0, _log37i = 0;
        c_log(_add36r, _add36i, &_log37r, &_log37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_log37r, _log37i, j, 0, &_mul38r, &_mul38i);
        double magnitude = _mul38r; /* +_mul38ii */
        double _attr39r = 0, _attr39i = 0;
        _attr39r = x1r; _attr39i = 0;
        double _attr40r = 0, _attr40i = 0;
        _attr40r = x2i; _attr40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_attr39r, _attr39i, _attr40r, _attr40i, &_mul41r, &_mul41i);
        double _div42r = 0, _div42i = 0;
        c_div(_mul41r, _mul41i, j, 0, &_div42r, &_div42i);
        double _add43r = 0, _add43i = 0;
        _add43r = x1r + x2r; _add43i = x1i + x2i;
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(_add43r, _add43i); _ang44i = 0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(j, 0, _ang44r, _ang44i, &_mul45r, &_mul45i);
        double _sin46r = 0, _sin46i = 0;
        c_sin(_mul45r, _mul45i, &_sin46r, &_sin46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _div42r + _sin46r; _add47i = _div42i + _sin46i;
        double angle = _add47r; /* +_add47ii */
        double _cos48r = 0, _cos48i = 0;
        c_cos(angle, 0, &_cos48r, &_cos48i);
        double _sin49r = 0, _sin49i = 0;
        c_sin(angle, 0, &_sin49r, &_sin49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_sin49r, _sin49i, _c50r, _c50i, &_mul51r, &_mul51i);
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

static void poly_560_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 35.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double re1 = _attr2r; /* +_attr2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1i; _attr3i = 0;
    double im1 = _attr3r; /* +_attr3ii */
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2r; _attr4i = 0;
    double re2 = _attr4r; /* +_attr4ii */
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double im2 = _attr5r; /* +_attr5ii */
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
        double _div12r = 0, _div12i = 0;
        c_div(_mul11r, _mul11i, n, 0, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log10r, _log10i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _sqrt16r = 0, _sqrt16i = 0;
        c_powr(j, 0, 0.5, &_sqrt16r, &_sqrt16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _sqrt16r; _add17i = _abs15i + _sqrt16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = n + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_mul19r, _mul19i, _add21r, _add21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log18r, _log18i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul14r + _mul24r; _add25i = _mul14i + _mul24i;
        double magnitude = _add25r; /* +_add25ii */
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
        _c32r = 5.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(j, 0, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang31r, _ang31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul30r + _mul35r; _add36i = _mul30i + _mul35i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_561_c(double x1r, double x1i, double x2r, double x2i,
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
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr4r, _attr4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr9r, _attr9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _mul13r; _add14i = _mul8i + _mul13i;
        double r = _add14r; /* +_add14ii */
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
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _mul17r - _mul20r; _sub21i = _mul17i - _mul20i;
        double theta = _sub21r; /* +_sub21ii */
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powr(_abs22r, _abs22i, j, &_pow23r, &_pow23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _c24r + j; _add25i = _c24i + 0;
        double _div26r = 0, _div26i = 0;
        c_div(_pow23r, _pow23i, _add25r, _add25i, &_div26r, &_div26i);
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x2r, x2i); _abs27i = 0;
        double _sqrt28r = 0, _sqrt28i = 0;
        c_powr(j, 0, 0.5, &_sqrt28r, &_sqrt28i);
        double _pow29r = 0, _pow29i = 0;
        c_powr(_abs27r, _abs27i, _sqrt28r, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _div26r + _pow29r; _add30i = _div26i + _pow29i;
        double mag_variation = _add30r; /* +_add30ii */
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, theta, 0, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = r + _mul32r; _add33i = 0 + _mul32i;
        double _sin34r = 0, _sin34i = 0;
        c_sin(j, 0, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = mag_variation + _sin34r; _add35i = 0 + _sin34i;
        double _cos36r = 0, _cos36i = 0;
        c_cos(j, 0, &_cos36r, &_cos36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _add35r - _cos36r; _sub37i = _add35i - _cos36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_add33r, _add33i, _sub37r, _sub37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_562_c(double x1r, double x1i, double x2r, double x2i,
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
        _abs18r = c_abs(rec, 0); _abs18i = 0;
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
        double _sub24r = 0, _sub24i = 0;
        _sub24r = n - j; _sub24i = 0 - 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _sub24r + _c25r; _add26i = _sub24i + _c25i;
        double _sqrt27r = 0, _sqrt27i = 0;
        c_powr(_add26r, _add26i, 0.5, &_sqrt27r, &_sqrt27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _pow23r + _sqrt27r; _add28i = _pow23i + _sqrt27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log21r, _log21i, _add28r, _add28i, &_mul29r, &_mul29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(j, 0, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_mul29r, _mul29i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul31r + 0; _add32i = _mul31i + 0;
        double mag = _add32r; /* +_add32ii */
        double _mul33r = 0, _mul33i = 0;
        c_mul(rec, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 7.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(imc, 0, M_PI, 0, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 5.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_mul37r, _mul37i, _c38r, _c38i, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _sin36r + _cos40r; _add41i = _sin36i + _cos40i;
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x1r, x1i); _ang42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = _add41r + _ang42r; _add43i = _add41i + _ang42i;
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x2r, x2i); _ang44i = 0;
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _add43r - _ang44r; _sub45i = _add43i - _ang44i;
        double angle = _sub45r; /* +_sub45ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_563_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
                     double *cRe, double *cIm, int *nCoeffs) {
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
                     double *cRe, double *cIm, int *nCoeffs) {
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
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + j; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double mag_part1 = _log6r; /* +_log6ii */
        double mag_part2 = 0; /* +0i */
        double _sqrt7r = 0, _sqrt7i = 0;
        c_powr(j, 0, 0.5, &_sqrt7r, &_sqrt7i);
        double _add8r = 0, _add8i = 0;
        _add8r = mag_part2 + _sqrt7r; _add8i = 0 + _sqrt7i;
        double _mul9r = 0, _mul9i = 0;
        c_mul(mag_part1, 0, _add8r, _add8i, &_mul9r, &_mul9i);
        double magnitude = _mul9r; /* +_mul9ii */
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double angle_part1 = _sin12r; /* +_sin12ii */
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _div16r = 0, _div16i = 0;
        c_div(_ang13r, _ang13i, _add15r, _add15i, &_div16r, &_div16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_div16r, _div16i, &_cos17r, &_cos17i);
        double angle_part2 = _cos17r; /* +_cos17ii */
        double _add18r = 0, _add18i = 0;
        _add18r = angle_part1 + angle_part2; _add18i = 0 + 0;
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
        c_mul(magnitude, 0, _add23r, _add23i, &_mul24r, &_mul24i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1i; _attr25i = 0;
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x2i; _attr26i = 0;
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _attr25r - _attr26r; _sub27i = _attr25i - _attr26i;
        double _sin28r = 0, _sin28i = 0;
        c_sin(j, 0, &_sin28r, &_sin28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_mul(_sin28r, _sin28i, _sin28r, _sin28i, &_pow30r, &_pow30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_sub27r, _sub27i, _pow30r, _pow30i, &_mul31r, &_mul31i);
        cRe[(j - 1)] += _mul31r; cIm[(j - 1)] += _mul31i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_567_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec[26];
    for (int _li = 0; _li < 26; _li++) {
        rec[_li] = _attr2r + (_attr3r - _attr2r) * _li / 25.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc[26];
    for (int _li = 0; _li < 26; _li++) {
        imc[_li] = _attr4r + (_attr5r - _attr4r) * _li / 25.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = degree + _c6r; _add7i = 0 + _c6i;
    for (int r = 1; r < (int)(_add7r); r++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (r - 1); _arr8r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr8i = 0; }
        double _mul9r = 0, _mul9i = 0;
        c_mul(x1r, x1i, _arr8r, _arr8i, &_mul9r, &_mul9i);
        double _arr10r = 0, _arr10i = 0;
        { int _idx = (r - 1); _arr10r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr10i = 0; }
        double _mul11r = 0, _mul11i = 0;
        c_mul(x2r, x2i, _arr10r, _arr10i, &_mul11r, &_mul11i);
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
        double _mul18r = 0, _mul18i = 0;
        c_mul(r, 0, M_PI, 0, &_mul18r, &_mul18i);
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
        double _mul24r = 0, _mul24i = 0;
        c_mul(r, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 5.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_div26r, _div26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul23r + _cos27r; _add28i = _mul23i + _cos27i;
        double mag = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _arr30r = 0, _arr30i = 0;
        { int _idx = (r - 1); _arr30r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr30i = 0; }
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _arr30r, _arr30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _arr33r = 0, _arr33i = 0;
        { int _idx = (r - 1); _arr33r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr33i = 0; }
        double _mul34r = 0, _mul34i = 0;
        c_mul(_ang32r, _ang32i, _arr33r, _arr33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul31r + _mul34r; _add35i = _mul31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(r, 0, M_PI, 0, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 6.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(_mul36r, _mul36i, _c37r, _c37i, &_div38r, &_div38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(_div38r, _div38i, &_sin39r, &_sin39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _add35r + _sin39r; _add40i = _add35i + _sin39i;
        double ang = _add40r; /* +_add40ii */
        double _cos41r = 0, _cos41i = 0;
        c_cos(ang, 0, &_cos41r, &_cos41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(ang, 0, &_sin42r, &_sin42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_sin42r, _sin42i, _c43r, _c43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos41r + _mul44r; _add45i = _cos41i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(mag, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_569_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec[26];
    for (int _li = 0; _li < 26; _li++) {
        rec[_li] = _attr2r + (_attr3r - _attr2r) * _li / 25.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc[26];
    for (int _li = 0; _li < 26; _li++) {
        imc[_li] = _attr4r + (_attr5r - _attr4r) * _li / 25.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = degree + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs8r + j; _add9i = _abs8i + 0;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr11i = 0; }
        double _mul12r = 0, _mul12i = 0;
        c_mul(_arr11r, _arr11i, M_PI, 0, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _div15r = 0, _div15i = 0;
        c_div(_mul12r, _mul12i, _add14r, _add14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _arr17r = 0, _arr17i = 0;
        { int _idx = (j - 1); _arr17r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr17i = 0; }
        double _mul18r = 0, _mul18i = 0;
        c_mul(_arr17r, _arr17i, M_PI, 0, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _div21r = 0, _div21i = 0;
        c_div(_mul18r, _mul18i, _add20r, _add20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sin16r, _sin16i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _log10r + _mul23r; _add24i = _log10i + _mul23i;
        double mag_part = _add24r; /* +_add24ii */
        double _ang25r = 0, _ang25i = 0;
        _ang25r = c_arg(x1r, x1i); _ang25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang25r, _ang25i, j, 0, &_mul26r, &_mul26i);
        double _arr27r = 0, _arr27i = 0;
        { int _idx = (j - 1); _arr27r = (_idx >= 0 && _idx < 26) ? rec[_idx] : 0.0; _arr27i = 0; }
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _div30r = 0, _div30i = 0;
        c_div(_arr27r, _arr27i, _add29r, _add29i, &_div30r, &_div30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_div30r, _div30i, &_sin31r, &_sin31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul26r + _sin31r; _add32i = _mul26i + _sin31i;
        double _arr33r = 0, _arr33i = 0;
        { int _idx = (j - 1); _arr33r = (_idx >= 0 && _idx < 26) ? imc[_idx] : 0.0; _arr33i = 0; }
        double _c34r = 0, _c34i = 0;
        _c34r = 2.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_arr33r, _arr33i, _add35r, _add35i, &_div36r, &_div36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_div36r, _div36i, &_cos37r, &_cos37i);
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _add32r - _cos37r; _sub38i = _add32i - _cos37i;
        double angle_part = _sub38r; /* +_sub38ii */
        double _cos39r = 0, _cos39i = 0;
        c_cos(angle_part, 0, &_cos39r, &_cos39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(angle_part, 0, &_sin40r, &_sin40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_sin40r, _sin40i, _c41r, _c41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cos39r + _mul42r; _add43i = _cos39i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag_part, 0, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_570_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double sum_mag = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        double sum_ang = _c5r; /* +_c5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        for (int k = 1; k < (int)(_add7r); k++) {
            double _mul8r = 0, _mul8i = 0;
            c_mul(x1r, x1i, k, 0, &_mul8r, &_mul8i);
            double _div9r = 0, _div9i = 0;
            c_div(x2r, x2i, k, 0, &_div9r, &_div9i);
            double _add10r = 0, _add10i = 0;
            _add10r = _mul8r + _div9r; _add10i = _mul8i + _div9i;
            double term = _add10r; /* +_add10ii */
            double _abs11r = 0, _abs11i = 0;
            _abs11r = c_abs(term, 0); _abs11i = 0;
            double _c12r = 0, _c12i = 0;
            _c12r = 1.0; _c12i = 0;
            double _add13r = 0, _add13i = 0;
            _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
            double _log14r = 0, _log14i = 0;
            c_log(_add13r, _add13i, &_log14r, &_log14i);
            sum_mag += _log14r;
            double _attr15r = 0, _attr15i = 0;
            _attr15r = x2r; _attr15i = 0;
            double _mul16r = 0, _mul16i = 0;
            c_mul(k, 0, _attr15r, _attr15i, &_mul16r, &_mul16i);
            double _sin17r = 0, _sin17i = 0;
            c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(x1r, x1i, _sin17r, _sin17i, &_mul18r, &_mul18i);
            double _attr19r = 0, _attr19i = 0;
            _attr19r = x1i; _attr19i = 0;
            double _mul20r = 0, _mul20i = 0;
            c_mul(k, 0, _attr19r, _attr19i, &_mul20r, &_mul20i);
            double _cos21r = 0, _cos21i = 0;
            c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
            double _mul22r = 0, _mul22i = 0;
            c_mul(x2r, x2i, _cos21r, _cos21i, &_mul22r, &_mul22i);
            double _add23r = 0, _add23i = 0;
            _add23r = _mul18r + _mul22r; _add23i = _mul18i + _mul22i;
            double angle_term = _add23r; /* +_add23ii */
            double _ang24r = 0, _ang24i = 0;
            _ang24r = c_arg(angle_term, 0); _ang24i = 0;
            sum_ang += _ang24r;
        }
        double _cos25r = 0, _cos25i = 0;
        c_cos(sum_ang, 0, &_cos25r, &_cos25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(sum_ang, 0, &_sin26r, &_sin26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sin26r, _sin26i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos25r + _mul28r; _add29i = _cos25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(sum_mag, 0, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_571_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
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
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1r; _attr8i = 0;
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr8r, _attr8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2r; _attr13i = 0;
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(k, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_attr13r, _attr13i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul12r + _mul17r; _add18i = _mul12i + _mul17i;
        double r = _add18r; /* +_add18ii */
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x1i; _attr19i = 0;
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2r; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_attr19r, _attr19i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _attr24r = 0, _attr24i = 0;
        _attr24r = x2i; _attr24i = 0;
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(k, 0, _attr25r, _attr25i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_attr24r, _attr24i, _sin27r, _sin27i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _mul23r - _mul28r; _sub29i = _mul23i - _mul28i;
        double i_part = _sub29r; /* +_sub29ii */
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs30r + j; _add31i = _abs30i + 0;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.5; _c33i = 0;
        double _pow34r = 0, _pow34i = 0;
        c_powr(j, 0, 1.5, &_pow34r, &_pow34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_log32r, _log32i, _pow34r, _pow34i, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = j + _c37r; _add38i = 0 + _c37i;
        double _log39r = 0, _log39i = 0;
        c_log(_add38r, _add38i, &_log39r, &_log39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _c36r + _log39r; _add40i = _c36i + _log39i;
        double _div41r = 0, _div41i = 0;
        c_div(_mul35r, _mul35i, _add40r, _add40i, &_div41r, &_div41i);
        double magnitude = _div41r; /* +_div41ii */
        double _ang42r = 0, _ang42i = 0;
        _ang42r = c_arg(x1r, x1i); _ang42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang42r, _ang42i, j, 0, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = j + _c44r; _add45i = 0 + _c44i;
        double _log46r = 0, _log46i = 0;
        c_log(_add45r, _add45i, &_log46r, &_log46i);
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x2r, x2i); _ang47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_log46r, _log46i, _ang47r, _ang47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul43r + _mul48r; _add49i = _mul43i + _mul48i;
        double angle = _add49r; /* +_add49ii */
        double _c50r = 0, _c50i = 0;
        _c50r = 0.0; _c50i = 1.0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c50r, _c50i, angle, 0, &_mul51r, &_mul51i);
        double _exp52r = 0, _exp52i = 0;
        c_exp2(_mul51r, _mul51i, &_exp52r, &_exp52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(magnitude, 0, _exp52r, _exp52i, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 0.0; _c54i = 1.0;
        double _mul55r = 0, _mul55i = 0;
        c_mul(_c54r, _c54i, i_part, 0, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = r + _mul55r; _add56i = 0 + _mul55i;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_mul53r, _mul53i, _add56r, _add56i, &_mul57r, &_mul57i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_572_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr4r, _attr4i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _ang10r = 0, _ang10i = 0;
        _ang10r = c_arg(x1r, x1i); _ang10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr9r, _attr9i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _mul13r; _add14i = _mul8i + _mul13i;
        double part1 = _add14r; /* +_add14ii */
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_attr15r, _attr15i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x1r; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_attr20r, _attr20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _mul19r - _mul24r; _sub25i = _mul19i - _mul24i;
        double part2 = _sub25r; /* +_sub25ii */
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs26r + j; _add27i = _abs26i + 0;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1r; _attr29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(0, 0, _attr29r, _attr29i, &_mul30r, &_mul30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_mul30r, _mul30i, &_sin31r, &_sin31i);
        double _attr32r = 0, _attr32i = 0;
        _attr32r = x2i; _attr32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(0, 0, _attr32r, _attr32i, &_mul33r, &_mul33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_mul33r, _mul33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sin31r, _sin31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _log28r + _mul35r; _add36i = _log28i + _mul35i;
        double magnitude = _add36r; /* +_add36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x1r, x1i); _ang37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang37r, _ang37i, j, 0, &_mul38r, &_mul38i);
        double _ang39r = 0, _ang39i = 0;
        _ang39r = c_arg(x2r, x2i); _ang39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 0.5; _c40i = 0;
        double _pow41r = 0, _pow41i = 0;
        c_powr(j, 0, 0.5, &_pow41r, &_pow41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang39r, _ang39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul38r + _mul42r; _add43i = _mul38i + _mul42i;
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

static void poly_574_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double deg = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = deg + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_powr(_attr4r, _attr4i, j, &_pow5r, &_pow5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_pow5r, _pow5i, _cos8r, _cos8i, &_mul9r, &_mul9i);
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x2r; _attr10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = deg + _c11r; _add12i = 0 + _c11i;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _add12r - j; _sub13i = _add12i - 0;
        double _pow14r = 0, _pow14i = 0;
        c_powr(_attr10r, _attr10i, _sub13r, &_pow14r, &_pow14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_pow14r, _pow14i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul9r + _mul18r; _add19i = _mul9i + _mul18i;
        double r_part = _add19r; /* +_add19ii */
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x1i; _attr20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_attr20r, _attr20i, j, &_pow21r, &_pow21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_pow21r, _pow21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _attr26r = 0, _attr26i = 0;
        _attr26r = x2i; _attr26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = deg + _c27r; _add28i = 0 + _c27i;
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _add28r - j; _sub29i = _add28i - 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(_attr26r, _attr26i, _sub29r, &_pow30r, &_pow30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(j, 0, _ang31r, _ang31i, &_mul32r, &_mul32i);
        double _cos33r = 0, _cos33i = 0;
        c_cos(_mul32r, _mul32i, &_cos33r, &_cos33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_pow30r, _pow30i, _cos33r, _cos33i, &_mul34r, &_mul34i);
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _mul25r - _mul34r; _sub35i = _mul25i - _mul34i;
        double im_part = _sub35r; /* +_sub35ii */
        double _add36r = 0, _add36i = 0;
        _add36r = r_part + im_part; _add36i = 0 + 0;
        double _abs37r = 0, _abs37i = 0;
        _abs37r = c_abs(_add36r, _add36i); _abs37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _abs37r + _c38r; _add39i = _abs37i + _c38i;
        double _log40r = 0, _log40i = 0;
        c_log(_add39r, _add39i, &_log40r, &_log40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 1.5; _c41i = 0;
        double _pow42r = 0, _pow42i = 0;
        c_powr(j, 0, 1.5, &_pow42r, &_pow42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_log40r, _log40i, _pow42r, _pow42i, &_mul43r, &_mul43i);
        double magnitude = _mul43r; /* +_mul43ii */
        double _ang44r = 0, _ang44i = 0;
        _ang44r = c_arg(x1r, x1i); _ang44i = 0;
        double _sin45r = 0, _sin45i = 0;
        c_sin(j, 0, &_sin45r, &_sin45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_ang44r, _ang44i, _sin45r, _sin45i, &_mul46r, &_mul46i);
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x2r, x2i); _ang47i = 0;
        double _cos48r = 0, _cos48i = 0;
        c_cos(j, 0, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_ang47r, _ang47i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul46r + _mul49r; _add50i = _mul46i + _mul49i;
        double angle = _add50r; /* +_add50ii */
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    }
    double _c57r = 0, _c57i = 0;
    _c57r = 2.0; _c57i = 0;
    double _add58r = 0, _add58i = 0;
    _add58r = deg + _c57r; _add58i = 0 + _c57i;
    for (int k = 1; k < (int)(_add58r); k++) {
        double _c59r = 0, _c59i = 0;
        _c59r = 3.0; _c59i = 0;
        double _mod60r = 0, _mod60i = 0;
        _mod60r = fmod(k, _c59r); _mod60i = 0;
        double _c61r = 0, _c61i = 0;
        _c61r = 1.0; _c61i = 0;
        double _add62r = 0, _add62i = 0;
        _add62r = _mod60r + _c61r; _add62i = _mod60i + _c61i;
        double factor = _add62r; /* +_add62ii */
        double _attr63r = 0, _attr63i = 0;
        _attr63r = x1r; _attr63i = 0;
        double _c64r = 0, _c64i = 0;
        _c64r = 5.0; _c64i = 0;
        double _mod65r = 0, _mod65i = 0;
        _mod65r = fmod(k, _c64r); _mod65i = 0;
        double _pow66r = 0, _pow66i = 0;
        c_powr(_attr63r, _attr63i, _mod65r, &_pow66r, &_pow66i);
        double _attr67r = 0, _attr67i = 0;
        _attr67r = x2i; _attr67i = 0;
        double _c68r = 0, _c68i = 0;
        _c68r = 4.0; _c68i = 0;
        double _mod69r = 0, _mod69i = 0;
        _mod69r = fmod(k, _c68r); _mod69i = 0;
        double _pow70r = 0, _pow70i = 0;
        c_powr(_attr67r, _attr67i, _mod69r, &_pow70r, &_pow70i);
        double _add71r = 0, _add71i = 0;
        _add71r = _pow66r + _pow70r; _add71i = _pow66i + _pow70i;
        double _cf72r = 0, _cf72i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf72r = cRe[_idx]; _cf72i = cIm[_idx]; } }
        double _ang73r = 0, _ang73i = 0;
        _ang73r = c_arg(_cf72r, _cf72i); _ang73i = 0;
        double _mul74r = 0, _mul74i = 0;
        c_mul(k, 0, _ang73r, _ang73i, &_mul74r, &_mul74i);
        double _sin75r = 0, _sin75i = 0;
        c_sin(_mul74r, _mul74i, &_sin75r, &_sin75i);
        double _mul76r = 0, _mul76i = 0;
        c_mul(_add71r, _add71i, _sin75r, _sin75i, &_mul76r, &_mul76i);
        { double _tr = cRe[(k - 1)]*_mul76r - cIm[(k - 1)]*_mul76i; cIm[(k - 1)] = cRe[(k - 1)]*_mul76i + cIm[(k - 1)]*_mul76r; cRe[(k - 1)] = _tr; }
    }
    for (int r = 2; r < (int)(deg); r++) {
        double _cf77r = 0, _cf77i = 0;
        { int _idx = (r - 2); if (_idx >= 0 && _idx < 36) { _cf77r = cRe[_idx]; _cf77i = cIm[_idx]; } }
        double _cf78r = 0, _cf78i = 0;
        { int _idx = r; if (_idx >= 0 && _idx < 36) { _cf78r = cRe[_idx]; _cf78i = cIm[_idx]; } }
        double _mul79r = 0, _mul79i = 0;
        c_mul(_cf77r, _cf77i, _cf78r, _cf78i, &_mul79r, &_mul79i);
        double _c80r = 0, _c80i = 0;
        _c80r = 1.0; _c80i = 0;
        double _cf81r = 0, _cf81i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf81r = cRe[_idx]; _cf81i = cIm[_idx]; } }
        double _abs82r = 0, _abs82i = 0;
        _abs82r = c_abs(_cf81r, _cf81i); _abs82i = 0;
        double _add83r = 0, _add83i = 0;
        _add83r = _c80r + _abs82r; _add83i = _c80i + _abs82i;
        double _div84r = 0, _div84i = 0;
        c_div(_mul79r, _mul79i, _add83r, _add83i, &_div84r, &_div84i);
        cRe[(r - 1)] += _div84r; cIm[(r - 1)] += _div84i;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_575_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 3.0; _c4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _c4r, _c4i, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 7.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mul5r + _c6r; _add7i = _mul5i + _c6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 10.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(_add7r, _c8r); _mod9i = 0;
        double k = _mod9r; /* +_mod9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _cos11r = 0, _cos11i = 0;
        c_cos(j, 0, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr10r, _attr10i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2r; _attr13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(k, 0, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr13r, _attr13i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _mul15r; _add16i = _mul12i + _mul15i;
        double r = _add16r; /* +_add16ii */
        double _attr17r = 0, _attr17i = 0;
        _attr17r = x1i; _attr17i = 0;
        double _sin18r = 0, _sin18i = 0;
        c_sin(j, 0, &_sin18r, &_sin18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_attr17r, _attr17i, _sin18r, _sin18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _cos21r = 0, _cos21i = 0;
        c_cos(k, 0, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_attr20r, _attr20i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul19r + _mul22r; _add23i = _mul19i + _mul22i;
        double im = _add23r; /* +_add23ii */
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
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _mod31r = 0, _mod31i = 0;
        _mod31r = fmod(j, _c30r); _mod31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = _c29r + _mod31r; _add32i = _c29i + _mod31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_log28r, _log28i, _add32r, _add32i, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _div36r = 0, _div36i = 0;
        c_div(_mul33r, _mul33i, _add35r, _add35i, &_div36r, &_div36i);
        double magnitude = _div36r; /* +_div36ii */
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x1r, x1i); _ang37i = 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(k, 0, &_sin38r, &_sin38i);
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

static void poly_576_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _div4r = 0, _div4i = 0;
        c_div(j, 0, degree, 0, &_div4r, &_div4i);
        double r = _div4r; /* +_div4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 3.0; _c7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c7r, _c7i, j, 0, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow6r + _mul8r; _add9i = _pow6i + _mul8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _add9r + _c10r; _add11i = _add9i + _c10i;
        double k = _add11r; /* +_add11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _abs13r; _add14i = _abs12i + _abs13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(r, 0, k, 0, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _add14r + _mul15r; _add16i = _add14i + _mul15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _sin19r = 0, _sin19i = 0;
        c_sin(j, 0, &_sin19r, &_sin19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(k, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_sin19r, _sin19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _c18r + _mul21r; _add22i = _c18i + _mul21i;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log17r, _log17i, _add22r, _add22i, &_mul23r, &_mul23i);
        double mag = _mul23r; /* +_mul23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _sin25r = 0, _sin25i = 0;
        c_sin(k, 0, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(r, 0, j, 0, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang27r, _ang27i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul26r + _mul30r; _add31i = _mul26i + _mul30i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_577_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec_seq[26];
    for (int _li = 0; _li < 26; _li++) {
        rec_seq[_li] = _attr2r + (_attr3r - _attr2r) * _li / 25.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc_seq[26];
    for (int _li = 0; _li < 26; _li++) {
        imc_seq[_li] = _attr4r + (_attr5r - _attr4r) * _li / 25.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = degree + _c6r; _add7i = 0 + _c6i;
    for (int j = 1; j < (int)(_add7r); j++) {
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 26) ? rec_seq[_idx] : 0.0; _arr8i = 0; }
        double r = _arr8r; /* +_arr8ii */
        double _arr9r = 0, _arr9i = 0;
        { int _idx = (j - 1); _arr9r = (_idx >= 0 && _idx < 26) ? imc_seq[_idx] : 0.0; _arr9i = 0; }
        double im = _arr9r; /* +_arr9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(r, 0); _abs10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(im, 0); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _abs11r; _add12i = _abs10i + _abs11i;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _add12r + _c13r; _add14i = _add12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, M_PI, 0, &_mul17r, &_mul17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul17r, _mul17i, r, 0, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log15r, _log15i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c21r, _c21i, M_PI, 0, &_mul22r, &_mul22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul22r, _mul22i, im, 0, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul20r + _cos24r; _add25i = _mul20i + _cos24i;
        double mag = _add25r; /* +_add25ii */
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang26r, _ang26i, j, 0, &_mul27r, &_mul27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(im, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul27r + _sin29r; _add30i = _mul27i + _sin29i;
        double ang = _add30r; /* +_add30ii */
        double _cos31r = 0, _cos31i = 0;
        c_cos(ang, 0, &_cos31r, &_cos31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(ang, 0, &_sin32r, &_sin32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin32r, _sin32i, _c33r, _c33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag, 0, _add35r, _add35i, &_mul36r, &_mul36i);
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x2r; _conj37i = -(x2i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(j, 0, M_PI, 0, &_mul38r, &_mul38i);
        double _div39r = 0, _div39i = 0;
        c_div(_mul38r, _mul38i, degree, 0, &_div39r, &_div39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_div39r, _div39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj37r, _conj37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul36r + _mul41r; _add42i = _mul36i + _mul41i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_578_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_powr(_abs4r, _abs4i, j, &_pow5r, &_pow5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_pow5r, _pow5i, _log9r, _log9i, &_mul10r, &_mul10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = degree + _c12r; _add13i = 0 + _c12i;
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _add13r - j; _sub14i = _add13i - 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_abs11r, _abs11i, _sub14r, &_pow15r, &_pow15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(j, 0, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_pow15r, _pow15i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul10r + _mul17r; _add18i = _mul10i + _mul17i;
        double mag_part = _add18r; /* +_add18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _cos20r = 0, _cos20i = 0;
        c_cos(j, 0, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang19r, _ang19i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(j, 0, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
        double angle_part = _add25r; /* +_add25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_c26r, _c26i, angle_part, 0, &_mul27r, &_mul27i);
        double _exp28r = 0, _exp28i = 0;
        c_exp2(_mul27r, _mul27i, &_exp28r, &_exp28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(mag_part, 0, _exp28r, _exp28i, &_mul29r, &_mul29i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    }
    double _c30r = 0, _c30i = 0;
    _c30r = 2.0; _c30i = 0;
    double _add31r = 0, _add31i = 0;
    _add31r = degree + _c30r; _add31i = 0 + _c30i;
    for (int k = 1; k < (int)(_add31r); k++) {
        double _conj32r = 0, _conj32i = 0;
        _conj32r = x1r; _conj32i = -(x1i);
        double _attr33r = 0, _attr33i = 0;
        _attr33r = x2r; _attr33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_conj32r, _conj32i, _attr33r, _attr33i, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = k + _c35r; _add36i = 0 + _c35i;
        double _div37r = 0, _div37i = 0;
        c_div(_mul34r, _mul34i, _add36r, _add36i, &_div37r, &_div37i);
        cRe[(k - 1)] += _div37r; cIm[(k - 1)] += _div37i;
        double _cf38r = 0, _cf38i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(k, 0, M_PI, 0, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 12.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(_mul40r, _mul40i, _c41r, _c41i, &_div42r, &_div42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_div42r, _div42i, &_sin43r, &_sin43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _c39r + _sin43r; _add44i = _c39i + _sin43i;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_cf38r, _cf38i, _add44r, _add44i, &_mul45r, &_mul45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(k, 0, M_PI, 0, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 18.0; _c47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(_mul46r, _mul46i, _c47r, _c47i, &_div48r, &_div48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_div48r, _div48i, &_cos49r, &_cos49i);
        double _attr50r = 0, _attr50i = 0;
        _attr50r = x1i; _attr50i = 0;
        double _mul51r = 0, _mul51i = 0;
        c_mul(_cos49r, _cos49i, _attr50r, _attr50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _mul45r + _mul51r; _add52i = _mul45i + _mul51i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    }
    double _c53r = 0, _c53i = 0;
    _c53r = 2.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = degree + _c53r; _add54i = 0 + _c53i;
    for (int r = 1; r < (int)(_add54r); r++) {
        double _abs55r = 0, _abs55i = 0;
        _abs55r = c_abs(x1r, x1i); _abs55i = 0;
        double _abs56r = 0, _abs56i = 0;
        _abs56r = c_abs(x2r, x2i); _abs56i = 0;
        double _prod57r = 0, _prod57i = 0;
        c_mul(_abs55r, _abs55i, _abs56r, _abs56i, &_prod57r, &_prod57i);
        double _c58r = 0, _c58i = 0;
        _c58r = 2.0; _c58i = 0;
        double _add59r = 0, _add59i = 0;
        _add59r = r + _c58r; _add59i = 0 + _c58i;
        double _div60r = 0, _div60i = 0;
        c_div(_prod57r, _prod57i, _add59r, _add59i, &_div60r, &_div60i);
        cRe[(r - 1)] += _div60r; cIm[(r - 1)] += _div60i;
        double _cf61r = 0, _cf61i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf61r = cRe[_idx]; _cf61i = cIm[_idx]; } }
        double _cf62r = 0, _cf62i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { _cf62r = cRe[_idx]; _cf62i = cIm[_idx]; } }
        double _abs63r = 0, _abs63i = 0;
        _abs63r = c_abs(_cf62r, _cf62i); _abs63i = 0;
        double _c64r = 0, _c64i = 0;
        _c64r = 1.0; _c64i = 0;
        double _add65r = 0, _add65i = 0;
        _add65r = _abs63r + _c64r; _add65i = _abs63i + _c64i;
        double _log66r = 0, _log66i = 0;
        c_log(_add65r, _add65i, &_log66r, &_log66i);
        double _mul67r = 0, _mul67i = 0;
        c_mul(_cf61r, _cf61i, _log66r, _log66i, &_mul67r, &_mul67i);
        double _c68r = 0, _c68i = 0;
        _c68r = 0.0; _c68i = 1.0;
        double _sin69r = 0, _sin69i = 0;
        c_sin(r, 0, &_sin69r, &_sin69i);
        double _mul70r = 0, _mul70i = 0;
        c_mul(_c68r, _c68i, _sin69r, _sin69i, &_mul70r, &_mul70i);
        double _exp71r = 0, _exp71i = 0;
        c_exp2(_mul70r, _mul70i, &_exp71r, &_exp71i);
        double _add72r = 0, _add72i = 0;
        _add72r = _mul67r + _exp71r; _add72i = _mul67i + _exp71i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add72r; cIm[_idx] = _add72i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_580_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + j; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c7r, _c7i, M_PI, 0, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x1r; _attr9i = 0;
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
        c_mul(_log6r, _log6i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double mag_part1 = _mul15r; /* +_mul15ii */
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs16r + j; _add17i = _abs16i + 0;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 2.0; _c19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c19r, _c19i, M_PI, 0, &_mul20r, &_mul20i);
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2i; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul20r, _mul20i, _attr21r, _attr21i, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = j + _c23r; _add24i = 0 + _c23i;
        double _div25r = 0, _div25i = 0;
        c_div(_mul22r, _mul22i, _add24r, _add24i, &_div25r, &_div25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_div25r, _div25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log18r, _log18i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double mag_part2 = _mul27r; /* +_mul27ii */
        double _add28r = 0, _add28i = 0;
        _add28r = mag_part1 + mag_part2; _add28i = 0 + 0;
        double _attr29r = 0, _attr29i = 0;
        _attr29r = x1r; _attr29i = 0;
        double _attr30r = 0, _attr30i = 0;
        _attr30r = x1i; _attr30i = 0;
        double _prod31r = 0, _prod31i = 0;
        c_mul(_attr29r, _attr29i, _attr30r, _attr30i, &_prod31r, &_prod31i);
        double _prod32r = 0, _prod32i = 0;
        c_mul(_prod31r, _prod31i, j, 0, &_prod32r, &_prod32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _add28r + _prod32r; _add33i = _add28i + _prod32i;
        double magnitude = _add33r; /* +_add33ii */
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x1r, x1i); _ang34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang34r, _ang34i, j, 0, &_mul35r, &_mul35i);
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = degree + _c37r; _add38i = 0 + _c37i;
        double _sub39r = 0, _sub39i = 0;
        _sub39r = _add38r - j; _sub39i = _add38i - 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_ang36r, _ang36i, _sub39r, _sub39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _mul35r + _mul40r; _add41i = _mul35i + _mul40i;
        double _sin42r = 0, _sin42i = 0;
        c_sin(j, 0, &_sin42r, &_sin42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add41r + _sin42r; _add43i = _add41i + _sin42i;
        double _cos44r = 0, _cos44i = 0;
        c_cos(j, 0, &_cos44r, &_cos44i);
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _add43r - _cos44r; _sub45i = _add43i - _cos44i;
        double angle = _sub45r; /* +_sub45ii */
        double _c46r = 0, _c46i = 0;
        _c46r = 0.0; _c46i = 1.0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_c46r, _c46i, angle, 0, &_mul47r, &_mul47i);
        double _exp48r = 0, _exp48i = 0;
        c_exp2(_mul47r, _mul47i, &_exp48r, &_exp48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(magnitude, 0, _exp48r, _exp48i, &_mul49r, &_mul49i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_581_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 26.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr4r, _attr4i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1i; _attr10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, _attr10r, _attr10i, &_mul11r, &_mul11i);
        double _cos12r = 0, _cos12i = 0;
        c_cos(_mul11r, _mul11i, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr9r, _attr9i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _mul8r + _mul13r; _add14i = _mul8i + _mul13i;
        double term1 = _add14r; /* +_add14ii */
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2r; _attr16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _attr16r, _attr16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_attr15r, _attr15i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _ang21r, _ang21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_attr20r, _attr20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _mul19r - _mul24r; _sub25i = _mul19i - _mul24i;
        double term2 = _sub25r; /* +_sub25ii */
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs26r + j; _add27i = _abs26i + 0;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x2r, x2i); _abs29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(_abs29r, _abs29i, j, &_pow30r, &_pow30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _log28r + _pow30r; _add31i = _log28i + _pow30i;
        double magnitude = _add31r; /* +_add31ii */
        double _add32r = 0, _add32i = 0;
        _add32r = term1 + term2; _add32i = 0 + 0;
        double angle = _add32r; /* +_add32ii */
        double _cos33r = 0, _cos33i = 0;
        c_cos(angle, 0, &_cos33r, &_cos33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(angle, 0, &_sin34r, &_sin34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_sin34r, _sin34i, _c35r, _c35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _cos33r + _mul36r; _add37i = _cos33i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(magnitude, 0, _add37r, _add37i, &_mul38r, &_mul38i);
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x1r; _conj39i = -(x1i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x2r; _conj40i = -(x2i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_conj39r, _conj39i, _conj40r, _conj40i, &_mul41r, &_mul41i);
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

static void poly_582_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 26.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_attr4r, _attr4i, _pow6r, _pow6i, &_mul7r, &_mul7i);
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _div11r = 0, _div11i = 0;
        c_div(_attr8r, _attr8i, _add10r, _add10i, &_div11r, &_div11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul7r + _div11r; _add12i = _mul7i + _div11i;
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
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(j, 0, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang31r, _ang31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _ang34r = 0, _ang34i = 0;
        _ang34r = c_arg(x2r, x2i); _ang34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_mul(j, 0, j, 0, &_pow36r, &_pow36i);
        double _cos37r = 0, _cos37i = 0;
        c_cos(_pow36r, _pow36i, &_cos37r, &_cos37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_ang34r, _ang34i, _cos37r, _cos37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul33r + _mul38r; _add39i = _mul33i + _mul38i;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = j + _c40r; _add41i = 0 + _c40i;
        double _log42r = 0, _log42i = 0;
        c_log(_add41r, _add41i, &_log42r, &_log42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add39r + _log42r; _add43i = _add39i + _log42i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_584_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 26.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _sin5r = 0, _sin5i = 0;
        c_sin(j, 0, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_attr4r, _attr4i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2r; _attr7i = 0;
        double _cos8r = 0, _cos8i = 0;
        c_cos(j, 0, &_cos8r, &_cos8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_attr7r, _attr7i, _cos8r, _cos8i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul6r + _mul9r; _add10i = _mul6i + _mul9i;
        double r = _add10r; /* +_add10ii */
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1i; _attr11i = 0;
        double _cos12r = 0, _cos12i = 0;
        c_cos(j, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr11r, _attr11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x2i; _attr14i = 0;
        double _sin15r = 0, _sin15i = 0;
        c_sin(j, 0, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_attr14r, _attr14i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _mul13r - _mul16r; _sub17i = _mul13i - _mul16i;
        double k = _sub17r; /* +_sub17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(r, 0); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 5.0; _c23i = 0;
        double _mod24r = 0, _mod24i = 0;
        _mod24r = fmod(j, _c23r); _mod24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _c22r + _mod24r; _add25i = _c22i + _mod24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log21r, _log21i, _add25r, _add25i, &_mul26r, &_mul26i);
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(k, 0); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.5; _c28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_abs27r, _abs27i, 1.5, &_pow29r, &_pow29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul26r + _pow29r; _add30i = _mul26i + _pow29i;
        double magnitude = _add30r; /* +_add30ii */
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x1r, x1i); _ang31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang31r, _ang31i, j, 0, &_mul32r, &_mul32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x2r, x2i); _ang33i = 0;
        double _sqrt34r = 0, _sqrt34i = 0;
        c_powr(j, 0, 0.5, &_sqrt34r, &_sqrt34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_ang33r, _ang33i, _sqrt34r, _sqrt34i, &_mul35r, &_mul35i);
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
        double _conj41r = 0, _conj41i = 0;
        _conj41r = x1r; _conj41i = -(x1i);
        double _conj42r = 0, _conj42i = 0;
        _conj42r = x2r; _conj42i = -(x2i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_conj41r, _conj41i, _conj42r, _conj42i, &_mul43r, &_mul43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = j + _c44r; _add45i = 0 + _c44i;
        double _div46r = 0, _div46i = 0;
        c_div(_mul43r, _mul43i, _add45r, _add45i, &_div46r, &_div46i);
        double _add47r = 0, _add47i = 0;
        _add47r = _mul40r + _div46r; _add47i = _mul40i + _div46i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_585_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 26.0; _c1i = 0;
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
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
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
        double _attr12r = 0, _attr12i = 0;
        _attr12r = x2r; _attr12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(k, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 5.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_attr12r, _attr12i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul11r + _mul17r; _add18i = _mul11i + _mul17i;
        double r = _add18r; /* +_add18ii */
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x1i; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(k, 0, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_attr19r, _attr19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2i; _attr25i = 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(k, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 4.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(_mul26r, _mul26i, _c27r, _c27i, &_div28r, &_div28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_div28r, _div28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_attr25r, _attr25i, _sin29r, _sin29i, &_mul30r, &_mul30i);
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
        _add35r = _add34r + k; _add35i = _add34i + 0;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _abs37r = 0, _abs37i = 0;
        _abs37r = c_abs(r, 0); _abs37i = 0;
        double _abs38r = 0, _abs38i = 0;
        _abs38r = c_abs(s, 0); _abs38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _abs37r + _abs38r; _add39i = _abs37i + _abs38i;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _add39r + _c40r; _add41i = _add39i + _c40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_log36r, _log36i, _add41r, _add41i, &_mul42r, &_mul42i);
        double mag = _mul42r; /* +_mul42ii */
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x1r, x1i); _ang43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = k + _c44r; _add45i = 0 + _c44i;
        double _log46r = 0, _log46i = 0;
        c_log(_add45r, _add45i, &_log46r, &_log46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_ang43r, _ang43i, _log46r, _log46i, &_mul47r, &_mul47i);
        double _sin48r = 0, _sin48i = 0;
        c_sin(r, 0, &_sin48r, &_sin48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _mul47r + _sin48r; _add49i = _mul47i + _sin48i;
        double _cos50r = 0, _cos50i = 0;
        c_cos(s, 0, &_cos50r, &_cos50i);
        double _sub51r = 0, _sub51i = 0;
        _sub51r = _add49r - _cos50r; _sub51i = _add49i - _cos50i;
        double angle = _sub51r; /* +_sub51ii */
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
        c_mul(mag, 0, _add56r, _add56i, &_mul57r, &_mul57i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul57r; cIm[_idx] = _mul57i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_586_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 26.0; _c1i = 0;
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
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, M_PI, 0, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
        double _div9r = 0, _div9i = 0;
        c_div(_mul7r, _mul7i, _c8r, _c8i, &_div9r, &_div9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_log6r, _log6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double mag_part1 = _mul11r; /* +_mul11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x2r, x2i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs12r + j; _add13i = _abs12i + 0;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, M_PI, 0, &_mul15r, &_mul15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 5.0; _c16i = 0;
        double _div17r = 0, _div17i = 0;
        c_div(_mul15r, _mul15i, _c16r, _c16i, &_div17r, &_div17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_log14r, _log14i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double mag_part2 = _mul19r; /* +_mul19ii */
        double _add20r = 0, _add20i = 0;
        _add20r = mag_part1 + mag_part2; _add20i = 0 + 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _add20r + j; _add21i = _add20i + 0;
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
        _c30r = 3.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_div31r, _div31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang28r, _ang28i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double angle_part2 = _mul33r; /* +_mul33ii */
        double _add34r = 0, _add34i = 0;
        _add34r = angle_part1 + angle_part2; _add34i = 0 + 0;
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
    double _c41r = 0, _c41i = 0;
    _c41r = 1.0; _c41i = 0;
    double _add42r = 0, _add42i = 0;
    _add42r = n + _c41r; _add42i = 0 + _c41i;
    for (int k = 1; k < (int)(_add42r); k++) {
        double _cf43r = 0, _cf43i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 0.05; _c45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 2.0; _c46i = 0;
        double _pow47r = 0, _pow47i = 0;
        c_mul(k, 0, k, 0, &_pow47r, &_pow47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c45r, _c45i, _pow47r, _pow47i, &_mul48r, &_mul48i);
        double _add49r = 0, _add49i = 0;
        _add49r = _c44r + _mul48r; _add49i = _c44i + _mul48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_cf43r, _cf43i, _add49r, _add49i, &_mul50r, &_mul50i);
        double _neg51r = 0, _neg51i = 0;
        _neg51r = -(k); _neg51i = -(0);
        double _div52r = 0, _div52i = 0;
        c_div(_neg51r, _neg51i, n, 0, &_div52r, &_div52i);
        double _exp53r = 0, _exp53i = 0;
        c_exp2(_div52r, _div52i, &_exp53r, &_exp53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_mul50r, _mul50i, _exp53r, _exp53i, &_mul54r, &_mul54i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_587_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
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
        c_mul(j, 0, _attr5r, _attr5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _attr4r + _mul6r; _add7i = _attr4i + _mul6i;
        double r1 = _add7r; /* +_add7ii */
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x1i; _attr8i = 0;
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2i; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _attr9r, _attr9i, &_mul10r, &_mul10i);
        double _sub11r = 0, _sub11i = 0;
        _sub11r = _attr8r - _mul10r; _sub11i = _attr8i - _mul10i;
        double i1 = _sub11r; /* +_sub11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs12r + j; _add13i = _abs12i + 0;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log14r, _log14i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(j, 0, M_PI, 0, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 12.0; _c20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_mul19r, _mul19i, _c20r, _c20i, &_div21r, &_div21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_div21r, _div21i, &_cos22r, &_cos22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul18r + _cos22r; _add23i = _mul18i + _cos22i;
        double magnitude = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _cos25r = 0, _cos25i = 0;
        c_cos(j, 0, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang24r, _ang24i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, _ang27r, _ang27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul26r + _sin29r; _add30i = _mul26i + _sin29i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_588_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.3; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_powr(j, 0, 1.3, &_pow6r, &_pow6i);
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
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_sin12r, _sin12i); _abs13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log8r, _log8i, _abs13r, _abs13i, &_mul14r, &_mul14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 6.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_abs15r, _abs15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul14r + _mul20r; _add21i = _mul14i + _mul20i;
        double mag = _add21r; /* +_add21ii */
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
        _c28r = 5.0; _c28i = 0;
        double _div29r = 0, _div29i = 0;
        c_div(j, 0, _c28r, _c28i, &_div29r, &_div29i);
        double _cos30r = 0, _cos30i = 0;
        c_cos(_div29r, _div29i, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang27r, _ang27i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul26r + _mul31r; _add32i = _mul26i + _mul31i;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 7.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add32r + _sin36r; _add37i = _add32i + _sin36i;
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
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    double _c44r = 0, _c44i = 0;
    _c44r = 1.0; _c44i = 0;
    double _add45r = 0, _add45i = 0;
    _add45r = degree + _c44r; _add45i = 0 + _c44i;
    for (int k = (int)(_add45r); k < 36; k++) {
        double _c46r = 0, _c46i = 0;
        _c46r = 1.0; _c46i = 0;
        double _add47r = 0, _add47i = 0;
        _add47r = k + _c46r; _add47i = 0 + _c46i;
        double _log48r = 0, _log48i = 0;
        c_log(_add47r, _add47i, &_log48r, &_log48i);
        double _ang49r = 0, _ang49i = 0;
        _ang49r = c_arg(x1r, x1i); _ang49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(k, 0, _ang49r, _ang49i, &_mul50r, &_mul50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(_mul50r, _mul50i, &_sin51r, &_sin51i);
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _c53r = 0, _c53i = 0;
        _c53r = 2.0; _c53i = 0;
        double _div54r = 0, _div54i = 0;
        c_div(k, 0, _c53r, _c53i, &_div54r, &_div54i);
        double _cos55r = 0, _cos55i = 0;
        c_cos(_div54r, _div54i, &_cos55r, &_cos55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c52r, _c52i, _cos55r, _cos55i, &_mul56r, &_mul56i);
        double _add57r = 0, _add57i = 0;
        _add57r = _sin51r + _mul56r; _add57i = _sin51i + _mul56i;
        double _mul58r = 0, _mul58i = 0;
        c_mul(_log48r, _log48i, _add57r, _add57i, &_mul58r, &_mul58i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_589_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_attr4r, _attr4i, j, 0, &_mul5r, &_mul5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2r; _attr6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _div9r = 0, _div9i = 0;
        c_div(_attr6r, _attr6i, _add8r, _add8i, &_div9r, &_div9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul5r + _div9r; _add10i = _mul5i + _div9i;
        double r_part = _add10r; /* +_add10ii */
        double _attr11r = 0, _attr11i = 0;
        _attr11r = x1i; _attr11i = 0;
        double _sin12r = 0, _sin12i = 0;
        c_sin(j, 0, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_attr11r, _attr11i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x2i; _attr14i = 0;
        double _cos15r = 0, _cos15i = 0;
        c_cos(j, 0, &_cos15r, &_cos15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_attr14r, _attr14i, _cos15r, _cos15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul13r + _mul16r; _add17i = _mul13i + _mul16i;
        double i_part = _add17r; /* +_add17ii */
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(r_part, 0); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_abs22r, _abs22i, _abs23r, _abs23i, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _div27r = 0, _div27i = 0;
        c_div(_mul24r, _mul24i, _add26r, _add26i, &_div27r, &_div27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _log21r + _div27r; _add28i = _log21i + _div27i;
        double magnitude = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang29r, _ang29i, j, 0, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = j + _c32r; _add33i = 0 + _c32i;
        double _div34r = 0, _div34i = 0;
        c_div(_ang31r, _ang31i, _add33r, _add33i, &_div34r, &_div34i);
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _mul30r - _div34r; _sub35i = _mul30i - _div34i;
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
        c_mul(magnitude, 0, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    double _c47r = 0, _c47i = 0;
    _c47r = 2.0; _c47i = 0;
    double _add48r = 0, _add48i = 0;
    _add48r = degree + _c47r; _add48i = 0 + _c47i;
    for (int k = 1; k < (int)(_add48r); k++) {
        double _c49r = 0, _c49i = 0;
        _c49r = 1.0; _c49i = 0;
        double _c50r = 0, _c50i = 0;
        _c50r = 0.05; _c50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 2.0; _c51i = 0;
        double _pow52r = 0, _pow52i = 0;
        c_mul(k, 0, k, 0, &_pow52r, &_pow52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_c50r, _c50i, _pow52r, _pow52i, &_mul53r, &_mul53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _c49r + _mul53r; _add54i = _c49i + _mul53i;
        { double _tr = cRe[(k - 1)]*_add54r - cIm[(k - 1)]*_add54i; cIm[(k - 1)] = cRe[(k - 1)]*_add54i + cIm[(k - 1)]*_add54r; cRe[(k - 1)] = _tr; }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_591_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 3.0; _c4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _c4r, _c4i, &_mul5r, &_mul5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x1r; _attr6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mul5r + _attr6r; _add7i = _mul5i + _attr6i;
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2i; _attr8i = 0;
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _add7r - _attr8r; _sub9i = _add7i - _attr8i;
        double k = _sub9r; /* +_sub9ii */
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
        double _add21r = 0, _add21i = 0;
        _add21r = _sin17r + _cos20r; _add21i = _sin17i + _cos20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log14r, _log14i, _add21r, _add21i, &_mul22r, &_mul22i);
        double r = _mul22r; /* +_mul22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(k, 0, _ang23r, _ang23i, &_mul24r, &_mul24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_mul24r, _mul24i, &_sin25r, &_sin25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(k, 0, _ang26r, _ang26i, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _sin25r + _cos28r; _add29i = _sin25i + _cos28i;
        double angle = _add29r; /* +_add29ii */
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
        c_mul(r, 0, _add34r, _add34i, &_mul35r, &_mul35i);
        double _conj36r = 0, _conj36i = 0;
        _conj36r = x1r; _conj36i = -(x1i);
        double _pow37r = 0, _pow37i = 0;
        c_powr(_conj36r, _conj36i, k, &_pow37r, &_pow37i);
        double _sin38r = 0, _sin38i = 0;
        c_sin(j, 0, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_pow37r, _pow37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul35r + _mul39r; _add40i = _mul35i + _mul39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_592_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        double sum_val = _c4r; /* +_c4ii */
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double prod_val = _c5r; /* +_c5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        for (int k = 1; k < (int)(_add7r); k++) {
            double _attr8r = 0, _attr8i = 0;
            _attr8r = x1r; _attr8i = 0;
            double _mul9r = 0, _mul9i = 0;
            c_mul(k, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
            double _sin10r = 0, _sin10i = 0;
            c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
            double _attr11r = 0, _attr11i = 0;
            _attr11r = x2i; _attr11i = 0;
            double _mul12r = 0, _mul12i = 0;
            c_mul(k, 0, _attr11r, _attr11i, &_mul12r, &_mul12i);
            double _cos13r = 0, _cos13i = 0;
            c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(_sin10r, _sin10i, _cos13r, _cos13i, &_mul14r, &_mul14i);
            sum_val += _mul14r;
            double _attr15r = 0, _attr15i = 0;
            _attr15r = x1r; _attr15i = 0;
            double _attr16r = 0, _attr16i = 0;
            _attr16r = x2i; _attr16i = 0;
            double _mul17r = 0, _mul17i = 0;
            c_mul(_attr16r, _attr16i, k, 0, &_mul17r, &_mul17i);
            double _add18r = 0, _add18i = 0;
            _add18r = _attr15r + _mul17r; _add18i = _attr15i + _mul17i;
            prod_val *= _add18r;
        }
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs19r + j; _add20i = _abs19i + 0;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_log21r, _log21i, sum_val, 0, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul22r + prod_val; _add23i = _mul22i + 0;
        double magnitude = _add23r; /* +_add23ii */
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x1r, x1i); _ang24i = 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang24r, _ang24i, j, 0, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_mul(j, 0, j, 0, &_pow28r, &_pow28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang26r, _ang26i, _pow28r, _pow28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul25r - _mul29r; _sub30i = _mul25i - _mul29i;
        double angle = _sub30r; /* +_sub30ii */
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
        c_mul(magnitude, 0, _add35r, _add35i, &_mul36r, &_mul36i);
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x1r; _conj37i = -(x1i);
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x2r; _conj38i = -(x2i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_conj37r, _conj37i, _conj38r, _conj38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul36r + _mul39r; _add40i = _mul36i + _mul39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_593_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double deg = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = deg + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs4r + j; _add5i = _abs4i + 0;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _attr8r = 0, _attr8i = 0;
        _attr8r = x2r; _attr8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(j, 0, _attr8r, _attr8i, &_mul9r, &_mul9i);
        double _sin10r = 0, _sin10i = 0;
        c_sin(_mul9r, _mul9i, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _c7r + _sin10r; _add11i = _c7i + _sin10i;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_log6r, _log6i, _add11r, _add11i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _attr14r = 0, _attr14i = 0;
        _attr14r = x1i; _attr14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _attr14r, _attr14i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _c13r + _cos16r; _add17i = _c13i + _cos16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul12r, _mul12i, _add17r, _add17i, &_mul18r, &_mul18i);
        double mag = _mul18r; /* +_mul18ii */
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x1r, x1i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang19r, _ang19i, j, 0, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _sqrt22r = 0, _sqrt22i = 0;
        c_powr(j, 0, 0.5, &_sqrt22r, &_sqrt22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang21r, _ang21i, _sqrt22r, _sqrt22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
        double ang = _add24r; /* +_add24ii */
        double _cos25r = 0, _cos25i = 0;
        c_cos(ang, 0, &_cos25r, &_cos25i);
        double _sin26r = 0, _sin26i = 0;
        c_sin(ang, 0, &_sin26r, &_sin26i);
        double _cplx27r = 0, _cplx27i = 0;
        _cplx27r = _cos25r; _cplx27i = _sin26r;
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag, 0, _cplx27r, _cplx27i, &_mul28r, &_mul28i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_594_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 26.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(j, 0, &_sin7r, &_sin7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _pow6r + _sin7r; _add8i = _pow6i + _sin7i;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_attr4r, _attr4i, _add8r, _add8i, &_mul9r, &_mul9i);
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
        double r = _add15r; /* +_add15ii */
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x1i; _attr16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 2.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(j, 0, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _cos19r + j; _add20i = _cos19i + 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_attr16r, _attr16i, _add20r, _add20i, &_mul21r, &_mul21i);
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x2i; _attr22i = 0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(j, 0, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_attr22r, _attr22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
        double im = _add25r; /* +_add25ii */
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(im, 0, _c26r, _c26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = r + _mul27r; _add28i = 0 + _mul27i;
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(_add28r, _add28i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, M_PI, 0, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 5.0; _c32i = 0;
        double _div33r = 0, _div33i = 0;
        c_div(_mul31r, _mul31i, _c32r, _c32i, &_div33r, &_div33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_div33r, _div33i, &_cos34r, &_cos34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _c30r + _cos34r; _add35i = _c30i + _cos34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_abs29r, _abs29i, _add35r, _add35i, &_mul36r, &_mul36i);
        double mag = _mul36r; /* +_mul36ii */
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(im, 0, _c37r, _c37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = r + _mul38r; _add39i = 0 + _mul38i;
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(_add39r, _add39i); _ang40i = 0;
        double angle = _ang40r; /* +_ang40ii */
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

static void poly_595_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 26.0; _c1i = 0;
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
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _c9r + _sin10r; _add11i = _c9i + _sin10i;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_mul8r, _mul8i, _add11r, _add11i, &_mul12r, &_mul12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _div16r = 0, _div16i = 0;
        c_div(_abs13r, _abs13i, _add15r, _add15i, &_div16r, &_div16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul12r + _div16r; _add17i = _mul12i + _div16i;
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
        double _mul25r = 0, _mul25i = 0;
        c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 3.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add24r + _sin28r; _add29i = _add24i + _sin28i;
        double angle = _add29r; /* +_add29ii */
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
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, n, 0, &_div37r, &_div37i);
        double _cos38r = 0, _cos38i = 0;
        c_cos(_div37r, _div37i, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_conj36r, _conj36i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul35r + _mul39r; _add40i = _mul35i + _mul39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_596_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 3.0; _c4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _c4r, _c4i, &_mul5r, &_mul5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 7.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mul5r + _c6r; _add7i = _mul5i + _c6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 10.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(_add7r, _c8r); _mod9i = 0;
        double k = _mod9r; /* +_mod9ii */
        double _attr10r = 0, _attr10i = 0;
        _attr10r = x1r; _attr10i = 0;
        double _sin11r = 0, _sin11i = 0;
        c_sin(j, 0, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_attr10r, _attr10i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _attr13r = 0, _attr13i = 0;
        _attr13r = x2i; _attr13i = 0;
        double _cos14r = 0, _cos14i = 0;
        c_cos(k, 0, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_attr13r, _attr13i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _mul15r; _add16i = _mul12i + _mul15i;
        double r = _add16r; /* +_add16ii */
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_mul(j, 0, j, 0, &_pow19r, &_pow19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _abs17r + _pow19r; _add20i = _abs17i + _pow19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(k, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 4.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log21r, _log21i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(r, 0, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul26r + _cos27r; _add28i = _mul26i + _cos27i;
        double mag = _add28r; /* +_add28ii */
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x1r, x1i); _ang29i = 0;
        double _cos30r = 0, _cos30i = 0;
        c_cos(j, 0, &_cos30r, &_cos30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_ang29r, _ang29i, _cos30r, _cos30i, &_mul31r, &_mul31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(k, 0, _ang32r, _ang32i, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _mul31r + _sin34r; _add35i = _mul31i + _sin34i;
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
        c_mul(mag, 0, _add40r, _add40i, &_mul41r, &_mul41i);
        double _conj42r = 0, _conj42i = 0;
        _conj42r = x2r; _conj42i = -(x2i);
        double _pow43r = 0, _pow43i = 0;
        c_powr(_conj42r, _conj42i, k, &_pow43r, &_pow43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul41r + _pow43r; _add44i = _mul41i + _pow43i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 26) { cRe[_idx] = _add44r; cIm[_idx] = _add44i; } }
    }
    double _unk45r = 0, _unk45i = 0;
    /* WARNING: unhandled node Slice(upper=Name(id='n', ctx=Load())) */
    double _cf46r = 0, _cf46i = 0;
    { int _idx = (int)(_unk45r); if (_idx >= 0 && _idx < 26) { _cf46r = cRe[_idx]; _cf46i = cIm[_idx]; } }
    double _abs47r = 0, _abs47i = 0;
    _abs47r = c_abs(_cf46r, _cf46i); _abs47i = 0;
    double _mul48r = 0, _mul48i = 0;
    c_mul(0, 0, M_PI, 0, &_mul48r, &_mul48i);
    double _c49r = 0, _c49i = 0;
    _c49r = 6.0; _c49i = 0;
    double _div50r = 0, _div50i = 0;
    c_div(_mul48r, _mul48i, _c49r, _c49i, &_div50r, &_div50i);
    double _cos51r = 0, _cos51i = 0;
    c_cos(_div50r, _div50i, &_cos51r, &_cos51i);
    double _mul52r = 0, _mul52i = 0;
    c_mul(_abs47r, _abs47i, _cos51r, _cos51i, &_mul52r, &_mul52i);
    double _abs53r = 0, _abs53i = 0;
    _abs53r = c_abs(x1r, x1i); _abs53i = 0;
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(x2r, x2i); _abs54i = 0;
    double _prod55r = 0, _prod55i = 0;
    c_mul(_abs53r, _abs53i, _abs54r, _abs54i, &_prod55r, &_prod55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _mul52r + _prod55r; _add56i = _mul52i + _prod55i;
    { int _idx = 25; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_597_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
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
    _add7r = degree + _c6r; _add7i = 0 + _c6i;
    for (int j = 0; j < (int)(_add7r); j++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 0;
        double mag = _c8r; /* +_c8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 0;
        double angle = _c9r; /* +_c9ii */
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        for (int k = 1; k < (int)(_add11r); k++) {
            double _abs12r = 0, _abs12i = 0;
            _abs12r = c_abs(x1r, x1i); _abs12i = 0;
            double _mul13r = 0, _mul13i = 0;
            c_mul(_abs12r, _abs12i, k, 0, &_mul13r, &_mul13i);
            double _c14r = 0, _c14i = 0;
            _c14r = 1.0; _c14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = _mul13r + _c14r; _add15i = _mul13i + _c14i;
            double _log16r = 0, _log16i = 0;
            c_log(_add15r, _add15i, &_log16r, &_log16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(k, 0, M_PI, 0, &_mul17r, &_mul17i);
            double _mul18r = 0, _mul18i = 0;
            c_mul(_mul17r, _mul17i, r1, 0, &_mul18r, &_mul18i);
            double _sin19r = 0, _sin19i = 0;
            c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
            double _mul20r = 0, _mul20i = 0;
            c_mul(_log16r, _log16i, _sin19r, _sin19i, &_mul20r, &_mul20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(k, 0, M_PI, 0, &_mul21r, &_mul21i);
            double _mul22r = 0, _mul22i = 0;
            c_mul(_mul21r, _mul21i, i2, 0, &_mul22r, &_mul22i);
            double _cos23r = 0, _cos23i = 0;
            c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
            double _add24r = 0, _add24i = 0;
            _add24r = _mul20r + _cos23r; _add24i = _mul20i + _cos23i;
            double term_mag = _add24r; /* +_add24ii */
            double _ang25r = 0, _ang25i = 0;
            _ang25r = c_arg(x1r, x1i); _ang25i = 0;
            double _c26r = 0, _c26i = 0;
            _c26r = 2.0; _c26i = 0;
            double _pow27r = 0, _pow27i = 0;
            c_mul(k, 0, k, 0, &_pow27r, &_pow27i);
            double _mul28r = 0, _mul28i = 0;
            c_mul(_ang25r, _ang25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
            double _ang29r = 0, _ang29i = 0;
            _ang29r = c_arg(x2r, x2i); _ang29i = 0;
            double _sqrt30r = 0, _sqrt30i = 0;
            c_powr(k, 0, 0.5, &_sqrt30r, &_sqrt30i);
            double _mul31r = 0, _mul31i = 0;
            c_mul(_ang29r, _ang29i, _sqrt30r, _sqrt30i, &_mul31r, &_mul31i);
            double _sub32r = 0, _sub32i = 0;
            _sub32r = _mul28r - _mul31r; _sub32i = _mul28i - _mul31i;
            double term_angle = _sub32r; /* +_sub32ii */
            double _c33r = 0, _c33i = 0;
            _c33r = 0.0; _c33i = 1.0;
            double _mul34r = 0, _mul34i = 0;
            c_mul(_c33r, _c33i, term_angle, 0, &_mul34r, &_mul34i);
            double _exp35r = 0, _exp35i = 0;
            c_exp2(_mul34r, _mul34i, &_exp35r, &_exp35i);
            double _mul36r = 0, _mul36i = 0;
            c_mul(term_mag, 0, _exp35r, _exp35i, &_mul36r, &_mul36i);
            mag += _mul36r;
        }
        double _c37r = 0, _c37i = 0;
        _c37r = 3.0; _c37i = 0;
        double _div38r = 0, _div38i = 0;
        c_div(degree, 0, _c37r, _c37i, &_div38r, &_div38i);
        if (j < _div38r) {
            double _c39r = 0, _c39i = 0;
            _c39r = 1.0; _c39i = 0;
            double _add40r = 0, _add40i = 0;
            _add40r = j + _c39r; _add40i = 0 + _c39i;
            mag *= _add40r;
        } else {
            double _c41r = 0, _c41i = 0;
            _c41r = 2.0; _c41i = 0;
            double _mul42r = 0, _mul42i = 0;
            c_mul(_c41r, _c41i, degree, 0, &_mul42r, &_mul42i);
            double _c43r = 0, _c43i = 0;
            _c43r = 3.0; _c43i = 0;
            double _div44r = 0, _div44i = 0;
            c_div(_mul42r, _mul42i, _c43r, _c43i, &_div44r, &_div44i);
            if (j < _div44r) {
                double _c45r = 0, _c45i = 0;
                _c45r = 1.0; _c45i = 0;
                double _add46r = 0, _add46i = 0;
                _add46r = j + _c45r; _add46i = 0 + _c45i;
            } else {
                double _c47r = 0, _c47i = 0;
                _c47r = 1.0; _c47i = 0;
                double _add48r = 0, _add48i = 0;
                _add48r = j + _c47r; _add48i = 0 + _c47i;
                double _c49r = 0, _c49i = 0;
                _c49r = 2.0; _c49i = 0;
                double _pow50r = 0, _pow50i = 0;
                c_mul(_add48r, _add48i, _add48r, _add48i, &_pow50r, &_pow50i);
                mag *= _pow50r;
            }
        }
        { int _idx = j; if (_idx >= 0 && _idx < 36) { cRe[_idx] = mag; cIm[_idx] = 0; } }
    }
    double _attr51r = 0, _attr51i = 0;
    _attr51r = x1r; _attr51i = 0;
    double _attr52r = 0, _attr52i = 0;
    _attr52r = x2r; _attr52i = 0;
    double _mul53r = 0, _mul53i = 0;
    c_mul(_attr51r, _attr51i, _attr52r, _attr52i, &_mul53r, &_mul53i);
    double _c54r = 0, _c54i = 0;
    _c54r = 0.0; _c54i = 1.0;
    double _attr55r = 0, _attr55i = 0;
    _attr55r = x1i; _attr55i = 0;
    double _attr56r = 0, _attr56i = 0;
    _attr56r = x2i; _attr56i = 0;
    double _sub57r = 0, _sub57i = 0;
    _sub57r = _attr55r - _attr56r; _sub57i = _attr55i - _attr56i;
    double _mul58r = 0, _mul58i = 0;
    c_mul(_c54r, _c54i, _sub57r, _sub57i, &_mul58r, &_mul58i);
    double _add59r = 0, _add59i = 0;
    _add59r = _mul53r + _mul58r; _add59i = _mul53i + _mul58i;
    double _attr60r = 0, _attr60i = 0;
    _attr60r = x1r; _attr60i = 0;
    double _sin61r = 0, _sin61i = 0;
    c_sin(_attr60r, _attr60i, &_sin61r, &_sin61i);
    double _attr62r = 0, _attr62i = 0;
    _attr62r = x2i; _attr62i = 0;
    double _cos63r = 0, _cos63i = 0;
    c_cos(_attr62r, _attr62i, &_cos63r, &_cos63i);
    double _mul64r = 0, _mul64i = 0;
    c_mul(_sin61r, _sin61i, _cos63r, _cos63i, &_mul64r, &_mul64i);
    double _add65r = 0, _add65i = 0;
    _add65r = _add59r + _mul64r; _add65i = _add59i + _mul64i;
    { int _idx = 0; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add65r; cIm[_idx] = _add65i; } }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_598_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 26.0; _c1i = 0;
    double n = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = n + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_attr4r, _attr4i, _log7r, _log7i, &_mul8r, &_mul8i);
        double rec1 = _mul8r; /* +_mul8ii */
        double _attr9r = 0, _attr9i = 0;
        _attr9r = x2r; _attr9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_div12r, _div12i, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_attr9r, _attr9i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double rec2 = _mul14r; /* +_mul14ii */
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 5.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_attr15r, _attr15i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double imc1 = _mul20r; /* +_mul20ii */
        double _attr21r = 0, _attr21i = 0;
        _attr21r = x2i; _attr21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, M_PI, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 3.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(_mul22r, _mul22i, _c23r, _c23i, &_div24r, &_div24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(_div24r, _div24i, &_sin25r, &_sin25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_attr21r, _attr21i, _sin25r, _sin25i, &_mul26r, &_mul26i);
        double imc2 = _mul26r; /* +_mul26ii */
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x1r, x1i); _abs27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs27r + j; _add28i = _abs27i + 0;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _mod32r = 0, _mod32i = 0;
        _mod32r = fmod(j, _c31r); _mod32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _c30r + _mod32r; _add33i = _c30i + _mod32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_log29r, _log29i, _add33r, _add33i, &_mul34r, &_mul34i);
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(x2r, x2i); _abs35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 2.0; _c36i = 0;
        double _div37r = 0, _div37i = 0;
        c_div(j, 0, _c36r, _c36i, &_div37r, &_div37i);
        double _pow38r = 0, _pow38i = 0;
        c_powr(_abs35r, _abs35i, _div37r, &_pow38r, &_pow38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul34r + _pow38r; _add39i = _mul34i + _pow38i;
        double mag = _add39r; /* +_add39ii */
        double _ang40r = 0, _ang40i = 0;
        _ang40r = c_arg(x1r, x1i); _ang40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 4.0; _c41i = 0;
        double _div42r = 0, _div42i = 0;
        c_div(j, 0, _c41r, _c41i, &_div42r, &_div42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(_div42r, _div42i, &_sin43r, &_sin43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_ang40r, _ang40i, _sin43r, _sin43i, &_mul44r, &_mul44i);
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(x2r, x2i); _ang45i = 0;
        double _c46r = 0, _c46i = 0;
        _c46r = 6.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(j, 0, _c46r, _c46i, &_div47r, &_div47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_div47r, _div47i, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_ang45r, _ang45i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _sub50r = 0, _sub50i = 0;
        _sub50r = _mul44r - _mul49r; _sub50i = _mul44i - _mul49i;
        double ang = _sub50r; /* +_sub50ii */
        double _add51r = 0, _add51i = 0;
        _add51r = rec1 + rec2; _add51i = 0 + 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _add53r = 0, _add53i = 0;
        _add53r = imc1 + imc2; _add53i = 0 + 0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c52r, _c52i, _add53r, _add53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _add51r + _mul54r; _add55i = _add51i + _mul54i;
        double _c56r = 0, _c56i = 0;
        _c56r = 0.0; _c56i = 1.0;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_c56r, _c56i, ang, 0, &_mul57r, &_mul57i);
        double _exp58r = 0, _exp58i = 0;
        c_exp2(_mul57r, _mul57i, &_exp58r, &_exp58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(mag, 0, _exp58r, _exp58i, &_mul59r, &_mul59i);
        double _add60r = 0, _add60i = 0;
        _add60r = _add55r + _mul59r; _add60i = _add55i + _mul59i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add60r; cIm[_idx] = _add60i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_599_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = degree + _c2r; _add3i = 0 + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _attr4r = 0, _attr4i = 0;
        _attr4r = x1r; _attr4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(j, 0, _attr4r, _attr4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _attr7r = 0, _attr7i = 0;
        _attr7r = x2i; _attr7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _attr7r, _attr7i, &_mul8r, &_mul8i);
        double _cos9r = 0, _cos9i = 0;
        c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_sin6r, _sin6i, _cos9r, _cos9i, &_mul10r, &_mul10i);
        double part1 = _mul10r; /* +_mul10ii */
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs11r + j; _add12i = _abs11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs14r + j; _add15i = _abs14i + 0;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _log13r + _log16r; _add17i = _log13i + _log16i;
        double part2 = _add17r; /* +_add17ii */
        double _attr18r = 0, _attr18i = 0;
        _attr18r = x1r; _attr18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powr(_attr18r, _attr18i, j, &_pow19r, &_pow19i);
        double _attr20r = 0, _attr20i = 0;
        _attr20r = x2i; _attr20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_attr20r, _attr20i, j, &_pow21r, &_pow21i);
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _pow19r - _pow21r; _sub22i = _pow19i - _pow21i;
        double part3 = _sub22r; /* +_sub22ii */
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang23r, _ang23i, j, 0, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x2r; _attr25i = 0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(j, 0, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_attr25r, _attr25i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul24r + _mul27r; _add28i = _mul24i + _mul27i;
        double angle = _add28r; /* +_add28ii */
        double _mul29r = 0, _mul29i = 0;
        c_mul(part1, 0, part2, 0, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul29r + part3; _add30i = _mul29i + 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, angle, 0, &_mul32r, &_mul32i);
        double _exp33r = 0, _exp33i = 0;
        c_exp2(_mul32r, _mul32i, &_exp33r, &_exp33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_add30r, _add30i, _exp33r, _exp33i, &_mul34r, &_mul34i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_600_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 25.0; _c1i = 0;
    double degree = _c1r; /* +_c1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double rec_step[26];
    for (int _li = 0; _li < 26; _li++) {
        rec_step[_li] = _attr2r + (_attr3r - _attr2r) * _li / 25.0;
    }
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double imc_step[26];
    for (int _li = 0; _li < 26; _li++) {
        imc_step[_li] = _attr4r + (_attr5r - _attr4r) * _li / 25.0;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = degree + _c6r; _add7i = 0 + _c6i;
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
        _c12r = 12.0; _c12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_mul11r, _mul11i, _c12r, _c12i, &_div13r, &_div13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_div13r, _div13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log10r, _log10i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double mag_part1 = _mul15r; /* +_mul15ii */
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, M_PI, 0, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 8.0; _c17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_mul16r, _mul16i, _c17r, _c17i, &_div18r, &_div18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_div18r, _div18i, &_cos19r, &_cos19i);
        double _arr20r = 0, _arr20i = 0;
        { int _idx = (j - 1); _arr20r = (_idx >= 0 && _idx < 26) ? rec_step[_idx] : 0.0; _arr20i = 0; }
        double _arr21r = 0, _arr21i = 0;
        { int _idx = (j - 1); _arr21r = (_idx >= 0 && _idx < 26) ? imc_step[_idx] : 0.0; _arr21i = 0; }
        double _sub22r = 0, _sub22i = 0;
        _sub22r = _arr20r - _arr21r; _sub22i = _arr20i - _arr21i;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_sub22r, _sub22i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _c24r; _add25i = _abs23i + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_cos19r, _cos19i, _log26r, _log26i, &_mul27r, &_mul27i);
        double mag_part2 = _mul27r; /* +_mul27ii */
        double _add28r = 0, _add28i = 0;
        _add28r = mag_part1 + mag_part2; _add28i = 0 + 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 0.8; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powr(j, 0, 0.8, &_pow30r, &_pow30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add28r + _pow30r; _add31i = _add28i + _pow30i;
        double magnitude = _add31r; /* +_add31ii */
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 10.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_ang32r, _ang32i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double angle_part1 = _mul37r; /* +_mul37ii */
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x2r, x2i); _ang38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(j, 0, M_PI, 0, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 14.0; _c40i = 0;
        double _div41r = 0, _div41i = 0;
        c_div(_mul39r, _mul39i, _c40r, _c40i, &_div41r, &_div41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_div41r, _div41i, &_cos42r, &_cos42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_ang38r, _ang38i, _cos42r, _cos42i, &_mul43r, &_mul43i);
        double angle_part2 = _mul43r; /* +_mul43ii */
        double _mul44r = 0, _mul44i = 0;
        c_mul(j, 0, M_PI, 0, &_mul44r, &_mul44i);
        double _c45r = 0, _c45i = 0;
        _c45r = 6.0; _c45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(_mul44r, _mul44i, _c45r, _c45i, &_div46r, &_div46i);
        double _sin47r = 0, _sin47i = 0;
        c_sin(_div46r, _div46i, &_sin47r, &_sin47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(j, 0, M_PI, 0, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 9.0; _c49i = 0;
        double _div50r = 0, _div50i = 0;
        c_div(_mul48r, _mul48i, _c49r, _c49i, &_div50r, &_div50i);
        double _cos51r = 0, _cos51i = 0;
        c_cos(_div50r, _div50i, &_cos51r, &_cos51i);
        double _sub52r = 0, _sub52i = 0;
        _sub52r = _sin47r - _cos51r; _sub52i = _sin47i - _cos51i;
        double angle_part3 = _sub52r; /* +_sub52ii */
        double _add53r = 0, _add53i = 0;
        _add53r = angle_part1 + angle_part2; _add53i = 0 + 0;
        double _add54r = 0, _add54i = 0;
        _add54r = _add53r + angle_part3; _add54i = _add53i + 0;
        double angle = _add54r; /* +_add54ii */
        double _cos55r = 0, _cos55i = 0;
        c_cos(angle, 0, &_cos55r, &_cos55i);
        double _c56r = 0, _c56i = 0;
        _c56r = 0.0; _c56i = 1.0;
        double _sin57r = 0, _sin57i = 0;
        c_sin(angle, 0, &_sin57r, &_sin57i);
        double _mul58r = 0, _mul58i = 0;
        c_mul(_c56r, _c56i, _sin57r, _sin57i, &_mul58r, &_mul58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _cos55r + _mul58r; _add59i = _cos55i + _mul58i;
        double _mul60r = 0, _mul60i = 0;
        c_mul(magnitude, 0, _add59r, _add59i, &_mul60r, &_mul60i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul60r; cIm[_idx] = _mul60i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
