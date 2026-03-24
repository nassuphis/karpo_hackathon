/* AUTO-GENERATED from giga.py — do not edit manually */
/* 150 coefficient functions */

static void poly_giga_1_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 30.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_pow3r, _pow3i, x2r, x2i, &_mul4r, &_mul4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c1r, _c1i, _mul4r, _mul4i, &_mul5r, &_mul5i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 30.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(x1r, x1i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c6r, _c6i, _mul9r, _mul9i, &_mul10r, &_mul10i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 40.0; _c11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 3.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow13r, &_pow13i);
    c_mul(_pow13r, _pow13i, x1r, x1i, &_pow13r, &_pow13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c11r, _c11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 40.0; _c15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 3.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow17r, &_pow17i);
    c_mul(_pow17r, _pow17i, x2r, x2i, &_pow17r, &_pow17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c15r, _c15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul18r; cIm[_idx] = _mul18i; } }
    double _c19r = 0, _c19i = 0;
    _c19r = 25.0; _c19i = 0;
    double _neg20r = 0, _neg20i = 0;
    _neg20r = -(_c19r); _neg20i = -(_c19i);
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_neg20r, _neg20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 25.0; _c24i = 0;
    double _neg25r = 0, _neg25i = 0;
    _neg25r = -(_c24r); _neg25i = -(_c24i);
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow27r, &_pow27i);
    double _mul28r = 0, _mul28i = 0;
    c_mul(_neg25r, _neg25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul28r; cIm[_idx] = _mul28i; } }
    double _c29r = 0, _c29i = 0;
    _c29r = 10.0; _c29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul30r, &_mul30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(_c29r, _c29i, _mul30r, _mul30i, &_mul31r, &_mul31i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    double _c32r = 0, _c32i = 0;
    _c32r = 100.0; _c32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 4.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow34r, &_pow34i);
    c_mul(_pow34r, _pow34i, _pow34r, _pow34i, &_pow34r, &_pow34i);
    double _c35r = 0, _c35i = 0;
    _c35r = 4.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow36r, &_pow36i);
    c_mul(_pow36r, _pow36i, _pow36r, _pow36i, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_pow34r, _pow34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c32r, _c32i, _mul37r, _mul37i, &_mul38r, &_mul38i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    double _c39r = 0, _c39i = 0;
    _c39r = 5.0; _c39i = 0;
    double _neg40r = 0, _neg40i = 0;
    _neg40r = -(_c39r); _neg40i = -(_c39i);
    double _mul41r = 0, _mul41i = 0;
    c_mul(_neg40r, _neg40i, x1r, x1i, &_mul41r, &_mul41i);
    { int _idx = 11; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    double _c42r = 0, _c42i = 0;
    _c42r = 5.0; _c42i = 0;
    double _mul43r = 0, _mul43i = 0;
    c_mul(_c42r, _c42i, x2r, x2i, &_mul43r, &_mul43i);
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 10.0; _c44i = 0;
    double _neg45r = 0, _neg45i = 0;
    _neg45r = -(_c44r); _neg45i = -(_c44i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _neg45r; cIm[_idx] = _neg45i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_2_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 5.0; _c1i = 0;
    double _neg2r = 0, _neg2i = 0;
    _neg2r = -(_c1r); _neg2i = -(_c1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _neg2r; cIm[_idx] = _neg2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 10.0; _c3i = 0;
    double _neg4r = 0, _neg4i = 0;
    _neg4r = -(_c3r); _neg4i = -(_c3i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul5r, &_mul5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_neg4r, _neg4i, _mul5r, _mul5i, &_mul6r, &_mul6i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul6r; cIm[_idx] = _mul6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 20.0; _c7i = 0;
    double _sub8r = 0, _sub8i = 0;
    _sub8r = x1r - x2r; _sub8i = x1i - x2i;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c7r, _c7i, _sub8r, _sub8i, &_mul9r, &_mul9i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 50.0; _c10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 3.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, x1r, x1i, &_pow12r, &_pow12i);
    double _add13r = 0, _add13i = 0;
    _add13r = _pow12r + x2r; _add13i = _pow12i + x2i;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c10r, _c10i, _add13r, _add13i, &_mul14r, &_mul14i);
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 80.0; _c15i = 0;
    double _neg16r = 0, _neg16i = 0;
    _neg16r = -(_c15r); _neg16i = -(_c15i);
    double _c17r = 0, _c17i = 0;
    _c17r = 4.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, _pow18r, _pow18i, &_pow18r, &_pow18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = _pow18r - _pow20r; _sub21i = _pow18i - _pow20i;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_neg16r, _neg16i, _sub21r, _sub21i, &_mul22r, &_mul22i);
    { int _idx = 7; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 200.0; _c23i = 0;
    double _c24r = 0, _c24i = 0;
    _c24r = 2.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow25r, &_pow25i);
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow27r, &_pow27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _pow25r + _pow27r; _add28i = _pow25i + _pow27i;
    double _mul29r = 0, _mul29i = 0;
    c_mul(_c23r, _c23i, _add28r, _add28i, &_mul29r, &_mul29i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 150.0; _c30i = 0;
    double _c31r = 0, _c31i = 0;
    _c31r = 3.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow32r, &_pow32i);
    c_mul(_pow32r, _pow32i, x1r, x1i, &_pow32r, &_pow32i);
    double _c33r = 0, _c33i = 0;
    _c33r = 5.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_powr(x2r, x2i, 5.0, &_pow34r, &_pow34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_pow32r, _pow32i, _pow34r, _pow34i, &_mul35r, &_mul35i);
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c30r, _c30i, _mul35r, _mul35i, &_mul36r, &_mul36i);
    { int _idx = 15; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    double _c37r = 0, _c37i = 0;
    _c37r = 30.0; _c37i = 0;
    double _neg38r = 0, _neg38i = 0;
    _neg38r = -(_c37r); _neg38i = -(_c37i);
    double _c39r = 0, _c39i = 0;
    _c39r = 5.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_powr(x1r, x1i, 5.0, &_pow40r, &_pow40i);
    double _c41r = 0, _c41i = 0;
    _c41r = 5.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_powr(x2r, x2i, 5.0, &_pow42r, &_pow42i);
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _pow40r - _pow42r; _sub43i = _pow40i - _pow42i;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_neg38r, _neg38i, _sub43r, _sub43i, &_mul44r, &_mul44i);
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 10.0; _c45i = 0;
    double _c46r = 0, _c46i = 0;
    _c46r = 3.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow47r, &_pow47i);
    c_mul(_pow47r, _pow47i, x2r, x2i, &_pow47r, &_pow47i);
    double _mul48r = 0, _mul48i = 0;
    c_mul(x1r, x1i, _pow47r, _pow47i, &_mul48r, &_mul48i);
    double _mul49r = 0, _mul49i = 0;
    c_mul(_c45r, _c45i, _mul48r, _mul48i, &_mul49r, &_mul49i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_3_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _sub2r = 0, _sub2i = 0;
    _sub2r = x1r - x2r; _sub2i = x1i - x2i;
    double _exp3r = 0, _exp3i = 0;
    c_exp2(_sub2r, _sub2i, &_exp3r, &_exp3i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _exp3r; cIm[_idx] = _exp3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 0.0; _c4i = 0;
    double _add5r = 0, _add5i = 0;
    _add5r = x1r + x2r; _add5i = x1i + x2i;
    double _exp6r = 0, _exp6i = 0;
    c_exp2(_add5r, _add5i, &_exp6r, &_exp6i);
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c4r, _c4i, _exp6r, _exp6i, &_mul7r, &_mul7i);
    { int _idx = 15; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 1.0; _c8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 0.0; _c9i = 1.0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c9r, _c9i, x1r, x1i, &_mul10r, &_mul10i);
    double _exp11r = 0, _exp11i = 0;
    c_exp2(_mul10r, _mul10i, &_exp11r, &_exp11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c8r, _c8i, _exp11r, _exp11i, &_mul12r, &_mul12i);
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _exp14r = 0, _exp14i = 0;
    c_exp2(x1r, x1i, &_exp14r, &_exp14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c13r, _c13i, _exp14r, _exp14i, &_mul15r, &_mul15i);
    { int _idx = 17; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _neg17r = 0, _neg17i = 0;
    _neg17r = -(x1r); _neg17i = -(x1i);
    double _exp18r = 0, _exp18i = 0;
    c_exp2(_neg17r, _neg17i, &_exp18r, &_exp18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _exp18r, _exp18i, &_mul19r, &_mul19i);
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 1.0; _c20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 1.0;
    double _neg22r = 0, _neg22i = 0;
    _neg22r = -(_c21r); _neg22i = -(_c21i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_neg22r, _neg22i, x1r, x1i, &_mul23r, &_mul23i);
    double _exp24r = 0, _exp24i = 0;
    c_exp2(_mul23r, _mul23i, &_exp24r, &_exp24i);
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c20r, _c20i, _exp24r, _exp24i, &_mul25r, &_mul25i);
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    double _c26r = 0, _c26i = 0;
    _c26r = 1.0; _c26i = 0;
    double _c27r = 0, _c27i = 0;
    _c27r = 0.0; _c27i = 1.0;
    double _mul28r = 0, _mul28i = 0;
    c_mul(_c27r, _c27i, x2r, x2i, &_mul28r, &_mul28i);
    double _exp29r = 0, _exp29i = 0;
    c_exp2(_mul28r, _mul28i, &_exp29r, &_exp29i);
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c26r, _c26i, _exp29r, _exp29i, &_mul30r, &_mul30i);
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 1.0; _c31i = 0;
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 1.0;
    double _add33r = 0, _add33i = 0;
    _add33r = _c31r + _c32r; _add33i = _c31i + _c32i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_4_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 25.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 100.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, x1r, x1i, &_pow4r, &_pow4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _pow4r, _pow4i, &_mul5r, &_mul5i);
    double _c6r = 0, _c6i = 0;
    _c6r = 100.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _mul5r + _mul9r; _add10i = _mul5i + _mul9i;
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c11r, _c11i, x1r, x1i, &_mul12r, &_mul12i);
    double _add13r = 0, _add13i = 0;
    _add13r = _add10r + _mul12r; _add13i = _add10i + _mul12i;
    double _c14r = 0, _c14i = 0;
    _c14r = 100.0; _c14i = 0;
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _add13r - _c14r; _sub15i = _add13i - _c14i;
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub15r; cIm[_idx] = _sub15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 100.0; _c16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, x1r, x1i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 100.0; _c20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _sub24r = 0, _sub24i = 0;
    _sub24r = _mul19r - _mul23r; _sub24i = _mul19i - _mul23i;
    double _c25r = 0, _c25i = 0;
    _c25r = 100.0; _c25i = 0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c25r, _c25i, x1r, x1i, &_mul26r, &_mul26i);
    double _add27r = 0, _add27i = 0;
    _add27r = _sub24r + _mul26r; _add27i = _sub24i + _mul26i;
    double _c28r = 0, _c28i = 0;
    _c28r = 100.0; _c28i = 0;
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _add27r - _c28r; _sub29i = _add27i - _c28i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 100.0; _c30i = 0;
    double _c31r = 0, _c31i = 0;
    _c31r = 3.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow32r, &_pow32i);
    c_mul(_pow32r, _pow32i, x1r, x1i, &_pow32r, &_pow32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c30r, _c30i, _pow32r, _pow32i, &_mul33r, &_mul33i);
    double _c34r = 0, _c34i = 0;
    _c34r = 100.0; _c34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 2.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c34r, _c34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _mul33r + _mul37r; _add38i = _mul33i + _mul37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 100.0; _c39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c39r, _c39i, x1r, x1i, &_mul40r, &_mul40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _add38r - _mul40r; _sub41i = _add38i - _mul40i;
    double _c42r = 0, _c42i = 0;
    _c42r = 100.0; _c42i = 0;
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _sub41r - _c42r; _sub43i = _sub41i - _c42i;
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 10.0; _c44i = 0;
    double _neg45r = 0, _neg45i = 0;
    _neg45r = -(_c44r); _neg45i = -(_c44i);
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _neg45r; cIm[_idx] = _neg45i; } }
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 0.2;
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c46r, _c46i, x2r, x2i, &_mul47r, &_mul47i);
    double _exp48r = 0, _exp48i = 0;
    c_exp2(_mul47r, _mul47i, &_exp48r, &_exp48i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _exp48r; cIm[_idx] = _exp48i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_5_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 26.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 4.0; _c2i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c2r; cIm[_idx] = _c2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 4.0; _c3i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c3r; cIm[_idx] = _c3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 9.0; _c4i = 0;
    double _neg5r = 0, _neg5i = 0;
    _neg5r = -(_c4r); _neg5i = -(_c4i);
    { int _idx = 19; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg5r; cIm[_idx] = _neg5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.9; _c6i = 0;
    double _neg7r = 0, _neg7i = 0;
    _neg7r = -(_c6r); _neg7i = -(_c6i);
    { int _idx = 20; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg7r; cIm[_idx] = _neg7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 0.2; _c8i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c8r; cIm[_idx] = _c8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 0.0; _c9i = 100.0;
    double _c10r = 0, _c10i = 0;
    _c10r = 3.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow11r, &_pow11i);
    c_mul(_pow11r, _pow11i, x2r, x2i, &_pow11r, &_pow11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c9r, _c9i, _pow11r, _pow11i, &_mul12r, &_mul12i);
    double _c13r = 0, _c13i = 0;
    _c13r = 0.0; _c13i = 100.0;
    double _c14r = 0, _c14i = 0;
    _c14r = 2.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow15r, &_pow15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c13r, _c13i, _pow15r, _pow15i, &_mul16r, &_mul16i);
    double _add17r = 0, _add17i = 0;
    _add17r = _mul12r + _mul16r; _add17i = _mul12i + _mul16i;
    double _c18r = 0, _c18i = 0;
    _c18r = 0.0; _c18i = 100.0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c18r, _c18i, x2r, x2i, &_mul19r, &_mul19i);
    double _sub20r = 0, _sub20i = 0;
    _sub20r = _add17r - _mul19r; _sub20i = _add17i - _mul19i;
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 100.0;
    double _sub22r = 0, _sub22i = 0;
    _sub22r = _sub20r - _c21r; _sub22i = _sub20i - _c21i;
    { int _idx = 6; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub22r; cIm[_idx] = _sub22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 100.0;
    double _c24r = 0, _c24i = 0;
    _c24r = 3.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow25r, &_pow25i);
    c_mul(_pow25r, _pow25i, x1r, x1i, &_pow25r, &_pow25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c23r, _c23i, _pow25r, _pow25i, &_mul26r, &_mul26i);
    double _c27r = 0, _c27i = 0;
    _c27r = 0.0; _c27i = 100.0;
    double _c28r = 0, _c28i = 0;
    _c28r = 2.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow29r, &_pow29i);
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c27r, _c27i, _pow29r, _pow29i, &_mul30r, &_mul30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _mul26r + _mul30r; _add31i = _mul26i + _mul30i;
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 100.0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c32r, _c32i, x2r, x2i, &_mul33r, &_mul33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _add31r + _mul33r; _add34i = _add31i + _mul33i;
    double _c35r = 0, _c35i = 0;
    _c35r = 0.0; _c35i = 100.0;
    double _sub36r = 0, _sub36i = 0;
    _sub36r = _add34r - _c35r; _sub36i = _add34i - _c35i;
    { int _idx = 8; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    double _c37r = 0, _c37i = 0;
    _c37r = 0.0; _c37i = 100.0;
    double _c38r = 0, _c38i = 0;
    _c38r = 3.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow39r, &_pow39i);
    c_mul(_pow39r, _pow39i, x2r, x2i, &_pow39r, &_pow39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c37r, _c37i, _pow39r, _pow39i, &_mul40r, &_mul40i);
    double _c41r = 0, _c41i = 0;
    _c41r = 0.0; _c41i = 100.0;
    double _c42r = 0, _c42i = 0;
    _c42r = 2.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow43r, &_pow43i);
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c41r, _c41i, _pow43r, _pow43i, &_mul44r, &_mul44i);
    double _sub45r = 0, _sub45i = 0;
    _sub45r = _mul40r - _mul44r; _sub45i = _mul40i - _mul44i;
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 100.0;
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c46r, _c46i, x2r, x2i, &_mul47r, &_mul47i);
    double _add48r = 0, _add48i = 0;
    _add48r = _sub45r + _mul47r; _add48i = _sub45i + _mul47i;
    double _c49r = 0, _c49i = 0;
    _c49r = 0.0; _c49i = 100.0;
    double _sub50r = 0, _sub50i = 0;
    _sub50r = _add48r - _c49r; _sub50i = _add48i - _c49i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub50r; cIm[_idx] = _sub50i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_6_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 150.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, x2r, x2i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 150.0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    double _sub9r = 0, _sub9i = 0;
    _sub9r = _mul4r - _mul8r; _sub9i = _mul4i - _mul8i;
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub9r; cIm[_idx] = _sub9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _c10r; cIm[_idx] = _c10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _sub12r = 0, _sub12i = 0;
    _sub12r = x1r - x2r; _sub12i = x1i - x2i;
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    _pow14r = _sub12r; _pow14i = _sub12i;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c11r, _c11i, _pow14r, _pow14i, &_mul15r, &_mul15i);
    { int _idx = (((int)(n) / 2) - 1); if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 0;
    { int _idx = ((int)(n) - 2); if (_idx >= 0 && _idx < 10) { cRe[_idx] = _c16r; cIm[_idx] = _c16i; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 0.0; _c17i = 10.0;
    { int _idx = ((int)(n) - 1); if (_idx >= 0 && _idx < 10) { cRe[_idx] = _c17r; cIm[_idx] = _c17i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_7: too complex for auto-transpile, stubbed */
static void poly_giga_7_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_7a: too complex for auto-transpile, stubbed */
static void poly_giga_7a_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_7b: too complex for auto-transpile, stubbed */
static void poly_giga_7b_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_7c: too complex for auto-transpile, stubbed */
static void poly_giga_7c_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_7d: too complex for auto-transpile, stubbed */
static void poly_giga_7d_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_8: auto-stubbed (unhandled constructs in source) */
static void poly_giga_8_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_9_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 20;
    for (int _i = 0; _i < 20; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 20.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double re1_r = _attr1r, re1_i = _attr1i;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double im1_r = _attr2r, im1_i = _attr2i;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double re2_r = _attr3r, re2_i = _attr3i;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double im2_r = _attr4r, im2_i = _attr4i;
    double rec[20];
    for (int _li = 0; _li < 20; _li++) {
        rec[_li] = re1_r + (re2_r - re1_r) * _li / 19.0;
    }
    double imc[20];
    for (int _li = 0; _li < 20; _li++) {
        imc[_li] = im1_r + (im2_r - im1_r) * _li / 19.0;
    }
    for (int _vi = 0; _vi < 20; _vi++) {
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 100.0;
        double _c6r = 0, _c6i = 0;
        _c6r = 9.0; _c6i = 0;
        double _vec7r = 0, _vec7i = 0;
        c_powr(imc[_vi], 0, _c6r, &_vec7r, &_vec7i);
        double _vec8r = 0, _vec8i = 0;
        c_mul(_c5r, _c5i, _vec7r, _vec7i, &_vec8r, &_vec8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 100.0; _c9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 9.0; _c10i = 0;
        double _vec11r = 0, _vec11i = 0;
        c_powr(rec[_vi], 0, _c10r, &_vec11r, &_vec11i);
        double _vec12r = 0, _vec12i = 0;
        c_mul(_c9r, _c9i, _vec11r, _vec11i, &_vec12r, &_vec12i);
        double _vec13r = 0, _vec13i = 0;
        _vec13r = _vec8r + _vec12r; _vec13i = _vec8i + _vec12i;
        cRe[_vi] = _vec13r; cIm[_vi] = _vec13i;
    }
    for (int _i = 0; _i < 20; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_10: auto-stubbed (unhandled constructs in source) */
static void poly_giga_10_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_11: auto-stubbed (unhandled constructs in source) */
static void poly_giga_11_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_12: auto-stubbed (unhandled constructs in source) */
static void poly_giga_12_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_13_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 15;
    for (int _i = 0; _i < 15; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 15.0;
    for (int _si = 0; _si < 15; _si++) {
        int _si_idx = _si + 0;
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 1.0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = 0 + _c2r; _add3i = 0 + _c2i;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_c1r, _c1i, _add3r, _add3i, &_mul4r, &_mul4i);
        double _mul5r = 0, _mul5i = 0;
        c_mul(_mul4r, _mul4i, x1r, x1i, &_mul5r, &_mul5i);
        double _exp6r = 0, _exp6i = 0;
        c_exp2(_mul5r, _mul5i, &_exp6r, &_exp6i);
        cRe[_si_idx] = _exp6r; cIm[_si_idx] = _exp6i;
    }
    for (int _si = 0; _si < 14; _si++) {
        int _si_idx = _si + 1;
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 1.0;
        double _neg8r = 0, _neg8i = 0;
        _neg8r = -(_c7r); _neg8i = -(_c7i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = 0 + _c9r; _add10i = 0 + _c9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_neg8r, _neg8i, _add10r, _add10i, &_mul11r, &_mul11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_mul11r, _mul11i, x2r, x2i, &_mul12r, &_mul12i);
        double _exp13r = 0, _exp13i = 0;
        c_exp2(_mul12r, _mul12i, &_exp13r, &_exp13i);
        cRe[_si_idx] = _exp13r; cIm[_si_idx] = _exp13i;
    }
    for (int _i = 0; _i < 15; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_14: auto-stubbed (unhandled constructs in source) */
static void poly_giga_14_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_15: auto-stubbed (unhandled constructs in source) */
static void poly_giga_15_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_16_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 12.0;
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_17: auto-stubbed (unhandled constructs in source) */
static void poly_giga_17_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_18: auto-stubbed (unhandled constructs in source) */
static void poly_giga_18_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_19: auto-stubbed (unhandled constructs in source) */
static void poly_giga_19_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_19_fixed1_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 90;
    for (int _i = 0; _i < 90; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double res = 1000.0;
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, res, 0, &_mul1r, &_mul1i);
    double _int2r = 0, _int2i = 0;
    _int2r = (int)(_mul1r); _int2i = 0;
    double _div3r = 0, _div3i = 0;
    c_div(_int2r, _int2i, res, 0, &_div3r, &_div3i);
    x1r = _div3r; x1i = _div3i;
    double _mul4r = 0, _mul4i = 0;
    c_mul(x2r, x2i, res, 0, &_mul4r, &_mul4i);
    double _int5r = 0, _int5i = 0;
    _int5r = (int)(_mul4r); _int5i = 0;
    double _div6r = 0, _div6i = 0;
    c_div(_int5r, _int5i, res, 0, &_div6r, &_div6i);
    x2r = _div6r; x2i = _div6i;
    double _sub7r = 0, _sub7i = 0;
    _sub7r = x1r - x2r; _sub7i = x1i - x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _sub7r; cIm[_idx] = _sub7i; } }
    for (int k = 1; k < 90; k++) {
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = k + _c8r; _add9i = 0 + _c8i;
        double _cf10r = 0, _cf10i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 90) { _cf10r = cRe[_idx]; _cf10i = cIm[_idx]; } }
        double _mul11r = 0, _mul11i = 0;
        c_mul(_add9r, _add9i, _cf10r, _cf10i, &_mul11r, &_mul11i);
        double _sin12r = 0, _sin12i = 0;
        c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = k + _c13r; _add14i = 0 + _c13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_add14r, _add14i, x1r, x1i, &_mul15r, &_mul15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_mul15r, _mul15i, &_cos16r, &_cos16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _sin12r + _cos16r; _add17i = _sin12i + _cos16i;
        double v_r = _add17r, v_i = _add17i;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(v_r, v_i); _abs18i = 0;
        double av_r = _abs18r, av_i = _abs18i;
        double _fin19r = 0, _fin19i = 0;
        _fin19r = (isfinite(av_r) && isfinite(av_i)) ? 1.0 : 0.0; _fin19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1e-10; _c20i = 0;
        if (((_fin19r != 0 || _fin19i != 0)) && (av_r > _c20r)) {
            double _c21r = 0, _c21i = 0;
            _c21r = 0.0; _c21i = 1.0;
            double _mul22r = 0, _mul22i = 0;
            c_mul(_c21r, _c21i, v_r, v_i, &_mul22r, &_mul22i);
            double _div23r = 0, _div23i = 0;
            c_div(_mul22r, _mul22i, av_r, av_i, &_div23r, &_div23i);
            { int _idx = k; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _div23r; cIm[_idx] = _div23i; } }
        } else {
            double _add24r = 0, _add24i = 0;
            _add24r = x1r + x2r; _add24i = x1i + x2i;
            { int _idx = k; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _add24r; cIm[_idx] = _add24i; } }
        }
    }
    for (int _i = 0; _i < 90; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_19_fixed2_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 90;
    for (int _i = 0; _i < 90; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _sub1r = 0, _sub1i = 0;
    _sub1r = x1r - x2r; _sub1i = x1i - x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _sub1r; cIm[_idx] = _sub1i; } }
    for (int k = 1; k < 90; k++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _cf4r = 0, _cf4i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 90) { _cf4r = cRe[_idx]; _cf4i = cIm[_idx]; } }
        double _mul5r = 0, _mul5i = 0;
        c_mul(_add3r, _add3i, _cf4r, _cf4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = k + _c7r; _add8i = 0 + _c7i;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_add8r, _add8i, x1r, x1i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _sin6r + _cos10r; _add11i = _sin6i + _cos10i;
        double v_r = _add11r, v_i = _add11i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(v_r, v_i); _abs12i = 0;
        double av_r = _abs12r, av_i = _abs12i;
        double _fin13r = 0, _fin13i = 0;
        _fin13r = (isfinite(av_r) && isfinite(av_i)) ? 1.0 : 0.0; _fin13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1e-10; _c14i = 0;
        if (((_fin13r != 0 || _fin13i != 0)) && (av_r > _c14r)) {
            double _c15r = 0, _c15i = 0;
            _c15r = 0.0; _c15i = 1.0;
            double _mul16r = 0, _mul16i = 0;
            c_mul(_c15r, _c15i, v_r, v_i, &_mul16r, &_mul16i);
            double _div17r = 0, _div17i = 0;
            c_div(_mul16r, _mul16i, av_r, av_i, &_div17r, &_div17i);
            { int _idx = k; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _div17r; cIm[_idx] = _div17i; } }
        } else {
            double _add18r = 0, _add18i = 0;
            _add18r = x1r + x2r; _add18i = x1i + x2i;
            { int _idx = k; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _add18r; cIm[_idx] = _add18i; } }
        }
    }
    for (int _i = 0; _i < 90; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_20_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 90;
    for (int _i = 0; _i < 90; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, x2r, x2i, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    for (int k = 1; k < 90; k++) {
        double _cf4r = 0, _cf4i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 90) { _cf4r = cRe[_idx]; _cf4i = cIm[_idx]; } }
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, _cf4r, _cf4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, x1r, x1i, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _sin6r + _cos8r; _add9i = _sin6i + _cos8i;
        double v_r = _add9r, v_i = _add9i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(v_r, v_i); _abs10i = 0;
        double av_r = _abs10r, av_i = _abs10i;
        double _fin11r = 0, _fin11i = 0;
        _fin11r = (isfinite(av_r) && isfinite(av_i)) ? 1.0 : 0.0; _fin11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1e-10; _c12i = 0;
        if (((_fin11r != 0 || _fin11i != 0)) && (av_r > _c12r)) {
            double _c13r = 0, _c13i = 0;
            _c13r = 0.0; _c13i = 1.0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(_c13r, _c13i, v_r, v_i, &_mul14r, &_mul14i);
            double _div15r = 0, _div15i = 0;
            c_div(_mul14r, _mul14i, av_r, av_i, &_div15r, &_div15i);
            { int _idx = k; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
        } else {
            double _add16r = 0, _add16i = 0;
            _add16r = x1r + x2r; _add16i = x1i + x2i;
            { int _idx = k; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _add16r; cIm[_idx] = _add16i; } }
        }
    }
    for (int _i = 0; _i < 90; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_21_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 50;
    for (int _i = 0; _i < 50; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int k = 1; k < 50; k++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 3.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _c4r = 0, _c4i = 0;
        _c4r = 10.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(_add3r, _c4r); _mod5i = 0;
        double _cf6r = 0, _cf6i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 50) { _cf6r = cRe[_idx]; _cf6i = cIm[_idx]; } }
        double _mul7r = 0, _mul7i = 0;
        c_mul(_mod5r, _mod5i, _cf6r, _cf6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = k + _c9r; _add10i = 0 + _c9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 10.0; _c11i = 0;
        double _mod12r = 0, _mod12i = 0;
        _mod12r = fmod(_add10r, _c11r); _mod12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_mod12r, _mod12i, x1r, x1i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _sin8r + _cos14r; _add15i = _sin8i + _cos14i;
        double v_r = _add15r, v_i = _add15i;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(v_r, v_i); _abs16i = 0;
        double av_r = _abs16r, av_i = _abs16i;
        double _fin17r = 0, _fin17i = 0;
        _fin17r = (isfinite(av_r) && isfinite(av_i)) ? 1.0 : 0.0; _fin17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1e-10; _c18i = 0;
        if (((_fin17r != 0 || _fin17i != 0)) && (av_r > _c18r)) {
            double _div19r = 0, _div19i = 0;
            c_div(v_r, v_i, av_r, av_i, &_div19r, &_div19i);
            { int _idx = k; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _div19r; cIm[_idx] = _div19i; } }
        } else {
            double _add20r = 0, _add20i = 0;
            _add20r = x1r + x2r; _add20i = x1i + x2i;
            { int _idx = k; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
        }
    }
    for (int _i = 0; _i < 50; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_22_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 26.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 100.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, x1r, x1i, &_pow4r, &_pow4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _pow4r, _pow4i, &_mul5r, &_mul5i);
    double _c6r = 0, _c6i = 0;
    _c6r = 100.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _mul5r + _mul9r; _add10i = _mul5i + _mul9i;
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c11r, _c11i, x1r, x1i, &_mul12r, &_mul12i);
    double _add13r = 0, _add13i = 0;
    _add13r = _add10r + _mul12r; _add13i = _add10i + _mul12i;
    double _c14r = 0, _c14i = 0;
    _c14r = 100.0; _c14i = 0;
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _add13r - _c14r; _sub15i = _add13i - _c14i;
    { int _idx = 12; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub15r; cIm[_idx] = _sub15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 100.0; _c16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, x2r, x2i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 100.0; _c20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _sub24r = 0, _sub24i = 0;
    _sub24r = _mul19r - _mul23r; _sub24i = _mul19i - _mul23i;
    double _c25r = 0, _c25i = 0;
    _c25r = 100.0; _c25i = 0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c25r, _c25i, x2r, x2i, &_mul26r, &_mul26i);
    double _add27r = 0, _add27i = 0;
    _add27r = _sub24r + _mul26r; _add27i = _sub24i + _mul26i;
    double _c28r = 0, _c28i = 0;
    _c28r = 100.0; _c28i = 0;
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _add27r - _c28r; _sub29i = _add27i - _c28i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 100.0; _c30i = 0;
    double _c31r = 0, _c31i = 0;
    _c31r = 3.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow32r, &_pow32i);
    c_mul(_pow32r, _pow32i, x2r, x2i, &_pow32r, &_pow32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c30r, _c30i, _pow32r, _pow32i, &_mul33r, &_mul33i);
    double _c34r = 0, _c34i = 0;
    _c34r = 100.0; _c34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 2.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c34r, _c34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _mul33r + _mul37r; _add38i = _mul33i + _mul37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 100.0; _c39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c39r, _c39i, x2r, x2i, &_mul40r, &_mul40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _add38r - _mul40r; _sub41i = _add38i - _mul40i;
    double _c42r = 0, _c42i = 0;
    _c42r = 100.0; _c42i = 0;
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _sub41r - _c42r; _sub43i = _sub41i - _c42i;
    { int _idx = 16; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 10.0; _c44i = 0;
    double _neg45r = 0, _neg45i = 0;
    _neg45r = -(_c44r); _neg45i = -(_c44i);
    { int _idx = 20; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg45r; cIm[_idx] = _neg45i; } }
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 0.2;
    { int _idx = 24; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c46r; cIm[_idx] = _c46i; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 0.0; _c47i = 0;
    { int _idx = 25; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c47r; cIm[_idx] = _c47i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_23: auto-stubbed (unhandled constructs in source) */
static void poly_giga_23_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_24: auto-stubbed (unhandled constructs in source) */
static void poly_giga_24_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_25_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 26.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 100.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, x1r, x1i, &_pow4r, &_pow4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _pow4r, _pow4i, &_mul5r, &_mul5i);
    double _c6r = 0, _c6i = 0;
    _c6r = 100.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _mul5r + _mul9r; _add10i = _mul5i + _mul9i;
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c11r, _c11i, x1r, x1i, &_mul12r, &_mul12i);
    double _add13r = 0, _add13i = 0;
    _add13r = _add10r + _mul12r; _add13i = _add10i + _mul12i;
    double _c14r = 0, _c14i = 0;
    _c14r = 100.0; _c14i = 0;
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _add13r - _c14r; _sub15i = _add13i - _c14i;
    { int _idx = 12; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub15r; cIm[_idx] = _sub15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 100.0; _c16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, x2r, x2i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 100.0; _c20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _sub24r = 0, _sub24i = 0;
    _sub24r = _mul19r - _mul23r; _sub24i = _mul19i - _mul23i;
    double _c25r = 0, _c25i = 0;
    _c25r = 100.0; _c25i = 0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c25r, _c25i, x2r, x2i, &_mul26r, &_mul26i);
    double _add27r = 0, _add27i = 0;
    _add27r = _sub24r + _mul26r; _add27i = _sub24i + _mul26i;
    double _c28r = 0, _c28i = 0;
    _c28r = 100.0; _c28i = 0;
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _add27r - _c28r; _sub29i = _add27i - _c28i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 100.0; _c30i = 0;
    double _c31r = 0, _c31i = 0;
    _c31r = 3.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow32r, &_pow32i);
    c_mul(_pow32r, _pow32i, x2r, x2i, &_pow32r, &_pow32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c30r, _c30i, _pow32r, _pow32i, &_mul33r, &_mul33i);
    double _c34r = 0, _c34i = 0;
    _c34r = 100.0; _c34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 2.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c34r, _c34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _mul33r + _mul37r; _add38i = _mul33i + _mul37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 100.0; _c39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c39r, _c39i, x2r, x2i, &_mul40r, &_mul40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _add38r - _mul40r; _sub41i = _add38i - _mul40i;
    double _c42r = 0, _c42i = 0;
    _c42r = 100.0; _c42i = 0;
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _sub41r - _c42r; _sub43i = _sub41i - _c42i;
    { int _idx = 16; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 10.0; _c44i = 0;
    double _neg45r = 0, _neg45i = 0;
    _neg45r = -(_c44r); _neg45i = -(_c44i);
    { int _idx = 20; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg45r; cIm[_idx] = _neg45i; } }
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 0.2;
    { int _idx = 24; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c46r; cIm[_idx] = _c46i; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 0.0; _c47i = 0;
    { int _idx = 25; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c47r; cIm[_idx] = _c47i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_26_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 26.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 100.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, x1r, x1i, &_pow4r, &_pow4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _pow4r, _pow4i, &_mul5r, &_mul5i);
    double _c6r = 0, _c6i = 0;
    _c6r = 100.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _mul5r + _mul9r; _add10i = _mul5i + _mul9i;
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c11r, _c11i, x1r, x1i, &_mul12r, &_mul12i);
    double _add13r = 0, _add13i = 0;
    _add13r = _add10r + _mul12r; _add13i = _add10i + _mul12i;
    double _c14r = 0, _c14i = 0;
    _c14r = 100.0; _c14i = 0;
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _add13r - _c14r; _sub15i = _add13i - _c14i;
    { int _idx = 12; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub15r; cIm[_idx] = _sub15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 100.0; _c16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, x2r, x2i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 100.0; _c20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _sub24r = 0, _sub24i = 0;
    _sub24r = _mul19r - _mul23r; _sub24i = _mul19i - _mul23i;
    double _c25r = 0, _c25i = 0;
    _c25r = 100.0; _c25i = 0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c25r, _c25i, x2r, x2i, &_mul26r, &_mul26i);
    double _add27r = 0, _add27i = 0;
    _add27r = _sub24r + _mul26r; _add27i = _sub24i + _mul26i;
    double _c28r = 0, _c28i = 0;
    _c28r = 100.0; _c28i = 0;
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _add27r - _c28r; _sub29i = _add27i - _c28i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 100.0; _c30i = 0;
    double _c31r = 0, _c31i = 0;
    _c31r = 3.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow32r, &_pow32i);
    c_mul(_pow32r, _pow32i, x2r, x2i, &_pow32r, &_pow32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c30r, _c30i, _pow32r, _pow32i, &_mul33r, &_mul33i);
    double _c34r = 0, _c34i = 0;
    _c34r = 100.0; _c34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 2.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c34r, _c34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _mul33r + _mul37r; _add38i = _mul33i + _mul37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 100.0; _c39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c39r, _c39i, x2r, x2i, &_mul40r, &_mul40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _add38r - _mul40r; _sub41i = _add38i - _mul40i;
    double _c42r = 0, _c42i = 0;
    _c42r = 100.0; _c42i = 0;
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _sub41r - _c42r; _sub43i = _sub41i - _c42i;
    { int _idx = 16; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    double _c44r = 0, _c44i = 0;
    _c44r = 10.0; _c44i = 0;
    double _neg45r = 0, _neg45i = 0;
    _neg45r = -(_c44r); _neg45i = -(_c44i);
    { int _idx = 20; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg45r; cIm[_idx] = _neg45i; } }
    double _c46r = 0, _c46i = 0;
    _c46r = 0.2; _c46i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c46r; cIm[_idx] = _c46i; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 0.0; _c47i = 0;
    { int _idx = 25; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c47r; cIm[_idx] = _c47i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_27: auto-stubbed (unhandled constructs in source) */
static void poly_giga_27_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 12;
    for (int _i = 0; _i < 12; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_28_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 6;
    for (int _i = 0; _i < 6; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 6.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, x2r, x2i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 100.0;
    double _c6r = 0, _c6i = 0;
    _c6r = 3.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
    c_mul(_pow7r, _pow7i, x1r, x1i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _mul4r + _mul8r; _add9i = _mul4i + _mul8i;
    { int _idx = 0; if (_idx >= 0 && _idx < 6) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 6) { cRe[_idx] = _c10r; cIm[_idx] = _c10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _div12r = 0, _div12i = 0;
    c_div(n, 0, _c11r, _c11i, &_div12r, &_div12i);
    double _int13r = 0, _int13i = 0;
    _int13r = (int)(_div12r); _int13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 150.0; _c14i = 0;
    { int _idx = ((int)(_int13r) - 1); if (_idx >= 0 && _idx < 6) { cRe[_idx] = _c14r; cIm[_idx] = _c14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 0;
    { int _idx = ((int)(n) - 2); if (_idx >= 0 && _idx < 6) { cRe[_idx] = _c15r; cIm[_idx] = _c15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 40.0;
    { int _idx = ((int)(n) - 1); if (_idx >= 0 && _idx < 6) { cRe[_idx] = _c16r; cIm[_idx] = _c16i; } }
    for (int _i = 0; _i < 6; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_29_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 150.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, x2r, x2i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 150.0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    double _sub9r = 0, _sub9i = 0;
    _sub9r = _mul4r - _mul8r; _sub9i = _mul4i - _mul8i;
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub9r; cIm[_idx] = _sub9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _c10r; cIm[_idx] = _c10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _sub12r = 0, _sub12i = 0;
    _sub12r = x1r - x2r; _sub12i = x1i - x2i;
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    _pow14r = _sub12r; _pow14i = _sub12i;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c11r, _c11i, _pow14r, _pow14i, &_mul15r, &_mul15i);
    { int _idx = (((int)(n) / 2) - 1); if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 0;
    { int _idx = ((int)(n) - 2); if (_idx >= 0 && _idx < 10) { cRe[_idx] = _c16r; cIm[_idx] = _c16i; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 0.0; _c17i = 10.0;
    { int _idx = ((int)(n) - 1); if (_idx >= 0 && _idx < 10) { cRe[_idx] = _c17r; cIm[_idx] = _c17i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_30_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 150.0;
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 100.0; _c5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 3.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
    c_mul(_pow7r, _pow7i, x1r, x1i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _mul4r + _mul8r; _add9i = _mul4i + _mul8i;
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 150.0; _c10i = 0;
    double _add11r = 0, _add11i = 0;
    _add11r = x1r + x2r; _add11i = x1i + x2i;
    double _c12r = 0, _c12i = 0;
    _c12r = 2.5; _c12i = 0;
    double _c13r = 0, _c13i = 0;
    _c13r = 0.0; _c13i = 1.0;
    double _c14r = 0, _c14i = 0;
    _c14r = 1.0; _c14i = 0;
    double _add15r = 0, _add15i = 0;
    _add15r = _c13r + _c14r; _add15i = _c13i + _c14i;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c12r, _c12i, _add15r, _add15i, &_mul16r, &_mul16i);
    double _sub17r = 0, _sub17i = 0;
    _sub17r = _add11r - _mul16r; _sub17i = _add11i - _mul16i;
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(_sub17r, _sub17i); _abs18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c10r, _c10i, _abs18r, _abs18i, &_mul19r, &_mul19i);
    { int _idx = (((int)(n) / 2) - 1); if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 100.0;
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, x1r, x1i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 150.0; _c24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c24r, _c24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _mul23r + _mul27r; _add28i = _mul23i + _mul27i;
    { int _idx = ((int)(n) - 1); if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add28r; cIm[_idx] = _add28i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_31: auto-stubbed (unhandled constructs in source) */
static void poly_giga_31_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 100;
    for (int _i = 0; _i < 100; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_32: auto-stubbed (unhandled constructs in source) */
static void poly_giga_32_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 12;
    for (int _i = 0; _i < 12; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_33_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 11.0;
    double _c2r = 0, _c2i = 0;
    _c2r = 4.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, _pow3r, _pow3i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 13.0;
    double _c6r = 0, _c6i = 0;
    _c6r = 3.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
    c_mul(_pow7r, _pow7i, x1r, x1i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _mul4r + _mul8r; _add9i = _mul4i + _mul8i;
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 17.0;
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow12r, &_pow12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c10r, _c10i, _pow12r, _pow12i, &_mul13r, &_mul13i);
    double _add14r = 0, _add14i = 0;
    _add14r = _add9r + _mul13r; _add14i = _add9i + _mul13i;
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 19.0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c15r, _c15i, x1r, x1i, &_mul16r, &_mul16i);
    double _add17r = 0, _add17i = 0;
    _add17r = _add14r + _mul16r; _add17i = _add14i + _mul16i;
    double _c18r = 0, _c18i = 0;
    _c18r = 0.0; _c18i = 23.0;
    double _add19r = 0, _add19i = 0;
    _add19r = _add17r + _c18r; _add19i = _add17i + _c18i;
    { int _idx = 0; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 100.0;
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, x1r, x1i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 100.0;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c24r, _c24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _mul23r + _mul27r; _add28i = _mul23i + _mul27i;
    double _c29r = 0, _c29i = 0;
    _c29r = 100.0; _c29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c29r, _c29i, x1r, x1i, &_mul30r, &_mul30i);
    double _sub31r = 0, _sub31i = 0;
    _sub31r = _add28r - _mul30r; _sub31i = _add28i - _mul30i;
    double _c32r = 0, _c32i = 0;
    _c32r = 100.0; _c32i = 0;
    double _sub33r = 0, _sub33i = 0;
    _sub33r = _sub31r - _c32r; _sub33i = _sub31i - _c32i;
    { int _idx = 1; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub33r; cIm[_idx] = _sub33i; } }
    double _c34r = 0, _c34i = 0;
    _c34r = 100.0; _c34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 3.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow36r, &_pow36i);
    c_mul(_pow36r, _pow36i, x2r, x2i, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c34r, _c34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 100.0;
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow40r, &_pow40i);
    double _mul41r = 0, _mul41i = 0;
    c_mul(_c38r, _c38i, _pow40r, _pow40i, &_mul41r, &_mul41i);
    double _add42r = 0, _add42i = 0;
    _add42r = _mul37r + _mul41r; _add42i = _mul37i + _mul41i;
    double _c43r = 0, _c43i = 0;
    _c43r = 0.0; _c43i = 100.0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c43r, _c43i, x2r, x2i, &_mul44r, &_mul44i);
    double _sub45r = 0, _sub45i = 0;
    _sub45r = _add42r - _mul44r; _sub45i = _add42i - _mul44i;
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 100.0;
    double _sub47r = 0, _sub47i = 0;
    _sub47r = _sub45r - _c46r; _sub47i = _sub45i - _c46i;
    { int _idx = 2; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub47r; cIm[_idx] = _sub47i; } }
    double _c48r = 0, _c48i = 0;
    _c48r = 0.0; _c48i = 100.0;
    double _c49r = 0, _c49i = 0;
    _c49r = 3.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow50r, &_pow50i);
    c_mul(_pow50r, _pow50i, x1r, x1i, &_pow50r, &_pow50i);
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c48r, _c48i, _pow50r, _pow50i, &_mul51r, &_mul51i);
    double _c52r = 0, _c52i = 0;
    _c52r = 100.0; _c52i = 0;
    double _c53r = 0, _c53i = 0;
    _c53r = 2.0; _c53i = 0;
    double _pow54r = 0, _pow54i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow54r, &_pow54i);
    double _mul55r = 0, _mul55i = 0;
    c_mul(_c52r, _c52i, _pow54r, _pow54i, &_mul55r, &_mul55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _mul51r + _mul55r; _add56i = _mul51i + _mul55i;
    double _c57r = 0, _c57i = 0;
    _c57r = 100.0; _c57i = 0;
    double _mul58r = 0, _mul58i = 0;
    c_mul(_c57r, _c57i, x1r, x1i, &_mul58r, &_mul58i);
    double _sub59r = 0, _sub59i = 0;
    _sub59r = _add56r - _mul58r; _sub59i = _add56i - _mul58i;
    double _c60r = 0, _c60i = 0;
    _c60r = 0.0; _c60i = 100.0;
    double _sub61r = 0, _sub61i = 0;
    _sub61r = _sub59r - _c60r; _sub61i = _sub59i - _c60i;
    { int _idx = 3; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub61r; cIm[_idx] = _sub61i; } }
    double _c62r = 0, _c62i = 0;
    _c62r = 3.0; _c62i = 0;
    double _neg63r = 0, _neg63i = 0;
    _neg63r = -(_c62r); _neg63i = -(_c62i);
    { int _idx = 4; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg63r; cIm[_idx] = _neg63i; } }
    double _c64r = 0, _c64i = 0;
    _c64r = 101.0; _c64i = 0;
    double _c65r = 0, _c65i = 0;
    _c65r = 3.0; _c65i = 0;
    double _pow66r = 0, _pow66i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow66r, &_pow66i);
    c_mul(_pow66r, _pow66i, x2r, x2i, &_pow66r, &_pow66i);
    double _mul67r = 0, _mul67i = 0;
    c_mul(_c64r, _c64i, _pow66r, _pow66i, &_mul67r, &_mul67i);
    double _c68r = 0, _c68i = 0;
    _c68r = 103.0; _c68i = 0;
    double _c69r = 0, _c69i = 0;
    _c69r = 2.0; _c69i = 0;
    double _pow70r = 0, _pow70i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow70r, &_pow70i);
    double _mul71r = 0, _mul71i = 0;
    c_mul(_c68r, _c68i, _pow70r, _pow70i, &_mul71r, &_mul71i);
    double _add72r = 0, _add72i = 0;
    _add72r = _mul67r + _mul71r; _add72i = _mul67i + _mul71i;
    double _c73r = 0, _c73i = 0;
    _c73r = 107.0; _c73i = 0;
    double _mul74r = 0, _mul74i = 0;
    c_mul(_c73r, _c73i, x2r, x2i, &_mul74r, &_mul74i);
    double _sub75r = 0, _sub75i = 0;
    _sub75r = _add72r - _mul74r; _sub75i = _add72i - _mul74i;
    double _c76r = 0, _c76i = 0;
    _c76r = 109.0; _c76i = 0;
    double _sub77r = 0, _sub77i = 0;
    _sub77r = _sub75r - _c76r; _sub77i = _sub75i - _c76i;
    { int _idx = 6; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub77r; cIm[_idx] = _sub77i; } }
    double _c78r = 0, _c78i = 0;
    _c78r = 113.0; _c78i = 0;
    double _c79r = 0, _c79i = 0;
    _c79r = 3.0; _c79i = 0;
    double _pow80r = 0, _pow80i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow80r, &_pow80i);
    c_mul(_pow80r, _pow80i, x1r, x1i, &_pow80r, &_pow80i);
    double _mul81r = 0, _mul81i = 0;
    c_mul(_c78r, _c78i, _pow80r, _pow80i, &_mul81r, &_mul81i);
    double _c82r = 0, _c82i = 0;
    _c82r = 127.0; _c82i = 0;
    double _c83r = 0, _c83i = 0;
    _c83r = 2.0; _c83i = 0;
    double _pow84r = 0, _pow84i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow84r, &_pow84i);
    double _mul85r = 0, _mul85i = 0;
    c_mul(_c82r, _c82i, _pow84r, _pow84i, &_mul85r, &_mul85i);
    double _add86r = 0, _add86i = 0;
    _add86r = _mul81r + _mul85r; _add86i = _mul81i + _mul85i;
    double _c87r = 0, _c87i = 0;
    _c87r = 131.0; _c87i = 0;
    double _mul88r = 0, _mul88i = 0;
    c_mul(_c87r, _c87i, x2r, x2i, &_mul88r, &_mul88i);
    double _add89r = 0, _add89i = 0;
    _add89r = _add86r + _mul88r; _add89i = _add86i + _mul88i;
    double _c90r = 0, _c90i = 0;
    _c90r = 137.0; _c90i = 0;
    double _sub91r = 0, _sub91i = 0;
    _sub91r = _add89r - _c90r; _sub91i = _add89i - _c90i;
    { int _idx = 8; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub91r; cIm[_idx] = _sub91i; } }
    double _c92r = 0, _c92i = 0;
    _c92r = 5.0; _c92i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c92r; cIm[_idx] = _c92i; } }
    double _c93r = 0, _c93i = 0;
    _c93r = 67.0; _c93i = 0;
    double _c94r = 0, _c94i = 0;
    _c94r = 3.0; _c94i = 0;
    double _pow95r = 0, _pow95i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow95r, &_pow95i);
    c_mul(_pow95r, _pow95i, x2r, x2i, &_pow95r, &_pow95i);
    double _mul96r = 0, _mul96i = 0;
    c_mul(_c93r, _c93i, _pow95r, _pow95i, &_mul96r, &_mul96i);
    double _c97r = 0, _c97i = 0;
    _c97r = 71.0; _c97i = 0;
    double _c98r = 0, _c98i = 0;
    _c98r = 2.0; _c98i = 0;
    double _pow99r = 0, _pow99i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow99r, &_pow99i);
    double _mul100r = 0, _mul100i = 0;
    c_mul(_c97r, _c97i, _pow99r, _pow99i, &_mul100r, &_mul100i);
    double _sub101r = 0, _sub101i = 0;
    _sub101r = _mul96r - _mul100r; _sub101i = _mul96i - _mul100i;
    double _c102r = 0, _c102i = 0;
    _c102r = 73.0; _c102i = 0;
    double _mul103r = 0, _mul103i = 0;
    c_mul(_c102r, _c102i, x2r, x2i, &_mul103r, &_mul103i);
    double _add104r = 0, _add104i = 0;
    _add104r = _sub101r + _mul103r; _add104i = _sub101i + _mul103i;
    double _c105r = 0, _c105i = 0;
    _c105r = 79.0; _c105i = 0;
    double _sub106r = 0, _sub106i = 0;
    _sub106r = _add104r - _c105r; _sub106i = _add104i - _c105i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub106r; cIm[_idx] = _sub106i; } }
    double _c107r = 0, _c107i = 0;
    _c107r = 7.0; _c107i = 0;
    double _neg108r = 0, _neg108i = 0;
    _neg108r = -(_c107r); _neg108i = -(_c107i);
    { int _idx = 16; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg108r; cIm[_idx] = _neg108i; } }
    double _c109r = 0, _c109i = 0;
    _c109r = 11.0; _c109i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c109r; cIm[_idx] = _c109i; } }
    double _c110r = 0, _c110i = 0;
    _c110r = 13.0; _c110i = 0;
    double _neg111r = 0, _neg111i = 0;
    _neg111r = -(_c110r); _neg111i = -(_c110i);
    { int _idx = 24; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg111r; cIm[_idx] = _neg111i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_34_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 120;
    for (int _i = 0; _i < 120; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 120.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _neg2r = 0, _neg2i = 0;
    _neg2r = -(_c1r); _neg2i = -(_c1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 120) { cRe[_idx] = _neg2r; cIm[_idx] = _neg2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 100.0; _c3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c3r, _c3i, x1r, x1i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 100.0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c5r, _c5i, x2r, x2i, &_mul6r, &_mul6i);
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _mul4r - _mul6r; _sub7i = _mul4i - _mul6i;
    { int _idx = (((int)(n) / 2) - 1); if (_idx >= 0 && _idx < 120) { cRe[_idx] = _sub7r; cIm[_idx] = _sub7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 0.4; _c8i = 0;
    { int _idx = ((int)(n) - 1); if (_idx >= 0 && _idx < 120) { cRe[_idx] = _c8r; cIm[_idx] = _c8i; } }
    for (int _i = 0; _i < 120; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_35_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 120;
    for (int _i = 0; _i < 120; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 120.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _neg2r = 0, _neg2i = 0;
    _neg2r = -(_c1r); _neg2i = -(_c1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 120) { cRe[_idx] = _neg2r; cIm[_idx] = _neg2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 100.0; _c3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c3r, _c3i, x1r, x1i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 100.0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c5r, _c5i, x2r, x2i, &_mul6r, &_mul6i);
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _mul4r - _mul6r; _sub7i = _mul4i - _mul6i;
    { int _idx = (((int)(n) / 2) - 1); if (_idx >= 0 && _idx < 120) { cRe[_idx] = _sub7r; cIm[_idx] = _sub7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 0.4; _c8i = 0;
    { int _idx = ((int)(n) - 1); if (_idx >= 0 && _idx < 120) { cRe[_idx] = _c8r; cIm[_idx] = _c8i; } }
    for (int _i = 0; _i < 120; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_36_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 100.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, x1r, x1i, &_pow4r, &_pow4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _pow4r, _pow4i, &_mul5r, &_mul5i);
    double _c6r = 0, _c6i = 0;
    _c6r = 100.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _mul5r + _mul9r; _add10i = _mul5i + _mul9i;
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c11r, _c11i, x1r, x1i, &_mul12r, &_mul12i);
    double _sub13r = 0, _sub13i = 0;
    _sub13r = _add10r - _mul12r; _sub13i = _add10i - _mul12i;
    double _c14r = 0, _c14i = 0;
    _c14r = 100.0; _c14i = 0;
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _sub13r - _c14r; _sub15i = _sub13i - _c14i;
    { int _idx = 1; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub15r; cIm[_idx] = _sub15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 100.0; _c16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, x2r, x2i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 100.0; _c20i = 0;
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _add24r = 0, _add24i = 0;
    _add24r = _mul19r + _mul23r; _add24i = _mul19i + _mul23i;
    double _c25r = 0, _c25i = 0;
    _c25r = 100.0; _c25i = 0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c25r, _c25i, x2r, x2i, &_mul26r, &_mul26i);
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _add24r - _mul26r; _sub27i = _add24i - _mul26i;
    double _c28r = 0, _c28i = 0;
    _c28r = 100.0; _c28i = 0;
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _sub27r - _c28r; _sub29i = _sub27i - _c28i;
    { int _idx = 2; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 4.0; _c30i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c30r; cIm[_idx] = _c30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 100.0; _c31i = 0;
    double _c32r = 0, _c32i = 0;
    _c32r = 3.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow33r, &_pow33i);
    c_mul(_pow33r, _pow33i, x2r, x2i, &_pow33r, &_pow33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c31r, _c31i, _pow33r, _pow33i, &_mul34r, &_mul34i);
    double _c35r = 0, _c35i = 0;
    _c35r = 100.0; _c35i = 0;
    double _c36r = 0, _c36i = 0;
    _c36r = 2.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow37r, &_pow37i);
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c35r, _c35i, _pow37r, _pow37i, &_mul38r, &_mul38i);
    double _add39r = 0, _add39i = 0;
    _add39r = _mul34r + _mul38r; _add39i = _mul34i + _mul38i;
    double _c40r = 0, _c40i = 0;
    _c40r = 100.0; _c40i = 0;
    double _mul41r = 0, _mul41i = 0;
    c_mul(_c40r, _c40i, x2r, x2i, &_mul41r, &_mul41i);
    double _sub42r = 0, _sub42i = 0;
    _sub42r = _add39r - _mul41r; _sub42i = _add39i - _mul41i;
    double _c43r = 0, _c43i = 0;
    _c43r = 100.0; _c43i = 0;
    double _sub44r = 0, _sub44i = 0;
    _sub44r = _sub42r - _c43r; _sub44i = _sub42i - _c43i;
    { int _idx = 6; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub44r; cIm[_idx] = _sub44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 100.0; _c45i = 0;
    double _c46r = 0, _c46i = 0;
    _c46r = 3.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow47r, &_pow47i);
    c_mul(_pow47r, _pow47i, x1r, x1i, &_pow47r, &_pow47i);
    double _mul48r = 0, _mul48i = 0;
    c_mul(_c45r, _c45i, _pow47r, _pow47i, &_mul48r, &_mul48i);
    double _c49r = 0, _c49i = 0;
    _c49r = 100.0; _c49i = 0;
    double _c50r = 0, _c50i = 0;
    _c50r = 2.0; _c50i = 0;
    double _pow51r = 0, _pow51i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow51r, &_pow51i);
    double _mul52r = 0, _mul52i = 0;
    c_mul(_c49r, _c49i, _pow51r, _pow51i, &_mul52r, &_mul52i);
    double _add53r = 0, _add53i = 0;
    _add53r = _mul48r + _mul52r; _add53i = _mul48i + _mul52i;
    double _c54r = 0, _c54i = 0;
    _c54r = 100.0; _c54i = 0;
    double _mul55r = 0, _mul55i = 0;
    c_mul(_c54r, _c54i, x2r, x2i, &_mul55r, &_mul55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _add53r + _mul55r; _add56i = _add53i + _mul55i;
    double _c57r = 0, _c57i = 0;
    _c57r = 100.0; _c57i = 0;
    double _sub58r = 0, _sub58i = 0;
    _sub58r = _add56r - _c57r; _sub58i = _add56i - _c57i;
    { int _idx = 8; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub58r; cIm[_idx] = _sub58i; } }
    double _c59r = 0, _c59i = 0;
    _c59r = 8.0; _c59i = 0;
    double _neg60r = 0, _neg60i = 0;
    _neg60r = -(_c59r); _neg60i = -(_c59i);
    { int _idx = 12; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg60r; cIm[_idx] = _neg60i; } }
    double _c61r = 0, _c61i = 0;
    _c61r = 100.0; _c61i = 0;
    double _c62r = 0, _c62i = 0;
    _c62r = 3.0; _c62i = 0;
    double _pow63r = 0, _pow63i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow63r, &_pow63i);
    c_mul(_pow63r, _pow63i, x2r, x2i, &_pow63r, &_pow63i);
    double _mul64r = 0, _mul64i = 0;
    c_mul(_c61r, _c61i, _pow63r, _pow63i, &_mul64r, &_mul64i);
    double _c65r = 0, _c65i = 0;
    _c65r = 100.0; _c65i = 0;
    double _c66r = 0, _c66i = 0;
    _c66r = 2.0; _c66i = 0;
    double _pow67r = 0, _pow67i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow67r, &_pow67i);
    double _mul68r = 0, _mul68i = 0;
    c_mul(_c65r, _c65i, _pow67r, _pow67i, &_mul68r, &_mul68i);
    double _sub69r = 0, _sub69i = 0;
    _sub69r = _mul64r - _mul68r; _sub69i = _mul64i - _mul68i;
    double _c70r = 0, _c70i = 0;
    _c70r = 100.0; _c70i = 0;
    double _mul71r = 0, _mul71i = 0;
    c_mul(_c70r, _c70i, x2r, x2i, &_mul71r, &_mul71i);
    double _add72r = 0, _add72i = 0;
    _add72r = _sub69r + _mul71r; _add72i = _sub69i + _mul71i;
    double _c73r = 0, _c73i = 0;
    _c73r = 100.0; _c73i = 0;
    double _sub74r = 0, _sub74i = 0;
    _sub74r = _add72r - _c73r; _sub74i = _add72i - _c73i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub74r; cIm[_idx] = _sub74i; } }
    double _c75r = 0, _c75i = 0;
    _c75r = 16.0; _c75i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c75r; cIm[_idx] = _c75i; } }
    double _c76r = 0, _c76i = 0;
    _c76r = 32.0; _c76i = 0;
    double _neg77r = 0, _neg77i = 0;
    _neg77r = -(_c76r); _neg77i = -(_c76i);
    { int _idx = 20; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg77r; cIm[_idx] = _neg77i; } }
    double _c78r = 0, _c78i = 0;
    _c78r = 64.0; _c78i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c78r; cIm[_idx] = _c78i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_37_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 11.0;
    double _c2r = 0, _c2i = 0;
    _c2r = 4.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, _pow3r, _pow3i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 13.0;
    double _c6r = 0, _c6i = 0;
    _c6r = 3.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow7r, &_pow7i);
    c_mul(_pow7r, _pow7i, x1r, x1i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _mul4r + _mul8r; _add9i = _mul4i + _mul8i;
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 17.0;
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow12r, &_pow12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c10r, _c10i, _pow12r, _pow12i, &_mul13r, &_mul13i);
    double _add14r = 0, _add14i = 0;
    _add14r = _add9r + _mul13r; _add14i = _add9i + _mul13i;
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 19.0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c15r, _c15i, x1r, x1i, &_mul16r, &_mul16i);
    double _add17r = 0, _add17i = 0;
    _add17r = _add14r + _mul16r; _add17i = _add14i + _mul16i;
    double _c18r = 0, _c18i = 0;
    _c18r = 0.0; _c18i = 23.0;
    double _add19r = 0, _add19i = 0;
    _add19r = _add17r + _c18r; _add19i = _add17i + _c18i;
    { int _idx = 0; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 100.0;
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, x1r, x1i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c20r, _c20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 100.0;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c24r, _c24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _mul23r + _mul27r; _add28i = _mul23i + _mul27i;
    double _c29r = 0, _c29i = 0;
    _c29r = 100.0; _c29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c29r, _c29i, x1r, x1i, &_mul30r, &_mul30i);
    double _sub31r = 0, _sub31i = 0;
    _sub31r = _add28r - _mul30r; _sub31i = _add28i - _mul30i;
    double _c32r = 0, _c32i = 0;
    _c32r = 100.0; _c32i = 0;
    double _sub33r = 0, _sub33i = 0;
    _sub33r = _sub31r - _c32r; _sub33i = _sub31i - _c32i;
    { int _idx = 1; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub33r; cIm[_idx] = _sub33i; } }
    double _c34r = 0, _c34i = 0;
    _c34r = 100.0; _c34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 3.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow36r, &_pow36i);
    c_mul(_pow36r, _pow36i, x2r, x2i, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c34r, _c34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 100.0;
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow40r, &_pow40i);
    double _mul41r = 0, _mul41i = 0;
    c_mul(_c38r, _c38i, _pow40r, _pow40i, &_mul41r, &_mul41i);
    double _add42r = 0, _add42i = 0;
    _add42r = _mul37r + _mul41r; _add42i = _mul37i + _mul41i;
    double _c43r = 0, _c43i = 0;
    _c43r = 0.0; _c43i = 100.0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c43r, _c43i, x2r, x2i, &_mul44r, &_mul44i);
    double _sub45r = 0, _sub45i = 0;
    _sub45r = _add42r - _mul44r; _sub45i = _add42i - _mul44i;
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 100.0;
    double _sub47r = 0, _sub47i = 0;
    _sub47r = _sub45r - _c46r; _sub47i = _sub45i - _c46i;
    { int _idx = 2; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub47r; cIm[_idx] = _sub47i; } }
    double _c48r = 0, _c48i = 0;
    _c48r = 0.0; _c48i = 100.0;
    double _c49r = 0, _c49i = 0;
    _c49r = 3.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow50r, &_pow50i);
    c_mul(_pow50r, _pow50i, x1r, x1i, &_pow50r, &_pow50i);
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c48r, _c48i, _pow50r, _pow50i, &_mul51r, &_mul51i);
    double _c52r = 0, _c52i = 0;
    _c52r = 100.0; _c52i = 0;
    double _c53r = 0, _c53i = 0;
    _c53r = 2.0; _c53i = 0;
    double _pow54r = 0, _pow54i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow54r, &_pow54i);
    double _mul55r = 0, _mul55i = 0;
    c_mul(_c52r, _c52i, _pow54r, _pow54i, &_mul55r, &_mul55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _mul51r + _mul55r; _add56i = _mul51i + _mul55i;
    double _c57r = 0, _c57i = 0;
    _c57r = 100.0; _c57i = 0;
    double _mul58r = 0, _mul58i = 0;
    c_mul(_c57r, _c57i, x1r, x1i, &_mul58r, &_mul58i);
    double _sub59r = 0, _sub59i = 0;
    _sub59r = _add56r - _mul58r; _sub59i = _add56i - _mul58i;
    double _c60r = 0, _c60i = 0;
    _c60r = 0.0; _c60i = 100.0;
    double _sub61r = 0, _sub61i = 0;
    _sub61r = _sub59r - _c60r; _sub61i = _sub59i - _c60i;
    { int _idx = 3; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub61r; cIm[_idx] = _sub61i; } }
    double _c62r = 0, _c62i = 0;
    _c62r = 3.0; _c62i = 0;
    double _neg63r = 0, _neg63i = 0;
    _neg63r = -(_c62r); _neg63i = -(_c62i);
    { int _idx = 4; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg63r; cIm[_idx] = _neg63i; } }
    double _c64r = 0, _c64i = 0;
    _c64r = 101.0; _c64i = 0;
    double _c65r = 0, _c65i = 0;
    _c65r = 3.0; _c65i = 0;
    double _pow66r = 0, _pow66i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow66r, &_pow66i);
    c_mul(_pow66r, _pow66i, x2r, x2i, &_pow66r, &_pow66i);
    double _mul67r = 0, _mul67i = 0;
    c_mul(_c64r, _c64i, _pow66r, _pow66i, &_mul67r, &_mul67i);
    double _c68r = 0, _c68i = 0;
    _c68r = 103.0; _c68i = 0;
    double _c69r = 0, _c69i = 0;
    _c69r = 2.0; _c69i = 0;
    double _pow70r = 0, _pow70i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow70r, &_pow70i);
    double _mul71r = 0, _mul71i = 0;
    c_mul(_c68r, _c68i, _pow70r, _pow70i, &_mul71r, &_mul71i);
    double _add72r = 0, _add72i = 0;
    _add72r = _mul67r + _mul71r; _add72i = _mul67i + _mul71i;
    double _c73r = 0, _c73i = 0;
    _c73r = 107.0; _c73i = 0;
    double _mul74r = 0, _mul74i = 0;
    c_mul(_c73r, _c73i, x2r, x2i, &_mul74r, &_mul74i);
    double _sub75r = 0, _sub75i = 0;
    _sub75r = _add72r - _mul74r; _sub75i = _add72i - _mul74i;
    double _c76r = 0, _c76i = 0;
    _c76r = 109.0; _c76i = 0;
    double _sub77r = 0, _sub77i = 0;
    _sub77r = _sub75r - _c76r; _sub77i = _sub75i - _c76i;
    { int _idx = 6; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub77r; cIm[_idx] = _sub77i; } }
    double _c78r = 0, _c78i = 0;
    _c78r = 113.0; _c78i = 0;
    double _c79r = 0, _c79i = 0;
    _c79r = 3.0; _c79i = 0;
    double _pow80r = 0, _pow80i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow80r, &_pow80i);
    c_mul(_pow80r, _pow80i, x1r, x1i, &_pow80r, &_pow80i);
    double _mul81r = 0, _mul81i = 0;
    c_mul(_c78r, _c78i, _pow80r, _pow80i, &_mul81r, &_mul81i);
    double _c82r = 0, _c82i = 0;
    _c82r = 127.0; _c82i = 0;
    double _c83r = 0, _c83i = 0;
    _c83r = 2.0; _c83i = 0;
    double _pow84r = 0, _pow84i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow84r, &_pow84i);
    double _mul85r = 0, _mul85i = 0;
    c_mul(_c82r, _c82i, _pow84r, _pow84i, &_mul85r, &_mul85i);
    double _add86r = 0, _add86i = 0;
    _add86r = _mul81r + _mul85r; _add86i = _mul81i + _mul85i;
    double _c87r = 0, _c87i = 0;
    _c87r = 131.0; _c87i = 0;
    double _mul88r = 0, _mul88i = 0;
    c_mul(_c87r, _c87i, x2r, x2i, &_mul88r, &_mul88i);
    double _add89r = 0, _add89i = 0;
    _add89r = _add86r + _mul88r; _add89i = _add86i + _mul88i;
    double _c90r = 0, _c90i = 0;
    _c90r = 137.0; _c90i = 0;
    double _sub91r = 0, _sub91i = 0;
    _sub91r = _add89r - _c90r; _sub91i = _add89i - _c90i;
    { int _idx = 8; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub91r; cIm[_idx] = _sub91i; } }
    double _c92r = 0, _c92i = 0;
    _c92r = 5.0; _c92i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c92r; cIm[_idx] = _c92i; } }
    double _c93r = 0, _c93i = 0;
    _c93r = 67.0; _c93i = 0;
    double _c94r = 0, _c94i = 0;
    _c94r = 3.0; _c94i = 0;
    double _pow95r = 0, _pow95i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow95r, &_pow95i);
    c_mul(_pow95r, _pow95i, x2r, x2i, &_pow95r, &_pow95i);
    double _mul96r = 0, _mul96i = 0;
    c_mul(_c93r, _c93i, _pow95r, _pow95i, &_mul96r, &_mul96i);
    double _c97r = 0, _c97i = 0;
    _c97r = 71.0; _c97i = 0;
    double _c98r = 0, _c98i = 0;
    _c98r = 2.0; _c98i = 0;
    double _pow99r = 0, _pow99i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow99r, &_pow99i);
    double _mul100r = 0, _mul100i = 0;
    c_mul(_c97r, _c97i, _pow99r, _pow99i, &_mul100r, &_mul100i);
    double _sub101r = 0, _sub101i = 0;
    _sub101r = _mul96r - _mul100r; _sub101i = _mul96i - _mul100i;
    double _c102r = 0, _c102i = 0;
    _c102r = 73.0; _c102i = 0;
    double _mul103r = 0, _mul103i = 0;
    c_mul(_c102r, _c102i, x2r, x2i, &_mul103r, &_mul103i);
    double _add104r = 0, _add104i = 0;
    _add104r = _sub101r + _mul103r; _add104i = _sub101i + _mul103i;
    double _c105r = 0, _c105i = 0;
    _c105r = 79.0; _c105i = 0;
    double _sub106r = 0, _sub106i = 0;
    _sub106r = _add104r - _c105r; _sub106i = _add104i - _c105i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub106r; cIm[_idx] = _sub106i; } }
    double _c107r = 0, _c107i = 0;
    _c107r = 7.0; _c107i = 0;
    double _neg108r = 0, _neg108i = 0;
    _neg108r = -(_c107r); _neg108i = -(_c107i);
    { int _idx = 16; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg108r; cIm[_idx] = _neg108i; } }
    double _c109r = 0, _c109i = 0;
    _c109r = 11.0; _c109i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _c109r; cIm[_idx] = _c109i; } }
    double _c110r = 0, _c110i = 0;
    _c110r = 13.0; _c110i = 0;
    double _neg111r = 0, _neg111i = 0;
    _neg111r = -(_c110r); _neg111i = -(_c110i);
    { int _idx = 24; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _neg111r; cIm[_idx] = _neg111i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_38: auto-stubbed (unhandled constructs in source) */
static void poly_giga_38_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_39_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 50;
    for (int _i = 0; _i < 50; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _c2r; cIm[_idx] = _c2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _neg4r = 0, _neg4i = 0;
    _neg4r = -(_c3r); _neg4i = -(_c3i);
    { int _idx = 19; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _neg4r; cIm[_idx] = _neg4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 4.0; _c5i = 0;
    { int _idx = 29; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _c5r; cIm[_idx] = _c5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 5.0; _c6i = 0;
    double _neg7r = 0, _neg7i = 0;
    _neg7r = -(_c6r); _neg7i = -(_c6i);
    { int _idx = 39; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _neg7r; cIm[_idx] = _neg7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 6.0; _c8i = 0;
    { int _idx = 49; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _c8r; cIm[_idx] = _c8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 100.0; _c9i = 0;
    double _c10r = 0, _c10i = 0;
    _c10r = 2.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow11r, &_pow11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 2.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow13r, &_pow13i);
    double _add14r = 0, _add14i = 0;
    _add14r = _pow11r + _pow13r; _add14i = _pow11i + _pow13i;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c9r, _c9i, _add14r, _add14i, &_mul15r, &_mul15i);
    { int _idx = 14; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 50.0; _c16i = 0;
    double _sin17r = 0, _sin17i = 0;
    c_sin(x1r, x1i, &_sin17r, &_sin17i);
    double _c18r = 0, _c18i = 0;
    _c18r = 0.0; _c18i = 1.0;
    double _cos19r = 0, _cos19i = 0;
    c_cos(x2r, x2i, &_cos19r, &_cos19i);
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c18r, _c18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _sin17r + _mul20r; _add21i = _sin17i + _mul20i;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c16r, _c16i, _add21r, _add21i, &_mul22r, &_mul22i);
    { int _idx = 24; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 200.0; _c23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul24r, &_mul24i);
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c23r, _c23i, _mul24r, _mul24i, &_mul25r, &_mul25i);
    double _c26r = 0, _c26i = 0;
    _c26r = 0.0; _c26i = 1.0;
    double _c27r = 0, _c27i = 0;
    _c27r = 3.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow28r, &_pow28i);
    c_mul(_pow28r, _pow28i, x1r, x1i, &_pow28r, &_pow28i);
    double _c29r = 0, _c29i = 0;
    _c29r = 3.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, x2r, x2i, &_pow30r, &_pow30i);
    double _sub31r = 0, _sub31i = 0;
    _sub31r = _pow28r - _pow30r; _sub31i = _pow28i - _pow30i;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c26r, _c26i, _sub31r, _sub31i, &_mul32r, &_mul32i);
    double _add33r = 0, _add33i = 0;
    _add33r = _mul25r + _mul32r; _add33i = _mul25i + _mul32i;
    { int _idx = 34; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    double _c34r = 0, _c34i = 0;
    _c34r = 0.0; _c34i = 1.0;
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c34r, _c34i, _add35r, _add35i, &_mul36r, &_mul36i);
    double _exp37r = 0, _exp37i = 0;
    c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _neg39r = 0, _neg39i = 0;
    _neg39r = -(_c38r); _neg39i = -(_c38i);
    double _sub40r = 0, _sub40i = 0;
    _sub40r = x1r - x2r; _sub40i = x1i - x2i;
    double _mul41r = 0, _mul41i = 0;
    c_mul(_neg39r, _neg39i, _sub40r, _sub40i, &_mul41r, &_mul41i);
    double _exp42r = 0, _exp42i = 0;
    c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _exp37r + _exp42r; _add43i = _exp37i + _exp42i;
    { int _idx = 44; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    for (int _i = 0; _i < 50; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_40_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    { int _idx = 6; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg3r; cIm[_idx] = _neg3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 3.0; _c4i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c4r; cIm[_idx] = _c4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 4.0; _c5i = 0;
    double _neg6r = 0, _neg6i = 0;
    _neg6r = -(_c5r); _neg6i = -(_c5i);
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg6r; cIm[_idx] = _neg6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 5.0; _c7i = 0;
    { int _idx = 26; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c7r; cIm[_idx] = _c7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 6.0; _c8i = 0;
    double _neg9r = 0, _neg9i = 0;
    _neg9r = -(_c8r); _neg9i = -(_c8i);
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg9r; cIm[_idx] = _neg9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 50.0;
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow12r, &_pow12i);
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow14r, &_pow14i);
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _pow12r - _pow14r; _sub15i = _pow12i - _pow14i;
    double _sin16r = 0, _sin16i = 0;
    c_sin(_sub15r, _sub15i, &_sin16r, &_sin16i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(_c10r, _c10i, _sin16r, _sin16i, &_mul17r, &_mul17i);
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul17r; cIm[_idx] = _mul17i; } }
    double _c18r = 0, _c18i = 0;
    _c18r = 100.0; _c18i = 0;
    double _cos19r = 0, _cos19i = 0;
    c_cos(x1r, x1i, &_cos19r, &_cos19i);
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 1.0;
    double _sin21r = 0, _sin21i = 0;
    c_sin(x2r, x2i, &_sin21r, &_sin21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c20r, _c20i, _sin21r, _sin21i, &_mul22r, &_mul22i);
    double _add23r = 0, _add23i = 0;
    _add23r = _cos19r + _mul22r; _add23i = _cos19i + _mul22i;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c18r, _c18i, _add23r, _add23i, &_mul24r, &_mul24i);
    { int _idx = 17; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    double _c25r = 0, _c25i = 0;
    _c25r = 50.0; _c25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 3.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow27r, &_pow27i);
    c_mul(_pow27r, _pow27i, x1r, x1i, &_pow27r, &_pow27i);
    double _c28r = 0, _c28i = 0;
    _c28r = 3.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow29r, &_pow29i);
    c_mul(_pow29r, _pow29i, x2r, x2i, &_pow29r, &_pow29i);
    double _sub30r = 0, _sub30i = 0;
    _sub30r = _pow27r - _pow29r; _sub30i = _pow27i - _pow29i;
    double _c31r = 0, _c31i = 0;
    _c31r = 0.0; _c31i = 1.0;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c31r, _c31i, x1r, x1i, &_mul32r, &_mul32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_mul32r, _mul32i, x2r, x2i, &_mul33r, &_mul33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _sub30r + _mul33r; _add34i = _sub30i + _mul33i;
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c25r, _c25i, _add34r, _add34i, &_mul35r, &_mul35i);
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    double _c36r = 0, _c36i = 0;
    _c36r = 200.0; _c36i = 0;
    double _c37r = 0, _c37i = 0;
    _c37r = 0.0; _c37i = 1.0;
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c37r, _c37i, x1r, x1i, &_mul38r, &_mul38i);
    double _exp39r = 0, _exp39i = 0;
    c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c36r, _c36i, _exp39r, _exp39i, &_mul40r, &_mul40i);
    double _c41r = 0, _c41i = 0;
    _c41r = 50.0; _c41i = 0;
    double _c42r = 0, _c42i = 0;
    _c42r = 0.0; _c42i = 1.0;
    double _neg43r = 0, _neg43i = 0;
    _neg43r = -(_c42r); _neg43i = -(_c42i);
    double _mul44r = 0, _mul44i = 0;
    c_mul(_neg43r, _neg43i, x2r, x2i, &_mul44r, &_mul44i);
    double _exp45r = 0, _exp45i = 0;
    c_exp2(_mul44r, _mul44i, &_exp45r, &_exp45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c41r, _c41i, _exp45r, _exp45i, &_mul46r, &_mul46i);
    double _add47r = 0, _add47i = 0;
    _add47r = _mul40r + _mul46r; _add47i = _mul40i + _mul46i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_41_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 60;
    for (int _i = 0; _i < 60; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 60) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 5.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    { int _idx = 9; if (_idx >= 0 && _idx < 60) { cRe[_idx] = _neg3r; cIm[_idx] = _neg3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 10.0; _c4i = 0;
    { int _idx = 29; if (_idx >= 0 && _idx < 60) { cRe[_idx] = _c4r; cIm[_idx] = _c4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 20.0; _c5i = 0;
    double _neg6r = 0, _neg6i = 0;
    _neg6r = -(_c5r); _neg6i = -(_c5i);
    { int _idx = 49; if (_idx >= 0 && _idx < 60) { cRe[_idx] = _neg6r; cIm[_idx] = _neg6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 100.0; _c7i = 0;
    double _add8r = 0, _add8i = 0;
    _add8r = x1r + x2r; _add8i = x1i + x2i;
    double _exp9r = 0, _exp9i = 0;
    c_exp2(_add8r, _add8i, &_exp9r, &_exp9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c7r, _c7i, _exp9r, _exp9i, &_mul10r, &_mul10i);
    { int _idx = 19; if (_idx >= 0 && _idx < 60) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 50.0; _c11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 2.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow13r, &_pow13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_pow13r, _pow13i, x2r, x2i, &_mul14r, &_mul14i);
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 1.0;
    double _c16r = 0, _c16i = 0;
    _c16r = 2.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow17r, &_pow17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c15r, _c15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _mul14r + _mul18r; _add19i = _mul14i + _mul18i;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c11r, _c11i, _add19r, _add19i, &_mul20r, &_mul20i);
    { int _idx = 39; if (_idx >= 0 && _idx < 60) { cRe[_idx] = _mul20r; cIm[_idx] = _mul20i; } }
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 1.0;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c21r, _c21i, x1r, x1i, &_mul22r, &_mul22i);
    double _exp23r = 0, _exp23i = 0;
    c_exp2(_mul22r, _mul22i, &_exp23r, &_exp23i);
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _neg25r = 0, _neg25i = 0;
    _neg25r = -(_c24r); _neg25i = -(_c24i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_neg25r, _neg25i, x2r, x2i, &_mul26r, &_mul26i);
    double _exp27r = 0, _exp27i = 0;
    c_exp2(_mul26r, _mul26i, &_exp27r, &_exp27i);
    double _mul28r = 0, _mul28i = 0;
    c_mul(_exp23r, _exp23i, _exp27r, _exp27i, &_mul28r, &_mul28i);
    double _c29r = 0, _c29i = 0;
    _c29r = 50.0; _c29i = 0;
    double _c30r = 0, _c30i = 0;
    _c30r = 3.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow31r, &_pow31i);
    c_mul(_pow31r, _pow31i, x1r, x1i, &_pow31r, &_pow31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c29r, _c29i, _pow31r, _pow31i, &_mul32r, &_mul32i);
    double _add33r = 0, _add33i = 0;
    _add33r = _mul28r + _mul32r; _add33i = _mul28i + _mul32i;
    { int _idx = 54; if (_idx >= 0 && _idx < 60) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    double _c34r = 0, _c34i = 0;
    _c34r = 300.0; _c34i = 0;
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _sin36r = 0, _sin36i = 0;
    c_sin(_add35r, _add35i, &_sin36r, &_sin36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c34r, _c34i, _sin36r, _sin36i, &_mul37r, &_mul37i);
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _sub39r = 0, _sub39i = 0;
    _sub39r = x1r - x2r; _sub39i = x1i - x2i;
    double _cos40r = 0, _cos40i = 0;
    c_cos(_sub39r, _sub39i, &_cos40r, &_cos40i);
    double _mul41r = 0, _mul41i = 0;
    c_mul(_c38r, _c38i, _cos40r, _cos40i, &_mul41r, &_mul41i);
    double _add42r = 0, _add42i = 0;
    _add42r = _mul37r + _mul41r; _add42i = _mul37i + _mul41i;
    { int _idx = 59; if (_idx >= 0 && _idx < 60) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    for (int _i = 0; _i < 60; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_42_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 50;
    for (int _i = 0; _i < 50; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    { int _idx = 7; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _neg3r; cIm[_idx] = _neg3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 3.0; _c4i = 0;
    { int _idx = 15; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _c4r; cIm[_idx] = _c4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _neg6r = 0, _neg6i = 0;
    _neg6r = -(_c5r); _neg6i = -(_c5i);
    { int _idx = 31; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _neg6r; cIm[_idx] = _neg6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    { int _idx = 39; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _c7r; cIm[_idx] = _c7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 100.0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow12r, &_pow12i);
    double _add13r = 0, _add13i = 0;
    _add13r = _pow10r + _pow12r; _add13i = _pow10i + _pow12i;
    double _exp14r = 0, _exp14i = 0;
    c_exp2(_add13r, _add13i, &_exp14r, &_exp14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c8r, _c8i, _exp14r, _exp14i, &_mul15r, &_mul15i);
    { int _idx = 11; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 50.0; _c16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, x1r, x1i, &_pow18r, &_pow18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 3.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
    c_mul(_pow20r, _pow20i, x2r, x2i, &_pow20r, &_pow20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _pow18r + _pow20r; _add21i = _pow18i + _pow20i;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c16r, _c16i, _add21r, _add21i, &_mul22r, &_mul22i);
    { int _idx = 19; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _sub24r = 0, _sub24i = 0;
    _sub24r = x1r - x2r; _sub24i = x1i - x2i;
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c23r, _c23i, _sub24r, _sub24i, &_mul25r, &_mul25i);
    double _exp26r = 0, _exp26i = 0;
    c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
    double _c27r = 0, _c27i = 0;
    _c27r = 10.0; _c27i = 0;
    double _c28r = 0, _c28i = 0;
    _c28r = 2.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow29r, &_pow29i);
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c27r, _c27i, _pow29r, _pow29i, &_mul30r, &_mul30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _exp26r + _mul30r; _add31i = _exp26i + _mul30i;
    { int _idx = 24; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    double _c32r = 0, _c32i = 0;
    _c32r = 200.0; _c32i = 0;
    double _add33r = 0, _add33i = 0;
    _add33r = x1r + x2r; _add33i = x1i + x2i;
    double _sin34r = 0, _sin34i = 0;
    c_sin(_add33r, _add33i, &_sin34r, &_sin34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c32r, _c32i, _sin34r, _sin34i, &_mul35r, &_mul35i);
    double _c36r = 0, _c36i = 0;
    _c36r = 0.0; _c36i = 1.0;
    double _sub37r = 0, _sub37i = 0;
    _sub37r = x1r - x2r; _sub37i = x1i - x2i;
    double _cos38r = 0, _cos38i = 0;
    c_cos(_sub37r, _sub37i, &_cos38r, &_cos38i);
    double _mul39r = 0, _mul39i = 0;
    c_mul(_c36r, _c36i, _cos38r, _cos38i, &_mul39r, &_mul39i);
    double _add40r = 0, _add40i = 0;
    _add40r = _mul35r + _mul39r; _add40i = _mul35i + _mul39i;
    { int _idx = 44; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    for (int _i = 0; _i < 50; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_43_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 5.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    { int _idx = 4; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _neg3r; cIm[_idx] = _neg3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 10.0; _c4i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _c4r; cIm[_idx] = _c4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 20.0; _c5i = 0;
    double _neg6r = 0, _neg6i = 0;
    _neg6r = -(_c5r); _neg6i = -(_c5i);
    { int _idx = 29; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _neg6r; cIm[_idx] = _neg6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 0.0; _c7i = 100.0;
    double _c8r = 0, _c8i = 0;
    _c8r = 3.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow9r, &_pow9i);
    c_mul(_pow9r, _pow9i, x1r, x1i, &_pow9r, &_pow9i);
    double _c10r = 0, _c10i = 0;
    _c10r = 3.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow11r, &_pow11i);
    c_mul(_pow11r, _pow11i, x2r, x2i, &_pow11r, &_pow11i);
    double _sub12r = 0, _sub12i = 0;
    _sub12r = _pow9r - _pow11r; _sub12i = _pow9i - _pow11i;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c7r, _c7i, _sub12r, _sub12i, &_mul13r, &_mul13i);
    { int _idx = 19; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 50.0; _c14i = 0;
    double _c15r = 0, _c15i = 0;
    _c15r = 2.0; _c15i = 0;
    double _pow16r = 0, _pow16i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow16r, &_pow16i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(_pow16r, _pow16i, x2r, x2i, &_mul17r, &_mul17i);
    double _c18r = 0, _c18i = 0;
    _c18r = 0.0; _c18i = 1.0;
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c18r, _c18i, _pow20r, _pow20i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _mul17r + _mul21r; _add22i = _mul17i + _mul21i;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c14r, _c14i, _add22r, _add22i, &_mul23r, &_mul23i);
    { int _idx = 9; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c24r, _c24i, x1r, x1i, &_mul25r, &_mul25i);
    double _exp26r = 0, _exp26i = 0;
    c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
    double _c27r = 0, _c27i = 0;
    _c27r = 0.0; _c27i = 1.0;
    double _neg28r = 0, _neg28i = 0;
    _neg28r = -(_c27r); _neg28i = -(_c27i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_neg28r, _neg28i, x2r, x2i, &_mul29r, &_mul29i);
    double _exp30r = 0, _exp30i = 0;
    c_exp2(_mul29r, _mul29i, &_exp30r, &_exp30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _exp26r + _exp30r; _add31i = _exp26i + _exp30i;
    { int _idx = 24; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    double _c32r = 0, _c32i = 0;
    _c32r = 200.0; _c32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c32r, _c32i, x1r, x1i, &_mul33r, &_mul33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_mul33r, _mul33i, x2r, x2i, &_mul34r, &_mul34i);
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _sin36r = 0, _sin36i = 0;
    c_sin(_add35r, _add35i, &_sin36r, &_sin36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_mul34r, _mul34i, _sin36r, _sin36i, &_mul37r, &_mul37i);
    { int _idx = 34; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    for (int _i = 0; _i < 40; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_44_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 30;
    for (int _i = 0; _i < 30; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 30) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 30) { cRe[_idx] = _c2r; cIm[_idx] = _c2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _neg4r = 0, _neg4i = 0;
    _neg4r = -(_c3r); _neg4i = -(_c3i);
    { int _idx = 11; if (_idx >= 0 && _idx < 30) { cRe[_idx] = _neg4r; cIm[_idx] = _neg4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 5.0; _c5i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 30) { cRe[_idx] = _c5r; cIm[_idx] = _c5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 100.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 3.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow8r, &_pow8i);
    c_mul(_pow8r, _pow8i, x1r, x1i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    double _c10r = 0, _c10i = 0;
    _c10r = 50.0; _c10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow12r, &_pow12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c10r, _c10i, _pow12r, _pow12i, &_mul13r, &_mul13i);
    double _add14r = 0, _add14i = 0;
    _add14r = _mul9r + _mul13r; _add14i = _mul9i + _mul13i;
    { int _idx = 9; if (_idx >= 0 && _idx < 30) { cRe[_idx] = _add14r; cIm[_idx] = _add14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 50.0;
    double _attr16r = 0, _attr16i = 0;
    _attr16r = x1r; _attr16i = 0;
    double _attr17r = 0, _attr17i = 0;
    _attr17r = x2i; _attr17i = 0;
    double _sub18r = 0, _sub18i = 0;
    _sub18r = _attr16r - _attr17r; _sub18i = _attr16i - _attr17i;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c15r, _c15i, _sub18r, _sub18i, &_mul19r, &_mul19i);
    { int _idx = 14; if (_idx >= 0 && _idx < 30) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 200.0; _c20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c20r, _c20i, x1r, x1i, &_mul21r, &_mul21i);
    double _c22r = 0, _c22i = 0;
    _c22r = 1.0; _c22i = 0;
    double _add23r = 0, _add23i = 0;
    _add23r = x2r + _c22r; _add23i = x2i + _c22i;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_mul21r, _mul21i, _add23r, _add23i, &_mul24r, &_mul24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 0.0; _c25i = 100.0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c25r, _c25i, x2r, x2i, &_mul26r, &_mul26i);
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _mul24r - _mul26r; _sub27i = _mul24i - _mul26i;
    { int _idx = 24; if (_idx >= 0 && _idx < 30) { cRe[_idx] = _sub27r; cIm[_idx] = _sub27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 0.0; _c28i = 1.0;
    double _mul29r = 0, _mul29i = 0;
    c_mul(_c28r, _c28i, x1r, x1i, &_mul29r, &_mul29i);
    double _exp30r = 0, _exp30i = 0;
    c_exp2(_mul29r, _mul29i, &_exp30r, &_exp30i);
    double _c31r = 0, _c31i = 0;
    _c31r = 3.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow32r, &_pow32i);
    c_mul(_pow32r, _pow32i, x2r, x2i, &_pow32r, &_pow32i);
    double _add33r = 0, _add33i = 0;
    _add33r = _exp30r + _pow32r; _add33i = _exp30i + _pow32i;
    { int _idx = 29; if (_idx >= 0 && _idx < 30) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    for (int _i = 0; _i < 30; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_45_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 50;
    for (int _i = 0; _i < 50; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 50.0; _c2i = 0;
    double _exp3r = 0, _exp3i = 0;
    c_exp2(x1r, x1i, &_exp3r, &_exp3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c2r, _c2i, _exp3r, _exp3i, &_mul4r, &_mul4i);
    { int _idx = 4; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 100.0; _c5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow7r, &_pow7i);
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 1.0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c8r, _c8i, x1r, x1i, &_mul9r, &_mul9i);
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _pow7r - _mul9r; _sub10i = _pow7i - _mul9i;
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c5r, _c5i, _sub10r, _sub10i, &_mul11r, &_mul11i);
    { int _idx = 9; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul11r; cIm[_idx] = _mul11i; } }
    double _c12r = 0, _c12i = 0;
    _c12r = 200.0; _c12i = 0;
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
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c12r, _c12i, _exp17r, _exp17i, &_mul18r, &_mul18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 50.0; _c19i = 0;
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 1.0;
    double _neg21r = 0, _neg21i = 0;
    _neg21r = -(_c20r); _neg21i = -(_c20i);
    double _c22r = 0, _c22i = 0;
    _c22r = 3.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow23r, &_pow23i);
    c_mul(_pow23r, _pow23i, x2r, x2i, &_pow23r, &_pow23i);
    double _mul24r = 0, _mul24i = 0;
    c_mul(_neg21r, _neg21i, _pow23r, _pow23i, &_mul24r, &_mul24i);
    double _exp25r = 0, _exp25i = 0;
    c_exp2(_mul24r, _mul24i, &_exp25r, &_exp25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c19r, _c19i, _exp25r, _exp25i, &_mul26r, &_mul26i);
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _mul18r - _mul26r; _sub27i = _mul18i - _mul26i;
    { int _idx = 19; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _sub27r; cIm[_idx] = _sub27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 100.0; _c28i = 0;
    double _mul29r = 0, _mul29i = 0;
    c_mul(_c28r, _c28i, x1r, x1i, &_mul29r, &_mul29i);
    double _c30r = 0, _c30i = 0;
    _c30r = 2.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow31r, &_pow31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_mul29r, _mul29i, _pow31r, _pow31i, &_mul32r, &_mul32i);
    double _c33r = 0, _c33i = 0;
    _c33r = 0.0; _c33i = 50.0;
    double _c34r = 0, _c34i = 0;
    _c34r = 3.0; _c34i = 0;
    double _pow35r = 0, _pow35i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow35r, &_pow35i);
    c_mul(_pow35r, _pow35i, x1r, x1i, &_pow35r, &_pow35i);
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c33r, _c33i, _pow35r, _pow35i, &_mul36r, &_mul36i);
    double _add37r = 0, _add37i = 0;
    _add37r = _mul32r + _mul36r; _add37i = _mul32i + _mul36i;
    { int _idx = 29; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _add39r = 0, _add39i = 0;
    _add39r = x1r + x2r; _add39i = x1i + x2i;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c38r, _c38i, _add39r, _add39i, &_mul40r, &_mul40i);
    double _exp41r = 0, _exp41i = 0;
    c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
    double _c42r = 0, _c42i = 0;
    _c42r = 50.0; _c42i = 0;
    double _sub43r = 0, _sub43i = 0;
    _sub43r = x1r - x2r; _sub43i = x1i - x2i;
    double _attr44r = 0, _attr44i = 0;
    _attr44r = _sub43i; _attr44i = 0;
    double _sin45r = 0, _sin45i = 0;
    c_sin(_attr44r, _attr44i, &_sin45r, &_sin45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c42r, _c42i, _sin45r, _sin45i, &_mul46r, &_mul46i);
    double _sub47r = 0, _sub47i = 0;
    _sub47r = _exp41r - _mul46r; _sub47i = _exp41i - _mul46i;
    { int _idx = 39; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _sub47r; cIm[_idx] = _sub47i; } }
    for (int _i = 0; _i < 50; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_46_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    { int _idx = 7; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _neg3r; cIm[_idx] = _neg3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 5.0; _c4i = 0;
    { int _idx = 15; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _c4r; cIm[_idx] = _c4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 7.0; _c5i = 0;
    double _neg6r = 0, _neg6i = 0;
    _neg6r = -(_c5r); _neg6i = -(_c5i);
    { int _idx = 23; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _neg6r; cIm[_idx] = _neg6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    { int _idx = 31; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _c7r; cIm[_idx] = _c7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 50.0; _c8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 3.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, x2r, x2i, &_pow12r, &_pow12i);
    double _sub13r = 0, _sub13i = 0;
    _sub13r = _pow10r - _pow12r; _sub13i = _pow10i - _pow12i;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c8r, _c8i, _sub13r, _sub13i, &_mul14r, &_mul14i);
    { int _idx = 4; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 100.0;
    double _c16r = 0, _c16i = 0;
    _c16r = 3.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow17r, &_pow17i);
    c_mul(_pow17r, _pow17i, x1r, x1i, &_pow17r, &_pow17i);
    double _add18r = 0, _add18i = 0;
    _add18r = _pow17r + x2r; _add18i = _pow17i + x2i;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c15r, _c15i, _add18r, _add18i, &_mul19r, &_mul19i);
    { int _idx = 11; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 1.0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c20r, _c20i, x1r, x1i, &_mul21r, &_mul21i);
    double _exp22r = 0, _exp22i = 0;
    c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _neg24r = 0, _neg24i = 0;
    _neg24r = -(_c23r); _neg24i = -(_c23i);
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_neg24r, _neg24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    double _exp28r = 0, _exp28i = 0;
    c_exp2(_mul27r, _mul27i, &_exp28r, &_exp28i);
    double _add29r = 0, _add29i = 0;
    _add29r = _exp22r + _exp28r; _add29i = _exp22i + _exp28i;
    { int _idx = 19; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 200.0; _c30i = 0;
    double _attr31r = 0, _attr31i = 0;
    _attr31r = x1r; _attr31i = 0;
    double _attr32r = 0, _attr32i = 0;
    _attr32r = x2i; _attr32i = 0;
    double _add33r = 0, _add33i = 0;
    _add33r = _attr31r + _attr32r; _add33i = _attr31i + _attr32i;
    double _sin34r = 0, _sin34i = 0;
    c_sin(_add33r, _add33i, &_sin34r, &_sin34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c30r, _c30i, _sin34r, _sin34i, &_mul35r, &_mul35i);
    double _c36r = 0, _c36i = 0;
    _c36r = 50.0; _c36i = 0;
    double _sub37r = 0, _sub37i = 0;
    _sub37r = x1r - x2r; _sub37i = x1i - x2i;
    double _attr38r = 0, _attr38i = 0;
    _attr38r = _sub37i; _attr38i = 0;
    double _cos39r = 0, _cos39i = 0;
    c_cos(_attr38r, _attr38i, &_cos39r, &_cos39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c36r, _c36i, _cos39r, _cos39i, &_mul40r, &_mul40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _mul35r - _mul40r; _sub41i = _mul35i - _mul40i;
    { int _idx = 29; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _sub41r; cIm[_idx] = _sub41i; } }
    double _c42r = 0, _c42i = 0;
    _c42r = 0.0; _c42i = 1.0;
    double _c43r = 0, _c43i = 0;
    _c43r = 3.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow44r, &_pow44i);
    c_mul(_pow44r, _pow44i, x1r, x1i, &_pow44r, &_pow44i);
    double _mul45r = 0, _mul45i = 0;
    c_mul(_c42r, _c42i, _pow44r, _pow44i, &_mul45r, &_mul45i);
    double _exp46r = 0, _exp46i = 0;
    c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
    double _c47r = 0, _c47i = 0;
    _c47r = 2.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow48r, &_pow48i);
    double _add49r = 0, _add49i = 0;
    _add49r = _exp46r + _pow48r; _add49i = _exp46i + _pow48i;
    { int _idx = 34; if (_idx >= 0 && _idx < 40) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
    for (int _i = 0; _i < 40; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_47: auto-stubbed (unhandled constructs in source) */
static void poly_giga_47_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 30;
    for (int _i = 0; _i < 30; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_48: auto-stubbed (unhandled constructs in source) */
static void poly_giga_48_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_49: auto-stubbed (unhandled constructs in source) */
static void poly_giga_49_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 30;
    for (int _i = 0; _i < 30; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_50: auto-stubbed (unhandled constructs in source) */
static void poly_giga_50_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 1000;
    for (int _i = 0; _i < 1000; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_51: too complex for auto-transpile, stubbed */
static void poly_giga_51_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 1000;
    for (int _i = 0; _i < 1000; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_52: auto-stubbed (unhandled constructs in source) */
static void poly_giga_52_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 100;
    for (int _i = 0; _i < 100; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_53_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
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
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub21r; cIm[_idx] = _sub21i; } }
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
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _div30r; cIm[_idx] = _div30i; } }
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
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _pow42r; cIm[_idx] = _pow42i; } }
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
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
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
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul62r; cIm[_idx] = _mul62i; } }
    double _attr63r = 0, _attr63i = 0;
    _attr63r = x1i; _attr63i = 0;
    double _c64r = 0, _c64i = 0;
    _c64r = 0.0; _c64i = 0;
    double _abs65r = 0, _abs65i = 0;
    _abs65r = c_abs(x2r, x2i); _abs65i = 0;
    double _sub66r = 0, _sub66i = 0;
    _sub66r = x1r - _abs65r; _sub66i = x1i - _abs65i;
    double _abs67r = 0, _abs67i = 0;
    _abs67r = c_abs(x1r, x1i); _abs67i = 0;
    double _sub68r = 0, _sub68i = 0;
    _sub68r = x2r - _abs67r; _sub68i = x2i - _abs67i;
    double _tern69r = 0, _tern69i = 0;
    if (_attr63r > _c64r) { _tern69r = _sub66r; _tern69i = _sub66i; }
    else { _tern69r = _sub68r; _tern69i = _sub68i; }
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _tern69r; cIm[_idx] = _tern69i; } }
    double _c70r = 0, _c70i = 0;
    _c70r = 0.0; _c70i = 1.0;
    double _mul71r = 0, _mul71i = 0;
    c_mul(_c70r, _c70i, x1r, x1i, &_mul71r, &_mul71i);
    double _mul72r = 0, _mul72i = 0;
    c_mul(_mul71r, _mul71i, x2r, x2i, &_mul72r, &_mul72i);
    double _c73r = 0, _c73i = 0;
    _c73r = 0.1; _c73i = 0;
    double _mul74r = 0, _mul74i = 0;
    c_mul(_c73r, _c73i, x1r, x1i, &_mul74r, &_mul74i);
    double _mul75r = 0, _mul75i = 0;
    c_mul(_mul74r, _mul74i, x2r, x2i, &_mul75r, &_mul75i);
    double _pow76r = 0, _pow76i = 0;
    c_powc(_mul72r, _mul72i, _mul75r, _mul75i, &_pow76r, &_pow76i);
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _pow76r; cIm[_idx] = _pow76i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_54_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 1.0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_c2r, _c2i, x2r, x2i, &_mul3r, &_mul3i);
    double _exp4r = 0, _exp4i = 0;
    c_exp2(_mul3r, _mul3i, &_exp4r, &_exp4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_attr1r, _attr1i, _exp4r, _exp4i, &_mul5r, &_mul5i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
    double _mul6r = 0, _mul6i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
    double _attr7r = 0, _attr7i = 0;
    _attr7r = _mul6i; _attr7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 1.0;
    double _neg9r = 0, _neg9i = 0;
    _neg9r = -(_c8r); _neg9i = -(_c8i);
    double _attr10r = 0, _attr10i = 0;
    _attr10r = x2r; _attr10i = 0;
    double _mul11r = 0, _mul11i = 0;
    c_mul(_neg9r, _neg9i, _attr10r, _attr10i, &_mul11r, &_mul11i);
    double _exp12r = 0, _exp12i = 0;
    c_exp2(_mul11r, _mul11i, &_exp12r, &_exp12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_attr7r, _attr7i, _exp12r, _exp12i, &_mul13r, &_mul13i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    double _attr14r = 0, _attr14i = 0;
    _attr14r = x1r; _attr14i = 0;
    double _attr15r = 0, _attr15i = 0;
    _attr15r = x2i; _attr15i = 0;
    double _add16r = 0, _add16i = 0;
    _add16r = _attr14r + _attr15r; _add16i = _attr14i + _attr15i;
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(_add16r, _add16i, _add16r, _add16i, &_pow18r, &_pow18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 0.0; _c19i = 10.0;
    double _add20r = 0, _add20i = 0;
    _add20r = _pow18r + _c19r; _add20i = _pow18i + _c19i;
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    double _attr21r = 0, _attr21i = 0;
    _attr21r = x2i; _attr21i = 0;
    double _c22r = 0, _c22i = 0;
    _c22r = 3.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(_attr21r, _attr21i, _attr21r, _attr21i, &_pow23r, &_pow23i);
    c_mul(_pow23r, _pow23i, _attr21r, _attr21i, &_pow23r, &_pow23i);
    double _attr24r = 0, _attr24i = 0;
    _attr24r = x1r; _attr24i = 0;
    double _div25r = 0, _div25i = 0;
    c_div(_pow23r, _pow23i, _attr24r, _attr24i, &_div25r, &_div25i);
    double _c26r = 0, _c26i = 0;
    _c26r = 0.0; _c26i = 1.0;
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _div25r - _c26r; _sub27i = _div25i - _c26i;
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub27r; cIm[_idx] = _sub27i; } }
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _attr29r = 0, _attr29i = 0;
    _attr29r = _mul28r; _attr29i = 0;
    double _c30r = 0, _c30i = 0;
    _c30r = 0.0; _c30i = 1.0;
    double _add31r = 0, _add31i = 0;
    _add31r = x1r + x2r; _add31i = x1i + x2i;
    double _attr32r = 0, _attr32i = 0;
    _attr32r = _add31i; _attr32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_attr32r, _attr32i, _attr32r, _attr32i, &_pow34r, &_pow34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c30r, _c30i, _pow34r, _pow34i, &_mul35r, &_mul35i);
    double _exp36r = 0, _exp36i = 0;
    c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_attr29r, _attr29i, _exp36r, _exp36i, &_mul37r, &_mul37i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    double _sum38r = 0, _sum38i = 0;
    _sum38r = 0; _sum38i = 0;
    for (int _si = 0; _si < 9; _si++) { _sum38r += cRe[_si]; _sum38i += cIm[_si]; }
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sum38r; cIm[_idx] = _sum38i; } }
    double _prod39r = 0, _prod39i = 0;
    _prod39r = 1; _prod39i = 0;
    for (int _pi = 0; _pi < 10; _pi++) { double _pr = _prod39r*cRe[_pi]-_prod39i*cIm[_pi]; double _pp = _prod39r*cIm[_pi]+_prod39i*cRe[_pi]; _prod39r=_pr; _prod39i=_pp; }
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _prod39r; cIm[_idx] = _prod39i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_55_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, x1r, x1i, &_mul2r, &_mul2i);
    double _exp3r = 0, _exp3i = 0;
    c_exp2(_mul2r, _mul2i, &_exp3r, &_exp3i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _exp3r; cIm[_idx] = _exp3i; } }
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
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add10r; cIm[_idx] = _add10i; } }
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
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub23r; cIm[_idx] = _sub23i; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c24r, _c24i, x2r, x2i, &_mul25r, &_mul25i);
    double _add26r = 0, _add26i = 0;
    _add26r = x1r + _mul25r; _add26i = x1i + _mul25i;
    double _log27r = 0, _log27i = 0;
    c_log(_add26r, _add26i, &_log27r, &_log27i);
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _log27r; cIm[_idx] = _log27i; } }
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
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
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
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub39r; cIm[_idx] = _sub39i; } }
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
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
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
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _sub53r; cIm[_idx] = _sub53i; } }
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
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul58r; cIm[_idx] = _mul58i; } }
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
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul68r; cIm[_idx] = _mul68i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_56_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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

static void poly_giga_57_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(_add1r, _add1i); _abs2i = 0;
    double _log3r = 0, _log3i = 0;
    c_log(_abs2r, _abs2i, &_log3r, &_log3i);
    double _sub4r = 0, _sub4i = 0;
    _sub4r = x1r - x2r; _sub4i = x1i - x2i;
    double _sin5r = 0, _sin5i = 0;
    c_sin(_sub4r, _sub4i, &_sin5r, &_sin5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_log3r, _log3i, _sin5r, _sin5i, &_mul6r, &_mul6i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul6r; cIm[_idx] = _mul6i; } }
    double _attr7r = 0, _attr7i = 0;
    _attr7r = x1r; _attr7i = 0;
    double _exp8r = 0, _exp8i = 0;
    c_exp2(_attr7r, _attr7i, &_exp8r, &_exp8i);
    double _attr9r = 0, _attr9i = 0;
    _attr9r = x2i; _attr9i = 0;
    double _exp10r = 0, _exp10i = 0;
    c_exp2(_attr9r, _attr9i, &_exp10r, &_exp10i);
    double _add11r = 0, _add11i = 0;
    _add11r = _exp8r + _exp10r; _add11i = _exp8i + _exp10i;
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add11r; cIm[_idx] = _add11i; } }
    double _abs12r = 0, _abs12i = 0;
    _abs12r = c_abs(x1r, x1i); _abs12i = 0;
    double _sqrt13r = 0, _sqrt13i = 0;
    c_powr(_abs12r, _abs12i, 0.5, &_sqrt13r, &_sqrt13i);
    double _cos14r = 0, _cos14i = 0;
    c_cos(x2r, x2i, &_cos14r, &_cos14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_sqrt13r, _sqrt13i, _cos14r, _cos14i, &_mul15r, &_mul15i);
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _add16r = 0, _add16i = 0;
    _add16r = x1r + x2r; _add16i = x1i + x2i;
    double _attr17r = 0, _attr17i = 0;
    _attr17r = _add16i; _attr17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 3.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(_attr17r, _attr17i, _attr17r, _attr17i, &_pow19r, &_pow19i);
    c_mul(_pow19r, _pow19i, _attr17r, _attr17i, &_pow19r, &_pow19i);
    double _sin20r = 0, _sin20i = 0;
    c_sin(_pow19r, _pow19i, &_sin20r, &_sin20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = x1r - x2r; _sub21i = x1i - x2i;
    double _attr22r = 0, _attr22i = 0;
    _attr22r = _sub21r; _attr22i = 0;
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(_attr22r, _attr22i, _attr22r, _attr22i, &_pow24r, &_pow24i);
    double _exp25r = 0, _exp25i = 0;
    c_exp2(_pow24r, _pow24i, &_exp25r, &_exp25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_sin20r, _sin20i, _exp25r, _exp25i, &_mul26r, &_mul26i);
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    double _mul27r = 0, _mul27i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul27r, &_mul27i);
    double _attr28r = 0, _attr28i = 0;
    _attr28r = _mul27i; _attr28i = 0;
    double _tsin29r = 0, _tsin29i = 0;
    double _tcos30r = 0, _tcos30i = 0;
    double _tan31r = 0, _tan31i = 0;
    c_sin(_attr28r, _attr28i, &_tsin29r, &_tsin29i);
    c_cos(_attr28r, _attr28i, &_tcos30r, &_tcos30i);
    c_div(_tsin29r, _tsin29i, _tcos30r, _tcos30i, &_tan31r, &_tan31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul32r, &_mul32i);
    double _attr33r = 0, _attr33i = 0;
    _attr33r = _mul32r; _attr33i = 0;
    double _cosh34r = 0, _cosh34i = 0;
    _cosh34r = cosh(_attr33r); _cosh34i = 0; /* approx real cosh */
    double _mul35r = 0, _mul35i = 0;
    c_mul(_tan31r, _tan31i, _cosh34r, _cosh34i, &_mul35r, &_mul35i);
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    double _sub36r = 0, _sub36i = 0;
    _sub36r = x1r - x2r; _sub36i = x1i - x2i;
    double _abs37r = 0, _abs37i = 0;
    _abs37r = c_abs(_sub36r, _sub36i); _abs37i = 0;
    double _add38r = 0, _add38i = 0;
    _add38r = x1r + x2r; _add38i = x1i + x2i;
    double _ang39r = 0, _ang39i = 0;
    _ang39r = c_arg(_add38r, _add38i); _ang39i = 0;
    double _sinh40r = 0, _sinh40i = 0;
    _sinh40r = sinh(_ang39r); _sinh40i = 0; /* approx real sinh */
    double _mul41r = 0, _mul41i = 0;
    c_mul(_abs37r, _abs37i, _sinh40r, _sinh40i, &_mul41r, &_mul41i);
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    double _c42r = 0, _c42i = 0;
    _c42r = 3.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow43r, &_pow43i);
    c_mul(_pow43r, _pow43i, x1r, x1i, &_pow43r, &_pow43i);
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow45r, &_pow45i);
    double _sub46r = 0, _sub46i = 0;
    _sub46r = _pow43r - _pow45r; _sub46i = _pow43i - _pow45i;
    double _attr47r = 0, _attr47i = 0;
    _attr47r = _sub46i; _attr47i = 0;
    double _mul48r = 0, _mul48i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul48r, &_mul48i);
    double _attr49r = 0, _attr49i = 0;
    _attr49r = _mul48r; _attr49i = 0;
    double _tsin50r = 0, _tsin50i = 0;
    double _tcos51r = 0, _tcos51i = 0;
    double _tan52r = 0, _tan52i = 0;
    c_sin(_attr49r, _attr49i, &_tsin50r, &_tsin50i);
    c_cos(_attr49r, _attr49i, &_tcos51r, &_tcos51i);
    c_div(_tsin50r, _tsin50i, _tcos51r, _tcos51i, &_tan52r, &_tan52i);
    double _mul53r = 0, _mul53i = 0;
    c_mul(_attr47r, _attr47i, _tan52r, _tan52i, &_mul53r, &_mul53i);
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(x1r, x1i); _abs54i = 0;
    double _abs55r = 0, _abs55i = 0;
    _abs55r = c_abs(x2r, x2i); _abs55i = 0;
    double _mul56r = 0, _mul56i = 0;
    c_mul(_abs54r, _abs54i, _abs55r, _abs55i, &_mul56r, &_mul56i);
    double _tanh57r = 0, _tanh57i = 0;
    _tanh57r = tanh(_mul56r); _tanh57i = 0; /* approx real tanh */
    double _div58r = 0, _div58i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div58r, &_div58i);
    double _ang59r = 0, _ang59i = 0;
    _ang59r = c_arg(_div58r, _div58i); _ang59i = 0;
    double _sin60r = 0, _sin60i = 0;
    c_sin(_ang59r, _ang59i, &_sin60r, &_sin60i);
    double _mul61r = 0, _mul61i = 0;
    c_mul(_tanh57r, _tanh57i, _sin60r, _sin60i, &_mul61r, &_mul61i);
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul61r; cIm[_idx] = _mul61i; } }
    double _sub62r = 0, _sub62i = 0;
    _sub62r = x1r - x2r; _sub62i = x1i - x2i;
    double _attr63r = 0, _attr63i = 0;
    _attr63r = _sub62r; _attr63i = 0;
    double _sgn64r = 0, _sgn64i = 0;
    _sgn64r = (_attr63r > 0) ? 1.0 : (_attr63r < 0) ? -1.0 : 0.0; _sgn64i = 0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul65r, &_mul65i);
    double _ang66r = 0, _ang66i = 0;
    _ang66r = c_arg(_mul65r, _mul65i); _ang66i = 0;
    double _cosh67r = 0, _cosh67i = 0;
    _cosh67r = cosh(_ang66r); _cosh67i = 0; /* approx real cosh */
    double _mul68r = 0, _mul68i = 0;
    c_mul(_sgn64r, _sgn64i, _cosh67r, _cosh67i, &_mul68r, &_mul68i);
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul68r; cIm[_idx] = _mul68i; } }
    double _c69r = 0, _c69i = 0;
    _c69r = 0.0; _c69i = 1.0;
    double _mul70r = 0, _mul70i = 0;
    c_mul(_c69r, _c69i, x1r, x1i, &_mul70r, &_mul70i);
    double _div71r = 0, _div71i = 0;
    c_div(_mul70r, _mul70i, x2r, x2i, &_div71r, &_div71i);
    double _atan72r = 0, _atan72i = 0;
    c_atan(_div71r, _div71i, &_atan72r, &_atan72i);
    double _add73r = 0, _add73i = 0;
    _add73r = x1r + x2r; _add73i = x1i + x2i;
    double _asinh74r = 0, _asinh74i = 0;
    c_asinh(_add73r, _add73i, &_asinh74r, &_asinh74i);
    double _add75r = 0, _add75i = 0;
    _add75r = _atan72r + _asinh74r; _add75i = _atan72i + _asinh74i;
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add75r; cIm[_idx] = _add75i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_58_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 10.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _add6r = 0, _add6i = 0;
    _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
    double _log7r = 0, _log7i = 0;
    c_log(_add6r, _add6i, &_log7r, &_log7i);
    { int _idx = 0; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _log7r; cIm[_idx] = _log7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 1.0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c8r, _c8i, x1r, x1i, &_mul9r, &_mul9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_mul9r, _mul9i, x2r, x2i, &_mul10r, &_mul10i);
    double _exp11r = 0, _exp11i = 0;
    c_exp2(_mul10r, _mul10i, &_exp11r, &_exp11i);
    double _c12r = 0, _c12i = 0;
    _c12r = 1.0; _c12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _exp11r + _c12r; _add13i = _exp11i + _c12i;
    { int _idx = 1; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add13r; cIm[_idx] = _add13i; } }
    double _mul14r = 0, _mul14i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul14r, &_mul14i);
    double _sin15r = 0, _sin15i = 0;
    c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
    double _add16r = 0, _add16i = 0;
    _add16r = _sin15r + x2r; _add16i = _sin15i + x2i;
    { int _idx = 2; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add16r; cIm[_idx] = _add16i; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 3.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
    c_mul(_pow18r, _pow18i, x1r, x1i, &_pow18r, &_pow18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _pow18r + _pow20r; _add21i = _pow18i + _pow20i;
    double _cos22r = 0, _cos22i = 0;
    c_cos(_add21r, _add21i, &_cos22r, &_cos22i);
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_cos22r, _cos22i, _c23r, _c23i, &_mul24r, &_mul24i);
    { int _idx = 3; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    double _mul25r = 0, _mul25i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul25r, &_mul25i);
    double _sub26r = 0, _sub26i = 0;
    _sub26r = x1r - x2r; _sub26i = x1i - x2i;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_mul25r, _mul25i, _sub26r, _sub26i, &_mul27r, &_mul27i);
    { int _idx = 4; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(_mul28r, _mul28i); _abs29i = 0;
    double _sqrt30r = 0, _sqrt30i = 0;
    c_powr(_abs29r, _abs29i, 0.5, &_sqrt30r, &_sqrt30i);
    double _add31r = 0, _add31i = 0;
    _add31r = x1r + x2r; _add31i = x1i + x2i;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_sqrt30r, _sqrt30i, _add31r, _add31i, &_mul32r, &_mul32i);
    { int _idx = 5; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 3.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow34r, &_pow34i);
    c_mul(_pow34r, _pow34i, x1r, x1i, &_pow34r, &_pow34i);
    double _c35r = 0, _c35i = 0;
    _c35r = 3.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow36r, &_pow36i);
    c_mul(_pow36r, _pow36i, x2r, x2i, &_pow36r, &_pow36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_pow34r, _pow34i, _pow36r, _pow36i, &_mul37r, &_mul37i);
    double _c38r = 0, _c38i = 0;
    _c38r = 0.0; _c38i = 1.0;
    double _mul39r = 0, _mul39i = 0;
    c_mul(_mul37r, _mul37i, _c38r, _c38i, &_mul39r, &_mul39i);
    { int _idx = 6; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    double _sub40r = 0, _sub40i = 0;
    _sub40r = x2r - x1r; _sub40i = x2i - x1i;
    double _add41r = 0, _add41i = 0;
    _add41r = x1r + x2r; _add41i = x1i + x2i;
    double _div42r = 0, _div42i = 0;
    c_div(_sub40r, _sub40i, _add41r, _add41i, &_div42r, &_div42i);
    double _c43r = 0, _c43i = 0;
    _c43r = 0.0; _c43i = 1.0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_div42r, _div42i, _c43r, _c43i, &_mul44r, &_mul44i);
    { int _idx = 7; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    double _mul45r = 0, _mul45i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul45r, &_mul45i);
    double _log46r = 0, _log46i = 0;
    c_log(_mul45r, _mul45i, &_log46r, &_log46i);
    double _add47r = 0, _add47i = 0;
    _add47r = x1r + x2r; _add47i = x1i + x2i;
    double _sin48r = 0, _sin48i = 0;
    c_sin(_add47r, _add47i, &_sin48r, &_sin48i);
    double _c49r = 0, _c49i = 0;
    _c49r = 0.0; _c49i = 1.0;
    double _mul50r = 0, _mul50i = 0;
    c_mul(_sin48r, _sin48i, _c49r, _c49i, &_mul50r, &_mul50i);
    double _add51r = 0, _add51i = 0;
    _add51r = _log46r + _mul50r; _add51i = _log46i + _mul50i;
    { int _idx = 8; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    double _c52r = 0, _c52i = 0;
    _c52r = 3.0; _c52i = 0;
    double _pow53r = 0, _pow53i = 0;
    c_powc(_c52r, _c52i, x1r, x1i, &_pow53r, &_pow53i);
    double _c54r = 0, _c54i = 0;
    _c54r = 2.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_powc(_c54r, _c54i, x2r, x2i, &_pow55r, &_pow55i);
    double _mul56r = 0, _mul56i = 0;
    c_mul(_pow53r, _pow53i, _pow55r, _pow55i, &_mul56r, &_mul56i);
    { int _idx = 9; if (_idx >= 0 && _idx < 10) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    for (int _i = 0; _i < 10; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_59_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 100.0;
    double _neg9r = 0, _neg9i = 0;
    _neg9r = -(_c8r); _neg9i = -(_c8i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul10r, &_mul10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow12r, &_pow12i);
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow14r, &_pow14i);
    double _add15r = 0, _add15i = 0;
    _add15r = _pow12r + _pow14r; _add15i = _pow12i + _pow14i;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_mul10r, _mul10i, _add15r, _add15i, &_mul16r, &_mul16i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(_neg9r, _neg9i, _mul16r, _mul16i, &_mul17r, &_mul17i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul17r; cIm[_idx] = _mul17i; } }
    double _c18r = 0, _c18i = 0;
    _c18r = 100.0; _c18i = 0;
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow20r, &_pow20i);
    double _c21r = 0, _c21i = 0;
    _c21r = 2.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_pow20r, _pow20i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double _attr24r = 0, _attr24i = 0;
    _attr24r = _mul23r; _attr24i = 0;
    double _sqrt25r = 0, _sqrt25i = 0;
    c_powr(_attr24r, _attr24i, 0.5, &_sqrt25r, &_sqrt25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c18r, _c18i, _sqrt25r, _sqrt25i, &_mul26r, &_mul26i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    double _c27r = 0, _c27i = 0;
    _c27r = 100.0; _c27i = 0;
    double _sub28r = 0, _sub28i = 0;
    _sub28r = x1r - x2r; _sub28i = x1i - x2i;
    double _attr29r = 0, _attr29i = 0;
    _attr29r = _sub28i; _attr29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c27r, _c27i, _attr29r, _attr29i, &_mul30r, &_mul30i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 0.0; _c31i = 100.0;
    double _c32r = 0, _c32i = 0;
    _c32r = 2.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow33r, &_pow33i);
    double _c34r = 0, _c34i = 0;
    _c34r = 2.0; _c34i = 0;
    double _pow35r = 0, _pow35i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow35r, &_pow35i);
    double _add36r = 0, _add36i = 0;
    _add36r = _pow33r + _pow35r; _add36i = _pow33i + _pow35i;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c31r, _c31i, _add36r, _add36i, &_mul37r, &_mul37i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    double _c38r = 0, _c38i = 0;
    _c38r = 100.0; _c38i = 0;
    double _c39r = 0, _c39i = 0;
    _c39r = 0.0; _c39i = 1.0;
    double _sub40r = 0, _sub40i = 0;
    _sub40r = x1r - x2r; _sub40i = x1i - x2i;
    double _abs41r = 0, _abs41i = 0;
    _abs41r = c_abs(_sub40r, _sub40i); _abs41i = 0;
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c39r, _c39i, _abs41r, _abs41i, &_mul42r, &_mul42i);
    double _exp43r = 0, _exp43i = 0;
    c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c38r, _c38i, _exp43r, _exp43i, &_mul44r, &_mul44i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 100.0; _c45i = 0;
    double _add46r = 0, _add46i = 0;
    _add46r = x1r + x2r; _add46i = x1i + x2i;
    double _attr47r = 0, _attr47i = 0;
    _attr47r = _add46r; _attr47i = 0;
    double _sin48r = 0, _sin48i = 0;
    c_sin(_attr47r, _attr47i, &_sin48r, &_sin48i);
    double _mul49r = 0, _mul49i = 0;
    c_mul(_c45r, _c45i, _sin48r, _sin48i, &_mul49r, &_mul49i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    double _c50r = 0, _c50i = 0;
    _c50r = 0.0; _c50i = 100.0;
    double _neg51r = 0, _neg51i = 0;
    _neg51r = -(_c50r); _neg51i = -(_c50i);
    double _sub52r = 0, _sub52i = 0;
    _sub52r = x1r - x2r; _sub52i = x1i - x2i;
    double _attr53r = 0, _attr53i = 0;
    _attr53r = _sub52i; _attr53i = 0;
    double _cos54r = 0, _cos54i = 0;
    c_cos(_attr53r, _attr53i, &_cos54r, &_cos54i);
    double _mul55r = 0, _mul55i = 0;
    c_mul(_neg51r, _neg51i, _cos54r, _cos54i, &_mul55r, &_mul55i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    double _c56r = 0, _c56i = 0;
    _c56r = 100.0; _c56i = 0;
    double _mul57r = 0, _mul57i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul57r, &_mul57i);
    double _mul58r = 0, _mul58i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul58r, &_mul58i);
    double _abs59r = 0, _abs59i = 0;
    _abs59r = c_abs(_mul58r, _mul58i); _abs59i = 0;
    double _div60r = 0, _div60i = 0;
    c_div(_mul57r, _mul57i, _abs59r, _abs59i, &_div60r, &_div60i);
    double _mul61r = 0, _mul61i = 0;
    c_mul(_c56r, _c56i, _div60r, _div60i, &_mul61r, &_mul61i);
    { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul61r; cIm[_idx] = _mul61i; } }
    double _c62r = 0, _c62i = 0;
    _c62r = 100.0; _c62i = 0;
    double _attr63r = 0, _attr63i = 0;
    _attr63r = x1r; _attr63i = 0;
    double _attr64r = 0, _attr64i = 0;
    _attr64r = x2r; _attr64i = 0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(_attr63r, _attr63i, _attr64r, _attr64i, &_mul65r, &_mul65i);
    double _sqrt66r = 0, _sqrt66i = 0;
    c_powr(_mul65r, _mul65i, 0.5, &_sqrt66r, &_sqrt66i);
    double _mul67r = 0, _mul67i = 0;
    c_mul(_c62r, _c62i, _sqrt66r, _sqrt66i, &_mul67r, &_mul67i);
    double _c68r = 0, _c68i = 0;
    _c68r = 0.0; _c68i = 100.0;
    double _attr69r = 0, _attr69i = 0;
    _attr69r = x1i; _attr69i = 0;
    double _attr70r = 0, _attr70i = 0;
    _attr70r = x2i; _attr70i = 0;
    double _mul71r = 0, _mul71i = 0;
    c_mul(_attr69r, _attr69i, _attr70r, _attr70i, &_mul71r, &_mul71i);
    double _sqrt72r = 0, _sqrt72i = 0;
    c_powr(_mul71r, _mul71i, 0.5, &_sqrt72r, &_sqrt72i);
    double _mul73r = 0, _mul73i = 0;
    c_mul(_c68r, _c68i, _sqrt72r, _sqrt72i, &_mul73r, &_mul73i);
    double _sub74r = 0, _sub74i = 0;
    _sub74r = _mul67r - _mul73r; _sub74i = _mul67i - _mul73i;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub74r; cIm[_idx] = _sub74i; } }
    double _c75r = 0, _c75i = 0;
    _c75r = 100.0; _c75i = 0;
    double _c76r = 0, _c76i = 0;
    _c76r = 0.0; _c76i = 1.0;
    double _ang77r = 0, _ang77i = 0;
    _ang77r = c_arg(x1r, x1i); _ang77i = 0;
    double _ang78r = 0, _ang78i = 0;
    _ang78r = c_arg(x2r, x2i); _ang78i = 0;
    double _sub79r = 0, _sub79i = 0;
    _sub79r = _ang77r - _ang78r; _sub79i = _ang77i - _ang78i;
    double _mul80r = 0, _mul80i = 0;
    c_mul(_c76r, _c76i, _sub79r, _sub79i, &_mul80r, &_mul80i);
    double _exp81r = 0, _exp81i = 0;
    c_exp2(_mul80r, _mul80i, &_exp81r, &_exp81i);
    double _mul82r = 0, _mul82i = 0;
    c_mul(_c75r, _c75i, _exp81r, _exp81i, &_mul82r, &_mul82i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul82r; cIm[_idx] = _mul82i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_60_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double _sin3r = 0, _sin3i = 0;
        c_sin(_add2r, _add2i, &_sin3r, &_sin3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(_sin3r, _sin3i, x1r, x1i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _c5r + _abs6r; _add7i = _c5i + _abs6i;
        double _div8r = 0, _div8i = 0;
        c_div(_mul4r, _mul4i, _add7r, _add7i, &_div8r, &_div8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = k + _c9r; _add10i = 0 + _c9i;
        double _cos11r = 0, _cos11i = 0;
        c_cos(_add10r, _add10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_cos11r, _cos11i, x2r, x2i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _c13r + _abs14r; _add15i = _c13i + _abs14i;
        double _div16r = 0, _div16i = 0;
        c_div(_mul12r, _mul12i, _add15r, _add15i, &_div16r, &_div16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _div8r + _div16r; _add17i = _div8i + _div16i;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = k + _c18r; _add19i = 0 + _c18i;
        double _sqrt20r = 0, _sqrt20i = 0;
        c_powr(_add19r, _add19i, 0.5, &_sqrt20r, &_sqrt20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _add17r + _sqrt20r; _add21i = _add17i + _sqrt20i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    }
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(x1r, x1i); _abs22i = 0;
    double _abs23r = 0, _abs23i = 0;
    _abs23r = c_abs(x2r, x2i); _abs23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_abs22r, _abs22i, _abs23r, _abs23i, &_mul24r, &_mul24i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    double _ang25r = 0, _ang25i = 0;
    _ang25r = c_arg(x1r, x1i); _ang25i = 0;
    double _abs26r = 0, _abs26i = 0;
    _abs26r = c_abs(x2r, x2i); _abs26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_ang25r, _ang25i, _abs26r, _abs26i, &_mul27r, &_mul27i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(x1r, x1i); _abs28i = 0;
    double _ang29r = 0, _ang29i = 0;
    _ang29r = c_arg(x2r, x2i); _ang29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_abs28r, _abs28i, _ang29r, _ang29i, &_mul30r, &_mul30i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _abs31r = 0, _abs31i = 0;
    _abs31r = c_abs(x1r, x1i); _abs31i = 0;
    double _attr32r = 0, _attr32i = 0;
    _attr32r = x2r; _attr32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_abs31r, _abs31i, _attr32r, _attr32i, &_mul33r, &_mul33i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(x1r, x1i); _abs34i = 0;
    double _attr35r = 0, _attr35i = 0;
    _attr35r = x2i; _attr35i = 0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_abs34r, _abs34i, _attr35r, _attr35i, &_mul36r, &_mul36i);
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    double _attr37r = 0, _attr37i = 0;
    _attr37r = x1r; _attr37i = 0;
    double _abs38r = 0, _abs38i = 0;
    _abs38r = c_abs(x2r, x2i); _abs38i = 0;
    double _mul39r = 0, _mul39i = 0;
    c_mul(_attr37r, _attr37i, _abs38r, _abs38i, &_mul39r, &_mul39i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_61_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(_add1r, _add1i); _abs2i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs2r; cIm[_idx] = _abs2i; } }
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1r; _attr4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c3r, _c3i, _attr4r, _attr4i, &_mul5r, &_mul5i);
    double _attr6r = 0, _attr6i = 0;
    _attr6r = x2i; _attr6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_mul5r, _mul5i, _attr6r, _attr6i, &_mul7r, &_mul7i);
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
    for (int k = 5; k < 21; k++) {
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _neg16r = 0, _neg16i = 0;
        _neg16r = -(_c15r); _neg16i = -(_c15i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = k + _c17r; _add18i = 0 + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powc(_neg16r, _neg16i, _add18r, _add18i, &_pow19r, &_pow19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow21r, &_pow21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_pow19r, _pow19i, _pow21r, _pow21i, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = k + _c23r; _add24i = 0 + _c23i;
        double _div25r = 0, _div25i = 0;
        c_div(_mul22r, _mul22i, _add24r, _add24i, &_div25r, &_div25i);
        double _add26r = 0, _add26i = 0;
        _add26r = x1r + _div25r; _add26i = x1i + _div25i;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _neg28r = 0, _neg28i = 0;
        _neg28r = -(_c27r); _neg28i = -(_c27i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = k + _c29r; _add30i = 0 + _c29i;
        double _pow31r = 0, _pow31i = 0;
        c_powc(_neg28r, _neg28i, _add30r, _add30i, &_pow31r, &_pow31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_pow31r, _pow31i, _pow33r, _pow33i, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = k + _c35r; _add36i = 0 + _c35i;
        double _div37r = 0, _div37i = 0;
        c_div(_mul34r, _mul34i, _add36r, _add36i, &_div37r, &_div37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add26r + _div37r; _add38i = _add26i + _div37i;
        double _abs39r = 0, _abs39i = 0;
        _abs39r = c_abs(_add38r, _add38i); _abs39i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs39r; cIm[_idx] = _abs39i; } }
    }
    double _cf40r = 0, _cf40i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
    double _cf41r = 0, _cf41i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
    double _add42r = 0, _add42i = 0;
    _add42r = _cf40r + _cf41r; _add42i = _cf40i + _cf41i;
    double _cf43r = 0, _cf43i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
    double _sub44r = 0, _sub44i = 0;
    _sub44r = _add42r - _cf43r; _sub44i = _add42i - _cf43i;
    double _cf45r = 0, _cf45i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf45r = cRe[_idx]; _cf45i = cIm[_idx]; } }
    double _add46r = 0, _add46i = 0;
    _add46r = _sub44r + _cf45r; _add46i = _sub44i + _cf45i;
    { int _idx = 21; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    double _cf47r = 0, _cf47i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf47r = cRe[_idx]; _cf47i = cIm[_idx]; } }
    double _cf48r = 0, _cf48i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
    double _mul49r = 0, _mul49i = 0;
    c_mul(_cf47r, _cf47i, _cf48r, _cf48i, &_mul49r, &_mul49i);
    double _cf50r = 0, _cf50i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
    double _mul51r = 0, _mul51i = 0;
    c_mul(_mul49r, _mul49i, _cf50r, _cf50i, &_mul51r, &_mul51i);
    double _cf52r = 0, _cf52i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
    double _mul53r = 0, _mul53i = 0;
    c_mul(_mul51r, _mul51i, _cf52r, _cf52i, &_mul53r, &_mul53i);
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(_mul53r, _mul53i); _abs54i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs54r; cIm[_idx] = _abs54i; } }
    double _c55r = 0, _c55i = 0;
    _c55r = 1.0; _c55i = 0;
    double _conj56r = 0, _conj56i = 0;
    _conj56r = x1r; _conj56i = -(x1i);
    double _mul57r = 0, _mul57i = 0;
    c_mul(_conj56r, _conj56i, x2r, x2i, &_mul57r, &_mul57i);
    double _attr58r = 0, _attr58i = 0;
    _attr58r = _mul57r; _attr58i = 0;
    double _add59r = 0, _add59i = 0;
    _add59r = _c55r + _attr58r; _add59i = _c55i + _attr58i;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    double _c60r = 0, _c60i = 0;
    _c60r = 0.0; _c60i = 1.0;
    double _conj61r = 0, _conj61i = 0;
    _conj61r = x1r; _conj61i = -(x1i);
    double _mul62r = 0, _mul62i = 0;
    c_mul(_conj61r, _conj61i, x2r, x2i, &_mul62r, &_mul62i);
    double _attr63r = 0, _attr63i = 0;
    _attr63r = _mul62i; _attr63i = 0;
    double _add64r = 0, _add64i = 0;
    _add64r = _c60r + _attr63r; _add64i = _c60i + _attr63i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_62: auto-stubbed (unhandled constructs in source) */
static void poly_giga_62_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_63_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 25; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = i + _c1r; _add2i = 0 + _c1i;
        double _mul3r = 0, _mul3i = 0;
        c_mul(x1r, x1i, _add2r, _add2i, &_mul3r, &_mul3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = i + _c4r; _add5i = 0 + _c4i;
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_add5r, _add5i, _c6r, _c6i, &_div7r, &_div7i);
        double _pow8r = 0, _pow8i = 0;
        c_powc(x2r, x2i, _div7r, _div7i, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul3r + _pow8r; _add9i = _mul3i + _pow8i;
        double numerator_r = _add9r, numerator_i = _add9i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = i + _c10r; _add11i = 0 + _c10i;
        double _mul12r = 0, _mul12i = 0;
        c_mul(x1r, x1i, _add11r, _add11i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = i + _c13r; _add14i = 0 + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powc(x2r, x2i, _add14r, _add14i, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _pow15r; _add16i = _mul12i + _pow15i;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_add16r, _add16i); _abs17i = 0;
        double denominator_r = _abs17r, denominator_i = _abs17i;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 0;
        if (denominator_r > _c18r) {
            double _div19r = 0, _div19i = 0;
            c_div(numerator_r, numerator_i, denominator_r, denominator_i, &_div19r, &_div19i);
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div19r; cIm[_idx] = _div19i; } }
        } else {
            double _c20r = 0, _c20i = 0;
            _c20r = 0.0; _c20i = 0;
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c20r; cIm[_idx] = _c20i; } }
        }
    }
    double _attr21r = 0, _attr21i = 0;
    _attr21r = x1r; _attr21i = 0;
    double _attr22r = 0, _attr22i = 0;
    _attr22r = x2i; _attr22i = 0;
    double _add23r = 0, _add23i = 0;
    _add23r = _attr21r + _attr22r; _add23i = _attr21i + _attr22i;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 1.0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul25r, &_mul25i);
    double _ang26r = 0, _ang26i = 0;
    _ang26r = c_arg(_mul25r, _mul25i); _ang26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c24r, _c24i, _ang26r, _ang26i, &_mul27r, &_mul27i);
    double _exp28r = 0, _exp28i = 0;
    c_exp2(_mul27r, _mul27i, &_exp28r, &_exp28i);
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(_exp28r, _exp28i); _abs29i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs29r; cIm[_idx] = _abs29i; } }
    double _mul30r = 0, _mul30i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul30r, &_mul30i);
    double _attr31r = 0, _attr31i = 0;
    _attr31r = _mul30r; _attr31i = 0;
    double _div32r = 0, _div32i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div32r, &_div32i);
    double _attr33r = 0, _attr33i = 0;
    _attr33r = _div32i; _attr33i = 0;
    double _add34r = 0, _add34i = 0;
    _add34r = _attr31r + _attr33r; _add34i = _attr31i + _attr33i;
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add34r; cIm[_idx] = _add34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 4.0; _c35i = 0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c35r, _c35i, x2r, x2i, &_mul36r, &_mul36i);
    double _add37r = 0, _add37i = 0;
    _add37r = x1r + _mul36r; _add37i = x1i + _mul36i;
    double _ang38r = 0, _ang38i = 0;
    _ang38r = c_arg(_add37r, _add37i); _ang38i = 0;
    double _c39r = 0, _c39i = 0;
    _c39r = 4.0; _c39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c39r, _c39i, x2r, x2i, &_mul40r, &_mul40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = x1r - _mul40r; _sub41i = x1i - _mul40i;
    double _conj42r = 0, _conj42i = 0;
    _conj42r = _sub41r; _conj42i = -(_sub41i);
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(_conj42r, _conj42i); _abs43i = 0;
    double _div44r = 0, _div44i = 0;
    c_div(_ang38r, _ang38i, _abs43r, _abs43i, &_div44r, &_div44i);
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div44r; cIm[_idx] = _div44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 0.0; _c45i = 1.0;
    double _sub46r = 0, _sub46i = 0;
    _sub46r = x1r - x2r; _sub46i = x1i - x2i;
    double _ang47r = 0, _ang47i = 0;
    _ang47r = c_arg(_sub46r, _sub46i); _ang47i = 0;
    double _mul48r = 0, _mul48i = 0;
    c_mul(_c45r, _c45i, _ang47r, _ang47i, &_mul48r, &_mul48i);
    double _exp49r = 0, _exp49i = 0;
    c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
    double _abs50r = 0, _abs50i = 0;
    _abs50r = c_abs(_exp49r, _exp49i); _abs50i = 0;
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs50r; cIm[_idx] = _abs50i; } }
    double _div51r = 0, _div51i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div51r, &_div51i);
    double _attr52r = 0, _attr52i = 0;
    _attr52r = _div51r; _attr52i = 0;
    double _mul53r = 0, _mul53i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul53r, &_mul53i);
    double _attr54r = 0, _attr54i = 0;
    _attr54r = _mul53i; _attr54i = 0;
    double _sub55r = 0, _sub55i = 0;
    _sub55r = _attr52r - _attr54r; _sub55i = _attr52i - _attr54i;
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub55r; cIm[_idx] = _sub55i; } }
    double _c56r = 0, _c56i = 0;
    _c56r = 0.0; _c56i = 1.0;
    double _add57r = 0, _add57i = 0;
    _add57r = x1r + x2r; _add57i = x1i + x2i;
    double _ang58r = 0, _ang58i = 0;
    _ang58r = c_arg(_add57r, _add57i); _ang58i = 0;
    double _mul59r = 0, _mul59i = 0;
    c_mul(_c56r, _c56i, _ang58r, _ang58i, &_mul59r, &_mul59i);
    double _exp60r = 0, _exp60i = 0;
    c_exp2(_mul59r, _mul59i, &_exp60r, &_exp60i);
    double _abs61r = 0, _abs61i = 0;
    _abs61r = c_abs(_exp60r, _exp60i); _abs61i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs61r; cIm[_idx] = _abs61i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_64_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
    for (int k = 3; k < 25; k++) {
        double _cf11r = 0, _cf11i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _attr12r = 0, _attr12i = 0;
        _attr12r = _cf11r; _attr12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 1.0;
        double _cf14r = 0, _cf14i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
        double _attr15r = 0, _attr15i = 0;
        _attr15r = _cf14i; _attr15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c13r, _c13i, _attr15r, _attr15i, &_mul16r, &_mul16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _attr12r + _mul16r; _add17i = _attr12i + _mul16i;
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
        double _attr25r = 0, _attr25i = 0;
        _attr25r = _cf24i; _attr25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 0;
        if (_attr25r == _c26r) {
            double _cf27r = 0, _cf27i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf27r = cRe[_idx]; _cf27i = cIm[_idx]; } }
            double _c28r = 0, _c28i = 0;
            _c28r = 1e-10; _c28i = 0;
            double _add29r = 0, _add29i = 0;
            _add29r = _cf27r + _c28r; _add29i = _cf27i + _c28i;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
        }
        double _cf30r = 0, _cf30i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(_cf30r, _cf30i); _abs31i = 0;
        double _log32r = 0, _log32i = 0;
        c_log(_abs31r, _abs31i, &_log32r, &_log32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_log32r, _log32i, _c33r, _c33i, &_div34r, &_div34i);
        double _cf35r = 0, _cf35i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_cf35r, _cf35i, _c36r, _c36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _div34r + _mul37r; _add38i = _div34i + _mul37i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_65_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _sub2r = 0, _sub2i = 0;
    _sub2r = x1r - x2r; _sub2i = x1i - x2i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub2r; cIm[_idx] = _sub2i; } }
    double _mul3r = 0, _mul3i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul3r, &_mul3i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    double _div4r = 0, _div4i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div4r, &_div4i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div4r; cIm[_idx] = _div4i; } }
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(x1r, x1i); _abs5i = 0;
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(x2r, x2i); _abs6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = _abs5r + _abs6r; _add7i = _abs5i + _abs6i;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(x1r, x1i); _abs8i = 0;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(x2r, x2i); _abs9i = 0;
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _abs8r - _abs9r; _sub10i = _abs8i - _abs9i;
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub10r; cIm[_idx] = _sub10i; } }
    double _ang11r = 0, _ang11i = 0;
    _ang11r = c_arg(x1r, x1i); _ang11i = 0;
    double _ang12r = 0, _ang12i = 0;
    _ang12r = c_arg(x2r, x2i); _ang12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _ang11r + _ang12r; _add13i = _ang11i + _ang12i;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add13r; cIm[_idx] = _add13i; } }
    double _ang14r = 0, _ang14i = 0;
    _ang14r = c_arg(x1r, x1i); _ang14i = 0;
    double _ang15r = 0, _ang15i = 0;
    _ang15r = c_arg(x2r, x2i); _ang15i = 0;
    double _sub16r = 0, _sub16i = 0;
    _sub16r = _ang14r - _ang15r; _sub16i = _ang14i - _ang15i;
    { int _idx = 7; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub16r; cIm[_idx] = _sub16i; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _pow18r + _pow20r; _add21i = _pow18i + _pow20i;
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
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
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add26r; cIm[_idx] = _add26i; } }
    double _c27r = 0, _c27i = 0;
    _c27r = 4.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow28r, &_pow28i);
    c_mul(_pow28r, _pow28i, _pow28r, _pow28i, &_pow28r, &_pow28i);
    double _c29r = 0, _c29i = 0;
    _c29r = 4.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, _pow30r, _pow30i, &_pow30r, &_pow30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _pow28r + _pow30r; _add31i = _pow28i + _pow30i;
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
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
    double _add38r = 0, _add38i = 0;
    _add38r = _pow34r + _pow37r; _add38i = _pow34i + _pow37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 1.0; _c39i = 0;
    double _add40r = 0, _add40i = 0;
    _add40r = _add38r + _c39r; _add40i = _add38i + _c39i;
    double _log41r = 0, _log41i = 0;
    c_log(_add40r, _add40i, &_log41r, &_log41i);
    { int _idx = 11; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _log41r; cIm[_idx] = _log41i; } }
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(x1r, x1i); _abs42i = 0;
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(x2r, x2i); _abs43i = 0;
    double _add44r = 0, _add44i = 0;
    _add44r = _abs42r + _abs43r; _add44i = _abs42i + _abs43i;
    double _exp45r = 0, _exp45i = 0;
    c_exp2(_add44r, _add44i, &_exp45r, &_exp45i);
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _exp45r; cIm[_idx] = _exp45i; } }
    double _conj46r = 0, _conj46i = 0;
    _conj46r = x1r; _conj46i = -(x1i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_conj46r, _conj46i, x2r, x2i, &_mul47r, &_mul47i);
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    double _conj48r = 0, _conj48i = 0;
    _conj48r = x2r; _conj48i = -(x2i);
    double _mul49r = 0, _mul49i = 0;
    c_mul(x1r, x1i, _conj48r, _conj48i, &_mul49r, &_mul49i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    double _conj50r = 0, _conj50i = 0;
    _conj50r = x1r; _conj50i = -(x1i);
    double _conj51r = 0, _conj51i = 0;
    _conj51r = x2r; _conj51i = -(x2i);
    double _mul52r = 0, _mul52i = 0;
    c_mul(_conj50r, _conj50i, _conj51r, _conj51i, &_mul52r, &_mul52i);
    { int _idx = 15; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    double _sub53r = 0, _sub53i = 0;
    _sub53r = x1r - x2r; _sub53i = x1i - x2i;
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(_sub53r, _sub53i); _abs54i = 0;
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs54r; cIm[_idx] = _abs54i; } }
    double _add55r = 0, _add55i = 0;
    _add55r = x1r + x2r; _add55i = x1i + x2i;
    double _abs56r = 0, _abs56i = 0;
    _abs56r = c_abs(_add55r, _add55i); _abs56i = 0;
    { int _idx = 17; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs56r; cIm[_idx] = _abs56i; } }
    for (int _si = 0; _si < 6; _si++) {
        int _si_idx = _si + 18;
        double _add57r = 0, _add57i = 0;
        _add57r = x1r + x2r; _add57i = x1i + x2i;
        double _abs58r = 0, _abs58i = 0;
        _abs58r = c_abs(_add57r, _add57i); _abs58i = 0;
        cRe[_si_idx] = _abs58r; cIm[_si_idx] = _abs58i;
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_66_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _sub2r = 0, _sub2i = 0;
    _sub2r = x1r - x2r; _sub2i = x1i - x2i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub2r; cIm[_idx] = _sub2i; } }
    double _mul3r = 0, _mul3i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul3r, &_mul3i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    double _div4r = 0, _div4i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div4r, &_div4i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div4r; cIm[_idx] = _div4i; } }
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(x1r, x1i); _abs5i = 0;
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(x2r, x2i); _abs6i = 0;
    double _add7r = 0, _add7i = 0;
    _add7r = _abs5r + _abs6r; _add7i = _abs5i + _abs6i;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(x1r, x1i); _abs8i = 0;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(x2r, x2i); _abs9i = 0;
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _abs8r - _abs9r; _sub10i = _abs8i - _abs9i;
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub10r; cIm[_idx] = _sub10i; } }
    double _ang11r = 0, _ang11i = 0;
    _ang11r = c_arg(x1r, x1i); _ang11i = 0;
    double _ang12r = 0, _ang12i = 0;
    _ang12r = c_arg(x2r, x2i); _ang12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _ang11r + _ang12r; _add13i = _ang11i + _ang12i;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add13r; cIm[_idx] = _add13i; } }
    double _ang14r = 0, _ang14i = 0;
    _ang14r = c_arg(x1r, x1i); _ang14i = 0;
    double _ang15r = 0, _ang15i = 0;
    _ang15r = c_arg(x2r, x2i); _ang15i = 0;
    double _sub16r = 0, _sub16i = 0;
    _sub16r = _ang14r - _ang15r; _sub16i = _ang14i - _ang15i;
    { int _idx = 7; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub16r; cIm[_idx] = _sub16i; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _pow18r + _pow20r; _add21i = _pow18i + _pow20i;
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
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
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add26r; cIm[_idx] = _add26i; } }
    double _c27r = 0, _c27i = 0;
    _c27r = 4.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow28r, &_pow28i);
    c_mul(_pow28r, _pow28i, _pow28r, _pow28i, &_pow28r, &_pow28i);
    double _c29r = 0, _c29i = 0;
    _c29r = 4.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, _pow30r, _pow30i, &_pow30r, &_pow30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _pow28r + _pow30r; _add31i = _pow28i + _pow30i;
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
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
    double _add38r = 0, _add38i = 0;
    _add38r = _pow34r + _pow37r; _add38i = _pow34i + _pow37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 1.0; _c39i = 0;
    double _add40r = 0, _add40i = 0;
    _add40r = _add38r + _c39r; _add40i = _add38i + _c39i;
    double _log41r = 0, _log41i = 0;
    c_log(_add40r, _add40i, &_log41r, &_log41i);
    { int _idx = 11; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _log41r; cIm[_idx] = _log41i; } }
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(x1r, x1i); _abs42i = 0;
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(x2r, x2i); _abs43i = 0;
    double _add44r = 0, _add44i = 0;
    _add44r = _abs42r + _abs43r; _add44i = _abs42i + _abs43i;
    double _exp45r = 0, _exp45i = 0;
    c_exp2(_add44r, _add44i, &_exp45r, &_exp45i);
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _exp45r; cIm[_idx] = _exp45i; } }
    double _conj46r = 0, _conj46i = 0;
    _conj46r = x1r; _conj46i = -(x1i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_conj46r, _conj46i, x2r, x2i, &_mul47r, &_mul47i);
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    double _conj48r = 0, _conj48i = 0;
    _conj48r = x2r; _conj48i = -(x2i);
    double _mul49r = 0, _mul49i = 0;
    c_mul(x1r, x1i, _conj48r, _conj48i, &_mul49r, &_mul49i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    double _conj50r = 0, _conj50i = 0;
    _conj50r = x1r; _conj50i = -(x1i);
    double _conj51r = 0, _conj51i = 0;
    _conj51r = x2r; _conj51i = -(x2i);
    double _mul52r = 0, _mul52i = 0;
    c_mul(_conj50r, _conj50i, _conj51r, _conj51i, &_mul52r, &_mul52i);
    { int _idx = 15; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    double _sub53r = 0, _sub53i = 0;
    _sub53r = x1r - x2r; _sub53i = x1i - x2i;
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(_sub53r, _sub53i); _abs54i = 0;
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs54r; cIm[_idx] = _abs54i; } }
    double _add55r = 0, _add55i = 0;
    _add55r = x1r + x2r; _add55i = x1i + x2i;
    double _abs56r = 0, _abs56i = 0;
    _abs56r = c_abs(_add55r, _add55i); _abs56i = 0;
    { int _idx = 17; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs56r; cIm[_idx] = _abs56i; } }
    for (int _si = 0; _si < 7; _si++) {
        int _si_idx = _si + 18;
        double _add57r = 0, _add57i = 0;
        _add57r = x1r + x2r; _add57i = x1i + x2i;
        double _abs58r = 0, _abs58i = 0;
        _abs58r = c_abs(_add57r, _add57i); _abs58i = 0;
        cRe[_si_idx] = _abs58r; cIm[_si_idx] = _abs58i;
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_67_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _mul2r = 0, _mul2i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul2r, &_mul2i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul2r; cIm[_idx] = _mul2i; } }
    double _add3r = 0, _add3i = 0;
    _add3r = x1r + x2r; _add3i = x1i + x2i;
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(_add3r, _add3i, _add3r, _add3i, &_pow5r, &_pow5i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow5r; cIm[_idx] = _pow5i; } }
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(x1r, x1i); _abs6i = 0;
    double _abs7r = 0, _abs7i = 0;
    _abs7r = c_abs(x2r, x2i); _abs7i = 0;
    double _mul8r = 0, _mul8i = 0;
    c_mul(_abs6r, _abs6i, _abs7r, _abs7i, &_mul8r, &_mul8i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul8r; cIm[_idx] = _mul8i; } }
    for (int k = 4; k < 9; k++) {
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 1.0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_c9r, _c9i, x2r, x2i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = x1r + _mul10r; _add11i = x1i + _mul10i;
        double _pow12r = 0, _pow12i = 0;
        c_powr(_add11r, _add11i, k, &_pow12r, &_pow12i);
        double r_r = _pow12r, r_i = _pow12i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(r_r, r_i); _abs13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(r_r, r_i, _abs13r, _abs13i, &_div14r, &_div14i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div14r; cIm[_idx] = _div14i; } }
    }
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(x1r, x1i); _abs15i = 0;
    double _log16r = 0, _log16i = 0;
    c_log(_abs15r, _abs15i, &_log16r, &_log16i);
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x2r, x2i); _abs17i = 0;
    double _log18r = 0, _log18i = 0;
    c_log(_abs17r, _abs17i, &_log18r, &_log18i);
    double _div19r = 0, _div19i = 0;
    c_div(_log16r, _log16i, _log18r, _log18i, &_div19r, &_div19i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div19r; cIm[_idx] = _div19i; } }
    double _ang20r = 0, _ang20i = 0;
    _ang20r = c_arg(x1r, x1i); _ang20i = 0;
    double _exp21r = 0, _exp21i = 0;
    c_exp2(_ang20r, _ang20i, &_exp21r, &_exp21i);
    double _ang22r = 0, _ang22i = 0;
    _ang22r = c_arg(x2r, x2i); _ang22i = 0;
    double _exp23r = 0, _exp23i = 0;
    c_exp2(_ang22r, _ang22i, &_exp23r, &_exp23i);
    double _mul24r = 0, _mul24i = 0;
    c_mul(_exp21r, _exp21i, _exp23r, _exp23i, &_mul24r, &_mul24i);
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    for (int k = 11; k < 16; k++) {
        double _conj25r = 0, _conj25i = 0;
        _conj25r = x1r; _conj25i = -(x1i);
        double _conj26r = 0, _conj26i = 0;
        _conj26r = x2r; _conj26i = -(x2i);
        double _div27r = 0, _div27i = 0;
        c_div(_conj25r, _conj25i, _conj26r, _conj26i, &_div27r, &_div27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _sub30r = 0, _sub30i = 0;
        _sub30r = k - _c29r; _sub30i = 0 - _c29i;
        double _pow31r = 0, _pow31i = 0;
        c_powc(_c28r, _c28i, _sub30r, _sub30i, &_pow31r, &_pow31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_div27r, _div27i, _pow31r, _pow31i, &_mul32r, &_mul32i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    }
    double _attr33r = 0, _attr33i = 0;
    _attr33r = x1r; _attr33i = 0;
    double _attr34r = 0, _attr34i = 0;
    _attr34r = x2i; _attr34i = 0;
    double _mul35r = 0, _mul35i = 0;
    c_mul(_attr33r, _attr33i, _attr34r, _attr34i, &_mul35r, &_mul35i);
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    double _attr36r = 0, _attr36i = 0;
    _attr36r = x1i; _attr36i = 0;
    double _attr37r = 0, _attr37i = 0;
    _attr37r = x2r; _attr37i = 0;
    double _mul38r = 0, _mul38i = 0;
    c_mul(_attr36r, _attr36i, _attr37r, _attr37i, &_mul38r, &_mul38i);
    { int _idx = 17; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    for (int k = 18; k < 23; k++) {
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c39r, _c39i, x2r, x2i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = x1r + _mul40r; _add41i = x1i + _mul40i;
        double _pow42r = 0, _pow42i = 0;
        c_powr(_add41r, _add41i, k, &_pow42r, &_pow42i);
        double z_r = _pow42r, z_i = _pow42i;
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(z_r, z_i); _ang43i = 0;
        double _sin44r = 0, _sin44i = 0;
        c_sin(_ang43r, _ang43i, &_sin44r, &_sin44i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sin44r; cIm[_idx] = _sin44i; } }
    }
    double _ang45r = 0, _ang45i = 0;
    _ang45r = c_arg(x1r, x1i); _ang45i = 0;
    double _ang46r = 0, _ang46i = 0;
    _ang46r = c_arg(x2r, x2i); _ang46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = _ang45r + _ang46r; _add47i = _ang45i + _ang46i;
    double _cos48r = 0, _cos48i = 0;
    c_cos(_add47r, _add47i, &_cos48r, &_cos48i);
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cos48r; cIm[_idx] = _cos48i; } }
    double _mul49r = 0, _mul49i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul49r, &_mul49i);
    double _abs50r = 0, _abs50i = 0;
    _abs50r = c_abs(_mul49r, _mul49i); _abs50i = 0;
    double _tanh51r = 0, _tanh51i = 0;
    _tanh51r = tanh(_abs50r); _tanh51i = 0; /* approx real tanh */
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _tanh51r; cIm[_idx] = _tanh51i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_68_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _div5r = 0, _div5i = 0;
        c_div(_add3r, _add3i, _c4r, _c4i, &_div5r, &_div5i);
        double _pow6r = 0, _pow6i = 0;
        c_powc(_abs1r, _abs1i, _div5r, _div5i, &_pow6r, &_pow6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = k + _c7r; _add8i = 0 + _c7i;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_add8r, _add8i, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 0.0; _c12i = 1.0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = k + _c13r; _add14i = 0 + _c13i;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_add14r, _add14i, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c12r, _c12i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _cos11r + _mul18r; _add19i = _cos11i + _mul18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_pow6r, _pow6i, _add19r, _add19i, &_mul20r, &_mul20i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul20r; cIm[_idx] = _mul20i; } }
    }
    double _cf21r = 0, _cf21i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(x1r, x1i); _abs22i = 0;
    double _log23r = 0, _log23i = 0;
    c_log(_abs22r, _abs22i, &_log23r, &_log23i);
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(x2r, x2i); _abs24i = 0;
    double _log25r = 0, _log25i = 0;
    c_log(_abs24r, _abs24i, &_log25r, &_log25i);
    double _add26r = 0, _add26i = 0;
    _add26r = _log23r + _log25r; _add26i = _log23i + _log25i;
    double _c27r = 0, _c27i = 0;
    _c27r = 2.0; _c27i = 0;
    double _div28r = 0, _div28i = 0;
    c_div(_add26r, _add26i, _c27r, _c27i, &_div28r, &_div28i);
    double _add29r = 0, _add29i = 0;
    _add29r = _cf21r + _div28r; _add29i = _cf21i + _div28i;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    double _cf30r = 0, _cf30i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
    double _mul31r = 0, _mul31i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul31r, &_mul31i);
    double _conj32r = 0, _conj32i = 0;
    _conj32r = _mul31r; _conj32i = -(_mul31i);
    double _add33r = 0, _add33i = 0;
    _add33r = _cf30r + _conj32r; _add33i = _cf30i + _conj32i;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    double _cf34r = 0, _cf34i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
    double _sub35r = 0, _sub35i = 0;
    _sub35r = x2r - x1r; _sub35i = x2i - x1i;
    double _abs36r = 0, _abs36i = 0;
    _abs36r = c_abs(_sub35r, _sub35i); _abs36i = 0;
    double _c37r = 0, _c37i = 0;
    _c37r = 2.0; _c37i = 0;
    double _pow38r = 0, _pow38i = 0;
    c_mul(_abs36r, _abs36i, _abs36r, _abs36i, &_pow38r, &_pow38i);
    double _add39r = 0, _add39i = 0;
    _add39r = _cf34r + _pow38r; _add39i = _cf34i + _pow38i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    double _cf40r = 0, _cf40i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
    double _ang41r = 0, _ang41i = 0;
    _ang41r = c_arg(x1r, x1i); _ang41i = 0;
    double _sin42r = 0, _sin42i = 0;
    c_sin(_ang41r, _ang41i, &_sin42r, &_sin42i);
    double _ang43r = 0, _ang43i = 0;
    _ang43r = c_arg(x2r, x2i); _ang43i = 0;
    double _cos44r = 0, _cos44i = 0;
    c_cos(_ang43r, _ang43i, &_cos44r, &_cos44i);
    double _div45r = 0, _div45i = 0;
    c_div(_sin42r, _sin42i, _cos44r, _cos44i, &_div45r, &_div45i);
    double _c46r = 0, _c46i = 0;
    _c46r = 3.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(_div45r, _div45i, _div45r, _div45i, &_pow47r, &_pow47i);
    c_mul(_pow47r, _pow47i, _div45r, _div45i, &_pow47r, &_pow47i);
    double _add48r = 0, _add48i = 0;
    _add48r = _cf40r + _pow47r; _add48i = _cf40i + _pow47i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
    double _cf49r = 0, _cf49i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { _cf49r = cRe[_idx]; _cf49i = cIm[_idx]; } }
    double _c50r = 0, _c50i = 0;
    _c50r = 0.0; _c50i = 1.0;
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c50r, _c50i, x1r, x1i, &_mul51r, &_mul51i);
    double _sub52r = 0, _sub52i = 0;
    _sub52r = _mul51r - x2r; _sub52i = _mul51i - x2i;
    double _c53r = 0, _c53i = 0;
    _c53r = 2.0; _c53i = 0;
    double _pow54r = 0, _pow54i = 0;
    c_mul(_sub52r, _sub52i, _sub52r, _sub52i, &_pow54r, &_pow54i);
    double _c55r = 0, _c55i = 0;
    _c55r = 1.0; _c55i = 0;
    double _add56r = 0, _add56i = 0;
    _add56r = x1r + x2r; _add56i = x1i + x2i;
    double _abs57r = 0, _abs57i = 0;
    _abs57r = c_abs(_add56r, _add56i); _abs57i = 0;
    double _c58r = 0, _c58i = 0;
    _c58r = 3.0; _c58i = 0;
    double _pow59r = 0, _pow59i = 0;
    c_mul(_abs57r, _abs57i, _abs57r, _abs57i, &_pow59r, &_pow59i);
    c_mul(_pow59r, _pow59i, _abs57r, _abs57i, &_pow59r, &_pow59i);
    double _add60r = 0, _add60i = 0;
    _add60r = _c55r + _pow59r; _add60i = _c55i + _pow59i;
    double _div61r = 0, _div61i = 0;
    c_div(_pow54r, _pow54i, _add60r, _add60i, &_div61r, &_div61i);
    double _c62r = 0, _c62i = 0;
    _c62r = 4.0; _c62i = 0;
    double _pow63r = 0, _pow63i = 0;
    c_mul(_div61r, _div61i, _div61r, _div61i, &_pow63r, &_pow63i);
    c_mul(_pow63r, _pow63i, _pow63r, _pow63i, &_pow63r, &_pow63i);
    double _add64r = 0, _add64i = 0;
    _add64r = _cf49r + _pow63r; _add64i = _cf49i + _pow63i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_69_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 25; i++) {
        double _attr1r = 0, _attr1i = 0;
        _attr1r = x1r; _attr1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = i + _c2r; _add3i = 0 + _c2i;
        double _pow4r = 0, _pow4i = 0;
        c_powc(_attr1r, _attr1i, _add3r, _add3i, &_pow4r, &_pow4i);
        double _attr5r = 0, _attr5i = 0;
        _attr5r = x2i; _attr5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 25.0; _c6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _c6r - i; _sub7i = _c6i - 0;
        double _pow8r = 0, _pow8i = 0;
        c_powc(_attr5r, _attr5i, _sub7r, _sub7i, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow4r + _pow8r; _add9i = _pow4i + _pow8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = x1r + x2r; _add11i = x1i + x2i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _c10r + _abs12r; _add13i = _c10i + _abs12i;
        double _div14r = 0, _div14i = 0;
        c_div(_add9r, _add9i, _add13r, _add13i, &_div14r, &_div14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 1.0;
        double _add16r = 0, _add16i = 0;
        _add16r = x1r + x2r; _add16i = x1i + x2i;
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(_add16r, _add16i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c15r, _c15i, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _exp19r = 0, _exp19i = 0;
        c_exp2(_mul18r, _mul18i, &_exp19r, &_exp19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_div14r, _div14i, _exp19r, _exp19i, &_mul20r, &_mul20i);
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul20r; cIm[_idx] = _mul20i; } }
    }
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _c22r = 0, _c22i = 0;
    _c22r = 2.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow23r, &_pow23i);
    double _add24r = 0, _add24i = 0;
    _add24r = _pow23r + x2r; _add24i = _pow23i + x2i;
    double _conj25r = 0, _conj25i = 0;
    _conj25r = _add24r; _conj25i = -(_add24i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c21r, _c21i, _conj25r, _conj25i, &_mul26r, &_mul26i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    double _c27r = 0, _c27i = 0;
    _c27r = 7.0; _c27i = 0;
    double _add28r = 0, _add28i = 0;
    _add28r = x1r + x2r; _add28i = x1i + x2i;
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(_add28r, _add28i); _abs29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c27r, _c27i, _abs29r, _abs29i, &_mul30r, &_mul30i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 11.0; _c31i = 0;
    double _div32r = 0, _div32i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div32r, &_div32i);
    double _div33r = 0, _div33i = 0;
    c_div(x2r, x2i, x1r, x1i, &_div33r, &_div33i);
    double _conj34r = 0, _conj34i = 0;
    _conj34r = _div33r; _conj34i = -(_div33i);
    double _add35r = 0, _add35i = 0;
    _add35r = _div32r + _conj34r; _add35i = _div32i + _conj34i;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c31r, _c31i, _add35r, _add35i, &_mul36r, &_mul36i);
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    double _c37r = 0, _c37i = 0;
    _c37r = 17.0; _c37i = 0;
    double _abs38r = 0, _abs38i = 0;
    _abs38r = c_abs(x1r, x1i); _abs38i = 0;
    double _abs39r = 0, _abs39i = 0;
    _abs39r = c_abs(x2r, x2i); _abs39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_abs38r, _abs38i, _abs39r, _abs39i, &_mul40r, &_mul40i);
    double _mul41r = 0, _mul41i = 0;
    c_mul(_c37r, _c37i, _mul40r, _mul40i, &_mul41r, &_mul41i);
    double _add42r = 0, _add42i = 0;
    _add42r = x1r + x2r; _add42i = x1i + x2i;
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(_add42r, _add42i); _abs43i = 0;
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(_abs43r, _abs43i, _abs43r, _abs43i, &_pow45r, &_pow45i);
    double _div46r = 0, _div46i = 0;
    c_div(_mul41r, _mul41i, _pow45r, _pow45i, &_div46r, &_div46i);
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div46r; cIm[_idx] = _div46i; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 23.0; _c47i = 0;
    double _conj48r = 0, _conj48i = 0;
    _conj48r = x1r; _conj48i = -(x1i);
    double _add49r = 0, _add49i = 0;
    _add49r = _conj48r + x2r; _add49i = _conj48i + x2i;
    double _mul50r = 0, _mul50i = 0;
    c_mul(_c47r, _c47i, _add49r, _add49i, &_mul50r, &_mul50i);
    double _c51r = 0, _c51i = 0;
    _c51r = 1.0; _c51i = 0;
    double _conj52r = 0, _conj52i = 0;
    _conj52r = x2r; _conj52i = -(x2i);
    double _mul53r = 0, _mul53i = 0;
    c_mul(x1r, x1i, _conj52r, _conj52i, &_mul53r, &_mul53i);
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(_mul53r, _mul53i); _abs54i = 0;
    double _add55r = 0, _add55i = 0;
    _add55r = _c51r + _abs54r; _add55i = _c51i + _abs54i;
    double _div56r = 0, _div56i = 0;
    c_div(_mul50r, _mul50i, _add55r, _add55i, &_div56r, &_div56i);
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div56r; cIm[_idx] = _div56i; } }
    double _c57r = 0, _c57i = 0;
    _c57r = 25.0; _c57i = 0;
    double _conj58r = 0, _conj58i = 0;
    _conj58r = x1r; _conj58i = -(x1i);
    double _conj59r = 0, _conj59i = 0;
    _conj59r = x2r; _conj59i = -(x2i);
    double _add60r = 0, _add60i = 0;
    _add60r = _conj58r + _conj59r; _add60i = _conj58i + _conj59i;
    double _mul61r = 0, _mul61i = 0;
    c_mul(_c57r, _c57i, _add60r, _add60i, &_mul61r, &_mul61i);
    double _mul62r = 0, _mul62i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul62r, &_mul62i);
    double _abs63r = 0, _abs63i = 0;
    _abs63r = c_abs(_mul62r, _mul62i); _abs63i = 0;
    double _div64r = 0, _div64i = 0;
    c_div(_mul61r, _mul61i, _abs63r, _abs63i, &_div64r, &_div64i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div64r; cIm[_idx] = _div64i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_70_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(x2r, x2i); _abs9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_abs8r, _abs8i, _abs9r, _abs9i, &_mul10r, &_mul10i);
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 6;
        double _add11r = 0, _add11i = 0;
        _add11r = x1r + x2r; _add11i = x1i + x2i;
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(_add11r, _add11i); _ang12i = 0;
        double _arange13r = 0, _arange13i = 0;
        _arange13r = (double)(_si + 6); _arange13i = 0;
        double _smul14r = 0, _smul14i = 0;
        c_mul(_ang12r, _ang12i, _arange13r, _arange13i, &_smul14r, &_smul14i);
        cRe[_si_idx] = _smul14r; cIm[_si_idx] = _smul14i;
    }
    double _conj15r = 0, _conj15i = 0;
    _conj15r = x1r; _conj15i = -(x1i);
    double _conj16r = 0, _conj16i = 0;
    _conj16r = x2r; _conj16i = -(x2i);
    double _add17r = 0, _add17i = 0;
    _add17r = _conj15r + _conj16r; _add17i = _conj15i + _conj16i;
    { int _idx = 11; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 12;
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 1.0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_c18r, _c18i, x2r, x2i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = x1r + _mul19r; _add20i = x1i + _mul19i;
        double _re21r = 0, _re21i = 0;
        _re21r = _add20r; _re21i = 0;
        double _arange22r = 0, _arange22i = 0;
        _arange22r = (double)(_si + 1); _arange22i = 0;
        double _smul23r = 0, _smul23i = 0;
        c_mul(_re21r, _re21i, _arange22r, _arange22i, &_smul23r, &_smul23i);
        cRe[_si_idx] = _smul23r; cIm[_si_idx] = _smul23i;
    }
    double _ang24r = 0, _ang24i = 0;
    _ang24r = c_arg(x1r, x1i); _ang24i = 0;
    double _ang25r = 0, _ang25i = 0;
    _ang25r = c_arg(x2r, x2i); _ang25i = 0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_ang24r, _ang24i, _ang25r, _ang25i, &_mul26r, &_mul26i);
    { int _idx = 17; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 18;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, x2r, x2i, &_mul28r, &_mul28i);
        double _sub29r = 0, _sub29i = 0;
        _sub29r = x1r - _mul28r; _sub29i = x1i - _mul28i;
        double _im30r = 0, _im30i = 0;
        _im30r = _sub29i; _im30i = 0;
        double _arange31r = 0, _arange31i = 0;
        _arange31r = (double)(_si + 1); _arange31i = 0;
        double _smul32r = 0, _smul32i = 0;
        c_mul(_im30r, _im30i, _arange31r, _arange31i, &_smul32r, &_smul32i);
        cRe[_si_idx] = _smul32r; cIm[_si_idx] = _smul32i;
    }
    double _mul33r = 0, _mul33i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul33r, &_mul33i);
    double _conj34r = 0, _conj34i = 0;
    _conj34r = _mul33r; _conj34i = -(_mul33i);
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _conj34r; cIm[_idx] = _conj34i; } }
    double _cf35r = 0, _cf35i = 0;
    { int _idx = 11; if (_idx >= 0 && _idx < 25) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
    double _abs36r = 0, _abs36i = 0;
    _abs36r = c_abs(_cf35r, _cf35i); _abs36i = 0;
    double _cf37r = 0, _cf37i = 0;
    { int _idx = 17; if (_idx >= 0 && _idx < 25) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
    double _ang38r = 0, _ang38i = 0;
    _ang38r = c_arg(_cf37r, _cf37i); _ang38i = 0;
    double _add39r = 0, _add39i = 0;
    _add39r = _abs36r + _ang38r; _add39i = _abs36i + _ang38i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_71: auto-stubbed (unhandled constructs in source) */
static void poly_giga_71_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_72_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
        double v_r = _add11r, v_i = _add11i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(v_r, v_i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1e-10; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _div15r = 0, _div15i = 0;
        c_div(v_r, v_i, _add14r, _add14i, &_div15r, &_div15i);
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

static void poly_giga_73_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _sub2r = 0, _sub2i = 0;
    _sub2r = x1r - x2r; _sub2i = x1i - x2i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub2r; cIm[_idx] = _sub2i; } }
    double _mul3r = 0, _mul3i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul3r, &_mul3i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    double _div4r = 0, _div4i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div4r, &_div4i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div4r; cIm[_idx] = _div4i; } }
    double _add5r = 0, _add5i = 0;
    _add5r = x1r + x2r; _add5i = x1i + x2i;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(_add5r, _add5i, _add5r, _add5i, &_pow7r, &_pow7i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow7r; cIm[_idx] = _pow7i; } }
    double _sub8r = 0, _sub8i = 0;
    _sub8r = x1r - x2r; _sub8i = x1i - x2i;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(_sub8r, _sub8i, _sub8r, _sub8i, &_pow10r, &_pow10i);
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow10r; cIm[_idx] = _pow10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow12r, &_pow12i);
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow14r, &_pow14i);
    double _add15r = 0, _add15i = 0;
    _add15r = _pow12r + _pow14r; _add15i = _pow12i + _pow14i;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add15r; cIm[_idx] = _add15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 2.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow17r, &_pow17i);
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow19r, &_pow19i);
    double _sub20r = 0, _sub20i = 0;
    _sub20r = _pow17r - _pow19r; _sub20i = _pow17i - _pow19i;
    { int _idx = 7; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub20r; cIm[_idx] = _sub20i; } }
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
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(_add25r, _add25i, _add25r, _add25i, &_pow27r, &_pow27i);
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow27r; cIm[_idx] = _pow27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 2.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow29r, &_pow29i);
    double _c30r = 0, _c30i = 0;
    _c30r = 2.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow31r, &_pow31i);
    double _sub32r = 0, _sub32i = 0;
    _sub32r = _pow29r - _pow31r; _sub32i = _pow29i - _pow31i;
    double _c33r = 0, _c33i = 0;
    _c33r = 2.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_sub32r, _sub32i, _sub32r, _sub32i, &_pow34r, &_pow34i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow34r; cIm[_idx] = _pow34i; } }
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _c36r = 0, _c36i = 0;
    _c36r = 0.0; _c36i = 1.0;
    double _add37r = 0, _add37i = 0;
    _add37r = _add35r + _c36r; _add37i = _add35i + _c36i;
    double _c38r = 0, _c38i = 0;
    _c38r = 2.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(_add37r, _add37i, _add37r, _add37i, &_pow39r, &_pow39i);
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow39r; cIm[_idx] = _pow39i; } }
    double _sub40r = 0, _sub40i = 0;
    _sub40r = x1r - x2r; _sub40i = x1i - x2i;
    double _c41r = 0, _c41i = 0;
    _c41r = 0.0; _c41i = 1.0;
    double _sub42r = 0, _sub42i = 0;
    _sub42r = _sub40r - _c41r; _sub42i = _sub40i - _c41i;
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(_sub42r, _sub42i, _sub42r, _sub42i, &_pow44r, &_pow44i);
    { int _idx = 11; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow44r; cIm[_idx] = _pow44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 0.0; _c45i = 1.0;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c45r, _c45i, x2r, x2i, &_mul46r, &_mul46i);
    double _add47r = 0, _add47i = 0;
    _add47r = x1r + _mul46r; _add47i = x1i + _mul46i;
    double _c48r = 0, _c48i = 0;
    _c48r = 3.0; _c48i = 0;
    double _pow49r = 0, _pow49i = 0;
    c_mul(_add47r, _add47i, _add47r, _add47i, &_pow49r, &_pow49i);
    c_mul(_pow49r, _pow49i, _add47r, _add47i, &_pow49r, &_pow49i);
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow49r; cIm[_idx] = _pow49i; } }
    double _c50r = 0, _c50i = 0;
    _c50r = 0.0; _c50i = 1.0;
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c50r, _c50i, x1r, x1i, &_mul51r, &_mul51i);
    double _sub52r = 0, _sub52i = 0;
    _sub52r = _mul51r - x2r; _sub52i = _mul51i - x2i;
    double _c53r = 0, _c53i = 0;
    _c53r = 3.0; _c53i = 0;
    double _pow54r = 0, _pow54i = 0;
    c_mul(_sub52r, _sub52i, _sub52r, _sub52i, &_pow54r, &_pow54i);
    c_mul(_pow54r, _pow54i, _sub52r, _sub52i, &_pow54r, &_pow54i);
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow54r; cIm[_idx] = _pow54i; } }
    double _add55r = 0, _add55i = 0;
    _add55r = x1r + x2r; _add55i = x1i + x2i;
    double _c56r = 0, _c56i = 0;
    _c56r = 3.0; _c56i = 0;
    double _pow57r = 0, _pow57i = 0;
    c_mul(_add55r, _add55i, _add55r, _add55i, &_pow57r, &_pow57i);
    c_mul(_pow57r, _pow57i, _add55r, _add55i, &_pow57r, &_pow57i);
    double _sub58r = 0, _sub58i = 0;
    _sub58r = x1r - x2r; _sub58i = x1i - x2i;
    double _c59r = 0, _c59i = 0;
    _c59r = 3.0; _c59i = 0;
    double _pow60r = 0, _pow60i = 0;
    c_mul(_sub58r, _sub58i, _sub58r, _sub58i, &_pow60r, &_pow60i);
    c_mul(_pow60r, _pow60i, _sub58r, _sub58i, &_pow60r, &_pow60i);
    double _add61r = 0, _add61i = 0;
    _add61r = _pow57r + _pow60r; _add61i = _pow57i + _pow60i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add61r; cIm[_idx] = _add61i; } }
    double _mul62r = 0, _mul62i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul62r, &_mul62i);
    double _c63r = 0, _c63i = 0;
    _c63r = 3.0; _c63i = 0;
    double _pow64r = 0, _pow64i = 0;
    c_mul(_mul62r, _mul62i, _mul62r, _mul62i, &_pow64r, &_pow64i);
    c_mul(_pow64r, _pow64i, _mul62r, _mul62i, &_pow64r, &_pow64i);
    double _c65r = 0, _c65i = 0;
    _c65r = 0.0; _c65i = 1.0;
    double _mul66r = 0, _mul66i = 0;
    c_mul(_c65r, _c65i, x1r, x1i, &_mul66r, &_mul66i);
    double _mul67r = 0, _mul67i = 0;
    c_mul(_mul66r, _mul66i, x2r, x2i, &_mul67r, &_mul67i);
    double _sub68r = 0, _sub68i = 0;
    _sub68r = _pow64r - _mul67r; _sub68i = _pow64i - _mul67i;
    { int _idx = 15; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub68r; cIm[_idx] = _sub68i; } }
    double _div69r = 0, _div69i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div69r, &_div69i);
    double _c70r = 0, _c70i = 0;
    _c70r = 4.0; _c70i = 0;
    double _pow71r = 0, _pow71i = 0;
    c_mul(_div69r, _div69i, _div69r, _div69i, &_pow71r, &_pow71i);
    c_mul(_pow71r, _pow71i, _pow71r, _pow71i, &_pow71r, &_pow71i);
    double _c72r = 0, _c72i = 0;
    _c72r = 0.0; _c72i = 1.0;
    double _mul73r = 0, _mul73i = 0;
    c_mul(_c72r, _c72i, x1r, x1i, &_mul73r, &_mul73i);
    double _div74r = 0, _div74i = 0;
    c_div(_mul73r, _mul73i, x2r, x2i, &_div74r, &_div74i);
    double _add75r = 0, _add75i = 0;
    _add75r = _pow71r + _div74r; _add75i = _pow71i + _div74i;
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add75r; cIm[_idx] = _add75i; } }
    double _mul76r = 0, _mul76i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul76r, &_mul76i);
    double _c77r = 0, _c77i = 0;
    _c77r = 0.0; _c77i = 1.0;
    double _add78r = 0, _add78i = 0;
    _add78r = _mul76r + _c77r; _add78i = _mul76i + _c77i;
    double _c79r = 0, _c79i = 0;
    _c79r = 4.0; _c79i = 0;
    double _pow80r = 0, _pow80i = 0;
    c_mul(_add78r, _add78i, _add78r, _add78i, &_pow80r, &_pow80i);
    c_mul(_pow80r, _pow80i, _pow80r, _pow80i, &_pow80r, &_pow80i);
    double _mul81r = 0, _mul81i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul81r, &_mul81i);
    double _sub82r = 0, _sub82i = 0;
    _sub82r = _pow80r - _mul81r; _sub82i = _pow80i - _mul81i;
    { int _idx = 17; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub82r; cIm[_idx] = _sub82i; } }
    double _add83r = 0, _add83i = 0;
    _add83r = x1r + x2r; _add83i = x1i + x2i;
    double _c84r = 0, _c84i = 0;
    _c84r = 0.0; _c84i = 1.0;
    double _add85r = 0, _add85i = 0;
    _add85r = _add83r + _c84r; _add85i = _add83i + _c84i;
    double _c86r = 0, _c86i = 0;
    _c86r = 5.0; _c86i = 0;
    double _pow87r = 0, _pow87i = 0;
    c_powr(_add85r, _add85i, 5.0, &_pow87r, &_pow87i);
    double _add88r = 0, _add88i = 0;
    _add88r = x1r + x2r; _add88i = x1i + x2i;
    double _sub89r = 0, _sub89i = 0;
    _sub89r = _pow87r - _add88r; _sub89i = _pow87i - _add88i;
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub89r; cIm[_idx] = _sub89i; } }
    double _sub90r = 0, _sub90i = 0;
    _sub90r = x1r - x2r; _sub90i = x1i - x2i;
    double _c91r = 0, _c91i = 0;
    _c91r = 0.0; _c91i = 1.0;
    double _sub92r = 0, _sub92i = 0;
    _sub92r = _sub90r - _c91r; _sub92i = _sub90i - _c91i;
    double _c93r = 0, _c93i = 0;
    _c93r = 5.0; _c93i = 0;
    double _pow94r = 0, _pow94i = 0;
    c_powr(_sub92r, _sub92i, 5.0, &_pow94r, &_pow94i);
    double _sub95r = 0, _sub95i = 0;
    _sub95r = x1r - x2r; _sub95i = x1i - x2i;
    double _add96r = 0, _add96i = 0;
    _add96r = _pow94r + _sub95r; _add96i = _pow94i + _sub95i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add96r; cIm[_idx] = _add96i; } }
    double _c97r = 0, _c97i = 0;
    _c97r = 0.0; _c97i = 1.0;
    double _mul98r = 0, _mul98i = 0;
    c_mul(_c97r, _c97i, x2r, x2i, &_mul98r, &_mul98i);
    double _add99r = 0, _add99i = 0;
    _add99r = x1r + _mul98r; _add99i = x1i + _mul98i;
    double _c100r = 0, _c100i = 0;
    _c100r = 6.0; _c100i = 0;
    double _pow101r = 0, _pow101i = 0;
    c_powr(_add99r, _add99i, 6.0, &_pow101r, &_pow101i);
    double _c102r = 0, _c102i = 0;
    _c102r = 0.0; _c102i = 1.0;
    double _mul103r = 0, _mul103i = 0;
    c_mul(_c102r, _c102i, x1r, x1i, &_mul103r, &_mul103i);
    double _mul104r = 0, _mul104i = 0;
    c_mul(_mul103r, _mul103i, x2r, x2i, &_mul104r, &_mul104i);
    double _sub105r = 0, _sub105i = 0;
    _sub105r = _pow101r - _mul104r; _sub105i = _pow101i - _mul104i;
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub105r; cIm[_idx] = _sub105i; } }
    double _c106r = 0, _c106i = 0;
    _c106r = 0.0; _c106i = 1.0;
    double _mul107r = 0, _mul107i = 0;
    c_mul(_c106r, _c106i, x1r, x1i, &_mul107r, &_mul107i);
    double _sub108r = 0, _sub108i = 0;
    _sub108r = _mul107r - x2r; _sub108i = _mul107i - x2i;
    double _c109r = 0, _c109i = 0;
    _c109r = 6.0; _c109i = 0;
    double _pow110r = 0, _pow110i = 0;
    c_powr(_sub108r, _sub108i, 6.0, &_pow110r, &_pow110i);
    double _mul111r = 0, _mul111i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul111r, &_mul111i);
    double _add112r = 0, _add112i = 0;
    _add112r = _pow110r + _mul111r; _add112i = _pow110i + _mul111i;
    { int _idx = 21; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add112r; cIm[_idx] = _add112i; } }
    double _add113r = 0, _add113i = 0;
    _add113r = x1r + x2r; _add113i = x1i + x2i;
    double _c114r = 0, _c114i = 0;
    _c114r = 7.0; _c114i = 0;
    double _pow115r = 0, _pow115i = 0;
    c_powr(_add113r, _add113i, 7.0, &_pow115r, &_pow115i);
    double _sub116r = 0, _sub116i = 0;
    _sub116r = x1r - x2r; _sub116i = x1i - x2i;
    double _c117r = 0, _c117i = 0;
    _c117r = 7.0; _c117i = 0;
    double _pow118r = 0, _pow118i = 0;
    c_powr(_sub116r, _sub116i, 7.0, &_pow118r, &_pow118i);
    double _sub119r = 0, _sub119i = 0;
    _sub119r = _pow115r - _pow118r; _sub119i = _pow115i - _pow118i;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub119r; cIm[_idx] = _sub119i; } }
    double _mul120r = 0, _mul120i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul120r, &_mul120i);
    double _c121r = 0, _c121i = 0;
    _c121r = 8.0; _c121i = 0;
    double _pow122r = 0, _pow122i = 0;
    c_powr(_mul120r, _mul120i, 8.0, &_pow122r, &_pow122i);
    double _div123r = 0, _div123i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div123r, &_div123i);
    double _c124r = 0, _c124i = 0;
    _c124r = 8.0; _c124i = 0;
    double _pow125r = 0, _pow125i = 0;
    c_powr(_div123r, _div123i, 8.0, &_pow125r, &_pow125i);
    double _sub126r = 0, _sub126i = 0;
    _sub126r = _pow122r - _pow125r; _sub126i = _pow122i - _pow125i;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub126r; cIm[_idx] = _sub126i; } }
    double _c127r = 0, _c127i = 0;
    _c127r = 0.0; _c127i = 1.0;
    double _mul128r = 0, _mul128i = 0;
    c_mul(_c127r, _c127i, x2r, x2i, &_mul128r, &_mul128i);
    double _add129r = 0, _add129i = 0;
    _add129r = x1r + _mul128r; _add129i = x1i + _mul128i;
    double _abs130r = 0, _abs130i = 0;
    _abs130r = c_abs(_add129r, _add129i); _abs130i = 0;
    double _log131r = 0, _log131i = 0;
    c_log(_abs130r, _abs130i, &_log131r, &_log131i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _log131r; cIm[_idx] = _log131i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_74_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _im4r = 0, _im4i = 0;
    _im4r = x2i; _im4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c3r, _c3i, _im4r, _im4i, &_mul5r, &_mul5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _re2r + _mul5r; _add6i = _re2i + _mul5i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add6r; cIm[_idx] = _add6i; } }
    double _abs7r = 0, _abs7i = 0;
    _abs7r = c_abs(x2r, x2i); _abs7i = 0;
    double _div8r = 0, _div8i = 0;
    c_div(x1r, x1i, _abs7r, _abs7i, &_div8r, &_div8i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div8r; cIm[_idx] = _div8i; } }
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(x2r, x2i); _abs9i = 0;
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 0;
    if (_abs9r != _c10r) {
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(x2r, x2i, _abs11r, _abs11i, &_div12r, &_div12i);
        { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div12r; cIm[_idx] = _div12i; } }
    }
    double _ang13r = 0, _ang13i = 0;
    _ang13r = c_arg(x1r, x1i); _ang13i = 0;
    double _ang14r = 0, _ang14i = 0;
    _ang14r = c_arg(x2r, x2i); _ang14i = 0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_ang13r, _ang13i, _ang14r, _ang14i, &_mul15r, &_mul15i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _conj16r = 0, _conj16i = 0;
    _conj16r = x1r; _conj16i = -(x1i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(_conj16r, _conj16i, x2r, x2i, &_mul17r, &_mul17i);
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(_mul17r, _mul17i); _abs18i = 0;
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs18r; cIm[_idx] = _abs18i; } }
    double _sub19r = 0, _sub19i = 0;
    _sub19r = x1r - x2r; _sub19i = x1i - x2i;
    double _abs20r = 0, _abs20i = 0;
    _abs20r = c_abs(_sub19r, _sub19i); _abs20i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs20r; cIm[_idx] = _abs20i; } }
    double _mul21r = 0, _mul21i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul21r, &_mul21i);
    double _ang22r = 0, _ang22i = 0;
    _ang22r = c_arg(_mul21r, _mul21i); _ang22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul23r, &_mul23i);
    double _ang24r = 0, _ang24i = 0;
    _ang24r = c_arg(_mul23r, _mul23i); _ang24i = 0;
    double _conj25r = 0, _conj25i = 0;
    _conj25r = _ang24r; _conj25i = -(_ang24i);
    double _add26r = 0, _add26i = 0;
    _add26r = _ang22r + _conj25r; _add26i = _ang22i + _conj25i;
    { int _idx = 7; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add26r; cIm[_idx] = _add26i; } }
    double _abs27r = 0, _abs27i = 0;
    _abs27r = c_abs(x1r, x1i); _abs27i = 0;
    double _log28r = 0, _log28i = 0;
    c_log(_abs27r, _abs27i, &_log28r, &_log28i);
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(x2r, x2i); _abs29i = 0;
    double _log30r = 0, _log30i = 0;
    c_log(_abs29r, _abs29i, &_log30r, &_log30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _log28r + _log30r; _add31i = _log28i + _log30i;
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(x1r, x1i); _abs32i = 0;
    double _im33r = 0, _im33i = 0;
    _im33r = x2i; _im33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_abs32r, _abs32i, _im33r, _im33i, &_mul34r, &_mul34i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    double _abs35r = 0, _abs35i = 0;
    _abs35r = c_abs(x1r, x1i); _abs35i = 0;
    double _c36r = 0, _c36i = 0;
    _c36r = 2.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(_abs35r, _abs35i, _abs35r, _abs35i, &_pow37r, &_pow37i);
    double _abs38r = 0, _abs38i = 0;
    _abs38r = c_abs(x2r, x2i); _abs38i = 0;
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(_abs38r, _abs38i, _abs38r, _abs38i, &_pow40r, &_pow40i);
    double _add41r = 0, _add41i = 0;
    _add41r = _pow37r + _pow40r; _add41i = _pow37i + _pow40i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    double _re42r = 0, _re42i = 0;
    _re42r = x1r; _re42i = 0;
    double _c43r = 0, _c43i = 0;
    _c43r = 3.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(_re42r, _re42i, _re42r, _re42i, &_pow44r, &_pow44i);
    c_mul(_pow44r, _pow44i, _re42r, _re42i, &_pow44r, &_pow44i);
    double _im45r = 0, _im45i = 0;
    _im45r = x2i; _im45i = 0;
    double _c46r = 0, _c46i = 0;
    _c46r = 2.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(_im45r, _im45i, _im45r, _im45i, &_pow47r, &_pow47i);
    double _add48r = 0, _add48i = 0;
    _add48r = _pow44r + _pow47r; _add48i = _pow44i + _pow47i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
    double _c49r = 0, _c49i = 0;
    _c49r = 0.0; _c49i = 1.0;
    double _abs50r = 0, _abs50i = 0;
    _abs50r = c_abs(x1r, x1i); _abs50i = 0;
    double _abs51r = 0, _abs51i = 0;
    _abs51r = c_abs(x2r, x2i); _abs51i = 0;
    double _add52r = 0, _add52i = 0;
    _add52r = _abs50r + _abs51r; _add52i = _abs50i + _abs51i;
    double _mul53r = 0, _mul53i = 0;
    c_mul(_c49r, _c49i, _add52r, _add52i, &_mul53r, &_mul53i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_75_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _abs1r = 0, _abs1i = 0;
    _abs1r = c_abs(x1r, x1i); _abs1i = 0;
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(x2r, x2i); _abs2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_abs1r, _abs1i, _abs2r, _abs2i, &_mul3r, &_mul3i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + x2r; _add4i = x1i + x2i;
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(_add4r, _add4i); _abs5i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs5r; cIm[_idx] = _abs5i; } }
    double _conj6r = 0, _conj6i = 0;
    _conj6r = x1r; _conj6i = -(x1i);
    double _conj7r = 0, _conj7i = 0;
    _conj7r = x2r; _conj7i = -(x2i);
    double _add8r = 0, _add8i = 0;
    _add8r = _conj6r + _conj7r; _add8i = _conj6i + _conj7i;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs9r; cIm[_idx] = _abs9i; } }
    double _ang10r = 0, _ang10i = 0;
    _ang10r = c_arg(x1r, x1i); _ang10i = 0;
    double _ang11r = 0, _ang11i = 0;
    _ang11r = c_arg(x2r, x2i); _ang11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_ang10r, _ang10i, _ang11r, _ang11i, &_mul12r, &_mul12i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    double _conj13r = 0, _conj13i = 0;
    _conj13r = x1r; _conj13i = -(x1i);
    double _conj14r = 0, _conj14i = 0;
    _conj14r = x2r; _conj14i = -(x2i);
    double _add15r = 0, _add15i = 0;
    _add15r = _conj13r + _conj14r; _add15i = _conj13i + _conj14i;
    double _ang16r = 0, _ang16i = 0;
    _ang16r = c_arg(_add15r, _add15i); _ang16i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _ang16r; cIm[_idx] = _ang16i; } }
    double _c17r = 0, _c17i = 0;
    _c17r = 0.0; _c17i = 1.0;
    double _add18r = 0, _add18i = 0;
    _add18r = x1r + x2r; _add18i = x1i + x2i;
    double _ang19r = 0, _ang19i = 0;
    _ang19r = c_arg(_add18r, _add18i); _ang19i = 0;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c17r, _c17i, _ang19r, _ang19i, &_mul20r, &_mul20i);
    double _exp21r = 0, _exp21i = 0;
    c_exp2(_mul20r, _mul20i, &_exp21r, &_exp21i);
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _exp21r; cIm[_idx] = _exp21i; } }
    double _c22r = 0, _c22i = 0;
    _c22r = 0.0; _c22i = 1.0;
    double _conj23r = 0, _conj23i = 0;
    _conj23r = x1r; _conj23i = -(x1i);
    double _conj24r = 0, _conj24i = 0;
    _conj24r = x2r; _conj24i = -(x2i);
    double _add25r = 0, _add25i = 0;
    _add25r = _conj23r + _conj24r; _add25i = _conj23i + _conj24i;
    double _ang26r = 0, _ang26i = 0;
    _ang26r = c_arg(_add25r, _add25i); _ang26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c22r, _c22i, _ang26r, _ang26i, &_mul27r, &_mul27i);
    double _exp28r = 0, _exp28i = 0;
    c_exp2(_mul27r, _mul27i, &_exp28r, &_exp28i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _exp28r; cIm[_idx] = _exp28i; } }
    double _conj29r = 0, _conj29i = 0;
    _conj29r = x1r; _conj29i = -(x1i);
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(_conj29r, _conj29i); _abs30i = 0;
    double _conj31r = 0, _conj31i = 0;
    _conj31r = x2r; _conj31i = -(x2i);
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_conj31r, _conj31i); _abs32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_abs30r, _abs30i, _abs32r, _abs32i, &_mul33r, &_mul33i);
    double _sin34r = 0, _sin34i = 0;
    c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
    { int _idx = 7; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sin34r; cIm[_idx] = _sin34i; } }
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _ang36r = 0, _ang36i = 0;
    _ang36r = c_arg(_add35r, _add35i); _ang36i = 0;
    double _cos37r = 0, _cos37i = 0;
    c_cos(_ang36r, _ang36i, &_cos37r, &_cos37i);
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cos37r; cIm[_idx] = _cos37i; } }
    double _add38r = 0, _add38i = 0;
    _add38r = x1r + x2r; _add38i = x1i + x2i;
    double _conj39r = 0, _conj39i = 0;
    _conj39r = _add38r; _conj39i = -(_add38i);
    double _abs40r = 0, _abs40i = 0;
    _abs40r = c_abs(_conj39r, _conj39i); _abs40i = 0;
    double _tanh41r = 0, _tanh41i = 0;
    _tanh41r = tanh(_abs40r); _tanh41i = 0; /* approx real tanh */
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _tanh41r; cIm[_idx] = _tanh41i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 10;
        double _abs42r = 0, _abs42i = 0;
        _abs42r = c_abs(x1r, x1i); _abs42i = 0;
        double _arange43r = 0, _arange43i = 0;
        _arange43r = (double)(_si + 11); _arange43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 10.0; _c44i = 0;
        double _sdiv45r = 0, _sdiv45i = 0;
        c_div(_arange43r, _arange43i, _c44r, _c44i, &_sdiv45r, &_sdiv45i);
        double _abs46r = 0, _abs46i = 0;
        _abs46r = c_abs(x1r, x1i); _abs46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 10.0; _c47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(0, 0, _c47r, _c47i, &_div48r, &_div48i);
        double _pow49r = 0, _pow49i = 0;
        c_powc(_abs46r, _abs46i, _div48r, _div48i, &_pow49r, &_pow49i);
        double _abs50r = 0, _abs50i = 0;
        _abs50r = c_abs(x2r, x2i); _abs50i = 0;
        double _arange51r = 0, _arange51i = 0;
        _arange51r = (double)(_si + 15); _arange51i = 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 10.0; _c52i = 0;
        double _sdiv53r = 0, _sdiv53i = 0;
        c_div(_arange51r, _arange51i, _c52r, _c52i, &_sdiv53r, &_sdiv53i);
        double _abs54r = 0, _abs54i = 0;
        _abs54r = c_abs(x2r, x2i); _abs54i = 0;
        double _c55r = 0, _c55i = 0;
        _c55r = 10.0; _c55i = 0;
        double _div56r = 0, _div56i = 0;
        c_div(0, 0, _c55r, _c55i, &_div56r, &_div56i);
        double _pow57r = 0, _pow57i = 0;
        c_powc(_abs54r, _abs54i, _div56r, _div56i, &_pow57r, &_pow57i);
        double _smul58r = 0, _smul58i = 0;
        c_mul(_pow49r, _pow49i, _pow57r, _pow57i, &_smul58r, &_smul58i);
        cRe[_si_idx] = _smul58r; cIm[_si_idx] = _smul58i;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 15;
        double _arange59r = 0, _arange59i = 0;
        _arange59r = (double)(_si + 16); _arange59i = 0;
        double _ang60r = 0, _ang60i = 0;
        _ang60r = c_arg(x1r, x1i); _ang60i = 0;
        double _ang61r = 0, _ang61i = 0;
        _ang61r = c_arg(x2r, x2i); _ang61i = 0;
        double _add62r = 0, _add62i = 0;
        _add62r = _ang60r + _ang61r; _add62i = _ang60i + _ang61i;
        double _smul63r = 0, _smul63i = 0;
        c_mul(_arange59r, _arange59i, _add62r, _add62i, &_smul63r, &_smul63i);
        double _c64r = 0, _c64i = 0;
        _c64r = 2.0; _c64i = 0;
        double _sdiv65r = 0, _sdiv65i = 0;
        c_div(_smul63r, _smul63i, _c64r, _c64i, &_sdiv65r, &_sdiv65i);
        cRe[_si_idx] = _sdiv65r; cIm[_si_idx] = _sdiv65i;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 20;
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
        double _add72r = 0, _add72i = 0;
        _add72r = _pow68r + _pow71r; _add72i = _pow68i + _pow71i;
        double _arange73r = 0, _arange73i = 0;
        _arange73r = (double)(_si + 21); _arange73i = 0;
        double _sadd74r = 0, _sadd74i = 0;
        _sadd74r = _add72r + _arange73r; _sadd74i = _add72i + _arange73i;
        cRe[_si_idx] = _sadd74r; cIm[_si_idx] = _sadd74i;
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_76_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int k = 1; k < 25; k++) {
        double _add2r = 0, _add2i = 0;
        _add2r = x1r + x2r; _add2i = x1i + x2i;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = k + _c3r; _add4i = 0 + _c3i;
        double _pow5r = 0, _pow5i = 0;
        c_powc(_add2r, _add2i, _add4r, _add4i, &_pow5r, &_pow5i);
        double _cf6r = 0, _cf6i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf6r = cRe[_idx]; _cf6i = cIm[_idx]; } }
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, _cf6r, _cf6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow5r + _sin8r; _add9i = _pow5i + _sin8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, x1r, x1i, &_mul10r, &_mul10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_mul10r, _mul10i); _abs11i = 0;
        double _log12r = 0, _log12i = 0;
        c_log(_abs11r, _abs11i, &_log12r, &_log12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _add9r + _log12r; _add13i = _add9i + _log12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = k + _c14r; _add15i = 0 + _c14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_add15r, _add15i, x2r, x2i, &_mul16r, &_mul16i);
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_mul16r, _mul16i); _abs17i = 0;
        double _log18r = 0, _log18i = 0;
        c_log(_abs17r, _abs17i, &_log18r, &_log18i);
        double _sub19r = 0, _sub19i = 0;
        _sub19r = _add13r - _log18r; _sub19i = _add13i - _log18i;
        double v_r = _sub19r, v_i = _sub19i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(v_r, v_i); _abs20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(v_r, v_i, _abs20r, _abs20i, &_div21r, &_div21i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div21r; cIm[_idx] = _div21i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_77_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 0;
        double _arange1r = 0, _arange1i = 0;
        _arange1r = (double)(_si + 1); _arange1i = 0;
        double _smul2r = 0, _smul2i = 0;
        c_mul(_arange1r, _arange1i, x1r, x1i, &_smul2r, &_smul2i);
        double _arange3r = 0, _arange3i = 0;
        _arange3r = (double)(_si + 11); _arange3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 1.0;
        double _smul5r = 0, _smul5i = 0;
        c_mul(_arange3r, _arange3i, _c4r, _c4i, &_smul5r, &_smul5i);
        double _smul6r = 0, _smul6i = 0;
        c_mul(_smul5r, _smul5i, x2r, x2i, &_smul6r, &_smul6i);
        double _sadd7r = 0, _sadd7i = 0;
        _sadd7r = _smul2r + _smul6r; _sadd7i = _smul2i + _smul6i;
        cRe[_si_idx] = _sadd7r; cIm[_si_idx] = _sadd7i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 10;
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 1.0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_c8r, _c8i, x2r, x2i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = x1r + _mul9r; _add10i = x1i + _mul9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(_add10r, _add10i, _add10r, _add10i, &_pow12r, &_pow12i);
        double _arange13r = 0, _arange13i = 0;
        _arange13r = (double)(_si + 11); _arange13i = 0;
        double _smul14r = 0, _smul14i = 0;
        c_mul(_pow12r, _pow12i, _arange13r, _arange13i, &_smul14r, &_smul14i);
        cRe[_si_idx] = _smul14r; cIm[_si_idx] = _smul14i;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 20;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _ang16r; _add17i = _abs15i + _ang16i;
        double _arange18r = 0, _arange18i = 0;
        _arange18r = (double)(_si + 1); _arange18i = 0;
        double _smul19r = 0, _smul19i = 0;
        c_mul(_add17r, _add17i, _arange18r, _arange18i, &_smul19r, &_smul19i);
        cRe[_si_idx] = _smul19r; cIm[_si_idx] = _smul19i;
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_78_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double _add3r = 0, _add3i = 0;
        _add3r = _add2r + x1r; _add3i = _add2i + x1i;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + x2r; _add6i = _add5i + x2i;
        double _div7r = 0, _div7i = 0;
        c_div(_add3r, _add3i, _add6r, _add6i, &_div7r, &_div7i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div7r; cIm[_idx] = _div7i; } }
    }
    double _cf8r = 0, _cf8i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
    double _add9r = 0, _add9i = 0;
    _add9r = x1r + x2r; _add9i = x1i + x2i;
    double _abs10r = 0, _abs10i = 0;
    _abs10r = c_abs(_add9r, _add9i); _abs10i = 0;
    double _log11r = 0, _log11i = 0;
    c_log(_abs10r, _abs10i, &_log11r, &_log11i);
    double _add12r = 0, _add12i = 0;
    _add12r = _cf8r + _log11r; _add12i = _cf8i + _log11i;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
    double _cf13r = 0, _cf13i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
    double _re14r = 0, _re14i = 0;
    _re14r = x1r; _re14i = 0;
    double _sin15r = 0, _sin15i = 0;
    c_sin(_re14r, _re14i, &_sin15r, &_sin15i);
    double _add16r = 0, _add16i = 0;
    _add16r = _cf13r + _sin15r; _add16i = _cf13i + _sin15i;
    double _im17r = 0, _im17i = 0;
    _im17r = x2i; _im17i = 0;
    double _cos18r = 0, _cos18i = 0;
    c_cos(_im17r, _im17i, &_cos18r, &_cos18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _add16r + _cos18r; _add19i = _add16i + _cos18i;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    double _cf20r = 0, _cf20i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
    double _cf21r = 0, _cf21i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(_cf21r, _cf21i); _abs22i = 0;
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(_abs22r, _abs22i, _abs22r, _abs22i, &_pow24r, &_pow24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _cf20r + _pow24r; _add25i = _cf20i + _pow24i;
    double _cf26r = 0, _cf26i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
    double _ang27r = 0, _ang27i = 0;
    _ang27r = c_arg(_cf26r, _cf26i); _ang27i = 0;
    double _c28r = 0, _c28i = 0;
    _c28r = 2.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(_ang27r, _ang27i, _ang27r, _ang27i, &_pow29r, &_pow29i);
    double _add30r = 0, _add30i = 0;
    _add30r = _add25r + _pow29r; _add30i = _add25i + _pow29i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    double _cf31r = 0, _cf31i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
    double _re32r = 0, _re32i = 0;
    _re32r = x2r; _re32i = 0;
    double _im33r = 0, _im33i = 0;
    _im33r = x1i; _im33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_re32r, _re32i, _im33r, _im33i, &_mul34r, &_mul34i);
    double _abs35r = 0, _abs35i = 0;
    _abs35r = c_abs(_mul34r, _mul34i); _abs35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _cf31r + _abs35r; _add36i = _cf31i + _abs35i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    double _cf37r = 0, _cf37i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
    double _conj38r = 0, _conj38i = 0;
    _conj38r = x2r; _conj38i = -(x2i);
    double _add39r = 0, _add39i = 0;
    _add39r = x1r + _conj38r; _add39i = x1i + _conj38i;
    double _abs40r = 0, _abs40i = 0;
    _abs40r = c_abs(_add39r, _add39i); _abs40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = _cf37r + _abs40r; _add41i = _cf37i + _abs40i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_79_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 0;
        double _arange1r = 0, _arange1i = 0;
        _arange1r = (double)(_si + 1); _arange1i = 0;
        double _smul2r = 0, _smul2i = 0;
        c_mul(_arange1r, _arange1i, x1r, x1i, &_smul2r, &_smul2i);
        double _arange3r = 0, _arange3i = 0;
        _arange3r = (double)(_si + 11); _arange3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 1.0;
        double _smul5r = 0, _smul5i = 0;
        c_mul(_arange3r, _arange3i, _c4r, _c4i, &_smul5r, &_smul5i);
        double _smul6r = 0, _smul6i = 0;
        c_mul(_smul5r, _smul5i, x2r, x2i, &_smul6r, &_smul6i);
        double _sadd7r = 0, _sadd7i = 0;
        _sadd7r = _smul2r + _smul6r; _sadd7i = _smul2i + _smul6i;
        cRe[_si_idx] = _sadd7r; cIm[_si_idx] = _sadd7i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 10;
        double _c8r = 0, _c8i = 0;
        _c8r = 0.0; _c8i = 1.0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_c8r, _c8i, x2r, x2i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = x1r + _mul9r; _add10i = x1i + _mul9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 2.0; _c11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_mul(_add10r, _add10i, _add10r, _add10i, &_pow12r, &_pow12i);
        double _arange13r = 0, _arange13i = 0;
        _arange13r = (double)(_si + 11); _arange13i = 0;
        double _smul14r = 0, _smul14i = 0;
        c_mul(_pow12r, _pow12i, _arange13r, _arange13i, &_smul14r, &_smul14i);
        cRe[_si_idx] = _smul14r; cIm[_si_idx] = _smul14i;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 20;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _ang16r; _add17i = _abs15i + _ang16i;
        double _arange18r = 0, _arange18i = 0;
        _arange18r = (double)(_si + 1); _arange18i = 0;
        double _smul19r = 0, _smul19i = 0;
        c_mul(_add17r, _add17i, _arange18r, _arange18i, &_smul19r, &_smul19i);
        cRe[_si_idx] = _smul19r; cIm[_si_idx] = _smul19i;
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_80_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double _add3r = 0, _add3i = 0;
        _add3r = _add2r + x1r; _add3i = _add2i + x1i;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        double _add6r = 0, _add6i = 0;
        _add6r = _add5r + x2r; _add6i = _add5i + x2i;
        double _div7r = 0, _div7i = 0;
        c_div(_add3r, _add3i, _add6r, _add6i, &_div7r, &_div7i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div7r; cIm[_idx] = _div7i; } }
    }
    double _cf8r = 0, _cf8i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
    double _add9r = 0, _add9i = 0;
    _add9r = x1r + x2r; _add9i = x1i + x2i;
    double _abs10r = 0, _abs10i = 0;
    _abs10r = c_abs(_add9r, _add9i); _abs10i = 0;
    double _log11r = 0, _log11i = 0;
    c_log(_abs10r, _abs10i, &_log11r, &_log11i);
    double _add12r = 0, _add12i = 0;
    _add12r = _cf8r + _log11r; _add12i = _cf8i + _log11i;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
    double _cf13r = 0, _cf13i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
    double _re14r = 0, _re14i = 0;
    _re14r = x1r; _re14i = 0;
    double _sin15r = 0, _sin15i = 0;
    c_sin(_re14r, _re14i, &_sin15r, &_sin15i);
    double _add16r = 0, _add16i = 0;
    _add16r = _cf13r + _sin15r; _add16i = _cf13i + _sin15i;
    double _im17r = 0, _im17i = 0;
    _im17r = x2i; _im17i = 0;
    double _cos18r = 0, _cos18i = 0;
    c_cos(_im17r, _im17i, &_cos18r, &_cos18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _add16r + _cos18r; _add19i = _add16i + _cos18i;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    double _cf20r = 0, _cf20i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
    double _cf21r = 0, _cf21i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(_cf21r, _cf21i); _abs22i = 0;
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(_abs22r, _abs22i, _abs22r, _abs22i, &_pow24r, &_pow24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _cf20r + _pow24r; _add25i = _cf20i + _pow24i;
    double _cf26r = 0, _cf26i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
    double _ang27r = 0, _ang27i = 0;
    _ang27r = c_arg(_cf26r, _cf26i); _ang27i = 0;
    double _c28r = 0, _c28i = 0;
    _c28r = 2.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(_ang27r, _ang27i, _ang27r, _ang27i, &_pow29r, &_pow29i);
    double _add30r = 0, _add30i = 0;
    _add30r = _add25r + _pow29r; _add30i = _add25i + _pow29i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
    double _cf31r = 0, _cf31i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
    double _re32r = 0, _re32i = 0;
    _re32r = x2r; _re32i = 0;
    double _im33r = 0, _im33i = 0;
    _im33r = x1i; _im33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_re32r, _re32i, _im33r, _im33i, &_mul34r, &_mul34i);
    double _abs35r = 0, _abs35i = 0;
    _abs35r = c_abs(_mul34r, _mul34i); _abs35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _cf31r + _abs35r; _add36i = _cf31i + _abs35i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    double _cf37r = 0, _cf37i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
    double _conj38r = 0, _conj38i = 0;
    _conj38r = x2r; _conj38i = -(x2i);
    double _add39r = 0, _add39i = 0;
    _add39r = x1r + _conj38r; _add39i = x1i + _conj38i;
    double _abs40r = 0, _abs40i = 0;
    _abs40r = c_abs(_add39r, _add39i); _abs40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = _cf37r + _abs40r; _add41i = _cf37i + _abs40i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_81_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 25; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = i + _c1r; _add2i = 0 + _c1i;
        double _mul3r = 0, _mul3i = 0;
        c_mul(x1r, x1i, _add2r, _add2i, &_mul3r, &_mul3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = i + _c4r; _add5i = 0 + _c4i;
        double _c6r = 0, _c6i = 0;
        _c6r = 2.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_add5r, _add5i, _c6r, _c6i, &_div7r, &_div7i);
        double _pow8r = 0, _pow8i = 0;
        c_powc(x2r, x2i, _div7r, _div7i, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul3r + _pow8r; _add9i = _mul3i + _pow8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = i + _c10r; _add11i = 0 + _c10i;
        double _mul12r = 0, _mul12i = 0;
        c_mul(x1r, x1i, _add11r, _add11i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = i + _c13r; _add14i = 0 + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powc(x2r, x2i, _add14r, _add14i, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul12r + _pow15r; _add16i = _mul12i + _pow15i;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_add16r, _add16i); _abs17i = 0;
        double _div18r = 0, _div18i = 0;
        c_div(_add9r, _add9i, _abs17r, _abs17i, &_div18r, &_div18i);
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div18r; cIm[_idx] = _div18i; } }
    }
    double _re19r = 0, _re19i = 0;
    _re19r = x1r; _re19i = 0;
    double _im20r = 0, _im20i = 0;
    _im20r = x2i; _im20i = 0;
    double _add21r = 0, _add21i = 0;
    _add21r = _re19r + _im20r; _add21i = _re19i + _im20i;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    double _c22r = 0, _c22i = 0;
    _c22r = 0.0; _c22i = 1.0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul23r, &_mul23i);
    double _ang24r = 0, _ang24i = 0;
    _ang24r = c_arg(_mul23r, _mul23i); _ang24i = 0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(_c22r, _c22i, _ang24r, _ang24i, &_mul25r, &_mul25i);
    double _exp26r = 0, _exp26i = 0;
    c_exp2(_mul25r, _mul25i, &_exp26r, &_exp26i);
    double _abs27r = 0, _abs27i = 0;
    _abs27r = c_abs(_exp26r, _exp26i); _abs27i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs27r; cIm[_idx] = _abs27i; } }
    double _mul28r = 0, _mul28i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul28r, &_mul28i);
    double _re29r = 0, _re29i = 0;
    _re29r = _mul28r; _re29i = 0;
    double _div30r = 0, _div30i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div30r, &_div30i);
    double _im31r = 0, _im31i = 0;
    _im31r = _div30i; _im31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = _re29r + _im31r; _add32i = _re29i + _im31i;
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 4.0; _c33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c33r, _c33i, x2r, x2i, &_mul34r, &_mul34i);
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + _mul34r; _add35i = x1i + _mul34i;
    double _ang36r = 0, _ang36i = 0;
    _ang36r = c_arg(_add35r, _add35i); _ang36i = 0;
    double _c37r = 0, _c37i = 0;
    _c37r = 4.0; _c37i = 0;
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c37r, _c37i, x2r, x2i, &_mul38r, &_mul38i);
    double _sub39r = 0, _sub39i = 0;
    _sub39r = x1r - _mul38r; _sub39i = x1i - _mul38i;
    double _conj40r = 0, _conj40i = 0;
    _conj40r = _sub39r; _conj40i = -(_sub39i);
    double _abs41r = 0, _abs41i = 0;
    _abs41r = c_abs(_conj40r, _conj40i); _abs41i = 0;
    double _div42r = 0, _div42i = 0;
    c_div(_ang36r, _ang36i, _abs41r, _abs41i, &_div42r, &_div42i);
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div42r; cIm[_idx] = _div42i; } }
    double _c43r = 0, _c43i = 0;
    _c43r = 0.0; _c43i = 1.0;
    double _sub44r = 0, _sub44i = 0;
    _sub44r = x1r - x2r; _sub44i = x1i - x2i;
    double _ang45r = 0, _ang45i = 0;
    _ang45r = c_arg(_sub44r, _sub44i); _ang45i = 0;
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c43r, _c43i, _ang45r, _ang45i, &_mul46r, &_mul46i);
    double _exp47r = 0, _exp47i = 0;
    c_exp2(_mul46r, _mul46i, &_exp47r, &_exp47i);
    double _abs48r = 0, _abs48i = 0;
    _abs48r = c_abs(_exp47r, _exp47i); _abs48i = 0;
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs48r; cIm[_idx] = _abs48i; } }
    double _div49r = 0, _div49i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div49r, &_div49i);
    double _re50r = 0, _re50i = 0;
    _re50r = _div49r; _re50i = 0;
    double _mul51r = 0, _mul51i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul51r, &_mul51i);
    double _im52r = 0, _im52i = 0;
    _im52r = _mul51i; _im52i = 0;
    double _sub53r = 0, _sub53i = 0;
    _sub53r = _re50r - _im52r; _sub53i = _re50i - _im52i;
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub53r; cIm[_idx] = _sub53i; } }
    double _c54r = 0, _c54i = 0;
    _c54r = 0.0; _c54i = 1.0;
    double _add55r = 0, _add55i = 0;
    _add55r = x1r + x2r; _add55i = x1i + x2i;
    double _ang56r = 0, _ang56i = 0;
    _ang56r = c_arg(_add55r, _add55i); _ang56i = 0;
    double _mul57r = 0, _mul57i = 0;
    c_mul(_c54r, _c54i, _ang56r, _ang56i, &_mul57r, &_mul57i);
    double _exp58r = 0, _exp58i = 0;
    c_exp2(_mul57r, _mul57i, &_exp58r, &_exp58i);
    double _abs59r = 0, _abs59i = 0;
    _abs59r = c_abs(_exp58r, _exp58i); _abs59i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _abs59r; cIm[_idx] = _abs59i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_82: auto-stubbed (unhandled constructs in source) */
static void poly_giga_82_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_83_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x2r; _attr2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_attr1r, _attr1i, _attr2r, _attr2i, &_mul3r, &_mul3i);
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x2i; _attr5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_attr4r, _attr4i, _attr5r, _attr5i, &_mul6r, &_mul6i);
    double _add7r = 0, _add7i = 0;
    _add7r = _mul3r + _mul6r; _add7i = _mul3i + _mul6i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    double _abs8r = 0, _abs8i = 0;
    _abs8r = c_abs(x1r, x1i); _abs8i = 0;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(x2r, x2i); _abs9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_abs8r, _abs8i, _abs9r, _abs9i, &_mul10r, &_mul10i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _ang11r = 0, _ang11i = 0;
    _ang11r = c_arg(x1r, x1i); _ang11i = 0;
    double _ang12r = 0, _ang12i = 0;
    _ang12r = c_arg(x2r, x2i); _ang12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _ang11r + _ang12r; _add13i = _ang11i + _ang12i;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add13r; cIm[_idx] = _add13i; } }
    double _conj14r = 0, _conj14i = 0;
    _conj14r = x1r; _conj14i = -(x1i);
    double _attr15r = 0, _attr15i = 0;
    _attr15r = _conj14r; _attr15i = 0;
    double _conj16r = 0, _conj16i = 0;
    _conj16r = x2r; _conj16i = -(x2i);
    double _attr17r = 0, _attr17i = 0;
    _attr17r = _conj16i; _attr17i = 0;
    double _add18r = 0, _add18i = 0;
    _add18r = _attr15r + _attr17r; _add18i = _attr15i + _attr17i;
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add18r; cIm[_idx] = _add18i; } }
    for (int k = 4; k < 25; k++) {
        double _cf19r = 0, _cf19i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
        double _cf20r = 0, _cf20i = 0;
        { int _idx = (k - 4); if (_idx >= 0 && _idx < 25) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
        double _mul21r = 0, _mul21i = 0;
        c_mul(_cf19r, _cf19i, _cf20r, _cf20i, &_mul21r, &_mul21i);
        double _cf22r = 0, _cf22i = 0;
        { int _idx = (k - 3); if (_idx >= 0 && _idx < 25) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
        double _cf23r = 0, _cf23i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
        double _mul24r = 0, _mul24i = 0;
        c_mul(_cf22r, _cf22i, _cf23r, _cf23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
        double _cf26r = 0, _cf26i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
        double _cf27r = 0, _cf27i = 0;
        { int _idx = (k - 4); if (_idx >= 0 && _idx < 25) { _cf27r = cRe[_idx]; _cf27i = cIm[_idx]; } }
        double _mul28r = 0, _mul28i = 0;
        c_mul(_cf26r, _cf26i, _cf27r, _cf27i, &_mul28r, &_mul28i);
        double _cf29r = 0, _cf29i = 0;
        { int _idx = (k - 3); if (_idx >= 0 && _idx < 25) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
        double _cf30r = 0, _cf30i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
        double _mul31r = 0, _mul31i = 0;
        c_mul(_cf29r, _cf29i, _cf30r, _cf30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul28r + _mul31r; _add32i = _mul28i + _mul31i;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_add32r, _add32i); _abs33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_add25r, _add25i, _abs33r, _abs33i, &_div34r, &_div34i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div34r; cIm[_idx] = _div34i; } }
    }
    double _cf35r = 0, _cf35i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
    double _cf36r = 0, _cf36i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf36r = cRe[_idx]; _cf36i = cIm[_idx]; } }
    double _div37r = 0, _div37i = 0;
    c_div(_cf35r, _cf35i, _cf36r, _cf36i, &_div37r, &_div37i);
    double _cf38r = 0, _cf38i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
    double _cf39r = 0, _cf39i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
    double _div40r = 0, _div40i = 0;
    c_div(_cf38r, _cf38i, _cf39r, _cf39i, &_div40r, &_div40i);
    double _add41r = 0, _add41i = 0;
    _add41r = _div37r + _div40r; _add41i = _div37i + _div40i;
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    double _cf42r = 0, _cf42i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
    double _cf43r = 0, _cf43i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
    double _mul44r = 0, _mul44i = 0;
    c_mul(_cf42r, _cf42i, _cf43r, _cf43i, &_mul44r, &_mul44i);
    double _abs45r = 0, _abs45i = 0;
    _abs45r = c_abs(_mul44r, _mul44i); _abs45i = 0;
    double _c46r = 0, _c46i = 0;
    _c46r = 1.0; _c46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = _abs45r + _c46r; _add47i = _abs45i + _c46i;
    double _log48r = 0, _log48i = 0;
    c_log(_add47r, _add47i, &_log48r, &_log48i);
    double _cf49r = 0, _cf49i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { _cf49r = cRe[_idx]; _cf49i = cIm[_idx]; } }
    double _cf50r = 0, _cf50i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
    double _mul51r = 0, _mul51i = 0;
    c_mul(_cf49r, _cf49i, _cf50r, _cf50i, &_mul51r, &_mul51i);
    double _abs52r = 0, _abs52i = 0;
    _abs52r = c_abs(_mul51r, _mul51i); _abs52i = 0;
    double _c53r = 0, _c53i = 0;
    _c53r = 1.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = _abs52r + _c53r; _add54i = _abs52i + _c53i;
    double _log55r = 0, _log55i = 0;
    c_log(_add54r, _add54i, &_log55r, &_log55i);
    double _div56r = 0, _div56i = 0;
    c_div(_log48r, _log48i, _log55r, _log55i, &_div56r, &_div56i);
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div56r; cIm[_idx] = _div56i; } }
    double _cf57r = 0, _cf57i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
    double _cf58r = 0, _cf58i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { _cf58r = cRe[_idx]; _cf58i = cIm[_idx]; } }
    double _mul59r = 0, _mul59i = 0;
    c_mul(_cf57r, _cf57i, _cf58r, _cf58i, &_mul59r, &_mul59i);
    double _cf60r = 0, _cf60i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
    double _mul61r = 0, _mul61i = 0;
    c_mul(_mul59r, _mul59i, _cf60r, _cf60i, &_mul61r, &_mul61i);
    double _cf62r = 0, _cf62i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { _cf62r = cRe[_idx]; _cf62i = cIm[_idx]; } }
    double _mul63r = 0, _mul63i = 0;
    c_mul(_mul61r, _mul61i, _cf62r, _cf62i, &_mul63r, &_mul63i);
    double _cf64r = 0, _cf64i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf64r = cRe[_idx]; _cf64i = cIm[_idx]; } }
    double _cf65r = 0, _cf65i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { _cf65r = cRe[_idx]; _cf65i = cIm[_idx]; } }
    double _mul66r = 0, _mul66i = 0;
    c_mul(_cf64r, _cf64i, _cf65r, _cf65i, &_mul66r, &_mul66i);
    double _cf67r = 0, _cf67i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
    double _mul68r = 0, _mul68i = 0;
    c_mul(_mul66r, _mul66i, _cf67r, _cf67i, &_mul68r, &_mul68i);
    double _cf69r = 0, _cf69i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { _cf69r = cRe[_idx]; _cf69i = cIm[_idx]; } }
    double _mul70r = 0, _mul70i = 0;
    c_mul(_mul68r, _mul68i, _cf69r, _cf69i, &_mul70r, &_mul70i);
    double _abs71r = 0, _abs71i = 0;
    _abs71r = c_abs(_mul70r, _mul70i); _abs71i = 0;
    double _div72r = 0, _div72i = 0;
    c_div(_mul63r, _mul63i, _abs71r, _abs71i, &_div72r, &_div72i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div72r; cIm[_idx] = _div72i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_84: auto-stubbed (unhandled constructs in source) */
static void poly_giga_84_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_85_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _sin1r = 0, _sin1i = 0;
        c_sin(x1r, x1i, &_sin1r, &_sin1i);
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 1.0;
        double _attr3r = 0, _attr3i = 0;
        _attr3r = x2r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(_attr3r, _attr3i, _attr3r, _attr3i, &_pow5r, &_pow5i);
        double _attr6r = 0, _attr6i = 0;
        _attr6r = x2i; _attr6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_mul(_attr6r, _attr6i, _attr6r, _attr6i, &_pow8r, &_pow8i);
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _pow5r - _pow8r; _sub9i = _pow5i - _pow8i;
        double _cos10r = 0, _cos10i = 0;
        c_cos(_sub9r, _sub9i, &_cos10r, &_cos10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_c2r, _c2i, _cos10r, _cos10i, &_mul11r, &_mul11i);
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_mul(_abs12r, _abs12i, _abs12r, _abs12i, &_pow14r, &_pow14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_mul(_abs15r, _abs15i, _abs15r, _abs15i, &_pow17r, &_pow17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _pow14r + _pow17r; _add18i = _pow14i + _pow17i;
        double _sqrt19r = 0, _sqrt19i = 0;
        c_powr(_add18r, _add18i, 0.5, &_sqrt19r, &_sqrt19i);
        double _div20r = 0, _div20i = 0;
        c_div(_mul11r, _mul11i, _sqrt19r, _sqrt19i, &_div20r, &_div20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _sin1r + _div20r; _add21i = _sin1i + _div20i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    }
    double _cf22r = 0, _cf22i = 0;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
    double _cf23r = 0, _cf23i = 0;
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
    double _mul24r = 0, _mul24i = 0;
    c_mul(_cf22r, _cf22i, _cf23r, _cf23i, &_mul24r, &_mul24i);
    double _cf25r = 0, _cf25i = 0;
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
    double _mul26r = 0, _mul26i = 0;
    c_mul(_mul24r, _mul24i, _cf25r, _cf25i, &_mul26r, &_mul26i);
    double _cf27r = 0, _cf27i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { _cf27r = cRe[_idx]; _cf27i = cIm[_idx]; } }
    double _div28r = 0, _div28i = 0;
    c_div(_mul26r, _mul26i, _cf27r, _cf27i, &_div28r, &_div28i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div28r; cIm[_idx] = _div28i; } }
    double _cf29r = 0, _cf29i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
    double _cf30r = 0, _cf30i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
    double _add31r = 0, _add31i = 0;
    _add31r = _cf29r + _cf30r; _add31i = _cf29i + _cf30i;
    double _cf32r = 0, _cf32i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
    double _add33r = 0, _add33i = 0;
    _add33r = _add31r + _cf32r; _add33i = _add31i + _cf32i;
    double _cf34r = 0, _cf34i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
    double _sub35r = 0, _sub35i = 0;
    _sub35r = _add33r - _cf34r; _sub35i = _add33i - _cf34i;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub35r; cIm[_idx] = _sub35i; } }
    double _c36r = 0, _c36i = 0;
    _c36r = 0.0; _c36i = 1.0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c36r, _c36i, x1r, x1i, &_mul37r, &_mul37i);
    double _mul38r = 0, _mul38i = 0;
    c_mul(_mul37r, _mul37i, x2r, x2i, &_mul38r, &_mul38i);
    double _sub39r = 0, _sub39i = 0;
    _sub39r = x1r - x2r; _sub39i = x1i - x2i;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_mul38r, _mul38i, _sub39r, _sub39i, &_mul40r, &_mul40i);
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    double _cf41r = 0, _cf41i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf41r = cRe[_idx]; _cf41i = cIm[_idx]; } }
    double _mul42r = 0, _mul42i = 0;
    c_mul(_cf41r, _cf41i, x1r, x1i, &_mul42r, &_mul42i);
    double _c43r = 0, _c43i = 0;
    _c43r = 1.0; _c43i = 0;
    double _abs44r = 0, _abs44i = 0;
    _abs44r = c_abs(x2r, x2i); _abs44i = 0;
    double _add45r = 0, _add45i = 0;
    _add45r = _c43r + _abs44r; _add45i = _c43i + _abs44i;
    double _div46r = 0, _div46i = 0;
    c_div(_mul42r, _mul42i, _add45r, _add45i, &_div46r, &_div46i);
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div46r; cIm[_idx] = _div46i; } }
    double _cf47r = 0, _cf47i = 0;
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { _cf47r = cRe[_idx]; _cf47i = cIm[_idx]; } }
    double _conj48r = 0, _conj48i = 0;
    _conj48r = _cf47r; _conj48i = -(_cf47i);
    double _div49r = 0, _div49i = 0;
    c_div(_conj48r, _conj48i, x2r, x2i, &_div49r, &_div49i);
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div49r; cIm[_idx] = _div49i; } }
    double _cf50r = 0, _cf50i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
    double _mul51r = 0, _mul51i = 0;
    c_mul(_cf50r, _cf50i, x2r, x2i, &_mul51r, &_mul51i);
    double _c52r = 0, _c52i = 0;
    _c52r = 1.0; _c52i = 0;
    double _abs53r = 0, _abs53i = 0;
    _abs53r = c_abs(x1r, x1i); _abs53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = _c52r + _abs53r; _add54i = _c52i + _abs53i;
    double _div55r = 0, _div55i = 0;
    c_div(_mul51r, _mul51i, _add54r, _add54i, &_div55r, &_div55i);
    double _abs56r = 0, _abs56i = 0;
    _abs56r = c_abs(_div55r, _div55i); _abs56i = 0;
    double _log57r = 0, _log57i = 0;
    c_log(_abs56r, _abs56i, &_log57r, &_log57i);
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _log57r; cIm[_idx] = _log57i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_86_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 3.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow2r, &_pow2i);
    c_mul(_pow2r, _pow2i, x1r, x1i, &_pow2r, &_pow2i);
    double _attr3r = 0, _attr3i = 0;
    _attr3r = _pow2r; _attr3i = 0;
    double _c4r = 0, _c4i = 0;
    _c4r = 3.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    c_mul(_pow5r, _pow5i, x2r, x2i, &_pow5r, &_pow5i);
    double _attr6r = 0, _attr6i = 0;
    _attr6r = _pow5r; _attr6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_attr3r, _attr3i, _attr6r, _attr6i, &_mul7r, &_mul7i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 3.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow9r, &_pow9i);
    c_mul(_pow9r, _pow9i, x1r, x1i, &_pow9r, &_pow9i);
    double _attr10r = 0, _attr10i = 0;
    _attr10r = _pow9i; _attr10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 3.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, x2r, x2i, &_pow12r, &_pow12i);
    double _attr13r = 0, _attr13i = 0;
    _attr13r = _pow12i; _attr13i = 0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_attr10r, _attr10i, _attr13r, _attr13i, &_mul14r, &_mul14i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    for (int k = 2; k < 25; k++) {
        double _c15r = 0, _c15i = 0;
        _c15r = 3.0; _c15i = 0;
        double _mod16r = 0, _mod16i = 0;
        _mod16r = fmod(k, _c15r); _mod16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 0.0; _c17i = 0;
        if (_mod16r == _c17r) {
            double _c18r = 0, _c18i = 0;
            _c18r = 0.0; _c18i = 1.0;
            double _mul19r = 0, _mul19i = 0;
            c_mul(_c18r, _c18i, x2r, x2i, &_mul19r, &_mul19i);
            double _add20r = 0, _add20i = 0;
            _add20r = x1r + _mul19r; _add20i = x1i + _mul19i;
            double _c21r = 0, _c21i = 0;
            _c21r = 3.0; _c21i = 0;
            double _div22r = 0, _div22i = 0;
            c_div(k, 0, _c21r, _c21i, &_div22r, &_div22i);
            double _pow23r = 0, _pow23i = 0;
            c_powc(_add20r, _add20i, _div22r, _div22i, &_pow23r, &_pow23i);
            double _div24r = 0, _div24i = 0;
            c_div(_pow23r, _pow23i, k, 0, &_div24r, &_div24i);
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div24r; cIm[_idx] = _div24i; } }
        } else {
            double _cf25r = 0, _cf25i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
            double _conj26r = 0, _conj26i = 0;
            _conj26r = _cf25r; _conj26i = -(_cf25i);
            double _c27r = 0, _c27i = 0;
            _c27r = 2.0; _c27i = 0;
            double _pow28r = 0, _pow28i = 0;
            c_mul(_conj26r, _conj26i, _conj26r, _conj26i, &_pow28r, &_pow28i);
            double _abs29r = 0, _abs29i = 0;
            _abs29r = c_abs(x1r, x1i); _abs29i = 0;
            double _abs30r = 0, _abs30i = 0;
            _abs30r = c_abs(x2r, x2i); _abs30i = 0;
            double _mul31r = 0, _mul31i = 0;
            c_mul(_abs29r, _abs29i, _abs30r, _abs30i, &_mul31r, &_mul31i);
            double _add32r = 0, _add32i = 0;
            _add32r = _pow28r + _mul31r; _add32i = _pow28i + _mul31i;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
        }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_87_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
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
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add10r; cIm[_idx] = _add10i; } }
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
    { int _idx = 2; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    for (int i = 3; i < 51; i++) {
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
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    }
    double _cf32r = 0, _cf32i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
    double _cf33r = 0, _cf33i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 51) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
    double _add34r = 0, _add34i = 0;
    _add34r = _cf32r + _cf33r; _add34i = _cf32i + _cf33i;
    double _sin35r = 0, _sin35i = 0;
    c_sin(x1r, x1i, &_sin35r, &_sin35i);
    double _sub36r = 0, _sub36i = 0;
    _sub36r = _add34r - _sin35r; _sub36i = _add34i - _sin35i;
    { int _idx = 10; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    double _cf37r = 0, _cf37i = 0;
    { int _idx = 30; if (_idx >= 0 && _idx < 51) { _cf37r = cRe[_idx]; _cf37i = cIm[_idx]; } }
    double _cf38r = 0, _cf38i = 0;
    { int _idx = 40; if (_idx >= 0 && _idx < 51) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
    double _add39r = 0, _add39i = 0;
    _add39r = _cf37r + _cf38r; _add39i = _cf37i + _cf38i;
    double _cos40r = 0, _cos40i = 0;
    c_cos(x2r, x2i, &_cos40r, &_cos40i);
    double _sub41r = 0, _sub41i = 0;
    _sub41r = _add39r - _cos40r; _sub41i = _add39i - _cos40i;
    { int _idx = 20; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub41r; cIm[_idx] = _sub41i; } }
    double _cf42r = 0, _cf42i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 51) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
    double _cf43r = 0, _cf43i = 0;
    { int _idx = 40; if (_idx >= 0 && _idx < 51) { _cf43r = cRe[_idx]; _cf43i = cIm[_idx]; } }
    double _add44r = 0, _add44i = 0;
    _add44r = _cf42r + _cf43r; _add44i = _cf42i + _cf43i;
    double _sin45r = 0, _sin45i = 0;
    c_sin(x1r, x1i, &_sin45r, &_sin45i);
    double _add46r = 0, _add46i = 0;
    _add46r = _add44r + _sin45r; _add46i = _add44i + _sin45i;
    { int _idx = 30; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add46r; cIm[_idx] = _add46i; } }
    double _cf47r = 0, _cf47i = 0;
    { int _idx = 30; if (_idx >= 0 && _idx < 51) { _cf47r = cRe[_idx]; _cf47i = cIm[_idx]; } }
    double _cf48r = 0, _cf48i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 51) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
    double _add49r = 0, _add49i = 0;
    _add49r = _cf47r + _cf48r; _add49i = _cf47i + _cf48i;
    double _cos50r = 0, _cos50i = 0;
    c_cos(x2r, x2i, &_cos50r, &_cos50i);
    double _sub51r = 0, _sub51i = 0;
    _sub51r = _add49r - _cos50r; _sub51i = _add49i - _cos50i;
    { int _idx = 40; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub51r; cIm[_idx] = _sub51i; } }
    double _cf52r = 0, _cf52i = 0;
    { int _idx = 40; if (_idx >= 0 && _idx < 51) { _cf52r = cRe[_idx]; _cf52i = cIm[_idx]; } }
    double _cf53r = 0, _cf53i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 51) { _cf53r = cRe[_idx]; _cf53i = cIm[_idx]; } }
    double _add54r = 0, _add54i = 0;
    _add54r = _cf52r + _cf53r; _add54i = _cf52i + _cf53i;
    double _sin55r = 0, _sin55i = 0;
    c_sin(x2r, x2i, &_sin55r, &_sin55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _add54r + _sin55r; _add56i = _add54i + _sin55i;
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_88: auto-stubbed (unhandled constructs in source) */
static void poly_giga_88_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_89_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 51; i++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(i, 0, x1r, x1i, &_mul1r, &_mul1i);
        double _cos2r = 0, _cos2i = 0;
        c_cos(_mul1r, _mul1i, &_cos2r, &_cos2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(i, 0, x2r, x2i, &_mul3r, &_mul3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_mul3r, _mul3i, &_sin4r, &_sin4i);
        double _add5r = 0, _add5i = 0;
        _add5r = _cos2r + _sin4r; _add5i = _cos2i + _sin4i;
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    }
    double _cf6r = 0, _cf6i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { _cf6r = cRe[_idx]; _cf6i = cIm[_idx]; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 50.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_powr(x1r, x1i, 50.0, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_cf6r, _cf6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
    double _cf10r = 0, _cf10i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { _cf10r = cRe[_idx]; _cf10i = cIm[_idx]; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 49.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_powr(x2r, x2i, 49.0, &_pow12r, &_pow12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_cf10r, _cf10i, _pow12r, _pow12i, &_mul13r, &_mul13i);
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    for (int i = 2; i < 51; i++) {
        double _cf14r = 0, _cf14i = 0;
        { int _idx = i; if (_idx >= 0 && _idx < 51) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
        double _c15r = 0, _c15i = 0;
        _c15r = 51.0; _c15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _c15r - _c16r; _sub17i = _c15i - _c16i;
        double _pow18r = 0, _pow18i = 0;
        c_powc(x1r, x1i, _sub17r, _sub17i, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_cf14r, _cf14i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = i - _c20r; _sub21i = 0 - _c20i;
        double _pow22r = 0, _pow22i = 0;
        c_powc(x2r, x2i, _sub21r, _sub21i, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul19r, _mul19i, _pow22r, _pow22i, &_mul23r, &_mul23i);
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_90: auto-stubbed (unhandled constructs in source) */
static void poly_giga_90_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_91: auto-stubbed (unhandled constructs in source) */
static void poly_giga_91_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_92: auto-stubbed (unhandled constructs in source) */
static void poly_giga_92_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_93_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_c2r, _c2i, M_PI, 0, &_mul3r, &_mul3i);
    double angles[35];
    for (int _li = 0; _li < 35; _li++) {
        angles[_li] = _c1r + (_mul3r - _c1r) * _li / 34.0;
    }
    for (int i = 0; i < 35; i++) {
        double _arr4r = 0, _arr4i = 0;
        { int _idx = i; _arr4r = (_idx >= 0 && _idx < 35) ? angles[_idx] : 0.0; _arr4i = 0; }
        double _mul5r = 0, _mul5i = 0;
        c_mul(_arr4r, _arr4i, x1r, x1i, &_mul5r, &_mul5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_mul5r, _mul5i, &_cos6r, &_cos6i);
        double _arr7r = 0, _arr7i = 0;
        { int _idx = i; _arr7r = (_idx >= 0 && _idx < 35) ? angles[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(_arr7r, _arr7i, x2r, x2i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_sin9r, _sin9i, _abs10r, _abs10i, &_div11r, &_div11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _cos6r + _div11r; _add12i = _cos6i + _div11i;
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
    }
    for (int i = 35; i < 71; i++) {
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 1.0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(x1r, x1i, _c13r, _c13i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 71.0; _c15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _c15r - _c16r; _sub17i = _c15i - _c16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(x2r, x2i, _sub17r, _sub17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul14r + _mul18r; _add19i = _mul14i + _mul18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 3.0; _c20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_mul(_add19r, _add19i, _add19r, _add19i, &_pow21r, &_pow21i);
        c_mul(_pow21r, _pow21i, _add19r, _add19i, &_pow21r, &_pow21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(x1r, x1i, _c22r, _c22i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, x2r, x2i, &_mul25r, &_mul25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 71.0; _c26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _c26r - _c27r; _sub28i = _c26i - _c27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_mul25r, _mul25i, _sub28r, _sub28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul23r + _mul29r; _add30i = _mul23i + _mul29i;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_mul(_add30r, _add30i, _add30r, _add30i, &_pow32r, &_pow32i);
        double _div33r = 0, _div33i = 0;
        c_div(_pow21r, _pow21i, _pow32r, _pow32i, &_div33r, &_div33i);
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div33r; cIm[_idx] = _div33i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_94: auto-stubbed (unhandled constructs in source) */
static void poly_giga_94_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_95: auto-stubbed (unhandled constructs in source) */
static void poly_giga_95_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_96_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 71; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = i + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(_add2r, _add2i, _add2r, _add2i, &_pow4r, &_pow4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 3.0;
        double _add6r = 0, _add6i = 0;
        _add6r = _pow4r + _c5r; _add6i = _pow4i + _c5i;
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
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = i + _c12r; _add13i = 0 + _c12i;
        double _c14r = 0, _c14i = 0;
        _c14r = 3.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(_add13r, _add13i, _add13r, _add13i, &_pow15r, &_pow15i);
        c_mul(_pow15r, _pow15i, _add13r, _add13i, &_pow15r, &_pow15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = i + _c16r; _add17i = 0 + _c16i;
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_mul(_add17r, _add17i, _add17r, _add17i, &_pow19r, &_pow19i);
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _pow15r - _pow19r; _sub20i = _pow15i - _pow19i;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _sub20r + _c21r; _add22i = _sub20i + _c21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_add22r, _add22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul11r + _mul25r; _add26i = _mul11i + _mul25i;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(x1r, x1i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul28r + x2r; _add29i = _mul28i + x2i;
        double _sin30r = 0, _sin30i = 0;
        c_sin(_add29r, _add29i, &_sin30r, &_sin30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add26r + _sin30r; _add31i = _add26i + _sin30i;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(x1r, x1i, _c32r, _c32i, &_mul33r, &_mul33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul33r - x2r; _sub34i = _mul33i - x2i;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(_sub34r, _sub34i); _abs35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = _abs35r + _c36r; _add37i = _abs35i + _c36i;
        double _log38r = 0, _log38i = 0;
        c_log(_add37r, _add37i, &_log38r, &_log38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add31r + _log38r; _add39i = _add31i + _log38i;
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = i + _c40r; _add41i = 0 + _c40i;
        double _c42r = 0, _c42i = 0;
        _c42r = 2.0; _c42i = 0;
        double _mod43r = 0, _mod43i = 0;
        _mod43r = fmod(_add41r, _c42r); _mod43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 0;
        if (_mod43r == _c44r) {
            double _cf45r = 0, _cf45i = 0;
            { int _idx = i; if (_idx >= 0 && _idx < 71) { _cf45r = cRe[_idx]; _cf45i = cIm[_idx]; } }
            double _c46r = 0, _c46i = 0;
            _c46r = 0.0; _c46i = 1.0;
            double _mul47r = 0, _mul47i = 0;
            c_mul(_c46r, _c46i, x2r, x2i, &_mul47r, &_mul47i);
            double _add48r = 0, _add48i = 0;
            _add48r = x1r + _mul47r; _add48i = x1i + _mul47i;
            double _c49r = 0, _c49i = 0;
            _c49r = 2.0; _c49i = 0;
            double _pow50r = 0, _pow50i = 0;
            c_mul(_add48r, _add48i, _add48r, _add48i, &_pow50r, &_pow50i);
            double _add51r = 0, _add51i = 0;
            _add51r = _cf45r + _pow50r; _add51i = _cf45i + _pow50i;
            { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
        } else {
            double _c52r = 0, _c52i = 0;
            _c52r = 1.0; _c52i = 0;
            double _add53r = 0, _add53i = 0;
            _add53r = i + _c52r; _add53i = 0 + _c52i;
            double _c54r = 0, _c54i = 0;
            _c54r = 3.0; _c54i = 0;
            double _mod55r = 0, _mod55i = 0;
            _mod55r = fmod(_add53r, _c54r); _mod55i = 0;
            double _c56r = 0, _c56i = 0;
            _c56r = 0.0; _c56i = 0;
            if (_mod55r == _c56r) {
                double _cf57r = 0, _cf57i = 0;
                { int _idx = i; if (_idx >= 0 && _idx < 71) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
                double _c58r = 0, _c58i = 0;
                _c58r = 0.0; _c58i = 1.0;
                double _mul59r = 0, _mul59i = 0;
                c_mul(_c58r, _c58i, x2r, x2i, &_mul59r, &_mul59i);
                double _add60r = 0, _add60i = 0;
                _add60r = x1r + _mul59r; _add60i = x1i + _mul59i;
                double _abs61r = 0, _abs61i = 0;
                _abs61r = c_abs(_add60r, _add60i); _abs61i = 0;
                double _c62r = 0, _c62i = 0;
                _c62r = 3.0; _c62i = 0;
                double _pow63r = 0, _pow63i = 0;
                c_mul(_abs61r, _abs61i, _abs61r, _abs61i, &_pow63r, &_pow63i);
                c_mul(_pow63r, _pow63i, _abs61r, _abs61i, &_pow63r, &_pow63i);
                double _add64r = 0, _add64i = 0;
                _add64r = _cf57r + _pow63r; _add64i = _cf57i + _pow63i;
                { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
            } else {
                double _cf65r = 0, _cf65i = 0;
                { int _idx = i; if (_idx >= 0 && _idx < 71) { _cf65r = cRe[_idx]; _cf65i = cIm[_idx]; } }
                double _c66r = 0, _c66i = 0;
                _c66r = 0.0; _c66i = 1.0;
                double _mul67r = 0, _mul67i = 0;
                c_mul(_c66r, _c66i, x2r, x2i, &_mul67r, &_mul67i);
                double _add68r = 0, _add68i = 0;
                _add68r = x1r + _mul67r; _add68i = x1i + _mul67i;
                double _attr69r = 0, _attr69i = 0;
                _attr69r = _add68r; _attr69i = 0;
                double _c70r = 0, _c70i = 0;
                _c70r = 4.0; _c70i = 0;
                double _pow71r = 0, _pow71i = 0;
                c_mul(_attr69r, _attr69i, _attr69r, _attr69i, &_pow71r, &_pow71i);
                c_mul(_pow71r, _pow71i, _pow71r, _pow71i, &_pow71r, &_pow71i);
                double _add72r = 0, _add72i = 0;
                _add72r = _cf65r + _pow71r; _add72i = _cf65i + _pow71i;
                { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add72r; cIm[_idx] = _add72i; } }
            }
        }
    }
    double _c73r = 0, _c73i = 0;
    _c73r = 10000.0; _c73i = 0;
    { double _tr = cRe[0]*_c73r - cIm[0]*_c73i; cIm[0] = cRe[0]*_c73i + cIm[0]*_c73r; cRe[0] = _tr; }
    double _c74r = 0, _c74i = 0;
    _c74r = 1000.0; _c74i = 0;
    { double _tr = cRe[1]*_c74r - cIm[1]*_c74i; cIm[1] = cRe[1]*_c74i + cIm[1]*_c74r; cRe[1] = _tr; }
    double _c75r = 0, _c75i = 0;
    _c75r = 100.0; _c75i = 0;
    { double _tr = cRe[2]*_c75r - cIm[2]*_c75i; cIm[2] = cRe[2]*_c75i + cIm[2]*_c75r; cRe[2] = _tr; }
    double _c76r = 0, _c76i = 0;
    _c76r = 10.0; _c76i = 0;
    { double _tr = cRe[3]*_c76r - cIm[3]*_c76i; cIm[3] = cRe[3]*_c76i + cIm[3]*_c76r; cRe[3] = _tr; }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_97: auto-stubbed (unhandled constructs in source) */
static void poly_giga_97_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_98: auto-stubbed (unhandled constructs in source) */
static void poly_giga_98_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_99_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
    double complex_val_r = _add5r, complex_val_i = _add5i;
    for (int k = 0; k < 71; k++) {
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = k + _c6r; _add7i = 0 + _c6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(_add7r, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 0.0; _c10i = 0;
        if (_mod9r == _c10r) {
            double _c11r = 0, _c11i = 0;
            _c11r = 1.0; _c11i = 0;
            double _add12r = 0, _add12i = 0;
            _add12r = k + _c11r; _add12i = 0 + _c11i;
            double _abs13r = 0, _abs13i = 0;
            _abs13r = c_abs(_add12r, _add12i); _abs13i = 0;
            double _div14r = 0, _div14i = 0;
            c_div(complex_val_r, complex_val_i, _abs13r, _abs13i, &_div14r, &_div14i);
            double _abs15r = 0, _abs15i = 0;
            _abs15r = c_abs(x1r, x1i); _abs15i = 0;
            double _sub16r = 0, _sub16i = 0;
            _sub16r = _div14r - _abs15r; _sub16i = _div14i - _abs15i;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub16r; cIm[_idx] = _sub16i; } }
        } else {
            double _c17r = 0, _c17i = 0;
            _c17r = 1.0; _c17i = 0;
            double _add18r = 0, _add18i = 0;
            _add18r = k + _c17r; _add18i = 0 + _c17i;
            double _abs19r = 0, _abs19i = 0;
            _abs19r = c_abs(_add18r, _add18i); _abs19i = 0;
            double _mul20r = 0, _mul20i = 0;
            c_mul(complex_val_r, complex_val_i, _abs19r, _abs19i, &_mul20r, &_mul20i);
            double _c21r = 0, _c21i = 0;
            _c21r = 1.0; _c21i = 0;
            double _add22r = 0, _add22i = 0;
            _add22r = k + _c21r; _add22i = 0 + _c21i;
            double _abs23r = 0, _abs23i = 0;
            _abs23r = c_abs(_add22r, _add22i); _abs23i = 0;
            double _c24r = 0, _c24i = 0;
            _c24r = 1.0; _c24i = 0;
            double _add25r = 0, _add25i = 0;
            _add25r = _abs23r + _c24r; _add25i = _abs23i + _c24i;
            double _log26r = 0, _log26i = 0;
            c_log(_add25r, _add25i, &_log26r, &_log26i);
            double _add27r = 0, _add27i = 0;
            _add27r = _mul20r + _log26r; _add27i = _mul20i + _log26i;
            double _attr28r = 0, _attr28i = 0;
            _attr28r = x2i; _attr28i = 0;
            double _add29r = 0, _add29i = 0;
            _add29r = _add27r + _attr28r; _add29i = _add27i + _attr28i;
            double _attr30r = 0, _attr30i = 0;
            _attr30r = x1r; _attr30i = 0;
            double _sub31r = 0, _sub31i = 0;
            _sub31r = _add29r - _attr30r; _sub31i = _add29i - _attr30i;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub31r; cIm[_idx] = _sub31i; } }
        }
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = k + _c32r; _add33i = 0 + _c32i;
        double _c34r = 0, _c34i = 0;
        _c34r = 3.0; _c34i = 0;
        double _mod35r = 0, _mod35i = 0;
        _mod35r = fmod(_add33r, _c34r); _mod35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 0;
        if ((_mod35r == _c36r) && (k > _c37r)) {
            double _cf38r = 0, _cf38i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { _cf38r = cRe[_idx]; _cf38i = cIm[_idx]; } }
            double _c39r = 0, _c39i = 0;
            _c39r = 3.0; _c39i = 0;
            double _cf40r = 0, _cf40i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { _cf40r = cRe[_idx]; _cf40i = cIm[_idx]; } }
            double _mul41r = 0, _mul41i = 0;
            c_mul(_c39r, _c39i, _cf40r, _cf40i, &_mul41r, &_mul41i);
            double _add42r = 0, _add42i = 0;
            _add42r = _cf38r + _mul41r; _add42i = _cf38i + _mul41i;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
        }
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = k + _c43r; _add44i = 0 + _c43i;
        double _c45r = 0, _c45i = 0;
        _c45r = 5.0; _c45i = 0;
        double _mod46r = 0, _mod46i = 0;
        _mod46r = fmod(_add44r, _c45r); _mod46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 0.0; _c47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 1.0; _c48i = 0;
        if ((_mod46r == _c47r) && (k > _c48r)) {
            double _cf49r = 0, _cf49i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { _cf49r = cRe[_idx]; _cf49i = cIm[_idx]; } }
            double _c50r = 0, _c50i = 0;
            _c50r = 5.0; _c50i = 0;
            double _cf51r = 0, _cf51i = 0;
            { int _idx = (k - 2); if (_idx >= 0 && _idx < 71) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
            double _mul52r = 0, _mul52i = 0;
            c_mul(_c50r, _c50i, _cf51r, _cf51i, &_mul52r, &_mul52i);
            double _add53r = 0, _add53i = 0;
            _add53r = _cf49r + _mul52r; _add53i = _cf49i + _mul52i;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add53r; cIm[_idx] = _add53i; } }
        }
        double _c54r = 0, _c54i = 0;
        _c54r = 1.0; _c54i = 0;
        double _add55r = 0, _add55i = 0;
        _add55r = k + _c54r; _add55i = 0 + _c54i;
        double _c56r = 0, _c56i = 0;
        _c56r = 7.0; _c56i = 0;
        double _mod57r = 0, _mod57i = 0;
        _mod57r = fmod(_add55r, _c56r); _mod57i = 0;
        double _c58r = 0, _c58i = 0;
        _c58r = 0.0; _c58i = 0;
        double _c59r = 0, _c59i = 0;
        _c59r = 2.0; _c59i = 0;
        if ((_mod57r == _c58r) && (k > _c59r)) {
            double _cf60r = 0, _cf60i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
            double _c61r = 0, _c61i = 0;
            _c61r = 7.0; _c61i = 0;
            double _cf62r = 0, _cf62i = 0;
            { int _idx = (k - 3); if (_idx >= 0 && _idx < 71) { _cf62r = cRe[_idx]; _cf62i = cIm[_idx]; } }
            double _mul63r = 0, _mul63i = 0;
            c_mul(_c61r, _c61i, _cf62r, _cf62i, &_mul63r, &_mul63i);
            double _add64r = 0, _add64i = 0;
            _add64r = _cf60r + _mul63r; _add64i = _cf60i + _mul63i;
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
        }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_100: auto-stubbed (unhandled constructs in source) */
static void poly_giga_100_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_101: auto-stubbed (unhandled constructs in source) */
static void poly_giga_101_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_102: auto-stubbed (unhandled constructs in source) */
static void poly_giga_102_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_103: auto-stubbed (unhandled constructs in source) */
static void poly_giga_103_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_104: too complex for auto-transpile, stubbed */
static void poly_giga_104_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_105: auto-stubbed (unhandled constructs in source) */
static void poly_giga_105_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_106_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(_add1r, _add1i); _abs2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(_abs2r, _abs2i, _abs2r, _abs2i, &_pow4r, &_pow4i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _pow4r; cIm[_idx] = _pow4i; } }
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x1r; _attr5i = 0;
    double _attr6r = 0, _attr6i = 0;
    _attr6r = x2i; _attr6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_attr5r, _attr5i, _attr6r, _attr6i, &_mul7r, &_mul7i);
    double _attr8r = 0, _attr8i = 0;
    _attr8r = x2r; _attr8i = 0;
    double _attr9r = 0, _attr9i = 0;
    _attr9r = x1i; _attr9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_attr8r, _attr8i, _attr9r, _attr9i, &_mul10r, &_mul10i);
    double _sub11r = 0, _sub11i = 0;
    _sub11r = _mul7r - _mul10r; _sub11i = _mul7i - _mul10i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub11r; cIm[_idx] = _sub11i; } }
    double _abs12r = 0, _abs12i = 0;
    _abs12r = c_abs(x1r, x1i); _abs12i = 0;
    double _abs13r = 0, _abs13i = 0;
    _abs13r = c_abs(x2r, x2i); _abs13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 2.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(_abs13r, _abs13i, _abs13r, _abs13i, &_pow15r, &_pow15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_abs12r, _abs12i, _pow15r, _pow15i, &_mul16r, &_mul16i);
    double _sub17r = 0, _sub17i = 0;
    _sub17r = x1r - x2r; _sub17i = x1i - x2i;
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(_sub17r, _sub17i); _abs18i = 0;
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(_abs18r, _abs18i, _abs18r, _abs18i, &_pow20r, &_pow20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = _mul16r - _pow20r; _sub21i = _mul16i - _pow20i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub21r; cIm[_idx] = _sub21i; } }
    for (int k = 3; k < 36; k++) {
        double _mul22r = 0, _mul22i = 0;
        c_mul(k, 0, x1r, x1i, &_mul22r, &_mul22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul22r, _mul22i, x2r, x2i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _add25r = 0, _add25i = 0;
        _add25r = x1r + x2r; _add25i = x1i + x2i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(k, 0, _add25r, _add25i, &_mul26r, &_mul26i);
        double _cos27r = 0, _cos27i = 0;
        c_cos(_mul26r, _mul26i, &_cos27r, &_cos27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sin24r + _cos27r; _add28i = _sin24i + _cos27i;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = k + _c29r; _add30i = 0 + _c29i;
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _div32r = 0, _div32i = 0;
        c_div(_add30r, _add30i, _c31r, _c31i, &_div32r, &_div32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = x1r - x2r; _sub33i = x1i - x2i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_div32r, _div32i, _sub33r, _sub33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = k + _c36r; _add37i = 0 + _c36i;
        double _c38r = 0, _c38i = 0;
        _c38r = 2.0; _c38i = 0;
        double _div39r = 0, _div39i = 0;
        c_div(_add37r, _add37i, _c38r, _c38i, &_div39r, &_div39i);
        double _add40r = 0, _add40i = 0;
        _add40r = x1r + x2r; _add40i = x1i + x2i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_div39r, _div39i, _add40r, _add40i, &_mul41r, &_mul41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_mul41r, _mul41i, &_cos42r, &_cos42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _sin35r + _cos42r; _add43i = _sin35i + _cos42i;
        double _abs44r = 0, _abs44i = 0;
        _abs44r = c_abs(_add43r, _add43i); _abs44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 2.0; _c45i = 0;
        double _pow46r = 0, _pow46i = 0;
        c_mul(_abs44r, _abs44i, _abs44r, _abs44i, &_pow46r, &_pow46i);
        double _div47r = 0, _div47i = 0;
        c_div(_add28r, _add28i, _pow46r, _pow46i, &_div47r, &_div47i);
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div47r; cIm[_idx] = _div47i; } }
    }
    double _abs48r = 0, _abs48i = 0;
    _abs48r = c_abs(x1r, x1i); _abs48i = 0;
    double _c49r = 0, _c49i = 0;
    _c49r = 1.0; _c49i = 0;
    double _add50r = 0, _add50i = 0;
    _add50r = _abs48r + _c49r; _add50i = _abs48i + _c49i;
    double _log51r = 0, _log51i = 0;
    c_log(_add50r, _add50i, &_log51r, &_log51i);
    double _abs52r = 0, _abs52i = 0;
    _abs52r = c_abs(x2r, x2i); _abs52i = 0;
    double _c53r = 0, _c53i = 0;
    _c53r = 1.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = _abs52r + _c53r; _add54i = _abs52i + _c53i;
    double _log55r = 0, _log55i = 0;
    c_log(_add54r, _add54i, &_log55r, &_log55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _log51r + _log55r; _add56i = _log51i + _log55i;
    { int _idx = 36; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    for (int k = 37; k < 71; k++) {
        double _mul57r = 0, _mul57i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul57r, &_mul57i);
        double _attr58r = 0, _attr58i = 0;
        _attr58r = _mul57r; _attr58i = 0;
        double _mul59r = 0, _mul59i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul59r, &_mul59i);
        double _attr60r = 0, _attr60i = 0;
        _attr60r = _mul59i; _attr60i = 0;
        double _sub61r = 0, _sub61i = 0;
        _sub61r = x1r - x2r; _sub61i = x1i - x2i;
        double _abs62r = 0, _abs62i = 0;
        _abs62r = c_abs(_sub61r, _sub61i); _abs62i = 0;
        double _div63r = 0, _div63i = 0;
        c_div(_attr60r, _attr60i, _abs62r, _abs62i, &_div63r, &_div63i);
        double _add64r = 0, _add64i = 0;
        _add64r = _attr58r + _div63r; _add64i = _attr58i + _div63i;
        double _mul65r = 0, _mul65i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul65r, &_mul65i);
        double _abs66r = 0, _abs66i = 0;
        _abs66r = c_abs(_mul65r, _mul65i); _abs66i = 0;
        double _c67r = 0, _c67i = 0;
        _c67r = 1.0; _c67i = 0;
        double _add68r = 0, _add68i = 0;
        _add68r = _abs66r + _c67r; _add68i = _abs66i + _c67i;
        double _log69r = 0, _log69i = 0;
        c_log(_add68r, _add68i, &_log69r, &_log69i);
        double _add70r = 0, _add70i = 0;
        _add70r = x1r + x2r; _add70i = x1i + x2i;
        double _abs71r = 0, _abs71i = 0;
        _abs71r = c_abs(_add70r, _add70i); _abs71i = 0;
        double _div72r = 0, _div72i = 0;
        c_div(_log69r, _log69i, _abs71r, _abs71i, &_div72r, &_div72i);
        double _add73r = 0, _add73i = 0;
        _add73r = _add64r + _div72r; _add73i = _add64i + _div72i;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add73r; cIm[_idx] = _add73i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_107: auto-stubbed (unhandled constructs in source) */
static void poly_giga_107_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_108_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow3r, &_pow3i);
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + _pow3r; _add4i = x1i + _pow3i;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c1r, _c1i, _add4r, _add4i, &_mul5r, &_mul5i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 90.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow8r, &_pow8i);
    double _add9r = 0, _add9i = 0;
    _add9r = _pow8r + x2r; _add9i = _pow8i + x2i;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c6r, _c6i, _add9r, _add9i, &_mul10r, &_mul10i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 80.0; _c11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 3.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow13r, &_pow13i);
    c_mul(_pow13r, _pow13i, x1r, x1i, &_pow13r, &_pow13i);
    double _c14r = 0, _c14i = 0;
    _c14r = 2.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow15r, &_pow15i);
    double _sub16r = 0, _sub16i = 0;
    _sub16r = _pow13r - _pow15r; _sub16i = _pow13i - _pow15i;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_c11r, _c11i, _sub16r, _sub16i, &_mul17r, &_mul17i);
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul17r; cIm[_idx] = _mul17i; } }
    double _c18r = 0, _c18i = 0;
    _c18r = 70.0; _c18i = 0;
    double _c19r = 0, _c19i = 0;
    _c19r = 4.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow20r, &_pow20i);
    c_mul(_pow20r, _pow20i, _pow20r, _pow20i, &_pow20r, &_pow20i);
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, x2r, x2i, &_pow22r, &_pow22i);
    double _add23r = 0, _add23i = 0;
    _add23r = _pow20r + _pow22r; _add23i = _pow20i + _pow22i;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c18r, _c18i, _add23r, _add23i, &_mul24r, &_mul24i);
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    double _c25r = 0, _c25i = 0;
    _c25r = 60.0; _c25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 5.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_powr(x1r, x1i, 5.0, &_pow27r, &_pow27i);
    double _c28r = 0, _c28i = 0;
    _c28r = 4.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow29r, &_pow29i);
    c_mul(_pow29r, _pow29i, _pow29r, _pow29i, &_pow29r, &_pow29i);
    double _sub30r = 0, _sub30i = 0;
    _sub30r = _pow27r - _pow29r; _sub30i = _pow27i - _pow29i;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_c25r, _c25i, _sub30r, _sub30i, &_mul31r, &_mul31i);
    { int _idx = 4; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    double _c32r = 0, _c32i = 0;
    _c32r = 50.0; _c32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 6.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_powr(x1r, x1i, 6.0, &_pow34r, &_pow34i);
    double _c35r = 0, _c35i = 0;
    _c35r = 5.0; _c35i = 0;
    double _pow36r = 0, _pow36i = 0;
    c_powr(x2r, x2i, 5.0, &_pow36r, &_pow36i);
    double _add37r = 0, _add37i = 0;
    _add37r = _pow34r + _pow36r; _add37i = _pow34i + _pow36i;
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c32r, _c32i, _add37r, _add37i, &_mul38r, &_mul38i);
    { int _idx = 5; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    double _c39r = 0, _c39i = 0;
    _c39r = 40.0; _c39i = 0;
    double _c40r = 0, _c40i = 0;
    _c40r = 7.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_powr(x1r, x1i, 7.0, &_pow41r, &_pow41i);
    double _c42r = 0, _c42i = 0;
    _c42r = 6.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_powr(x2r, x2i, 6.0, &_pow43r, &_pow43i);
    double _sub44r = 0, _sub44i = 0;
    _sub44r = _pow41r - _pow43r; _sub44i = _pow41i - _pow43i;
    double _mul45r = 0, _mul45i = 0;
    c_mul(_c39r, _c39i, _sub44r, _sub44i, &_mul45r, &_mul45i);
    { int _idx = 6; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    double _c46r = 0, _c46i = 0;
    _c46r = 30.0; _c46i = 0;
    double _c47r = 0, _c47i = 0;
    _c47r = 8.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_powr(x1r, x1i, 8.0, &_pow48r, &_pow48i);
    double _c49r = 0, _c49i = 0;
    _c49r = 7.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_powr(x2r, x2i, 7.0, &_pow50r, &_pow50i);
    double _add51r = 0, _add51i = 0;
    _add51r = _pow48r + _pow50r; _add51i = _pow48i + _pow50i;
    double _mul52r = 0, _mul52i = 0;
    c_mul(_c46r, _c46i, _add51r, _add51i, &_mul52r, &_mul52i);
    { int _idx = 7; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    double _c53r = 0, _c53i = 0;
    _c53r = 20.0; _c53i = 0;
    double _c54r = 0, _c54i = 0;
    _c54r = 9.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_powr(x1r, x1i, 9.0, &_pow55r, &_pow55i);
    double _c56r = 0, _c56i = 0;
    _c56r = 8.0; _c56i = 0;
    double _pow57r = 0, _pow57i = 0;
    c_powr(x2r, x2i, 8.0, &_pow57r, &_pow57i);
    double _sub58r = 0, _sub58i = 0;
    _sub58r = _pow55r - _pow57r; _sub58i = _pow55i - _pow57i;
    double _mul59r = 0, _mul59i = 0;
    c_mul(_c53r, _c53i, _sub58r, _sub58i, &_mul59r, &_mul59i);
    { int _idx = 8; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
    double _c60r = 0, _c60i = 0;
    _c60r = 10.0; _c60i = 0;
    double _c61r = 0, _c61i = 0;
    _c61r = 10.0; _c61i = 0;
    double _pow62r = 0, _pow62i = 0;
    c_powr(x1r, x1i, 10.0, &_pow62r, &_pow62i);
    double _c63r = 0, _c63i = 0;
    _c63r = 9.0; _c63i = 0;
    double _pow64r = 0, _pow64i = 0;
    c_powr(x2r, x2i, 9.0, &_pow64r, &_pow64i);
    double _add65r = 0, _add65i = 0;
    _add65r = _pow62r + _pow64r; _add65i = _pow62i + _pow64i;
    double _mul66r = 0, _mul66i = 0;
    c_mul(_c60r, _c60i, _add65r, _add65i, &_mul66r, &_mul66i);
    { int _idx = 9; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul66r; cIm[_idx] = _mul66i; } }
    for (int k = 10; k < 71; k++) {
        double _cf67r = 0, _cf67i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
        double _c68r = 0, _c68i = 0;
        _c68r = 1.0; _c68i = 0;
        double _add69r = 0, _add69i = 0;
        _add69r = _cf67r + _c68r; _add69i = _cf67i + _c68i;
        double _abs70r = 0, _abs70i = 0;
        _abs70r = c_abs(_add69r, _add69i); _abs70i = 0;
        double _log71r = 0, _log71i = 0;
        c_log(_abs70r, _abs70i, &_log71r, &_log71i);
        double _mul72r = 0, _mul72i = 0;
        c_mul(k, 0, _log71r, _log71i, &_mul72r, &_mul72i);
        double _sin73r = 0, _sin73i = 0;
        c_sin(_mul72r, _mul72i, &_sin73r, &_sin73i);
        double _mul74r = 0, _mul74i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul74r, &_mul74i);
        double _c75r = 0, _c75i = 0;
        _c75r = 1.0; _c75i = 0;
        double _add76r = 0, _add76i = 0;
        _add76r = _mul74r + _c75r; _add76i = _mul74i + _c75i;
        double _abs77r = 0, _abs77i = 0;
        _abs77r = c_abs(_add76r, _add76i); _abs77i = 0;
        double _log78r = 0, _log78i = 0;
        c_log(_abs77r, _abs77i, &_log78r, &_log78i);
        double _mul79r = 0, _mul79i = 0;
        c_mul(k, 0, _log78r, _log78i, &_mul79r, &_mul79i);
        double _cos80r = 0, _cos80i = 0;
        c_cos(_mul79r, _mul79i, &_cos80r, &_cos80i);
        double _add81r = 0, _add81i = 0;
        _add81r = _sin73r + _cos80r; _add81i = _sin73i + _cos80i;
        double v_r = _add81r, v_i = _add81i;
        double _conj82r = 0, _conj82i = 0;
        _conj82r = v_r; _conj82i = -(v_i);
        double _abs83r = 0, _abs83i = 0;
        _abs83r = c_abs(v_r, v_i); _abs83i = 0;
        double _div84r = 0, _div84i = 0;
        c_div(_conj82r, _conj82i, _abs83r, _abs83i, &_div84r, &_div84i);
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div84r; cIm[_idx] = _div84i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_109: auto-stubbed (unhandled constructs in source) */
static void poly_giga_109_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_110: auto-stubbed (unhandled constructs in source) */
static void poly_giga_110_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_111_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _pow2r = 0, _pow2i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow2r, &_pow2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c3r, _c3i, _pow5r, _pow5i, &_mul6r, &_mul6i);
    double _add7r = 0, _add7i = 0;
    _add7r = _pow2r + _mul6r; _add7i = _pow2i + _mul6i;
    double _c8r = 0, _c8i = 0;
    _c8r = 1.0; _c8i = 0;
    double _add9r = 0, _add9i = 0;
    _add9r = _add7r + _c8r; _add9i = _add7i + _c8i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
    double _mul10r = 0, _mul10i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul10r, &_mul10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 2.0; _c11i = 0;
    double _div12r = 0, _div12i = 0;
    c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 3.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow15r, &_pow15i);
    c_mul(_pow15r, _pow15i, x1r, x1i, &_pow15r, &_pow15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c13r, _c13i, _pow15r, _pow15i, &_mul16r, &_mul16i);
    double _add17r = 0, _add17i = 0;
    _add17r = _div12r + _mul16r; _add17i = _div12i + _mul16i;
    double _c18r = 0, _c18i = 0;
    _c18r = 3.0; _c18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c18r, _c18i, x2r, x2i, &_mul19r, &_mul19i);
    double _sub20r = 0, _sub20i = 0;
    _sub20r = _add17r - _mul19r; _sub20i = _add17i - _mul19i;
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 1.0;
    double _add22r = 0, _add22i = 0;
    _add22r = _sub20r + _c21r; _add22i = _sub20i + _c21i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    for (int k = 2; k < 71; k++) {
        double _c23r = 0, _c23i = 0;
        _c23r = 7.0; _c23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c23r, _c23i, x1r, x1i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul24r + x2r; _add25i = _mul24i + x2i;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = k + _c26r; _add27i = 0 + _c26i;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_add27r, _add27i, x1r, x1i, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = k + _c29r; _add30i = 0 + _c29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_add30r, _add30i, x2r, x2i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _mul28r + _mul31r; _add32i = _mul28i + _mul31i;
        double _div33r = 0, _div33i = 0;
        c_div(_add25r, _add25i, _add32r, _add32i, &_div33r, &_div33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = k + _c34r; _add35i = 0 + _c34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_add35r, _add35i, x1r, x1i, &_mul36r, &_mul36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _div33r + _sin37r; _add38i = _div33i + _sin37i;
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = k + _c39r; _add40i = 0 + _c39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_add40r, _add40i, x2r, x2i, &_mul41r, &_mul41i);
        double _cos42r = 0, _cos42i = 0;
        c_cos(_mul41r, _mul41i, &_cos42r, &_cos42i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = _add38r - _cos42r; _sub43i = _add38i - _cos42i;
        double _abs44r = 0, _abs44i = 0;
        _abs44r = c_abs(x1r, x1i); _abs44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 1.0; _c45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = k + _c45r; _add46i = 0 + _c45i;
        double _c47r = 0, _c47i = 0;
        _c47r = 2.0; _c47i = 0;
        double _pow48r = 0, _pow48i = 0;
        c_mul(_add46r, _add46i, _add46r, _add46i, &_pow48r, &_pow48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_abs44r, _abs44i, _pow48r, _pow48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _sub43r + _mul49r; _add50i = _sub43i + _mul49i;
        double _abs51r = 0, _abs51i = 0;
        _abs51r = c_abs(x2r, x2i); _abs51i = 0;
        double _c52r = 0, _c52i = 0;
        _c52r = 1.0; _c52i = 0;
        double _add53r = 0, _add53i = 0;
        _add53r = k + _c52r; _add53i = 0 + _c52i;
        double _c54r = 0, _c54i = 0;
        _c54r = 3.0; _c54i = 0;
        double _pow55r = 0, _pow55i = 0;
        c_mul(_add53r, _add53i, _add53r, _add53i, &_pow55r, &_pow55i);
        c_mul(_pow55r, _pow55i, _add53r, _add53i, &_pow55r, &_pow55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_abs51r, _abs51i, _pow55r, _pow55i, &_mul56r, &_mul56i);
        double _sub57r = 0, _sub57i = 0;
        _sub57r = _add50r - _mul56r; _sub57i = _add50i - _mul56i;
        double _c58r = 0, _c58i = 0;
        _c58r = 1.0; _c58i = 0;
        double _add59r = 0, _add59i = 0;
        _add59r = _sub57r + _c58r; _add59i = _sub57i + _c58i;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    for (int _si = 0; _si < 11; _si++) {
        int _si_idx = _si + 39;
        double _cfrd60r = 0, _cfrd60i = 0;
        _cfrd60r = cRe[(_si + 39)]; _cfrd60i = cIm[(_si + 39)];
        double _add61r = 0, _add61i = 0;
        _add61r = x1r + x2r; _add61i = x1i + x2i;
        double _cos62r = 0, _cos62i = 0;
        c_cos(_add61r, _add61i, &_cos62r, &_cos62i);
        double _smul63r = 0, _smul63i = 0;
        c_mul(_cfrd60r, _cfrd60i, _cos62r, _cos62i, &_smul63r, &_smul63i);
        double _c64r = 0, _c64i = 0;
        _c64r = 0.0; _c64i = 1.0;
        double _sub65r = 0, _sub65i = 0;
        _sub65r = x1r - x2r; _sub65i = x1i - x2i;
        double _sin66r = 0, _sin66i = 0;
        c_sin(_sub65r, _sub65i, &_sin66r, &_sin66i);
        double _mul67r = 0, _mul67i = 0;
        c_mul(_c64r, _c64i, _sin66r, _sin66i, &_mul67r, &_mul67i);
        double _sadd68r = 0, _sadd68i = 0;
        _sadd68r = _smul63r + _mul67r; _sadd68i = _smul63i + _mul67i;
        cRe[_si_idx] = _sadd68r; cIm[_si_idx] = _sadd68i;
    }
    for (int _si = 0; _si < 12; _si++) {
        int _si_idx = _si + 59;
        double _cfrd69r = 0, _cfrd69i = 0;
        _cfrd69r = cRe[(_si + 59)]; _cfrd69i = cIm[(_si + 59)];
        double _add70r = 0, _add70i = 0;
        _add70r = x1r + x2r; _add70i = x1i + x2i;
        double _abs71r = 0, _abs71i = 0;
        _abs71r = c_abs(_add70r, _add70i); _abs71i = 0;
        double _c72r = 0, _c72i = 0;
        _c72r = 1.0; _c72i = 0;
        double _add73r = 0, _add73i = 0;
        _add73r = _abs71r + _c72r; _add73i = _abs71i + _c72i;
        double _log74r = 0, _log74i = 0;
        c_log(_add73r, _add73i, &_log74r, &_log74i);
        double _smul75r = 0, _smul75i = 0;
        c_mul(_cfrd69r, _cfrd69i, _log74r, _log74i, &_smul75r, &_smul75i);
        double _c76r = 0, _c76i = 0;
        _c76r = 0.0; _c76i = 1.0;
        double _sub77r = 0, _sub77i = 0;
        _sub77r = x1r - x2r; _sub77i = x1i - x2i;
        double _abs78r = 0, _abs78i = 0;
        _abs78r = c_abs(_sub77r, _sub77i); _abs78i = 0;
        double _c79r = 0, _c79i = 0;
        _c79r = 1.0; _c79i = 0;
        double _add80r = 0, _add80i = 0;
        _add80r = _abs78r + _c79r; _add80i = _abs78i + _c79i;
        double _log81r = 0, _log81i = 0;
        c_log(_add80r, _add80i, &_log81r, &_log81i);
        double _mul82r = 0, _mul82i = 0;
        c_mul(_c76r, _c76i, _log81r, _log81i, &_mul82r, &_mul82i);
        double _ssub83r = 0, _ssub83i = 0;
        _ssub83r = _smul75r - _mul82r; _ssub83i = _smul75i - _mul82i;
        cRe[_si_idx] = _ssub83r; cIm[_si_idx] = _ssub83i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_112: auto-stubbed (unhandled constructs in source) */
static void poly_giga_112_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_113_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    double _add6r = 0, _add6i = 0;
    _add6r = x1r + x2r; _add6i = x1i + x2i;
    double _c7r = 0, _c7i = 0;
    _c7r = 2.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(_add6r, _add6i, _add6r, _add6i, &_pow8r, &_pow8i);
    double _div9r = 0, _div9i = 0;
    c_div(_add5r, _add5i, _pow8r, _pow8i, &_div9r, &_div9i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div9r; cIm[_idx] = _div9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 1.0;
    double _re11r = 0, _re11i = 0;
    _re11r = x1r; _re11i = 0;
    double _re12r = 0, _re12i = 0;
    _re12r = x2r; _re12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _re11r + _re12r; _add13i = _re11i + _re12i;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c10r, _c10i, _add13r, _add13i, &_mul14r, &_mul14i);
    double _im15r = 0, _im15i = 0;
    _im15r = x1i; _im15i = 0;
    double _im16r = 0, _im16i = 0;
    _im16r = x2i; _im16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = _im15r + _im16r; _add17i = _im15i + _im16i;
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(_add17r, _add17i, _add17r, _add17i, &_pow19r, &_pow19i);
    double _div20r = 0, _div20i = 0;
    c_div(_mul14r, _mul14i, _pow19r, _pow19i, &_div20r, &_div20i);
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div20r; cIm[_idx] = _div20i; } }
    { int _lc_i = 0;
    for (int i = 0; i < 8; i += 1) {
        double _mul21r = 0, _mul21i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = i + _c22r; _add23i = 0 + _c22i;
        double _pow24r = 0, _pow24i = 0;
        c_powc(_mul21r, _mul21i, _add23r, _add23i, &_pow24r, &_pow24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = x1r - x2r; _sub25i = x1i - x2i;
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_sub25r, _sub25i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = i + _c27r; _add28i = 0 + _c27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_abs26r, _abs26i, _add28r, _add28i, &_mul29r, &_mul29i);
        double _div30r = 0, _div30i = 0;
        c_div(_pow24r, _pow24i, _mul29r, _mul29i, &_div30r, &_div30i);
        if (_lc_i < 8) { cRe[2 + _lc_i] = _div30r; cIm[2 + _lc_i] = _div30i; }
        _lc_i++;
    } }
    for (int _si = 0; _si < 11; _si++) {
        int _si_idx = _si + 10;
        double _add31r = 0, _add31i = 0;
        _add31r = x1r + x2r; _add31i = x1i + x2i;
        double _c32r = 0, _c32i = 0;
        _c32r = 2.0; _c32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_mul(_add31r, _add31i, _add31r, _add31i, &_pow33r, &_pow33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(_pow33r, _pow33i, &_sin34r, &_sin34i);
        double _re35r = 0, _re35i = 0;
        _re35r = x1r; _re35i = 0;
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _pow37r = 0, _pow37i = 0;
        c_mul(_re35r, _re35i, _re35r, _re35i, &_pow37r, &_pow37i);
        c_mul(_pow37r, _pow37i, _re35r, _re35i, &_pow37r, &_pow37i);
        double _im38r = 0, _im38i = 0;
        _im38r = x2i; _im38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _pow40r = 0, _pow40i = 0;
        c_mul(_im38r, _im38i, _im38r, _im38i, &_pow40r, &_pow40i);
        c_mul(_pow40r, _pow40i, _im38r, _im38i, &_pow40r, &_pow40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _pow37r + _pow40r; _add41i = _pow37i + _pow40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_sin34r, _sin34i, _add41r, _add41i, &_mul42r, &_mul42i);
        cRe[_si_idx] = _mul42r; cIm[_si_idx] = _mul42i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 21;
        double _sub43r = 0, _sub43i = 0;
        _sub43r = x1r - x2r; _sub43i = x1i - x2i;
        double _c44r = 0, _c44i = 0;
        _c44r = 2.0; _c44i = 0;
        double _pow45r = 0, _pow45i = 0;
        c_mul(_sub43r, _sub43i, _sub43r, _sub43i, &_pow45r, &_pow45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(_pow45r, _pow45i, &_cos46r, &_cos46i);
        double _im47r = 0, _im47i = 0;
        _im47r = x1i; _im47i = 0;
        double _c48r = 0, _c48i = 0;
        _c48r = 3.0; _c48i = 0;
        double _pow49r = 0, _pow49i = 0;
        c_mul(_im47r, _im47i, _im47r, _im47i, &_pow49r, &_pow49i);
        c_mul(_pow49r, _pow49i, _im47r, _im47i, &_pow49r, &_pow49i);
        double _re50r = 0, _re50i = 0;
        _re50r = x2r; _re50i = 0;
        double _c51r = 0, _c51i = 0;
        _c51r = 3.0; _c51i = 0;
        double _pow52r = 0, _pow52i = 0;
        c_mul(_re50r, _re50i, _re50r, _re50i, &_pow52r, &_pow52i);
        c_mul(_pow52r, _pow52i, _re50r, _re50i, &_pow52r, &_pow52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _pow49r + _pow52r; _add53i = _pow49i + _pow52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_cos46r, _cos46i, _add53r, _add53i, &_mul54r, &_mul54i);
        cRe[_si_idx] = _mul54r; cIm[_si_idx] = _mul54i;
    }
    for (int j = 31; j < 43; j++) {
        double _cf55r = 0, _cf55i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 71) { _cf55r = cRe[_idx]; _cf55i = cIm[_idx]; } }
        double _cf56r = 0, _cf56i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 71) { _cf56r = cRe[_idx]; _cf56i = cIm[_idx]; } }
        double _cf57r = 0, _cf57i = 0;
        { int _idx = (j - 2); if (_idx >= 0 && _idx < 71) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
        double _add58r = 0, _add58i = 0;
        _add58r = _cf56r + _cf57r; _add58i = _cf56i + _cf57i;
        double _abs59r = 0, _abs59i = 0;
        _abs59r = c_abs(_add58r, _add58i); _abs59i = 0;
        double _div60r = 0, _div60i = 0;
        c_div(_cf55r, _cf55i, _abs59r, _abs59i, &_div60r, &_div60i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div60r; cIm[_idx] = _div60i; } }
    }
    { int _lc_i = 0;
    for (int i = 0; i < 7; i += 1) {
        double _c61r = 0, _c61i = 0;
        _c61r = 0.0; _c61i = 1.0;
        double _mul62r = 0, _mul62i = 0;
        c_mul(_c61r, _c61i, x2r, x2i, &_mul62r, &_mul62i);
        double _add63r = 0, _add63i = 0;
        _add63r = x1r + _mul62r; _add63i = x1i + _mul62i;
        double _abs64r = 0, _abs64i = 0;
        _abs64r = c_abs(_add63r, _add63i); _abs64i = 0;
        double _log65r = 0, _log65i = 0;
        c_log(_abs64r, _abs64i, &_log65r, &_log65i);
        double _c66r = 0, _c66i = 0;
        _c66r = 1.0; _c66i = 0;
        double _add67r = 0, _add67i = 0;
        _add67r = i + _c66r; _add67i = 0 + _c66i;
        double _c68r = 0, _c68i = 0;
        _c68r = 2.0; _c68i = 0;
        double _pow69r = 0, _pow69i = 0;
        c_mul(_add67r, _add67i, _add67r, _add67i, &_pow69r, &_pow69i);
        double _mul70r = 0, _mul70i = 0;
        c_mul(_log65r, _log65i, _pow69r, _pow69i, &_mul70r, &_mul70i);
        if (_lc_i < 7) { cRe[43 + _lc_i] = _mul70r; cIm[43 + _lc_i] = _mul70i; }
        _lc_i++;
    } }
    { int _lc_i = 0;
    for (int i = 0; i < 5; i += 1) {
        double _add71r = 0, _add71i = 0;
        _add71r = x1r + x2r; _add71i = x1i + x2i;
        double _c72r = 0, _c72i = 0;
        _c72r = 5.0; _c72i = 0;
        double _pow73r = 0, _pow73i = 0;
        c_powr(_add71r, _add71i, 5.0, &_pow73r, &_pow73i);
        double _sub74r = 0, _sub74i = 0;
        _sub74r = x1r - x2r; _sub74i = x1i - x2i;
        double _abs75r = 0, _abs75i = 0;
        _abs75r = c_abs(_sub74r, _sub74i); _abs75i = 0;
        double _c76r = 0, _c76i = 0;
        _c76r = 5.0; _c76i = 0;
        double _sub77r = 0, _sub77i = 0;
        _sub77r = _c76r - i; _sub77i = _c76i - 0;
        double _mul78r = 0, _mul78i = 0;
        c_mul(_abs75r, _abs75i, _sub77r, _sub77i, &_mul78r, &_mul78i);
        double _c79r = 0, _c79i = 0;
        _c79r = 2.0; _c79i = 0;
        double _pow80r = 0, _pow80i = 0;
        c_mul(_mul78r, _mul78i, _mul78r, _mul78i, &_pow80r, &_pow80i);
        double _div81r = 0, _div81i = 0;
        c_div(_pow73r, _pow73i, _pow80r, _pow80i, &_div81r, &_div81i);
        if (_lc_i < 5) { cRe[50 + _lc_i] = _div81r; cIm[50 + _lc_i] = _div81i; }
        _lc_i++;
    } }
    { int _lc_i = 0;
    for (int i = 0; i < 5; i += 1) {
        double _sub82r = 0, _sub82i = 0;
        _sub82r = x1r - x2r; _sub82i = x1i - x2i;
        double _c83r = 0, _c83i = 0;
        _c83r = 4.0; _c83i = 0;
        double _pow84r = 0, _pow84i = 0;
        c_mul(_sub82r, _sub82i, _sub82r, _sub82i, &_pow84r, &_pow84i);
        c_mul(_pow84r, _pow84i, _pow84r, _pow84i, &_pow84r, &_pow84i);
        double _c85r = 0, _c85i = 0;
        _c85r = 0.0; _c85i = 1.0;
        double _mul86r = 0, _mul86i = 0;
        c_mul(x1r, x1i, _c85r, _c85i, &_mul86r, &_mul86i);
        double _mul87r = 0, _mul87i = 0;
        c_mul(_mul86r, _mul86i, x2r, x2i, &_mul87r, &_mul87i);
        double _abs88r = 0, _abs88i = 0;
        _abs88r = c_abs(_mul87r, _mul87i); _abs88i = 0;
        double _c89r = 0, _c89i = 0;
        _c89r = 5.0; _c89i = 0;
        double _sub90r = 0, _sub90i = 0;
        _sub90r = _c89r - i; _sub90i = _c89i - 0;
        double _mul91r = 0, _mul91i = 0;
        c_mul(_abs88r, _abs88i, _sub90r, _sub90i, &_mul91r, &_mul91i);
        double _c92r = 0, _c92i = 0;
        _c92r = 2.0; _c92i = 0;
        double _pow93r = 0, _pow93i = 0;
        c_mul(_mul91r, _mul91i, _mul91r, _mul91i, &_pow93r, &_pow93i);
        double _div94r = 0, _div94i = 0;
        c_div(_pow84r, _pow84i, _pow93r, _pow93i, &_div94r, &_div94i);
        if (_lc_i < 5) { cRe[55 + _lc_i] = _div94r; cIm[55 + _lc_i] = _div94i; }
        _lc_i++;
    } }
    { int _lc_i = 0;
    for (int i = 0; i < 5; i += 1) {
        double _c95r = 0, _c95i = 0;
        _c95r = 0.0; _c95i = 1.0;
        double _mul96r = 0, _mul96i = 0;
        c_mul(x1r, x1i, _c95r, _c95i, &_mul96r, &_mul96i);
        double _mul97r = 0, _mul97i = 0;
        c_mul(_mul96r, _mul96i, x2r, x2i, &_mul97r, &_mul97i);
        double _c98r = 0, _c98i = 0;
        _c98r = 3.0; _c98i = 0;
        double _pow99r = 0, _pow99i = 0;
        c_mul(_mul97r, _mul97i, _mul97r, _mul97i, &_pow99r, &_pow99i);
        c_mul(_pow99r, _pow99i, _mul97r, _mul97i, &_pow99r, &_pow99i);
        double _sub100r = 0, _sub100i = 0;
        _sub100r = x1r - x2r; _sub100i = x1i - x2i;
        double _abs101r = 0, _abs101i = 0;
        _abs101r = c_abs(_sub100r, _sub100i); _abs101i = 0;
        double _c102r = 0, _c102i = 0;
        _c102r = 5.0; _c102i = 0;
        double _sub103r = 0, _sub103i = 0;
        _sub103r = _c102r - i; _sub103i = _c102i - 0;
        double _mul104r = 0, _mul104i = 0;
        c_mul(_abs101r, _abs101i, _sub103r, _sub103i, &_mul104r, &_mul104i);
        double _div105r = 0, _div105i = 0;
        c_div(_pow99r, _pow99i, _mul104r, _mul104i, &_div105r, &_div105i);
        if (_lc_i < 5) { cRe[60 + _lc_i] = _div105r; cIm[60 + _lc_i] = _div105i; }
        _lc_i++;
    } }
    { int _lc_i = 0;
    for (int i = 0; i < 5; i += 1) {
        double _c106r = 0, _c106i = 0;
        _c106r = 2.0; _c106i = 0;
        double _pow107r = 0, _pow107i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow107r, &_pow107i);
        double _c108r = 0, _c108i = 0;
        _c108r = 2.0; _c108i = 0;
        double _pow109r = 0, _pow109i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow109r, &_pow109i);
        double _sub110r = 0, _sub110i = 0;
        _sub110r = _pow107r - _pow109r; _sub110i = _pow107i - _pow109i;
        double _c111r = 0, _c111i = 0;
        _c111r = 1.0; _c111i = 0;
        double _add112r = 0, _add112i = 0;
        _add112r = i + _c111r; _add112i = 0 + _c111i;
        double _c113r = 0, _c113i = 0;
        _c113r = 2.0; _c113i = 0;
        double _pow114r = 0, _pow114i = 0;
        c_mul(_add112r, _add112i, _add112r, _add112i, &_pow114r, &_pow114i);
        double _mul115r = 0, _mul115i = 0;
        c_mul(_sub110r, _sub110i, _pow114r, _pow114i, &_mul115r, &_mul115i);
        double _abs116r = 0, _abs116i = 0;
        _abs116r = c_abs(x1r, x1i); _abs116i = 0;
        double _c117r = 0, _c117i = 0;
        _c117r = 4.0; _c117i = 0;
        double _pow118r = 0, _pow118i = 0;
        c_mul(_abs116r, _abs116i, _abs116r, _abs116i, &_pow118r, &_pow118i);
        c_mul(_pow118r, _pow118i, _pow118r, _pow118i, &_pow118r, &_pow118i);
        double _abs119r = 0, _abs119i = 0;
        _abs119r = c_abs(x2r, x2i); _abs119i = 0;
        double _c120r = 0, _c120i = 0;
        _c120r = 4.0; _c120i = 0;
        double _pow121r = 0, _pow121i = 0;
        c_mul(_abs119r, _abs119i, _abs119r, _abs119i, &_pow121r, &_pow121i);
        c_mul(_pow121r, _pow121i, _pow121r, _pow121i, &_pow121r, &_pow121i);
        double _add122r = 0, _add122i = 0;
        _add122r = _pow118r + _pow121r; _add122i = _pow118i + _pow121i;
        double _div123r = 0, _div123i = 0;
        c_div(_mul115r, _mul115i, _add122r, _add122i, &_div123r, &_div123i);
        if (_lc_i < 5) { cRe[65 + _lc_i] = _div123r; cIm[65 + _lc_i] = _div123i; }
        _lc_i++;
    } }
    double _mul124r = 0, _mul124i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul124r, &_mul124i);
    double _sub125r = 0, _sub125i = 0;
    _sub125r = x1r - x2r; _sub125i = x1i - x2i;
    double _mul126r = 0, _mul126i = 0;
    c_mul(_mul124r, _mul124i, _sub125r, _sub125i, &_mul126r, &_mul126i);
    double _add127r = 0, _add127i = 0;
    _add127r = x1r + x2r; _add127i = x1i + x2i;
    double _abs128r = 0, _abs128i = 0;
    _abs128r = c_abs(_add127r, _add127i); _abs128i = 0;
    double _sub129r = 0, _sub129i = 0;
    _sub129r = x1r - x2r; _sub129i = x1i - x2i;
    double _abs130r = 0, _abs130i = 0;
    _abs130r = c_abs(_sub129r, _sub129i); _abs130i = 0;
    double _mul131r = 0, _mul131i = 0;
    c_mul(_abs128r, _abs128i, _abs130r, _abs130i, &_mul131r, &_mul131i);
    double _div132r = 0, _div132i = 0;
    c_div(_mul126r, _mul126i, _mul131r, _mul131i, &_div132r, &_div132i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div132r; cIm[_idx] = _div132i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_114: auto-stubbed (unhandled constructs in source) */
static void poly_giga_114_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_115_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 0;
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 1.0;
        double _add2r = 0, _add2i = 0;
        _add2r = x1r + _c1r; _add2i = x1i + _c1i;
        double _sin3r = 0, _sin3i = 0;
        c_sin(_add2r, _add2i, &_sin3r, &_sin3i);
        double _arange4r = 0, _arange4i = 0;
        _arange4r = (double)(_si + 1); _arange4i = 0;
        double _smul5r = 0, _smul5i = 0;
        c_mul(_sin3r, _sin3i, _arange4r, _arange4i, &_smul5r, &_smul5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 2.0; _c7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_mul(_abs6r, _abs6i, _abs6r, _abs6i, &_pow8r, &_pow8i);
        double _smul9r = 0, _smul9i = 0;
        c_mul(_smul5r, _smul5i, _pow8r, _pow8i, &_smul9r, &_smul9i);
        cRe[_si_idx] = _smul9r; cIm[_si_idx] = _smul9i;
    }
    for (int _si = 0; _si < 36; _si++) {
        int _si_idx = _si + 35;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _log11r = 0, _log11i = 0;
        c_log(_abs10r, _abs10i, &_log11r, &_log11i);
        double _arange12r = 0, _arange12i = 0;
        _arange12r = (double)(_si + 36); _arange12i = 0;
        double _smul13r = 0, _smul13i = 0;
        c_mul(_log11r, _log11i, _arange12r, _arange12i, &_smul13r, &_smul13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _smul15r = 0, _smul15i = 0;
        c_mul(_smul13r, _smul13i, _abs14r, _abs14i, &_smul15r, &_smul15i);
        cRe[_si_idx] = _smul15r; cIm[_si_idx] = _smul15i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 0;
        double _cfrd16r = 0, _cfrd16i = 0;
        _cfrd16r = cRe[_si]; _cfrd16i = cIm[_si];
        double _smul17r = 0, _smul17i = 0;
        c_mul(_cfrd16r, _cfrd16i, x1r, x1i, &_smul17r, &_smul17i);
        double _sadd18r = 0, _sadd18i = 0;
        _sadd18r = _smul17r + x2r; _sadd18i = _smul17i + x2i;
        cRe[_si_idx] = _sadd18r; cIm[_si_idx] = _sadd18i;
    }
    for (int _si = 0; _si < 11; _si++) {
        int _si_idx = _si + 60;
        double _cfrd19r = 0, _cfrd19i = 0;
        _cfrd19r = cRe[(_si + 60)]; _cfrd19i = cIm[(_si + 60)];
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c20r, _c20i, x2r, x2i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = x1r + _mul21r; _add22i = x1i + _mul21i;
        double _smul23r = 0, _smul23i = 0;
        c_mul(_cfrd19r, _cfrd19i, _add22r, _add22i, &_smul23r, &_smul23i);
        cRe[_si_idx] = _smul23r; cIm[_si_idx] = _smul23i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 10;
        double _cfrd24r = 0, _cfrd24i = 0;
        _cfrd24r = cRe[(_si + 10)]; _cfrd24i = cIm[(_si + 10)];
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow26r, &_pow26i);
        double _sdiv27r = 0, _sdiv27i = 0;
        c_div(_cfrd24r, _cfrd24i, _pow26r, _pow26i, &_sdiv27r, &_sdiv27i);
        cRe[_si_idx] = _sdiv27r; cIm[_si_idx] = _sdiv27i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 20;
        double _cfrd28r = 0, _cfrd28i = 0;
        _cfrd28r = cRe[(_si + 20)]; _cfrd28i = cIm[(_si + 20)];
        double _conj29r = 0, _conj29i = 0;
        _conj29r = x2r; _conj29i = -(x2i);
        double _c30r = 0, _c30i = 0;
        _c30r = 3.0; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_mul(_conj29r, _conj29i, _conj29r, _conj29i, &_pow31r, &_pow31i);
        c_mul(_pow31r, _pow31i, _conj29r, _conj29i, &_pow31r, &_pow31i);
        double _smul32r = 0, _smul32i = 0;
        c_mul(_cfrd28r, _cfrd28i, _pow31r, _pow31i, &_smul32r, &_smul32i);
        cRe[_si_idx] = _smul32r; cIm[_si_idx] = _smul32i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 30;
        double _cfrd33r = 0, _cfrd33i = 0;
        _cfrd33r = cRe[(_si + 30)]; _cfrd33i = cIm[(_si + 30)];
        double _smul34r = 0, _smul34i = 0;
        c_mul(_cfrd33r, _cfrd33i, x1r, x1i, &_smul34r, &_smul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 2.0; _c35i = 0;
        double _re36r = 0, _re36i = 0;
        _re36r = x2r; _re36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c35r, _c35i, _re36r, _re36i, &_mul37r, &_mul37i);
        double _sadd38r = 0, _sadd38i = 0;
        _sadd38r = _smul34r + _mul37r; _sadd38i = _smul34i + _mul37i;
        cRe[_si_idx] = _sadd38r; cIm[_si_idx] = _sadd38i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 40;
        double _cfrd39r = 0, _cfrd39i = 0;
        _cfrd39r = cRe[(_si + 40)]; _cfrd39i = cIm[(_si + 40)];
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, x2r, x2i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = x1r + _mul41r; _add42i = x1i + _mul41i;
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _sub44r = 0, _sub44i = 0;
        _sub44r = _add42r - _c43r; _sub44i = _add42i - _c43i;
        double _sdiv45r = 0, _sdiv45i = 0;
        c_div(_cfrd39r, _cfrd39i, _sub44r, _sub44i, &_sdiv45r, &_sdiv45i);
        cRe[_si_idx] = _sdiv45r; cIm[_si_idx] = _sdiv45i;
    }
    for (int _si = 0; _si < 10; _si++) {
        int _si_idx = _si + 50;
        double _cfrd46r = 0, _cfrd46i = 0;
        _cfrd46r = cRe[(_si + 50)]; _cfrd46i = cIm[(_si + 50)];
        double _c47r = 0, _c47i = 0;
        _c47r = 3.0; _c47i = 0;
        double _mul48r = 0, _mul48i = 0;
        c_mul(_c47r, _c47i, x1r, x1i, &_mul48r, &_mul48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c49r, _c49i, x2r, x2i, &_mul50r, &_mul50i);
        double _sub51r = 0, _sub51i = 0;
        _sub51r = _mul48r - _mul50r; _sub51i = _mul48i - _mul50i;
        double _smul52r = 0, _smul52i = 0;
        c_mul(_cfrd46r, _cfrd46i, _sub51r, _sub51i, &_smul52r, &_smul52i);
        cRe[_si_idx] = _smul52r; cIm[_si_idx] = _smul52i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_116_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 71; i++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x2r, x2i); _abs2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _abs1r + _abs2r; _add3i = _abs1i + _abs2i;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = i + _c4r; _add5i = 0 + _c4i;
        double _mul6r = 0, _mul6i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
        double _div7r = 0, _div7i = 0;
        c_div(_add5r, _add5i, _mul6r, _mul6i, &_div7r, &_div7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_div7r, _div7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_add3r, _add3i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = i + _c10r; _add11i = 0 + _c10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 3.0; _c13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_c12r, _c12i, _c13r, _c13i, &_div14r, &_div14i);
        double _pow15r = 0, _pow15i = 0;
        c_powc(_add11r, _add11i, _div14r, _div14i, &_pow15r, &_pow15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_mul9r, _mul9i, _pow15r, _pow15i, &_mul16r, &_mul16i);
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul16r; cIm[_idx] = _mul16i; } }
    }
    for (int _si = 0; _si < 23; _si++) {
        int _si_idx = _si + 0;
        double _cfrd17r = 0, _cfrd17i = 0;
        _cfrd17r = cRe[_si]; _cfrd17i = cIm[_si];
        double _add18r = 0, _add18i = 0;
        _add18r = x1r + x2r; _add18i = x1i + x2i;
        double _sub19r = 0, _sub19i = 0;
        _sub19r = x1r - x2r; _sub19i = x1i - x2i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_sub19r, _sub19i); _abs20i = 0;
        double _div21r = 0, _div21i = 0;
        c_div(_add18r, _add18i, _abs20r, _abs20i, &_div21r, &_div21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(_div21r, _div21i, _div21r, _div21i, &_pow23r, &_pow23i);
        double _smul24r = 0, _smul24i = 0;
        c_mul(_cfrd17r, _cfrd17i, _pow23r, _pow23i, &_smul24r, &_smul24i);
        cRe[_si_idx] = _smul24r; cIm[_si_idx] = _smul24i;
    }
    for (int _si = 0; _si < 24; _si++) {
        int _si_idx = _si + 23;
        double _cfrd25r = 0, _cfrd25i = 0;
        _cfrd25r = cRe[(_si + 23)]; _cfrd25i = cIm[(_si + 23)];
        double _add26r = 0, _add26i = 0;
        _add26r = x1r + x2r; _add26i = x1i + x2i;
        double _c27r = 0, _c27i = 0;
        _c27r = 3.0; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_mul(_add26r, _add26i, _add26r, _add26i, &_pow28r, &_pow28i);
        c_mul(_pow28r, _pow28i, _add26r, _add26i, &_pow28r, &_pow28i);
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(_pow28r, _pow28i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs29r + _c30r; _add31i = _abs29i + _c30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _smul33r = 0, _smul33i = 0;
        c_mul(_cfrd25r, _cfrd25i, _log32r, _log32i, &_smul33r, &_smul33i);
        cRe[_si_idx] = _smul33r; cIm[_si_idx] = _smul33i;
    }
    for (int _si = 0; _si < 24; _si++) {
        int _si_idx = _si + 47;
        double _cfrd34r = 0, _cfrd34i = 0;
        _cfrd34r = cRe[(_si + 47)]; _cfrd34i = cIm[(_si + 47)];
        double _sub35r = 0, _sub35i = 0;
        _sub35r = x1r - x2r; _sub35i = x1i - x2i;
        double _c36r = 0, _c36i = 0;
        _c36r = 3.0; _c36i = 0;
        double _pow37r = 0, _pow37i = 0;
        c_mul(_sub35r, _sub35i, _sub35r, _sub35i, &_pow37r, &_pow37i);
        c_mul(_pow37r, _pow37i, _sub35r, _sub35i, &_pow37r, &_pow37i);
        double _abs38r = 0, _abs38i = 0;
        _abs38r = c_abs(_pow37r, _pow37i); _abs38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = _abs38r + _c39r; _add40i = _abs38i + _c39i;
        double _log41r = 0, _log41i = 0;
        c_log(_add40r, _add40i, &_log41r, &_log41i);
        double _smul42r = 0, _smul42i = 0;
        c_mul(_cfrd34r, _cfrd34i, _log41r, _log41i, &_smul42r, &_smul42i);
        cRe[_si_idx] = _smul42r; cIm[_si_idx] = _smul42i;
    }
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    { double _tr = cRe[0]*_c43r - cIm[0]*_c43i; cIm[0] = cRe[0]*_c43i + cIm[0]*_c43r; cRe[0] = _tr; }
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    { double _tr = cRe[22]*_c44r - cIm[22]*_c44i; cIm[22] = cRe[22]*_c44i + cIm[22]*_c44r; cRe[22] = _tr; }
    double _c45r = 0, _c45i = 0;
    _c45r = 2.0; _c45i = 0;
    { double _tr = cRe[46]*_c45r - cIm[46]*_c45i; cIm[46] = cRe[46]*_c45i + cIm[46]*_c45r; cRe[46] = _tr; }
    double _c46r = 0, _c46i = 0;
    _c46r = 2.0; _c46i = 0;
    { double _tr = cRe[70]*_c46r - cIm[70]*_c46i; cIm[70] = cRe[70]*_c46i + cIm[70]*_c46r; cRe[70] = _tr; }
    for (int _si = 0; _si < 21; _si++) {
        int _si_idx = _si + 1;
        double _cfrd47r = 0, _cfrd47i = 0;
        _cfrd47r = cRe[(_si + 1)]; _cfrd47i = cIm[(_si + 1)];
        double _re48r = 0, _re48i = 0;
        _re48r = x1r; _re48i = 0;
        double _sdiv49r = 0, _sdiv49i = 0;
        c_div(_cfrd47r, _cfrd47i, _re48r, _re48i, &_sdiv49r, &_sdiv49i);
        cRe[_si_idx] = _sdiv49r; cIm[_si_idx] = _sdiv49i;
    }
    for (int _si = 0; _si < 23; _si++) {
        int _si_idx = _si + 23;
        double _cfrd50r = 0, _cfrd50i = 0;
        _cfrd50r = cRe[(_si + 23)]; _cfrd50i = cIm[(_si + 23)];
        double _im51r = 0, _im51i = 0;
        _im51r = x2i; _im51i = 0;
        double _sdiv52r = 0, _sdiv52i = 0;
        c_div(_cfrd50r, _cfrd50i, _im51r, _im51i, &_sdiv52r, &_sdiv52i);
        cRe[_si_idx] = _sdiv52r; cIm[_si_idx] = _sdiv52i;
    }
    for (int _si = 0; _si < 20; _si++) {
        int _si_idx = _si + 2;
        double _cfrd53r = 0, _cfrd53i = 0;
        _cfrd53r = cRe[(_si + 2)]; _cfrd53i = cIm[(_si + 2)];
        double _cfrd54r = 0, _cfrd54i = 0;
        _cfrd54r = cRe[(_si + 2)]; _cfrd54i = cIm[(_si + 2)];
        double _sfn55r = 0, _sfn55i = 0;
        _sfn55r = c_abs(_cfrd54r, _cfrd54i); _sfn55i = 0;
        double _sdiv56r = 0, _sdiv56i = 0;
        c_div(_cfrd53r, _cfrd53i, _sfn55r, _sfn55i, &_sdiv56r, &_sdiv56i);
        cRe[_si_idx] = _sdiv56r; cIm[_si_idx] = _sdiv56i;
    }
    for (int _si = 0; _si < 22; _si++) {
        int _si_idx = _si + 24;
        double _cfrd57r = 0, _cfrd57i = 0;
        _cfrd57r = cRe[(_si + 24)]; _cfrd57i = cIm[(_si + 24)];
        double _cfrd58r = 0, _cfrd58i = 0;
        _cfrd58r = cRe[(_si + 24)]; _cfrd58i = cIm[(_si + 24)];
        double _sfn59r = 0, _sfn59i = 0;
        _sfn59r = c_abs(_cfrd58r, _cfrd58i); _sfn59i = 0;
        double _sdiv60r = 0, _sdiv60i = 0;
        c_div(_cfrd57r, _cfrd57i, _sfn59r, _sfn59i, &_sdiv60r, &_sdiv60i);
        cRe[_si_idx] = _sdiv60r; cIm[_si_idx] = _sdiv60i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_117: auto-stubbed (unhandled constructs in source) */
static void poly_giga_117_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_118_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 35; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = i + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = i + _c3r; _add4i = 0 + _c3i;
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 1.0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_add4r, _add4i, _c5r, _c5i, &_mul6r, &_mul6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_mul6r, _mul6i, x2r, x2i, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = x1r + _mul7r; _add8i = x1i + _mul7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = i + _c10r; _add11i = 0 + _c10i;
        double _div12r = 0, _div12i = 0;
        c_div(_c9r, _c9i, _add11r, _add11i, &_div12r, &_div12i);
        double _pow13r = 0, _pow13i = 0;
        c_powc(_add8r, _add8i, _div12r, _div12i, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_add2r, _add2i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        { int _idx = i; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
        double _cf15r = 0, _cf15i = 0;
        { int _idx = i; if (_idx >= 0 && _idx < 71) { _cf15r = cRe[_idx]; _cf15i = cIm[_idx]; } }
        double _conj16r = 0, _conj16i = 0;
        _conj16r = _cf15r; _conj16i = -(_cf15i);
        { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _conj16r; cIm[_idx] = _conj16i; } }
    }
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c17r, _c17i, x1r, x1i, &_mul18r, &_mul18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 3.0; _c19i = 0;
    double _abs20r = 0, _abs20i = 0;
    _abs20r = c_abs(x2r, x2i); _abs20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c19r, _c19i, _abs20r, _abs20i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _mul18r + _mul21r; _add22i = _mul18i + _mul21i;
    { int _idx = 35; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_119_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int n = 1; n < 71; n++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = n + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 4.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(_add2r, _add2i, _add2r, _add2i, &_pow4r, &_pow4i);
        c_mul(_pow4r, _pow4i, _pow4r, _pow4i, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_abs5r, _abs5i, _abs6r, _abs6i, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = n + _c8r; _add9i = 0 + _c8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 70.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_add9r, _add9i, _c10r, _c10i, &_div11r, &_div11i);
        double _pow12r = 0, _pow12i = 0;
        c_powc(_mul7r, _mul7i, _div11r, _div11i, &_pow12r, &_pow12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_pow4r, _pow4i, _pow12r, _pow12i, &_mul13r, &_mul13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x1r, x1i); _ang14i = 0;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x2r, x2i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_ang14r, _ang14i, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul13r, _mul13i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = n + _c19r; _add20i = 0 + _c19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_mul18r, _mul18i, _add20r, _add20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = x1r + x2r; _add22i = x1i + x2i;
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_add22r, _add22i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_mul(_abs23r, _abs23i, _abs23r, _abs23i, &_pow25r, &_pow25i);
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
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = n + _c32r; _add33i = 0 + _c32i;
        double _div34r = 0, _div34i = 0;
        c_div(_c31r, _c31i, _add33r, _add33i, &_div34r, &_div34i);
        double _pow35r = 0, _pow35i = 0;
        c_powc(_log30r, _log30i, _div34r, _div34i, &_pow35r, &_pow35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_pow25r, _pow25i, _pow35r, _pow35i, &_mul36r, &_mul36i);
        double _div37r = 0, _div37i = 0;
        c_div(_mul21r, _mul21i, _mul36r, _mul36i, &_div37r, &_div37i);
        { int _idx = n; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div37r; cIm[_idx] = _div37i; } }
    }
    double _mul38r = 0, _mul38i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul38r, &_mul38i);
    double _abs39r = 0, _abs39i = 0;
    _abs39r = c_abs(_mul38r, _mul38i); _abs39i = 0;
    double _c40r = 0, _c40i = 0;
    _c40r = 3.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(_abs39r, _abs39i, _abs39r, _abs39i, &_pow41r, &_pow41i);
    c_mul(_pow41r, _pow41i, _abs39r, _abs39i, &_pow41r, &_pow41i);
    double _add42r = 0, _add42i = 0;
    _add42r = x1r + x2r; _add42i = x1i + x2i;
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(_add42r, _add42i); _abs43i = 0;
    double _c44r = 0, _c44i = 0;
    _c44r = 3.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(_abs43r, _abs43i, _abs43r, _abs43i, &_pow45r, &_pow45i);
    c_mul(_pow45r, _pow45i, _abs43r, _abs43i, &_pow45r, &_pow45i);
    double _sub46r = 0, _sub46i = 0;
    _sub46r = _pow41r - _pow45r; _sub46i = _pow41i - _pow45i;
    double _sub47r = 0, _sub47i = 0;
    _sub47r = x1r - x2r; _sub47i = x1i - x2i;
    double _abs48r = 0, _abs48i = 0;
    _abs48r = c_abs(_sub47r, _sub47i); _abs48i = 0;
    double _c49r = 0, _c49i = 0;
    _c49r = 3.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(_abs48r, _abs48i, _abs48r, _abs48i, &_pow50r, &_pow50i);
    c_mul(_pow50r, _pow50i, _abs48r, _abs48i, &_pow50r, &_pow50i);
    double _add51r = 0, _add51i = 0;
    _add51r = _sub46r + _pow50r; _add51i = _sub46i + _pow50i;
    double _add52r = 0, _add52i = 0;
    _add52r = x1r + x2r; _add52i = x1i + x2i;
    double _abs53r = 0, _abs53i = 0;
    _abs53r = c_abs(_add52r, _add52i); _abs53i = 0;
    double _c54r = 0, _c54i = 0;
    _c54r = 1.0; _c54i = 0;
    double _add55r = 0, _add55i = 0;
    _add55r = _abs53r + _c54r; _add55i = _abs53i + _c54i;
    double _log56r = 0, _log56i = 0;
    c_log(_add55r, _add55i, &_log56r, &_log56i);
    double _c57r = 0, _c57i = 0;
    _c57r = 3.0; _c57i = 0;
    double _pow58r = 0, _pow58i = 0;
    c_mul(_log56r, _log56i, _log56r, _log56i, &_pow58r, &_pow58i);
    c_mul(_pow58r, _pow58i, _log56r, _log56i, &_pow58r, &_pow58i);
    double _add59r = 0, _add59i = 0;
    _add59r = x1r + x2r; _add59i = x1i + x2i;
    double _ang60r = 0, _ang60i = 0;
    _ang60r = c_arg(_add59r, _add59i); _ang60i = 0;
    double _mul61r = 0, _mul61i = 0;
    c_mul(_pow58r, _pow58i, _ang60r, _ang60i, &_mul61r, &_mul61i);
    double _div62r = 0, _div62i = 0;
    c_div(_add51r, _add51i, _mul61r, _mul61i, &_div62r, &_div62i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div62r; cIm[_idx] = _div62i; } }
    for (int _si = 0; _si < 3; _si++) {
        int _si_idx = _si + 2;
        double _cfrd63r = 0, _cfrd63i = 0;
        _cfrd63r = cRe[(_si + 2)]; _cfrd63i = cIm[(_si + 2)];
        double _conj64r = 0, _conj64i = 0;
        _conj64r = x1r; _conj64i = -(x1i);
        double _smul65r = 0, _smul65i = 0;
        c_mul(_cfrd63r, _cfrd63i, _conj64r, _conj64i, &_smul65r, &_smul65i);
        cRe[_si_idx] = _smul65r; cIm[_si_idx] = _smul65i;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 5;
        double _cfrd66r = 0, _cfrd66i = 0;
        _cfrd66r = cRe[(_si + 5)]; _cfrd66i = cIm[(_si + 5)];
        double _conj67r = 0, _conj67i = 0;
        _conj67r = x2r; _conj67i = -(x2i);
        double _smul68r = 0, _smul68i = 0;
        c_mul(_cfrd66r, _cfrd66i, _conj67r, _conj67i, &_smul68r, &_smul68i);
        cRe[_si_idx] = _smul68r; cIm[_si_idx] = _smul68i;
    }
    for (int _si = 0; _si < 7; _si++) {
        int _si_idx = _si + 12;
        double _cfrd69r = 0, _cfrd69i = 0;
        _cfrd69r = cRe[(_si + 12)]; _cfrd69i = cIm[(_si + 12)];
        double _abs70r = 0, _abs70i = 0;
        _abs70r = c_abs(x1r, x1i); _abs70i = 0;
        double _smul71r = 0, _smul71i = 0;
        c_mul(_cfrd69r, _cfrd69i, _abs70r, _abs70i, &_smul71r, &_smul71i);
        double _abs72r = 0, _abs72i = 0;
        _abs72r = c_abs(x2r, x2i); _abs72i = 0;
        double _smul73r = 0, _smul73i = 0;
        c_mul(_smul71r, _smul71i, _abs72r, _abs72i, &_smul73r, &_smul73i);
        cRe[_si_idx] = _smul73r; cIm[_si_idx] = _smul73i;
    }
    for (int _si = 0; _si < 11; _si++) {
        int _si_idx = _si + 19;
        double _cfrd74r = 0, _cfrd74i = 0;
        _cfrd74r = cRe[(_si + 19)]; _cfrd74i = cIm[(_si + 19)];
        double _mul75r = 0, _mul75i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul75r, &_mul75i);
        double _sin76r = 0, _sin76i = 0;
        c_sin(_mul75r, _mul75i, &_sin76r, &_sin76i);
        double _im77r = 0, _im77i = 0;
        _im77r = _sin76i; _im77i = 0;
        double _mul78r = 0, _mul78i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul78r, &_mul78i);
        double _cos79r = 0, _cos79i = 0;
        c_cos(_mul78r, _mul78i, &_cos79r, &_cos79i);
        double _re80r = 0, _re80i = 0;
        _re80r = _cos79r; _re80i = 0;
        double _add81r = 0, _add81i = 0;
        _add81r = _im77r + _re80r; _add81i = _im77i + _re80i;
        double _smul82r = 0, _smul82i = 0;
        c_mul(_cfrd74r, _cfrd74i, _add81r, _add81i, &_smul82r, &_smul82i);
        double _add83r = 0, _add83i = 0;
        _add83r = x1r + x2r; _add83i = x1i + x2i;
        double _abs84r = 0, _abs84i = 0;
        _abs84r = c_abs(_add83r, _add83i); _abs84i = 0;
        double _sdiv85r = 0, _sdiv85i = 0;
        c_div(_smul82r, _smul82i, _abs84r, _abs84i, &_sdiv85r, &_sdiv85i);
        cRe[_si_idx] = _sdiv85r; cIm[_si_idx] = _sdiv85i;
    }
    for (int _si = 0; _si < 20; _si++) {
        int _si_idx = _si + 30;
        double _cfrd86r = 0, _cfrd86i = 0;
        _cfrd86r = cRe[(_si + 30)]; _cfrd86i = cIm[(_si + 30)];
        double _im87r = 0, _im87i = 0;
        _im87r = x1i; _im87i = 0;
        double _abs88r = 0, _abs88i = 0;
        _abs88r = c_abs(x2r, x2i); _abs88i = 0;
        double _c89r = 0, _c89i = 0;
        _c89r = 1.0; _c89i = 0;
        double _add90r = 0, _add90i = 0;
        _add90r = _abs88r + _c89r; _add90i = _abs88i + _c89i;
        double _log91r = 0, _log91i = 0;
        c_log(_add90r, _add90i, &_log91r, &_log91i);
        double _mul92r = 0, _mul92i = 0;
        c_mul(_im87r, _im87i, _log91r, _log91i, &_mul92r, &_mul92i);
        double _re93r = 0, _re93i = 0;
        _re93r = x2r; _re93i = 0;
        double _abs94r = 0, _abs94i = 0;
        _abs94r = c_abs(x1r, x1i); _abs94i = 0;
        double _c95r = 0, _c95i = 0;
        _c95r = 1.0; _c95i = 0;
        double _add96r = 0, _add96i = 0;
        _add96r = _abs94r + _c95r; _add96i = _abs94i + _c95i;
        double _log97r = 0, _log97i = 0;
        c_log(_add96r, _add96i, &_log97r, &_log97i);
        double _mul98r = 0, _mul98i = 0;
        c_mul(_re93r, _re93i, _log97r, _log97i, &_mul98r, &_mul98i);
        double _sub99r = 0, _sub99i = 0;
        _sub99r = _mul92r - _mul98r; _sub99i = _mul92i - _mul98i;
        double _smul100r = 0, _smul100i = 0;
        c_mul(_cfrd86r, _cfrd86i, _sub99r, _sub99i, &_smul100r, &_smul100i);
        double _add101r = 0, _add101i = 0;
        _add101r = x1r + x2r; _add101i = x1i + x2i;
        double _abs102r = 0, _abs102i = 0;
        _abs102r = c_abs(_add101r, _add101i); _abs102i = 0;
        double _smul103r = 0, _smul103i = 0;
        c_mul(_smul100r, _smul100i, _abs102r, _abs102i, &_smul103r, &_smul103i);
        cRe[_si_idx] = _smul103r; cIm[_si_idx] = _smul103i;
    }
    for (int _si = 0; _si < 21; _si++) {
        int _si_idx = _si + 50;
        double _cfrd104r = 0, _cfrd104i = 0;
        _cfrd104r = cRe[(_si + 50)]; _cfrd104i = cIm[(_si + 50)];
        double _re105r = 0, _re105i = 0;
        _re105r = x1r; _re105i = 0;
        double _abs106r = 0, _abs106i = 0;
        _abs106r = c_abs(x2r, x2i); _abs106i = 0;
        double _c107r = 0, _c107i = 0;
        _c107r = 1.0; _c107i = 0;
        double _add108r = 0, _add108i = 0;
        _add108r = _abs106r + _c107r; _add108i = _abs106i + _c107i;
        double _log109r = 0, _log109i = 0;
        c_log(_add108r, _add108i, &_log109r, &_log109i);
        double _mul110r = 0, _mul110i = 0;
        c_mul(_re105r, _re105i, _log109r, _log109i, &_mul110r, &_mul110i);
        double _im111r = 0, _im111i = 0;
        _im111r = x2i; _im111i = 0;
        double _abs112r = 0, _abs112i = 0;
        _abs112r = c_abs(x1r, x1i); _abs112i = 0;
        double _c113r = 0, _c113i = 0;
        _c113r = 1.0; _c113i = 0;
        double _add114r = 0, _add114i = 0;
        _add114r = _abs112r + _c113r; _add114i = _abs112i + _c113i;
        double _log115r = 0, _log115i = 0;
        c_log(_add114r, _add114i, &_log115r, &_log115i);
        double _mul116r = 0, _mul116i = 0;
        c_mul(_im111r, _im111i, _log115r, _log115i, &_mul116r, &_mul116i);
        double _sub117r = 0, _sub117i = 0;
        _sub117r = _mul110r - _mul116r; _sub117i = _mul110i - _mul116i;
        double _smul118r = 0, _smul118i = 0;
        c_mul(_cfrd104r, _cfrd104i, _sub117r, _sub117i, &_smul118r, &_smul118i);
        double _add119r = 0, _add119i = 0;
        _add119r = x1r + x2r; _add119i = x1i + x2i;
        double _abs120r = 0, _abs120i = 0;
        _abs120r = c_abs(_add119r, _add119i); _abs120i = 0;
        double _c121r = 0, _c121i = 0;
        _c121r = 2.0; _c121i = 0;
        double _pow122r = 0, _pow122i = 0;
        c_mul(_abs120r, _abs120i, _abs120r, _abs120i, &_pow122r, &_pow122i);
        double _sdiv123r = 0, _sdiv123i = 0;
        c_div(_smul118r, _smul118i, _pow122r, _pow122i, &_sdiv123r, &_sdiv123i);
        cRe[_si_idx] = _sdiv123r; cIm[_si_idx] = _sdiv123i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_120_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 71; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(_add2r, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 0.0; _c5i = 0;
        if (_mod4r == _c5r) {
            double _abs6r = 0, _abs6i = 0;
            _abs6r = c_abs(x1r, x1i); _abs6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 1.0; _c7i = 0;
            double _add8r = 0, _add8i = 0;
            _add8r = _abs6r + _c7r; _add8i = _abs6i + _c7i;
            double _log9r = 0, _log9i = 0;
            c_log(_add8r, _add8i, &_log9r, &_log9i);
            double _c10r = 0, _c10i = 0;
            _c10r = 1.0; _c10i = 0;
            double _add11r = 0, _add11i = 0;
            _add11r = k + _c10r; _add11i = 0 + _c10i;
            double _pow12r = 0, _pow12i = 0;
            c_powc(_log9r, _log9i, _add11r, _add11i, &_pow12r, &_pow12i);
            double _abs13r = 0, _abs13i = 0;
            _abs13r = c_abs(x2r, x2i); _abs13i = 0;
            double _c14r = 0, _c14i = 0;
            _c14r = 1.0; _c14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = _abs13r + _c14r; _add15i = _abs13i + _c14i;
            double _log16r = 0, _log16i = 0;
            c_log(_add15r, _add15i, &_log16r, &_log16i);
            double _c17r = 0, _c17i = 0;
            _c17r = 71.0; _c17i = 0;
            double _c18r = 0, _c18i = 0;
            _c18r = 1.0; _c18i = 0;
            double _add19r = 0, _add19i = 0;
            _add19r = k + _c18r; _add19i = 0 + _c18i;
            double _sub20r = 0, _sub20i = 0;
            _sub20r = _c17r - _add19r; _sub20i = _c17i - _add19i;
            double _pow21r = 0, _pow21i = 0;
            c_powc(_log16r, _log16i, _sub20r, _sub20i, &_pow21r, &_pow21i);
            double _add22r = 0, _add22i = 0;
            _add22r = _pow12r + _pow21r; _add22i = _pow12i + _pow21i;
            double _c23r = 0, _c23i = 0;
            _c23r = 1.0; _c23i = 0;
            double _add24r = 0, _add24i = 0;
            _add24r = k + _c23r; _add24i = 0 + _c23i;
            double _mul25r = 0, _mul25i = 0;
            c_mul(_add24r, _add24i, x1r, x1i, &_mul25r, &_mul25i);
            double _c26r = 0, _c26i = 0;
            _c26r = 71.0; _c26i = 0;
            double _c27r = 0, _c27i = 0;
            _c27r = 1.0; _c27i = 0;
            double _add28r = 0, _add28i = 0;
            _add28r = k + _c27r; _add28i = 0 + _c27i;
            double _sub29r = 0, _sub29i = 0;
            _sub29r = _c26r - _add28r; _sub29i = _c26i - _add28i;
            double _mul30r = 0, _mul30i = 0;
            c_mul(_sub29r, _sub29i, x2r, x2i, &_mul30r, &_mul30i);
            double _add31r = 0, _add31i = 0;
            _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
            double _sin32r = 0, _sin32i = 0;
            c_sin(_add31r, _add31i, &_sin32r, &_sin32i);
            double _mul33r = 0, _mul33i = 0;
            c_mul(_add22r, _add22i, _sin32r, _sin32i, &_mul33r, &_mul33i);
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
        } else {
            double _abs34r = 0, _abs34i = 0;
            _abs34r = c_abs(x1r, x1i); _abs34i = 0;
            double _c35r = 0, _c35i = 0;
            _c35r = 1.0; _c35i = 0;
            double _add36r = 0, _add36i = 0;
            _add36r = _abs34r + _c35r; _add36i = _abs34i + _c35i;
            double _log37r = 0, _log37i = 0;
            c_log(_add36r, _add36i, &_log37r, &_log37i);
            double _c38r = 0, _c38i = 0;
            _c38r = 1.0; _c38i = 0;
            double _add39r = 0, _add39i = 0;
            _add39r = k + _c38r; _add39i = 0 + _c38i;
            double _pow40r = 0, _pow40i = 0;
            c_powc(_log37r, _log37i, _add39r, _add39i, &_pow40r, &_pow40i);
            double _abs41r = 0, _abs41i = 0;
            _abs41r = c_abs(x2r, x2i); _abs41i = 0;
            double _c42r = 0, _c42i = 0;
            _c42r = 1.0; _c42i = 0;
            double _add43r = 0, _add43i = 0;
            _add43r = _abs41r + _c42r; _add43i = _abs41i + _c42i;
            double _log44r = 0, _log44i = 0;
            c_log(_add43r, _add43i, &_log44r, &_log44i);
            double _c45r = 0, _c45i = 0;
            _c45r = 71.0; _c45i = 0;
            double _c46r = 0, _c46i = 0;
            _c46r = 1.0; _c46i = 0;
            double _add47r = 0, _add47i = 0;
            _add47r = k + _c46r; _add47i = 0 + _c46i;
            double _sub48r = 0, _sub48i = 0;
            _sub48r = _c45r - _add47r; _sub48i = _c45i - _add47i;
            double _pow49r = 0, _pow49i = 0;
            c_powc(_log44r, _log44i, _sub48r, _sub48i, &_pow49r, &_pow49i);
            double _sub50r = 0, _sub50i = 0;
            _sub50r = _pow40r - _pow49r; _sub50i = _pow40i - _pow49i;
            double _c51r = 0, _c51i = 0;
            _c51r = 1.0; _c51i = 0;
            double _add52r = 0, _add52i = 0;
            _add52r = k + _c51r; _add52i = 0 + _c51i;
            double _mul53r = 0, _mul53i = 0;
            c_mul(_add52r, _add52i, x1r, x1i, &_mul53r, &_mul53i);
            double _c54r = 0, _c54i = 0;
            _c54r = 71.0; _c54i = 0;
            double _c55r = 0, _c55i = 0;
            _c55r = 1.0; _c55i = 0;
            double _add56r = 0, _add56i = 0;
            _add56r = k + _c55r; _add56i = 0 + _c55i;
            double _sub57r = 0, _sub57i = 0;
            _sub57r = _c54r - _add56r; _sub57i = _c54i - _add56i;
            double _mul58r = 0, _mul58i = 0;
            c_mul(_sub57r, _sub57i, x2r, x2i, &_mul58r, &_mul58i);
            double _sub59r = 0, _sub59i = 0;
            _sub59r = _mul53r - _mul58r; _sub59i = _mul53i - _mul58i;
            double _cos60r = 0, _cos60i = 0;
            c_cos(_sub59r, _sub59i, &_cos60r, &_cos60i);
            double _mul61r = 0, _mul61i = 0;
            c_mul(_sub50r, _sub50i, _cos60r, _cos60i, &_mul61r, &_mul61i);
            { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul61r; cIm[_idx] = _mul61i; } }
        }
    }
    double _abs62r = 0, _abs62i = 0;
    _abs62r = c_abs(x1r, x1i); _abs62i = 0;
    double _abs63r = 0, _abs63i = 0;
    _abs63r = c_abs(x2r, x2i); _abs63i = 0;
    double _mul64r = 0, _mul64i = 0;
    c_mul(_abs62r, _abs62i, _abs63r, _abs63i, &_mul64r, &_mul64i);
    double r_r = _mul64r, r_i = _mul64i;
    for (int k = 49; k < 71; k++) {
        double _cf65r = 0, _cf65i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { _cf65r = cRe[_idx]; _cf65i = cIm[_idx]; } }
        double _c66r = 0, _c66i = 0;
        _c66r = 1.0; _c66i = 0;
        double _add67r = 0, _add67i = 0;
        _add67r = k + _c66r; _add67i = 0 + _c66i;
        double _c68r = 0, _c68i = 0;
        _c68r = 50.0; _c68i = 0;
        double _sub69r = 0, _sub69i = 0;
        _sub69r = _add67r - _c68r; _sub69i = _add67i - _c68i;
        double _pow70r = 0, _pow70i = 0;
        c_powc(r_r, r_i, _sub69r, _sub69i, &_pow70r, &_pow70i);
        double _mul71r = 0, _mul71i = 0;
        c_mul(_cf65r, _cf65i, _pow70r, _pow70i, &_mul71r, &_mul71i);
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul71r; cIm[_idx] = _mul71i; } }
    }
    for (int k = 14; k < 35; k++) {
        double _cf72r = 0, _cf72i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { _cf72r = cRe[_idx]; _cf72i = cIm[_idx]; } }
        double _c73r = 0, _c73i = 0;
        _c73r = 2.0; _c73i = 0;
        double _mul74r = 0, _mul74i = 0;
        c_mul(_cf72r, _cf72i, _c73r, _c73i, &_mul74r, &_mul74i);
        double _c75r = 0, _c75i = 0;
        _c75r = 71.0; _c75i = 0;
        double _c76r = 0, _c76i = 0;
        _c76r = 1.0; _c76i = 0;
        double _add77r = 0, _add77i = 0;
        _add77r = k + _c76r; _add77i = 0 + _c76i;
        double _sub78r = 0, _sub78i = 0;
        _sub78r = _c75r - _add77r; _sub78i = _c75i - _add77i;
        double _pow79r = 0, _pow79i = 0;
        c_powc(r_r, r_i, _sub78r, _sub78i, &_pow79r, &_pow79i);
        double _mul80r = 0, _mul80i = 0;
        c_mul(_mul74r, _mul74i, _pow79r, _pow79i, &_mul80r, &_mul80i);
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul80r; cIm[_idx] = _mul80i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_121_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 1000.0;
    double _add3r = 0, _add3i = 0;
    _add3r = _attr1r + _c2r; _add3i = _attr1i + _c2i;
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
    for (int k = 2; k < 35; k++) {
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
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div24r; cIm[_idx] = _div24i; } }
    }
    for (int k = 35; k < 70; k++) {
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _neg26r = 0, _neg26i = 0;
        _neg26r = -(_c25r); _neg26i = -(_c25i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = k + _c27r; _add28i = 0 + _c27i;
        double _pow29r = 0, _pow29i = 0;
        c_powc(_neg26r, _neg26i, _add28r, _add28i, &_pow29r, &_pow29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x1r, x1i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 70.0; _c31i = 0;
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _c31r - k; _sub32i = _c31i - 0;
        double _pow33r = 0, _pow33i = 0;
        c_powc(_abs30r, _abs30i, _sub32r, _sub32i, &_pow33r, &_pow33i);
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
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div41r; cIm[_idx] = _div41i; } }
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

static void poly_giga_122_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 35; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 1.0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_c1r, _c1i, x2r, x2i, &_mul2r, &_mul2i);
        double _add3r = 0, _add3i = 0;
        _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powc(_add3r, _add3i, _add5r, _add5i, &_pow6r, &_pow6i);
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = k + _c7r; _add8i = 0 + _c7i;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_add8r, _add8i, x2r, x2i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = x1r + _mul9r; _add10i = x1i + _mul9i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(_add10r, _add10i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _abs11r + _c12r; _add13i = _abs11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul15r, &_mul15i);
        double _re16r = 0, _re16i = 0;
        _re16r = _mul15r; _re16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log14r, _log14i, _re16r, _re16i, &_mul17r, &_mul17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _pow6r + _mul17r; _add18i = _pow6i + _mul17i;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add18r; cIm[_idx] = _add18i; } }
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = k + _c19r; _add20i = 0 + _c19i;
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c21r, _c21i, x2r, x2i, &_mul22r, &_mul22i);
        double _sub23r = 0, _sub23i = 0;
        _sub23r = x1r - _mul22r; _sub23i = x1i - _mul22i;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = k + _c24r; _add25i = 0 + _c24i;
        double _pow26r = 0, _pow26i = 0;
        c_powc(_sub23r, _sub23i, _add25r, _add25i, &_pow26r, &_pow26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_add20r, _add20i, _pow26r, _pow26i, &_mul27r, &_mul27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = k + _c28r; _add29i = 0 + _c28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_add29r, _add29i, x1r, x1i, &_mul30r, &_mul30i);
        double _sub31r = 0, _sub31i = 0;
        _sub31r = x2r - _mul30r; _sub31i = x2i - _mul30i;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(_sub31r, _sub31i); _abs32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _abs32r + _c33r; _add34i = _abs32i + _c33i;
        double _log35r = 0, _log35i = 0;
        c_log(_add34r, _add34i, &_log35r, &_log35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul36r, &_mul36i);
        double _im37r = 0, _im37i = 0;
        _im37r = _mul36i; _im37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_log35r, _log35i, _im37r, _im37i, &_mul38r, &_mul38i);
        double _sub39r = 0, _sub39i = 0;
        _sub39r = _mul27r - _mul38r; _sub39i = _mul27i - _mul38i;
        { int _idx = (70 - k); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub39r; cIm[_idx] = _sub39i; } }
    }
    double _c40r = 0, _c40i = 0;
    _c40r = 100.0; _c40i = 0;
    double _abs41r = 0, _abs41i = 0;
    _abs41r = c_abs(x1r, x1i); _abs41i = 0;
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c40r, _c40i, _abs41r, _abs41i, &_mul42r, &_mul42i);
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(x2r, x2i); _abs43i = 0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_mul42r, _mul42i, _abs43r, _abs43i, &_mul44r, &_mul44i);
    { int _idx = 35; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 200.0; _c45i = 0;
    double _ang46r = 0, _ang46i = 0;
    _ang46r = c_arg(x1r, x1i); _ang46i = 0;
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c45r, _c45i, _ang46r, _ang46i, &_mul47r, &_mul47i);
    double _ang48r = 0, _ang48i = 0;
    _ang48r = c_arg(x2r, x2i); _ang48i = 0;
    double _mul49r = 0, _mul49i = 0;
    c_mul(_mul47r, _mul47i, _ang48r, _ang48i, &_mul49r, &_mul49i);
    { int _idx = 36; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul49r; cIm[_idx] = _mul49i; } }
    for (int _si = 0; _si < 34; _si++) {
        int _si_idx = _si + 37;
        double _cfrd50r = 0, _cfrd50i = 0;
        _cfrd50r = cRe[_si]; _cfrd50i = cIm[_si];
        double _cfrd51r = 0, _cfrd51i = 0;
        _cfrd51r = cRe[(_si + 37)]; _cfrd51i = cIm[(_si + 37)];
        double _ssub52r = 0, _ssub52i = 0;
        _ssub52r = _cfrd50r - _cfrd51r; _ssub52i = _cfrd50i - _cfrd51i;
        cRe[_si_idx] = _ssub52r; cIm[_si_idx] = _ssub52i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_123: auto-stubbed (unhandled constructs in source) */
static void poly_giga_123_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_124_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 70; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double _pow3r = 0, _pow3i = 0;
        c_powc(x1r, x1i, _add2r, _add2i, &_pow3r, &_pow3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = k + _c4r; _add5i = 0 + _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powc(x2r, x2i, _add5r, _add5i, &_pow6r, &_pow6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_pow3r, _pow3i, _pow6r, _pow6i, &_mul7r, &_mul7i);
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    }
    double _add8r = 0, _add8i = 0;
    _add8r = x1r + x2r; _add8i = x1i + x2i;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
    double _log10r = 0, _log10i = 0;
    c_log(_abs9r, _abs9i, &_log10r, &_log10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 1.0; _c11i = 0;
    double _add12r = 0, _add12i = 0;
    _add12r = _log10r + _c11r; _add12i = _log10i + _c11i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
    double _mul13r = 0, _mul13i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul13r, &_mul13i);
    double _abs14r = 0, _abs14i = 0;
    _abs14r = c_abs(_mul13r, _mul13i); _abs14i = 0;
    double _log15r = 0, _log15i = 0;
    c_log(_abs14r, _abs14i, &_log15r, &_log15i);
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = _log15r + _c16r; _add17i = _log15i + _c16i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(x1r, x1i); _abs18i = 0;
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(_abs18r, _abs18i, _abs18r, _abs18i, &_pow20r, &_pow20i);
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(x2r, x2i); _abs21i = 0;
    double _c22r = 0, _c22i = 0;
    _c22r = 2.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(_abs21r, _abs21i, _abs21r, _abs21i, &_pow23r, &_pow23i);
    double _add24r = 0, _add24i = 0;
    _add24r = _pow20r + _pow23r; _add24i = _pow20i + _pow23i;
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add24r; cIm[_idx] = _add24i; } }
    double _abs25r = 0, _abs25i = 0;
    _abs25r = c_abs(x1r, x1i); _abs25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 3.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(_abs25r, _abs25i, _abs25r, _abs25i, &_pow27r, &_pow27i);
    c_mul(_pow27r, _pow27i, _abs25r, _abs25i, &_pow27r, &_pow27i);
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(x2r, x2i); _abs28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 3.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(_abs28r, _abs28i, _abs28r, _abs28i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, _abs28r, _abs28i, &_pow30r, &_pow30i);
    double _sub31r = 0, _sub31i = 0;
    _sub31r = _pow27r - _pow30r; _sub31i = _pow27i - _pow30i;
    { int _idx = 5; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub31r; cIm[_idx] = _sub31i; } }
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(x1r, x1i); _abs32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 4.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_mul(_abs32r, _abs32i, _abs32r, _abs32i, &_pow34r, &_pow34i);
    c_mul(_pow34r, _pow34i, _pow34r, _pow34i, &_pow34r, &_pow34i);
    double _abs35r = 0, _abs35i = 0;
    _abs35r = c_abs(x2r, x2i); _abs35i = 0;
    double _c36r = 0, _c36i = 0;
    _c36r = 4.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(_abs35r, _abs35i, _abs35r, _abs35i, &_pow37r, &_pow37i);
    c_mul(_pow37r, _pow37i, _pow37r, _pow37i, &_pow37r, &_pow37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _pow34r + _pow37r; _add38i = _pow34i + _pow37i;
    { int _idx = 7; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    double _c39r = 0, _c39i = 0;
    _c39r = 5.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_powr(x1r, x1i, 5.0, &_pow40r, &_pow40i);
    double _c41r = 0, _c41i = 0;
    _c41r = 5.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_powr(x2r, x2i, 5.0, &_pow42r, &_pow42i);
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _pow40r - _pow42r; _sub43i = _pow40i - _pow42i;
    { int _idx = 9; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    double _abs44r = 0, _abs44i = 0;
    _abs44r = c_abs(x1r, x1i); _abs44i = 0;
    double _ang45r = 0, _ang45i = 0;
    _ang45r = c_arg(x2r, x2i); _ang45i = 0;
    double _sin46r = 0, _sin46i = 0;
    c_sin(_ang45r, _ang45i, &_sin46r, &_sin46i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_abs44r, _abs44i, _sin46r, _sin46i, &_mul47r, &_mul47i);
    { int _idx = 19; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    double _abs48r = 0, _abs48i = 0;
    _abs48r = c_abs(x2r, x2i); _abs48i = 0;
    double _re49r = 0, _re49i = 0;
    _re49r = x1r; _re49i = 0;
    double _mul50r = 0, _mul50i = 0;
    c_mul(_abs48r, _abs48i, _re49r, _re49i, &_mul50r, &_mul50i);
    { int _idx = 29; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    double _abs51r = 0, _abs51i = 0;
    _abs51r = c_abs(x1r, x1i); _abs51i = 0;
    double _im52r = 0, _im52i = 0;
    _im52r = x2i; _im52i = 0;
    double _mul53r = 0, _mul53i = 0;
    c_mul(_abs51r, _abs51i, _im52r, _im52i, &_mul53r, &_mul53i);
    { int _idx = 39; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(x2r, x2i); _abs54i = 0;
    double _ang55r = 0, _ang55i = 0;
    _ang55r = c_arg(x1r, x1i); _ang55i = 0;
    double _mul56r = 0, _mul56i = 0;
    c_mul(_abs54r, _abs54i, _ang55r, _ang55i, &_mul56r, &_mul56i);
    { int _idx = 49; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    double _mul57r = 0, _mul57i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul57r, &_mul57i);
    double _abs58r = 0, _abs58i = 0;
    _abs58r = c_abs(_mul57r, _mul57i); _abs58i = 0;
    double _ang59r = 0, _ang59i = 0;
    _ang59r = c_arg(x2r, x2i); _ang59i = 0;
    double _cos60r = 0, _cos60i = 0;
    c_cos(_ang59r, _ang59i, &_cos60r, &_cos60i);
    double _mul61r = 0, _mul61i = 0;
    c_mul(_abs58r, _abs58i, _cos60r, _cos60i, &_mul61r, &_mul61i);
    { int _idx = 59; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul61r; cIm[_idx] = _mul61i; } }
    double _mul62r = 0, _mul62i = 0;
    c_mul(x2r, x2i, x1r, x1i, &_mul62r, &_mul62i);
    double _abs63r = 0, _abs63i = 0;
    _abs63r = c_abs(_mul62r, _mul62i); _abs63i = 0;
    double _re64r = 0, _re64i = 0;
    _re64r = x1r; _re64i = 0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(_abs63r, _abs63i, _re64r, _re64i, &_mul65r, &_mul65i);
    { int _idx = 69; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul65r; cIm[_idx] = _mul65i; } }
    double _sum66r = 0, _sum66i = 0;
    _sum66r = 0; _sum66i = 0;
    for (int _si = 0; _si < 70; _si++) { _sum66r += cRe[_si]; _sum66i += cIm[_si]; }
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sum66r; cIm[_idx] = _sum66i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_125_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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

/* poly_giga_126: auto-stubbed (unhandled constructs in source) */
static void poly_giga_126_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_127_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double primes[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0, 59.0, 61.0, 67.0, 71.0, 73.0, 79.0};
    for (int i = 0; i < (int)(sizeof(primes)/sizeof(primes[0])); i++) {
        double _arr1r = 0, _arr1i = 0;
        { int _idx = i; _arr1r = (_idx >= 0 && _idx < 22) ? primes[_idx] : 0.0; _arr1i = 0; }
        double _pow2r = 0, _pow2i = 0;
        c_powc(x1r, x1i, _arr1r, _arr1i, &_pow2r, &_pow2i);
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_abs3r, _abs3i, _abs4r, _abs4i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = x2r + _mul5r; _add6i = x2i + _mul5i;
        double _div7r = 0, _div7i = 0;
        c_div(_pow2r, _pow2i, _add6r, _add6i, &_div7r, &_div7i);
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _div7r; cIm[_idx] = _div7i; } }
        double _arr8r = 0, _arr8i = 0;
        { int _idx = i; _arr8r = (_idx >= 0 && _idx < 22) ? primes[_idx] : 0.0; _arr8i = 0; }
        double _pow9r = 0, _pow9i = 0;
        c_powc(x2r, x2i, _arr8r, _arr8i, &_pow9r, &_pow9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_abs10r, _abs10i, _abs11r, _abs11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = x1r + _mul12r; _add13i = x1i + _mul12i;
        double _div14r = 0, _div14i = 0;
        c_div(_pow9r, _pow9i, _add13r, _add13i, &_div14r, &_div14i);
        { int _idx = (i + 22); if (_idx >= 0 && _idx < 51) { cRe[_idx] = _div14r; cIm[_idx] = _div14i; } }
    }
    double _sin15r = 0, _sin15i = 0;
    c_sin(x1r, x1i, &_sin15r, &_sin15i);
    double _div16r = 0, _div16i = 0;
    c_div(_sin15r, _sin15i, x2r, x2i, &_div16r, &_div16i);
    double _sin17r = 0, _sin17i = 0;
    c_sin(x2r, x2i, &_sin17r, &_sin17i);
    double _div18r = 0, _div18i = 0;
    c_div(_sin17r, _sin17i, x1r, x1i, &_div18r, &_div18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _div16r + _div18r; _add19i = _div16i + _div18i;
    { int _idx = 39; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add19r; cIm[_idx] = _add19i; } }
    for (int _si = 0; _si < 3; _si++) {
        int _si_idx = _si + 40;
        double _add20r = 0, _add20i = 0;
        _add20r = x1r + x2r; _add20i = x1i + x2i;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(_add20r, _add20i); _abs21i = 0;
        double _log22r = 0, _log22i = 0;
        c_log(_abs21r, _abs21i, &_log22r, &_log22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _log22r + _c23r; _add24i = _log22i + _c23i;
        cRe[_si_idx] = _add24r; cIm[_si_idx] = _add24i;
    }
    double _mul25r = 0, _mul25i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul25r, &_mul25i);
    double _im26r = 0, _im26i = 0;
    _im26r = x1i; _im26i = 0;
    double _im27r = 0, _im27i = 0;
    _im27r = x2i; _im27i = 0;
    double _mul28r = 0, _mul28i = 0;
    c_mul(_im26r, _im26i, _im27r, _im27i, &_mul28r, &_mul28i);
    double _add29r = 0, _add29i = 0;
    _add29r = _mul25r + _mul28r; _add29i = _mul25i + _mul28i;
    { int _idx = 43; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    double _mul30r = 0, _mul30i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul30r, &_mul30i);
    double _re31r = 0, _re31i = 0;
    _re31r = x1r; _re31i = 0;
    double _re32r = 0, _re32i = 0;
    _re32r = x2r; _re32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_re31r, _re31i, _re32r, _re32i, &_mul33r, &_mul33i);
    double _sub34r = 0, _sub34i = 0;
    _sub34r = _mul30r - _mul33r; _sub34i = _mul30i - _mul33i;
    { int _idx = 44; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub34r; cIm[_idx] = _sub34i; } }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 45;
        double _re35r = 0, _re35i = 0;
        _re35r = x1r; _re35i = 0;
        double _im36r = 0, _im36i = 0;
        _im36r = x2i; _im36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_re35r, _re35i, _im36r, _im36i, &_mul37r, &_mul37i);
        double _im38r = 0, _im38i = 0;
        _im38r = x1i; _im38i = 0;
        double _re39r = 0, _re39i = 0;
        _re39r = x2r; _re39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_im38r, _im38i, _re39r, _re39i, &_mul40r, &_mul40i);
        double _sub41r = 0, _sub41i = 0;
        _sub41r = _mul37r - _mul40r; _sub41i = _mul37i - _mul40i;
        cRe[_si_idx] = _sub41r; cIm[_si_idx] = _sub41i;
    }
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(x1r, x1i); _abs42i = 0;
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(x2r, x2i); _abs43i = 0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_abs42r, _abs42i, _abs43r, _abs43i, &_mul44r, &_mul44i);
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_128_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
        { int _idx = (k - 1); _arr4r = (_idx >= 0 && _idx < 50) ? triangleNums[_idx] : 0.0; _arr4i = 0; }
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
        { int _idx = (k - 1); _arr13r = (_idx >= 0 && _idx < 50) ? triangleNums[_idx] : 0.0; _arr13i = 0; }
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

/* poly_giga_129: too complex for auto-transpile, stubbed */
static void poly_giga_129_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_130_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    for (int i = 1; i < 51; i++) {
        double _cf2r = 0, _cf2i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 51) { _cf2r = cRe[_idx]; _cf2i = cIm[_idx]; } }
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = i + _c3r; _add4i = 0 + _c3i;
        double _mul5r = 0, _mul5i = 0;
        c_mul(_add4r, _add4i, x1r, x1i, &_mul5r, &_mul5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _cf2r + _mul5r; _add6i = _cf2i + _mul5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = i + _c8r; _add9i = 0 + _c8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_add9r, _add9i, x2r, x2i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _c7r + _mul10r; _add11i = _c7i + _mul10i;
        double _div12r = 0, _div12i = 0;
        c_div(_add6r, _add6i, _add11r, _add11i, &_div12r, &_div12i);
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _div12r; cIm[_idx] = _div12i; } }
    }
    for (int j = 0; j < 25; j++) {
        double _cf13r = 0, _cf13i = 0;
        { int _idx = ((2 * j) + 1); if (_idx >= 0 && _idx < 51) { _cf13r = cRe[_idx]; _cf13i = cIm[_idx]; } }
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _pow16r = 0, _pow16i = 0;
        c_powc(x1r, x1i, _add15r, _add15i, &_pow16r, &_pow16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_cf13r, _cf13i, _pow16r, _pow16i, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _pow20r = 0, _pow20i = 0;
        c_powc(x2r, x2i, _add19r, _add19i, &_pow20r, &_pow20i);
        double _div21r = 0, _div21i = 0;
        c_div(_mul17r, _mul17i, _pow20r, _pow20i, &_div21r, &_div21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_div21r, _div21i); _abs22i = 0;
        { int _idx = ((2 * j) + 1); if (_idx >= 0 && _idx < 51) { cRe[_idx] = _abs22r; cIm[_idx] = _abs22i; } }
    }
    double _add23r = 0, _add23i = 0;
    _add23r = x1r + x2r; _add23i = x1i + x2i;
    double _re24r = 0, _re24i = 0;
    _re24r = _add23r; _re24i = 0;
    double _sub25r = 0, _sub25i = 0;
    _sub25r = x1r - x2r; _sub25i = x1i - x2i;
    double _im26r = 0, _im26i = 0;
    _im26r = _sub25i; _im26i = 0;
    double _add27r = 0, _add27i = 0;
    _add27r = _re24r + _im26r; _add27i = _re24i + _im26i;
    { int _idx = 29; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
    double _ang28r = 0, _ang28i = 0;
    _ang28r = c_arg(x1r, x1i); _ang28i = 0;
    double _ang29r = 0, _ang29i = 0;
    _ang29r = c_arg(x2r, x2i); _ang29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_ang28r, _ang28i, _ang29r, _ang29i, &_mul30r, &_mul30i);
    { int _idx = 39; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _mul31r = 0, _mul31i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul31r, &_mul31i);
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_mul31r, _mul31i); _abs32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 1.0; _c33i = 0;
    double _add34r = 0, _add34i = 0;
    _add34r = _abs32r + _c33r; _add34i = _abs32i + _c33i;
    double _log35r = 0, _log35i = 0;
    c_log(_add34r, _add34i, &_log35r, &_log35i);
    { int _idx = 49; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _log35r; cIm[_idx] = _log35i; } }
    double _cf36r = 0, _cf36i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { _cf36r = cRe[_idx]; _cf36i = cIm[_idx]; } }
    double _c37r = 0, _c37i = 0;
    _c37r = 2.0; _c37i = 0;
    double _pow38r = 0, _pow38i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow38r, &_pow38i);
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow40r, &_pow40i);
    double _add41r = 0, _add41i = 0;
    _add41r = _pow38r + _pow40r; _add41i = _pow38i + _pow40i;
    double _re42r = 0, _re42i = 0;
    _re42r = _add41r; _re42i = 0;
    double _add43r = 0, _add43i = 0;
    _add43r = _cf36r + _re42r; _add43i = _cf36i + _re42i;
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow45r, &_pow45i);
    double _c46r = 0, _c46i = 0;
    _c46r = 2.0; _c46i = 0;
    double _pow47r = 0, _pow47i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow47r, &_pow47i);
    double _sub48r = 0, _sub48i = 0;
    _sub48r = _pow45r - _pow47r; _sub48i = _pow45i - _pow47i;
    double _im49r = 0, _im49i = 0;
    _im49r = _sub48i; _im49i = 0;
    double _sub50r = 0, _sub50i = 0;
    _sub50r = _add43r - _im49r; _sub50i = _add43i - _im49i;
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub50r; cIm[_idx] = _sub50i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_131_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    for (int i = 1; i < 51; i++) {
        double _mul2r = 0, _mul2i = 0;
        c_mul(i, 0, x1r, x1i, &_mul2r, &_mul2i);
        double _sin3r = 0, _sin3i = 0;
        c_sin(_mul2r, _mul2i, &_sin3r, &_sin3i);
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(i, 0, i, 0, &_pow5r, &_pow5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_pow5r, _pow5i, x2r, x2i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_sin3r, _sin3i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _cf9r = 0, _cf9i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 51) { _cf9r = cRe[_idx]; _cf9i = cIm[_idx]; } }
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_cf9r, _cf9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = i + _c11r; _add12i = 0 + _c11i;
        double _pow13r = 0, _pow13i = 0;
        c_powc(_abs10r, _abs10i, _add12r, _add12i, &_pow13r, &_pow13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = i + _c14r; _add15i = 0 + _c14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_pow13r, _pow13i, _add15r, _add15i, &_mul16r, &_mul16i);
        double _log17r = 0, _log17i = 0;
        c_log(_mul16r, _mul16i, &_log17r, &_log17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul8r + _log17r; _add18i = _mul8i + _log17i;
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add18r; cIm[_idx] = _add18i; } }
    }
    double _div19r = 0, _div19i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div19r, &_div19i);
    double _abs20r = 0, _abs20i = 0;
    _abs20r = c_abs(x1r, x1i); _abs20i = 0;
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(x2r, x2i); _abs21i = 0;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_abs20r, _abs20i, _abs21r, _abs21i, &_mul22r, &_mul22i);
    double _add23r = 0, _add23i = 0;
    _add23r = _div19r + _mul22r; _add23i = _div19i + _mul22i;
    { int _idx = 24; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    for (int i = 29; i < 50; i++) {
        double _cf24r = 0, _cf24i = 0;
        { int _idx = i; if (_idx >= 0 && _idx < 51) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _add25r = 0, _add25i = 0;
        _add25r = _cf24r + x1r; _add25i = _cf24i + x1i;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = i + _c26r; _add27i = 0 + _c26i;
        double _div28r = 0, _div28i = 0;
        c_div(x2r, x2i, _add27r, _add27i, &_div28r, &_div28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add25r + _div28r; _add29i = _add25i + _div28i;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = i + _c30r; _add31i = 0 + _c30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(x1r, x1i, _add31r, _add31i, &_mul32r, &_mul32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _add29r - _mul32r; _sub33i = _add29i - _mul32i;
        double _cf34r = 0, _cf34i = 0;
        { int _idx = 24; if (_idx >= 0 && _idx < 51) { _cf34r = cRe[_idx]; _cf34i = cIm[_idx]; } }
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(_cf34r, _cf34i); _abs35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(x2r, x2i, _abs35r, _abs35i, &_mul36r, &_mul36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.5; _c37i = 0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, x2r, x2i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = x1r + _mul38r; _add39i = x1i + _mul38i;
        double _div40r = 0, _div40i = 0;
        c_div(_mul36r, _mul36i, _add39r, _add39i, &_div40r, &_div40i);
        double _sub41r = 0, _sub41i = 0;
        _sub41r = _sub33r - _div40r; _sub41i = _sub33i - _div40i;
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub41r; cIm[_idx] = _sub41i; } }
    }
    double _sum42r = 0, _sum42i = 0;
    _sum42r = 0; _sum42i = 0;
    for (int _si = 24; _si < 49; _si++) { _sum42r += cRe[_si]; _sum42i += cIm[_si]; }
    double _sub43r = 0, _sub43i = 0;
    _sub43r = _sum42r - x2r; _sub43i = _sum42i - x2i;
    { int _idx = 49; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
    double _sum44r = 0, _sum44i = 0;
    _sum44r = 0; _sum44i = 0;
    for (int _si = 48; _si < 50; _si++) { _sum44r += cRe[_si]; _sum44i += cIm[_si]; }
    double _add45r = 0, _add45i = 0;
    _add45r = _sum44r + x1r; _add45i = _sum44i + x1i;
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_132_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double primes[] = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0, 59.0, 61.0, 67.0, 71.0, 73.0, 79.0, 83.0, 89.0, 97.0, 101.0, 103.0, 107.0, 109.0, 113.0, 127.0, 131.0, 137.0, 139.0, 149.0, 151.0, 157.0, 163.0, 167.0, 173.0, 179.0, 181.0, 191.0, 193.0, 197.0, 199.0, 211.0, 223.0, 227.0, 229.0, 233.0, 239.0, 241.0};
    for (int i = 0; i < 50; i++) {
        double _arr1r = 0, _arr1i = 0;
        { int _idx = i; _arr1r = (_idx >= 0 && _idx < 53) ? primes[_idx] : 0.0; _arr1i = 0; }
        double _mul2r = 0, _mul2i = 0;
        c_mul(_arr1r, _arr1i, x1r, x1i, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 1.0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = i + _c4r; _add5i = 0 + _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powc(x2r, x2i, _add5r, _add5i, &_pow6r, &_pow6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c3r, _c3i, _pow6r, _pow6i, &_mul7r, &_mul7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _mul2r + _mul7r; _add8i = _mul2i + _mul7i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _c9r + _abs10r; _add11i = _c9i + _abs10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = i + _c12r; _add13i = 0 + _c12i;
        double _pow14r = 0, _pow14i = 0;
        c_powc(_add11r, _add11i, _add13r, _add13i, &_pow14r, &_pow14i);
        double _div15r = 0, _div15i = 0;
        c_div(_add8r, _add8i, _pow14r, _pow14i, &_div15r, &_div15i);
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
    }
    double _sum16r = 0, _sum16i = 0;
    _sum16r = 0; _sum16i = 0;
    for (int _si = 0; _si < 50; _si++) { _sum16r += cRe[_si]; _sum16i += cIm[_si]; }
    { int _idx = 50; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _sum16r; cIm[_idx] = _sum16i; } }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_giga_133_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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

static void poly_giga_134_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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

/* poly_giga_135: auto-stubbed (unhandled constructs in source) */
static void poly_giga_135_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_136_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c4r, _c4i, x2r, x2i, &_mul5r, &_mul5i);
    double _sub6r = 0, _sub6i = 0;
    _sub6r = _pow3r - _mul5r; _sub6i = _pow3i - _mul5i;
    double _c7r = 0, _c7i = 0;
    _c7r = 5.0; _c7i = 0;
    double _add8r = 0, _add8i = 0;
    _add8r = _sub6r + _c7r; _add8i = _sub6i + _c7i;
    { int _idx = 1; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add8r; cIm[_idx] = _add8i; } }
    double _conj9r = 0, _conj9i = 0;
    _conj9r = x1r; _conj9i = -(x1i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_conj9r, _conj9i, x2r, x2i, &_mul10r, &_mul10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 7.0; _c11i = 0;
    double _add12r = 0, _add12i = 0;
    _add12r = _mul10r + _c11r; _add12i = _mul10i + _c11i;
    { int _idx = 2; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
    double _c13r = 0, _c13i = 0;
    _c13r = 2.0; _c13i = 0;
    double _pow14r = 0, _pow14i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow14r, &_pow14i);
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _pow14r - x1r; _sub15i = _pow14i - x1i;
    double _c16r = 0, _c16i = 0;
    _c16r = 11.0; _c16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = _sub15r + _c16r; _add17i = _sub15i + _c16i;
    { int _idx = 3; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
    double _add18r = 0, _add18i = 0;
    _add18r = x1r + x2r; _add18i = x1i + x2i;
    double _abs19r = 0, _abs19i = 0;
    _abs19r = c_abs(_add18r, _add18i); _abs19i = 0;
    double _c20r = 0, _c20i = 0;
    _c20r = 13.0; _c20i = 0;
    double _add21r = 0, _add21i = 0;
    _add21r = _abs19r + _c20r; _add21i = _abs19i + _c20i;
    { int _idx = 4; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    double _ang22r = 0, _ang22i = 0;
    _ang22r = c_arg(x1r, x1i); _ang22i = 0;
    double _ang23r = 0, _ang23i = 0;
    _ang23r = c_arg(x2r, x2i); _ang23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_ang22r, _ang22i, _ang23r, _ang23i, &_mul24r, &_mul24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 17.0; _c25i = 0;
    double _add26r = 0, _add26i = 0;
    _add26r = _mul24r + _c25r; _add26i = _mul24i + _c25i;
    { int _idx = 5; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add26r; cIm[_idx] = _add26i; } }
    double _mul27r = 0, _mul27i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul27r, &_mul27i);
    double _c28r = 0, _c28i = 0;
    _c28r = 19.0; _c28i = 0;
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _mul27r - _c28r; _sub29i = _mul27i - _c28i;
    { int _idx = 6; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _c30r = 0, _c30i = 0;
    _c30r = 3.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow31r, &_pow31i);
    c_mul(_pow31r, _pow31i, x1r, x1i, &_pow31r, &_pow31i);
    double _c32r = 0, _c32i = 0;
    _c32r = 3.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow33r, &_pow33i);
    c_mul(_pow33r, _pow33i, x2r, x2i, &_pow33r, &_pow33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _pow31r + _pow33r; _add34i = _pow31i + _pow33i;
    double _c35r = 0, _c35i = 0;
    _c35r = 23.0; _c35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _add34r + _c35r; _add36i = _add34i + _c35i;
    { int _idx = 7; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    double _sin37r = 0, _sin37i = 0;
    c_sin(x1r, x1i, &_sin37r, &_sin37i);
    double _cos38r = 0, _cos38i = 0;
    c_cos(x2r, x2i, &_cos38r, &_cos38i);
    double _add39r = 0, _add39i = 0;
    _add39r = _sin37r + _cos38r; _add39i = _sin37i + _cos38i;
    double _c40r = 0, _c40i = 0;
    _c40r = 29.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = _add39r + _c40r; _add41i = _add39i + _c40i;
    { int _idx = 8; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    double _add42r = 0, _add42i = 0;
    _add42r = x1r + x2r; _add42i = x1i + x2i;
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(_add42r, _add42i); _abs43i = 0;
    double _c44r = 0, _c44i = 0;
    _c44r = 1.0; _c44i = 0;
    double _add45r = 0, _add45i = 0;
    _add45r = _abs43r + _c44r; _add45i = _abs43i + _c44i;
    double _log46r = 0, _log46i = 0;
    c_log(_add45r, _add45i, &_log46r, &_log46i);
    double _c47r = 0, _c47i = 0;
    _c47r = 31.0; _c47i = 0;
    double _add48r = 0, _add48i = 0;
    _add48r = _log46r + _c47r; _add48i = _log46i + _c47i;
    { int _idx = 9; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
    double _c49r = 0, _c49i = 0;
    _c49r = 2.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow50r, &_pow50i);
    double _c51r = 0, _c51i = 0;
    _c51r = 2.0; _c51i = 0;
    double _pow52r = 0, _pow52i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow52r, &_pow52i);
    double _sub53r = 0, _sub53i = 0;
    _sub53r = _pow50r - _pow52r; _sub53i = _pow50i - _pow52i;
    double _c54r = 0, _c54i = 0;
    _c54r = 37.0; _c54i = 0;
    double _add55r = 0, _add55i = 0;
    _add55r = _sub53r + _c54r; _add55i = _sub53i + _c54i;
    { int _idx = 10; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    double _conj56r = 0, _conj56i = 0;
    _conj56r = x2r; _conj56i = -(x2i);
    double _mul57r = 0, _mul57i = 0;
    c_mul(_conj56r, _conj56i, x1r, x1i, &_mul57r, &_mul57i);
    double _c58r = 0, _c58i = 0;
    _c58r = 41.0; _c58i = 0;
    double _add59r = 0, _add59i = 0;
    _add59r = _mul57r + _c58r; _add59i = _mul57i + _c58i;
    { int _idx = 11; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    double _im60r = 0, _im60i = 0;
    _im60r = x1i; _im60i = 0;
    double _re61r = 0, _re61i = 0;
    _re61r = x2r; _re61i = 0;
    double _mul62r = 0, _mul62i = 0;
    c_mul(_im60r, _im60i, _re61r, _re61i, &_mul62r, &_mul62i);
    double _c63r = 0, _c63i = 0;
    _c63r = 43.0; _c63i = 0;
    double _sub64r = 0, _sub64i = 0;
    _sub64r = _mul62r - _c63r; _sub64i = _mul62i - _c63i;
    { int _idx = 12; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub64r; cIm[_idx] = _sub64i; } }
    double _conj65r = 0, _conj65i = 0;
    _conj65r = x2r; _conj65i = -(x2i);
    double _mul66r = 0, _mul66i = 0;
    c_mul(x1r, x1i, _conj65r, _conj65i, &_mul66r, &_mul66i);
    double _c67r = 0, _c67i = 0;
    _c67r = 47.0; _c67i = 0;
    double _add68r = 0, _add68i = 0;
    _add68r = _mul66r + _c67r; _add68i = _mul66i + _c67i;
    { int _idx = 13; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add68r; cIm[_idx] = _add68i; } }
    double _sub69r = 0, _sub69i = 0;
    _sub69r = x1r - x2r; _sub69i = x1i - x2i;
    double _abs70r = 0, _abs70i = 0;
    _abs70r = c_abs(_sub69r, _sub69i); _abs70i = 0;
    double _c71r = 0, _c71i = 0;
    _c71r = 53.0; _c71i = 0;
    double _add72r = 0, _add72i = 0;
    _add72r = _abs70r + _c71r; _add72i = _abs70i + _c71i;
    { int _idx = 14; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add72r; cIm[_idx] = _add72i; } }
    double _c73r = 0, _c73i = 0;
    _c73r = 4.0; _c73i = 0;
    double _pow74r = 0, _pow74i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow74r, &_pow74i);
    c_mul(_pow74r, _pow74i, _pow74r, _pow74i, &_pow74r, &_pow74i);
    double _c75r = 0, _c75i = 0;
    _c75r = 4.0; _c75i = 0;
    double _pow76r = 0, _pow76i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow76r, &_pow76i);
    c_mul(_pow76r, _pow76i, _pow76r, _pow76i, &_pow76r, &_pow76i);
    double _sub77r = 0, _sub77i = 0;
    _sub77r = _pow74r - _pow76r; _sub77i = _pow74i - _pow76i;
    double _c78r = 0, _c78i = 0;
    _c78r = 59.0; _c78i = 0;
    double _add79r = 0, _add79i = 0;
    _add79r = _sub77r + _c78r; _add79i = _sub77i + _c78i;
    { int _idx = 15; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add79r; cIm[_idx] = _add79i; } }
    double _c80r = 0, _c80i = 0;
    _c80r = 61.0; _c80i = 0;
    double _c81r = 0, _c81i = 0;
    _c81r = 5.0; _c81i = 0;
    double _mul82r = 0, _mul82i = 0;
    c_mul(_c81r, _c81i, x1r, x1i, &_mul82r, &_mul82i);
    double _mul83r = 0, _mul83i = 0;
    c_mul(_mul82r, _mul82i, x2r, x2i, &_mul83r, &_mul83i);
    double _sub84r = 0, _sub84i = 0;
    _sub84r = _c80r - _mul83r; _sub84i = _c80i - _mul83i;
    { int _idx = 16; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub84r; cIm[_idx] = _sub84i; } }
    double _c85r = 0, _c85i = 0;
    _c85r = 67.0; _c85i = 0;
    double _c86r = 0, _c86i = 0;
    _c86r = 2.0; _c86i = 0;
    double _pow87r = 0, _pow87i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow87r, &_pow87i);
    double _c88r = 0, _c88i = 0;
    _c88r = 2.0; _c88i = 0;
    double _pow89r = 0, _pow89i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow89r, &_pow89i);
    double _add90r = 0, _add90i = 0;
    _add90r = _pow87r + _pow89r; _add90i = _pow87i + _pow89i;
    double _abs91r = 0, _abs91i = 0;
    _abs91r = c_abs(_add90r, _add90i); _abs91i = 0;
    double _add92r = 0, _add92i = 0;
    _add92r = _c85r + _abs91r; _add92i = _c85i + _abs91i;
    { int _idx = 17; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add92r; cIm[_idx] = _add92i; } }
    double _c93r = 0, _c93i = 0;
    _c93r = 71.0; _c93i = 0;
    double _c94r = 0, _c94i = 0;
    _c94r = 5.0; _c94i = 0;
    double _pow95r = 0, _pow95i = 0;
    c_powr(x1r, x1i, 5.0, &_pow95r, &_pow95i);
    double _add96r = 0, _add96i = 0;
    _add96r = _c93r + _pow95r; _add96i = _c93i + _pow95i;
    double _c97r = 0, _c97i = 0;
    _c97r = 5.0; _c97i = 0;
    double _pow98r = 0, _pow98i = 0;
    c_powr(x2r, x2i, 5.0, &_pow98r, &_pow98i);
    double _add99r = 0, _add99i = 0;
    _add99r = _add96r + _pow98r; _add99i = _add96i + _pow98i;
    { int _idx = 18; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add99r; cIm[_idx] = _add99i; } }
    double _c100r = 0, _c100i = 0;
    _c100r = 73.0; _c100i = 0;
    double _ang101r = 0, _ang101i = 0;
    _ang101r = c_arg(x1r, x1i); _ang101i = 0;
    double _ang102r = 0, _ang102i = 0;
    _ang102r = c_arg(x2r, x2i); _ang102i = 0;
    double _mul103r = 0, _mul103i = 0;
    c_mul(_ang101r, _ang101i, _ang102r, _ang102i, &_mul103r, &_mul103i);
    double _sub104r = 0, _sub104i = 0;
    _sub104r = _c100r - _mul103r; _sub104i = _c100i - _mul103i;
    { int _idx = 19; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub104r; cIm[_idx] = _sub104i; } }
    double _c105r = 0, _c105i = 0;
    _c105r = 79.0; _c105i = 0;
    double _c106r = 0, _c106i = 0;
    _c106r = 3.0; _c106i = 0;
    double _pow107r = 0, _pow107i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow107r, &_pow107i);
    c_mul(_pow107r, _pow107i, x1r, x1i, &_pow107r, &_pow107i);
    double _c108r = 0, _c108i = 0;
    _c108r = 3.0; _c108i = 0;
    double _pow109r = 0, _pow109i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow109r, &_pow109i);
    c_mul(_pow109r, _pow109i, x2r, x2i, &_pow109r, &_pow109i);
    double _add110r = 0, _add110i = 0;
    _add110r = _pow107r + _pow109r; _add110i = _pow107i + _pow109i;
    double _abs111r = 0, _abs111i = 0;
    _abs111r = c_abs(_add110r, _add110i); _abs111i = 0;
    double _add112r = 0, _add112i = 0;
    _add112r = _c105r + _abs111r; _add112i = _c105i + _abs111i;
    { int _idx = 20; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add112r; cIm[_idx] = _add112i; } }
    double _c113r = 0, _c113i = 0;
    _c113r = 83.0; _c113i = 0;
    double _c114r = 0, _c114i = 0;
    _c114r = 6.0; _c114i = 0;
    double _pow115r = 0, _pow115i = 0;
    c_powr(x1r, x1i, 6.0, &_pow115r, &_pow115i);
    double _sub116r = 0, _sub116i = 0;
    _sub116r = _c113r - _pow115r; _sub116i = _c113i - _pow115i;
    double _c117r = 0, _c117i = 0;
    _c117r = 6.0; _c117i = 0;
    double _pow118r = 0, _pow118i = 0;
    c_powr(x2r, x2i, 6.0, &_pow118r, &_pow118i);
    double _add119r = 0, _add119i = 0;
    _add119r = _sub116r + _pow118r; _add119i = _sub116i + _pow118i;
    { int _idx = 21; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add119r; cIm[_idx] = _add119i; } }
    double _c120r = 0, _c120i = 0;
    _c120r = 89.0; _c120i = 0;
    double _add121r = 0, _add121i = 0;
    _add121r = x1r + x2r; _add121i = x1i + x2i;
    double _sin122r = 0, _sin122i = 0;
    c_sin(_add121r, _add121i, &_sin122r, &_sin122i);
    double _add123r = 0, _add123i = 0;
    _add123r = _c120r + _sin122r; _add123i = _c120i + _sin122i;
    { int _idx = 22; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add123r; cIm[_idx] = _add123i; } }
    double _re124r = 0, _re124i = 0;
    _re124r = x1r; _re124i = 0;
    double _im125r = 0, _im125i = 0;
    _im125r = x2i; _im125i = 0;
    double _mul126r = 0, _mul126i = 0;
    c_mul(_re124r, _re124i, _im125r, _im125i, &_mul126r, &_mul126i);
    double _abs127r = 0, _abs127i = 0;
    _abs127r = c_abs(_mul126r, _mul126i); _abs127i = 0;
    double _c128r = 0, _c128i = 0;
    _c128r = 97.0; _c128i = 0;
    double _add129r = 0, _add129i = 0;
    _add129r = _abs127r + _c128r; _add129i = _abs127i + _c128i;
    { int _idx = 23; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add129r; cIm[_idx] = _add129i; } }
    double _c130r = 0, _c130i = 0;
    _c130r = 101.0; _c130i = 0;
    double _c131r = 0, _c131i = 0;
    _c131r = 2.0; _c131i = 0;
    double _pow132r = 0, _pow132i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow132r, &_pow132i);
    double _mul133r = 0, _mul133i = 0;
    c_mul(x1r, x1i, _pow132r, _pow132i, &_mul133r, &_mul133i);
    double _add134r = 0, _add134i = 0;
    _add134r = _c130r + _mul133r; _add134i = _c130i + _mul133i;
    { int _idx = 24; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add134r; cIm[_idx] = _add134i; } }
    double _c135r = 0, _c135i = 0;
    _c135r = 103.0; _c135i = 0;
    double _conj136r = 0, _conj136i = 0;
    _conj136r = x1r; _conj136i = -(x1i);
    double _re137r = 0, _re137i = 0;
    _re137r = x2r; _re137i = 0;
    double _mul138r = 0, _mul138i = 0;
    c_mul(_conj136r, _conj136i, _re137r, _re137i, &_mul138r, &_mul138i);
    double _sub139r = 0, _sub139i = 0;
    _sub139r = _c135r - _mul138r; _sub139i = _c135i - _mul138i;
    { int _idx = 25; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub139r; cIm[_idx] = _sub139i; } }
    double _c140r = 0, _c140i = 0;
    _c140r = 107.0; _c140i = 0;
    double _c141r = 0, _c141i = 0;
    _c141r = 7.0; _c141i = 0;
    double _pow142r = 0, _pow142i = 0;
    c_powr(x1r, x1i, 7.0, &_pow142r, &_pow142i);
    double _add143r = 0, _add143i = 0;
    _add143r = _c140r + _pow142r; _add143i = _c140i + _pow142i;
    double _c144r = 0, _c144i = 0;
    _c144r = 7.0; _c144i = 0;
    double _pow145r = 0, _pow145i = 0;
    c_powr(x2r, x2i, 7.0, &_pow145r, &_pow145i);
    double _sub146r = 0, _sub146i = 0;
    _sub146r = _add143r - _pow145r; _sub146i = _add143i - _pow145i;
    { int _idx = 26; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub146r; cIm[_idx] = _sub146i; } }
    double _c147r = 0, _c147i = 0;
    _c147r = 109.0; _c147i = 0;
    double _sub148r = 0, _sub148i = 0;
    _sub148r = x1r - x2r; _sub148i = x1i - x2i;
    double _conj149r = 0, _conj149i = 0;
    _conj149r = _sub148r; _conj149i = -(_sub148i);
    double _abs150r = 0, _abs150i = 0;
    _abs150r = c_abs(_conj149r, _conj149i); _abs150i = 0;
    double _add151r = 0, _add151i = 0;
    _add151r = _c147r + _abs150r; _add151i = _c147i + _abs150i;
    { int _idx = 27; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add151r; cIm[_idx] = _add151i; } }
    double _c152r = 0, _c152i = 0;
    _c152r = 113.0; _c152i = 0;
    double _c153r = 0, _c153i = 0;
    _c153r = 2.0; _c153i = 0;
    double _pow154r = 0, _pow154i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow154r, &_pow154i);
    double _c155r = 0, _c155i = 0;
    _c155r = 2.0; _c155i = 0;
    double _pow156r = 0, _pow156i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow156r, &_pow156i);
    double _sub157r = 0, _sub157i = 0;
    _sub157r = _pow154r - _pow156r; _sub157i = _pow154i - _pow156i;
    double _abs158r = 0, _abs158i = 0;
    _abs158r = c_abs(_sub157r, _sub157i); _abs158i = 0;
    double _sub159r = 0, _sub159i = 0;
    _sub159r = _c152r - _abs158r; _sub159i = _c152i - _abs158i;
    { int _idx = 28; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub159r; cIm[_idx] = _sub159i; } }
    double _c160r = 0, _c160i = 0;
    _c160r = 127.0; _c160i = 0;
    double _c161r = 0, _c161i = 0;
    _c161r = 8.0; _c161i = 0;
    double _pow162r = 0, _pow162i = 0;
    c_powr(x1r, x1i, 8.0, &_pow162r, &_pow162i);
    double _c163r = 0, _c163i = 0;
    _c163r = 8.0; _c163i = 0;
    double _pow164r = 0, _pow164i = 0;
    c_powr(x2r, x2i, 8.0, &_pow164r, &_pow164i);
    double _mul165r = 0, _mul165i = 0;
    c_mul(_pow162r, _pow162i, _pow164r, _pow164i, &_mul165r, &_mul165i);
    double _add166r = 0, _add166i = 0;
    _add166r = _c160r + _mul165r; _add166i = _c160i + _mul165i;
    { int _idx = 29; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add166r; cIm[_idx] = _add166i; } }
    double _sub167r = 0, _sub167i = 0;
    _sub167r = x1r - x2r; _sub167i = x1i - x2i;
    double _mul168r = 0, _mul168i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul168r, &_mul168i);
    double _abs169r = 0, _abs169i = 0;
    _abs169r = c_abs(_mul168r, _mul168i); _abs169i = 0;
    double _add170r = 0, _add170i = 0;
    _add170r = _sub167r + _abs169r; _add170i = _sub167i + _abs169i;
    double _c171r = 0, _c171i = 0;
    _c171r = 131.0; _c171i = 0;
    double _add172r = 0, _add172i = 0;
    _add172r = _add170r + _c171r; _add172i = _add170i + _c171i;
    { int _idx = 30; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add172r; cIm[_idx] = _add172i; } }
    double _c173r = 0, _c173i = 0;
    _c173r = 137.0; _c173i = 0;
    double _c174r = 0, _c174i = 0;
    _c174r = 2.0; _c174i = 0;
    double _pow175r = 0, _pow175i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow175r, &_pow175i);
    double _ang176r = 0, _ang176i = 0;
    _ang176r = c_arg(_pow175r, _pow175i); _ang176i = 0;
    double _add177r = 0, _add177i = 0;
    _add177r = _c173r + _ang176r; _add177i = _c173i + _ang176i;
    double _c178r = 0, _c178i = 0;
    _c178r = 2.0; _c178i = 0;
    double _pow179r = 0, _pow179i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow179r, &_pow179i);
    double _ang180r = 0, _ang180i = 0;
    _ang180r = c_arg(_pow179r, _pow179i); _ang180i = 0;
    double _sub181r = 0, _sub181i = 0;
    _sub181r = _add177r - _ang180r; _sub181i = _add177i - _ang180i;
    { int _idx = 31; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub181r; cIm[_idx] = _sub181i; } }
    double _c182r = 0, _c182i = 0;
    _c182r = 139.0; _c182i = 0;
    double _c183r = 0, _c183i = 0;
    _c183r = 9.0; _c183i = 0;
    double _pow184r = 0, _pow184i = 0;
    c_powr(x1r, x1i, 9.0, &_pow184r, &_pow184i);
    double _sub185r = 0, _sub185i = 0;
    _sub185r = _c182r - _pow184r; _sub185i = _c182i - _pow184i;
    double _c186r = 0, _c186i = 0;
    _c186r = 9.0; _c186i = 0;
    double _pow187r = 0, _pow187i = 0;
    c_powr(x2r, x2i, 9.0, &_pow187r, &_pow187i);
    double _add188r = 0, _add188i = 0;
    _add188r = _sub185r + _pow187r; _add188i = _sub185i + _pow187i;
    { int _idx = 32; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add188r; cIm[_idx] = _add188i; } }
    double _mul189r = 0, _mul189i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul189r, &_mul189i);
    double _abs190r = 0, _abs190i = 0;
    _abs190r = c_abs(_mul189r, _mul189i); _abs190i = 0;
    double _c191r = 0, _c191i = 0;
    _c191r = 1.0; _c191i = 0;
    double _add192r = 0, _add192i = 0;
    _add192r = _abs190r + _c191r; _add192i = _abs190i + _c191i;
    double _log193r = 0, _log193i = 0;
    c_log(_add192r, _add192i, &_log193r, &_log193i);
    double _c194r = 0, _c194i = 0;
    _c194r = 149.0; _c194i = 0;
    double _add195r = 0, _add195i = 0;
    _add195r = _log193r + _c194r; _add195i = _log193i + _c194i;
    { int _idx = 33; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add195r; cIm[_idx] = _add195i; } }
    double _c196r = 0, _c196i = 0;
    _c196r = 151.0; _c196i = 0;
    double _abs197r = 0, _abs197i = 0;
    _abs197r = c_abs(x1r, x1i); _abs197i = 0;
    double _abs198r = 0, _abs198i = 0;
    _abs198r = c_abs(x2r, x2i); _abs198i = 0;
    double _add199r = 0, _add199i = 0;
    _add199r = _abs197r + _abs198r; _add199i = _abs197i + _abs198i;
    double _c200r = 0, _c200i = 0;
    _c200r = 2.0; _c200i = 0;
    double _pow201r = 0, _pow201i = 0;
    c_mul(_add199r, _add199i, _add199r, _add199i, &_pow201r, &_pow201i);
    double _add202r = 0, _add202i = 0;
    _add202r = _c196r + _pow201r; _add202i = _c196i + _pow201i;
    { int _idx = 34; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add202r; cIm[_idx] = _add202i; } }
    double _c203r = 0, _c203i = 0;
    _c203r = 2.0; _c203i = 0;
    double _mul204r = 0, _mul204i = 0;
    c_mul(_c203r, _c203i, x1r, x1i, &_mul204r, &_mul204i);
    double _sin205r = 0, _sin205i = 0;
    c_sin(_mul204r, _mul204i, &_sin205r, &_sin205i);
    double _c206r = 0, _c206i = 0;
    _c206r = 2.0; _c206i = 0;
    double _mul207r = 0, _mul207i = 0;
    c_mul(_c206r, _c206i, x2r, x2i, &_mul207r, &_mul207i);
    double _cos208r = 0, _cos208i = 0;
    c_cos(_mul207r, _mul207i, &_cos208r, &_cos208i);
    double _sub209r = 0, _sub209i = 0;
    _sub209r = _sin205r - _cos208r; _sub209i = _sin205i - _cos208i;
    double _c210r = 0, _c210i = 0;
    _c210r = 157.0; _c210i = 0;
    double _add211r = 0, _add211i = 0;
    _add211r = _sub209r + _c210r; _add211i = _sub209i + _c210i;
    { int _idx = 35; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add211r; cIm[_idx] = _add211i; } }
    double _sub212r = 0, _sub212i = 0;
    _sub212r = x1r - x2r; _sub212i = x1i - x2i;
    double _abs213r = 0, _abs213i = 0;
    _abs213r = c_abs(_sub212r, _sub212i); _abs213i = 0;
    double _c214r = 0, _c214i = 0;
    _c214r = 1.0; _c214i = 0;
    double _add215r = 0, _add215i = 0;
    _add215r = _abs213r + _c214r; _add215i = _abs213i + _c214i;
    double _log216r = 0, _log216i = 0;
    c_log(_add215r, _add215i, &_log216r, &_log216i);
    double _c217r = 0, _c217i = 0;
    _c217r = 163.0; _c217i = 0;
    double _add218r = 0, _add218i = 0;
    _add218r = _log216r + _c217r; _add218i = _log216i + _c217i;
    { int _idx = 36; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add218r; cIm[_idx] = _add218i; } }
    double _c219r = 0, _c219i = 0;
    _c219r = 167.0; _c219i = 0;
    double _c220r = 0, _c220i = 0;
    _c220r = 3.0; _c220i = 0;
    double _pow221r = 0, _pow221i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow221r, &_pow221i);
    c_mul(_pow221r, _pow221i, x1r, x1i, &_pow221r, &_pow221i);
    double _re222r = 0, _re222i = 0;
    _re222r = _pow221r; _re222i = 0;
    double _add223r = 0, _add223i = 0;
    _add223r = _c219r + _re222r; _add223i = _c219i + _re222i;
    double _c224r = 0, _c224i = 0;
    _c224r = 3.0; _c224i = 0;
    double _pow225r = 0, _pow225i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow225r, &_pow225i);
    c_mul(_pow225r, _pow225i, x2r, x2i, &_pow225r, &_pow225i);
    double _im226r = 0, _im226i = 0;
    _im226r = _pow225i; _im226i = 0;
    double _sub227r = 0, _sub227i = 0;
    _sub227r = _add223r - _im226r; _sub227i = _add223i - _im226i;
    { int _idx = 37; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub227r; cIm[_idx] = _sub227i; } }
    double _c228r = 0, _c228i = 0;
    _c228r = 173.0; _c228i = 0;
    double _c229r = 0, _c229i = 0;
    _c229r = 2.0; _c229i = 0;
    double _pow230r = 0, _pow230i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow230r, &_pow230i);
    double _c231r = 0, _c231i = 0;
    _c231r = 2.0; _c231i = 0;
    double _pow232r = 0, _pow232i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow232r, &_pow232i);
    double _mul233r = 0, _mul233i = 0;
    c_mul(_pow230r, _pow230i, _pow232r, _pow232i, &_mul233r, &_mul233i);
    double _c234r = 0, _c234i = 0;
    _c234r = 1.5; _c234i = 0;
    double _pow235r = 0, _pow235i = 0;
    c_powr(_mul233r, _mul233i, 1.5, &_pow235r, &_pow235i);
    double _sub236r = 0, _sub236i = 0;
    _sub236r = _c228r - _pow235r; _sub236i = _c228i - _pow235i;
    { int _idx = 38; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub236r; cIm[_idx] = _sub236i; } }
    double _c237r = 0, _c237i = 0;
    _c237r = 179.0; _c237i = 0;
    double _mul238r = 0, _mul238i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul238r, &_mul238i);
    double _ang239r = 0, _ang239i = 0;
    _ang239r = c_arg(_mul238r, _mul238i); _ang239i = 0;
    double _add240r = 0, _add240i = 0;
    _add240r = _c237r + _ang239r; _add240i = _c237i + _ang239i;
    double _c241r = 0, _c241i = 0;
    _c241r = 0.0; _c241i = 1.0;
    double _add242r = 0, _add242i = 0;
    _add242r = _add240r + _c241r; _add242i = _add240i + _c241i;
    { int _idx = 39; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add242r; cIm[_idx] = _add242i; } }
    double _c243r = 0, _c243i = 0;
    _c243r = 181.0; _c243i = 0;
    double _c244r = 0, _c244i = 0;
    _c244r = 3.0; _c244i = 0;
    double _pow245r = 0, _pow245i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow245r, &_pow245i);
    c_mul(_pow245r, _pow245i, x1r, x1i, &_pow245r, &_pow245i);
    double _c246r = 0, _c246i = 0;
    _c246r = 3.0; _c246i = 0;
    double _pow247r = 0, _pow247i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow247r, &_pow247i);
    c_mul(_pow247r, _pow247i, x2r, x2i, &_pow247r, &_pow247i);
    double _sub248r = 0, _sub248i = 0;
    _sub248r = _pow245r - _pow247r; _sub248i = _pow245i - _pow247i;
    double _conj249r = 0, _conj249i = 0;
    _conj249r = _sub248r; _conj249i = -(_sub248i);
    double _sub250r = 0, _sub250i = 0;
    _sub250r = _c243r - _conj249r; _sub250i = _c243i - _conj249i;
    { int _idx = 40; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub250r; cIm[_idx] = _sub250i; } }
    double _c251r = 0, _c251i = 0;
    _c251r = 191.0; _c251i = 0;
    double _abs252r = 0, _abs252i = 0;
    _abs252r = c_abs(x1r, x1i); _abs252i = 0;
    double _abs253r = 0, _abs253i = 0;
    _abs253r = c_abs(x2r, x2i); _abs253i = 0;
    double _mul254r = 0, _mul254i = 0;
    c_mul(_abs252r, _abs252i, _abs253r, _abs253i, &_mul254r, &_mul254i);
    double _add255r = 0, _add255i = 0;
    _add255r = _c251r + _mul254r; _add255i = _c251i + _mul254i;
    { int _idx = 41; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add255r; cIm[_idx] = _add255i; } }
    double _c256r = 0, _c256i = 0;
    _c256r = 193.0; _c256i = 0;
    double _re257r = 0, _re257i = 0;
    _re257r = x1r; _re257i = 0;
    double _im258r = 0, _im258i = 0;
    _im258r = x2i; _im258i = 0;
    double _add259r = 0, _add259i = 0;
    _add259r = _re257r + _im258r; _add259i = _re257i + _im258i;
    double _abs260r = 0, _abs260i = 0;
    _abs260r = c_abs(_add259r, _add259i); _abs260i = 0;
    double _sub261r = 0, _sub261i = 0;
    _sub261r = _c256r - _abs260r; _sub261i = _c256i - _abs260i;
    { int _idx = 42; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub261r; cIm[_idx] = _sub261i; } }
    double _c262r = 0, _c262i = 0;
    _c262r = 197.0; _c262i = 0;
    double _c263r = 0, _c263i = 0;
    _c263r = 2.0; _c263i = 0;
    double _pow264r = 0, _pow264i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow264r, &_pow264i);
    double _c265r = 0, _c265i = 0;
    _c265r = 2.0; _c265i = 0;
    double _pow266r = 0, _pow266i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow266r, &_pow266i);
    double _add267r = 0, _add267i = 0;
    _add267r = _pow264r + _pow266r; _add267i = _pow264i + _pow266i;
    double _sin268r = 0, _sin268i = 0;
    c_sin(_add267r, _add267i, &_sin268r, &_sin268i);
    double _add269r = 0, _add269i = 0;
    _add269r = _c262r + _sin268r; _add269i = _c262i + _sin268i;
    { int _idx = 43; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add269r; cIm[_idx] = _add269i; } }
    double _c270r = 0, _c270i = 0;
    _c270r = 199.0; _c270i = 0;
    double _c271r = 0, _c271i = 0;
    _c271r = 3.0; _c271i = 0;
    double _pow272r = 0, _pow272i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow272r, &_pow272i);
    c_mul(_pow272r, _pow272i, x2r, x2i, &_pow272r, &_pow272i);
    double _mul273r = 0, _mul273i = 0;
    c_mul(x1r, x1i, _pow272r, _pow272i, &_mul273r, &_mul273i);
    double _sub274r = 0, _sub274i = 0;
    _sub274r = _c270r - _mul273r; _sub274i = _c270i - _mul273i;
    { int _idx = 44; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub274r; cIm[_idx] = _sub274i; } }
    double _im275r = 0, _im275i = 0;
    _im275r = x2i; _im275i = 0;
    double _mul276r = 0, _mul276i = 0;
    c_mul(x1r, x1i, _im275r, _im275i, &_mul276r, &_mul276i);
    double _c277r = 0, _c277i = 0;
    _c277r = 211.0; _c277i = 0;
    double _add278r = 0, _add278i = 0;
    _add278r = _mul276r + _c277r; _add278i = _mul276i + _c277i;
    { int _idx = 45; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add278r; cIm[_idx] = _add278i; } }
    double _c279r = 0, _c279i = 0;
    _c279r = 4.0; _c279i = 0;
    double _pow280r = 0, _pow280i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow280r, &_pow280i);
    c_mul(_pow280r, _pow280i, _pow280r, _pow280i, &_pow280r, &_pow280i);
    double _c281r = 0, _c281i = 0;
    _c281r = 4.0; _c281i = 0;
    double _pow282r = 0, _pow282i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow282r, &_pow282i);
    c_mul(_pow282r, _pow282i, _pow282r, _pow282i, &_pow282r, &_pow282i);
    double _add283r = 0, _add283i = 0;
    _add283r = _pow280r + _pow282r; _add283i = _pow280i + _pow282i;
    double _abs284r = 0, _abs284i = 0;
    _abs284r = c_abs(_add283r, _add283i); _abs284i = 0;
    double _c285r = 0, _c285i = 0;
    _c285r = 223.0; _c285i = 0;
    double _add286r = 0, _add286i = 0;
    _add286r = _abs284r + _c285r; _add286i = _abs284i + _c285i;
    { int _idx = 46; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add286r; cIm[_idx] = _add286i; } }
    double _c287r = 0, _c287i = 0;
    _c287r = 227.0; _c287i = 0;
    double _c288r = 0, _c288i = 0;
    _c288r = 2.0; _c288i = 0;
    double _pow289r = 0, _pow289i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow289r, &_pow289i);
    double _conj290r = 0, _conj290i = 0;
    _conj290r = _pow289r; _conj290i = -(_pow289i);
    double _c291r = 0, _c291i = 0;
    _c291r = 2.0; _c291i = 0;
    double _pow292r = 0, _pow292i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow292r, &_pow292i);
    double _conj293r = 0, _conj293i = 0;
    _conj293r = _pow292r; _conj293i = -(_pow292i);
    double _mul294r = 0, _mul294i = 0;
    c_mul(_conj290r, _conj290i, _conj293r, _conj293i, &_mul294r, &_mul294i);
    double _sub295r = 0, _sub295i = 0;
    _sub295r = _c287r - _mul294r; _sub295i = _c287i - _mul294i;
    { int _idx = 47; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub295r; cIm[_idx] = _sub295i; } }
    double _c296r = 0, _c296i = 0;
    _c296r = 229.0; _c296i = 0;
    double _mul297r = 0, _mul297i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul297r, &_mul297i);
    double _sin298r = 0, _sin298i = 0;
    c_sin(_mul297r, _mul297i, &_sin298r, &_sin298i);
    double _add299r = 0, _add299i = 0;
    _add299r = _c296r + _sin298r; _add299i = _c296i + _sin298i;
    double _sub300r = 0, _sub300i = 0;
    _sub300r = x1r - x2r; _sub300i = x1i - x2i;
    double _cos301r = 0, _cos301i = 0;
    c_cos(_sub300r, _sub300i, &_cos301r, &_cos301i);
    double _sub302r = 0, _sub302i = 0;
    _sub302r = _add299r - _cos301r; _sub302i = _add299i - _cos301i;
    { int _idx = 48; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub302r; cIm[_idx] = _sub302i; } }
    double _c303r = 0, _c303i = 0;
    _c303r = 233.0; _c303i = 0;
    double _c304r = 0, _c304i = 0;
    _c304r = 9.0; _c304i = 0;
    double _pow305r = 0, _pow305i = 0;
    c_powr(x1r, x1i, 9.0, &_pow305r, &_pow305i);
    double _add306r = 0, _add306i = 0;
    _add306r = _c303r + _pow305r; _add306i = _c303i + _pow305i;
    double _c307r = 0, _c307i = 0;
    _c307r = 9.0; _c307i = 0;
    double _pow308r = 0, _pow308i = 0;
    c_powr(x2r, x2i, 9.0, &_pow308r, &_pow308i);
    double _sub309r = 0, _sub309i = 0;
    _sub309r = _add306r - _pow308r; _sub309i = _add306i - _pow308i;
    { int _idx = 49; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub309r; cIm[_idx] = _sub309i; } }
    double _c310r = 0, _c310i = 0;
    _c310r = 239.0; _c310i = 0;
    double _c311r = 0, _c311i = 0;
    _c311r = 2.0; _c311i = 0;
    double _pow312r = 0, _pow312i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow312r, &_pow312i);
    double _c313r = 0, _c313i = 0;
    _c313r = 2.0; _c313i = 0;
    double _pow314r = 0, _pow314i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow314r, &_pow314i);
    double _add315r = 0, _add315i = 0;
    _add315r = _pow312r + _pow314r; _add315i = _pow312i + _pow314i;
    double _conj316r = 0, _conj316i = 0;
    _conj316r = _add315r; _conj316i = -(_add315i);
    double _abs317r = 0, _abs317i = 0;
    _abs317r = c_abs(_conj316r, _conj316i); _abs317i = 0;
    double _sub318r = 0, _sub318i = 0;
    _sub318r = _c310r - _abs317r; _sub318i = _c310i - _abs317i;
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub318r; cIm[_idx] = _sub318i; } }
    double _c319r = 0, _c319i = 0;
    _c319r = 241.0; _c319i = 0;
    double _c320r = 0, _c320i = 0;
    _c320r = 3.0; _c320i = 0;
    double _pow321r = 0, _pow321i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow321r, &_pow321i);
    c_mul(_pow321r, _pow321i, x1r, x1i, &_pow321r, &_pow321i);
    double _add322r = 0, _add322i = 0;
    _add322r = _c319r + _pow321r; _add322i = _c319i + _pow321i;
    double _c323r = 0, _c323i = 0;
    _c323r = 3.0; _c323i = 0;
    double _pow324r = 0, _pow324i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow324r, &_pow324i);
    c_mul(_pow324r, _pow324i, x2r, x2i, &_pow324r, &_pow324i);
    double _add325r = 0, _add325i = 0;
    _add325r = _add322r + _pow324r; _add325i = _add322i + _pow324i;
    { int _idx = 51; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add325r; cIm[_idx] = _add325i; } }
    double _c326r = 0, _c326i = 0;
    _c326r = 10.0; _c326i = 0;
    double _pow327r = 0, _pow327i = 0;
    c_powr(x1r, x1i, 10.0, &_pow327r, &_pow327i);
    double _c328r = 0, _c328i = 0;
    _c328r = 10.0; _c328i = 0;
    double _pow329r = 0, _pow329i = 0;
    c_powr(x2r, x2i, 10.0, &_pow329r, &_pow329i);
    double _add330r = 0, _add330i = 0;
    _add330r = _pow327r + _pow329r; _add330i = _pow327i + _pow329i;
    double _c331r = 0, _c331i = 0;
    _c331r = 251.0; _c331i = 0;
    double _add332r = 0, _add332i = 0;
    _add332r = _add330r + _c331r; _add332i = _add330i + _c331i;
    { int _idx = 52; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add332r; cIm[_idx] = _add332i; } }
    double _mul333r = 0, _mul333i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul333r, &_mul333i);
    double _add334r = 0, _add334i = 0;
    _add334r = x1r + x2r; _add334i = x1i + x2i;
    double _re335r = 0, _re335i = 0;
    _re335r = _add334r; _re335i = 0;
    double _mul336r = 0, _mul336i = 0;
    c_mul(_mul333r, _mul333i, _re335r, _re335i, &_mul336r, &_mul336i);
    double _c337r = 0, _c337i = 0;
    _c337r = 257.0; _c337i = 0;
    double _sub338r = 0, _sub338i = 0;
    _sub338r = _mul336r - _c337r; _sub338i = _mul336i - _c337i;
    { int _idx = 53; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub338r; cIm[_idx] = _sub338i; } }
    double _sub339r = 0, _sub339i = 0;
    _sub339r = x1r - x2r; _sub339i = x1i - x2i;
    double _abs340r = 0, _abs340i = 0;
    _abs340r = c_abs(_sub339r, _sub339i); _abs340i = 0;
    double _c341r = 0, _c341i = 0;
    _c341r = 263.0; _c341i = 0;
    double _sub342r = 0, _sub342i = 0;
    _sub342r = _abs340r - _c341r; _sub342i = _abs340i - _c341i;
    { int _idx = 54; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub342r; cIm[_idx] = _sub342i; } }
    double _c343r = 0, _c343i = 0;
    _c343r = 11.0; _c343i = 0;
    double _pow344r = 0, _pow344i = 0;
    c_powr(x1r, x1i, 11.0, &_pow344r, &_pow344i);
    double _c345r = 0, _c345i = 0;
    _c345r = 11.0; _c345i = 0;
    double _pow346r = 0, _pow346i = 0;
    c_powr(x2r, x2i, 11.0, &_pow346r, &_pow346i);
    double _sub347r = 0, _sub347i = 0;
    _sub347r = _pow344r - _pow346r; _sub347i = _pow344i - _pow346i;
    double _c348r = 0, _c348i = 0;
    _c348r = 269.0; _c348i = 0;
    double _add349r = 0, _add349i = 0;
    _add349r = _sub347r + _c348r; _add349i = _sub347i + _c348i;
    { int _idx = 55; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add349r; cIm[_idx] = _add349i; } }
    double _c350r = 0, _c350i = 0;
    _c350r = 271.0; _c350i = 0;
    double _c351r = 0, _c351i = 0;
    _c351r = 2.0; _c351i = 0;
    double _pow352r = 0, _pow352i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow352r, &_pow352i);
    double _mul353r = 0, _mul353i = 0;
    c_mul(x1r, x1i, _pow352r, _pow352i, &_mul353r, &_mul353i);
    double _c354r = 0, _c354i = 0;
    _c354r = 3.0; _c354i = 0;
    double _pow355r = 0, _pow355i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow355r, &_pow355i);
    c_mul(_pow355r, _pow355i, x2r, x2i, &_pow355r, &_pow355i);
    double _sub356r = 0, _sub356i = 0;
    _sub356r = _mul353r - _pow355r; _sub356i = _mul353i - _pow355i;
    double _abs357r = 0, _abs357i = 0;
    _abs357r = c_abs(_sub356r, _sub356i); _abs357i = 0;
    double _add358r = 0, _add358i = 0;
    _add358r = _c350r + _abs357r; _add358i = _c350i + _abs357i;
    { int _idx = 56; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add358r; cIm[_idx] = _add358i; } }
    double _c359r = 0, _c359i = 0;
    _c359r = 277.0; _c359i = 0;
    double _c360r = 0, _c360i = 0;
    _c360r = 3.0; _c360i = 0;
    double _pow361r = 0, _pow361i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow361r, &_pow361i);
    c_mul(_pow361r, _pow361i, x1r, x1i, &_pow361r, &_pow361i);
    double _c362r = 0, _c362i = 0;
    _c362r = 3.0; _c362i = 0;
    double _pow363r = 0, _pow363i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow363r, &_pow363i);
    c_mul(_pow363r, _pow363i, x2r, x2i, &_pow363r, &_pow363i);
    double _sub364r = 0, _sub364i = 0;
    _sub364r = _pow361r - _pow363r; _sub364i = _pow361i - _pow363i;
    double _sin365r = 0, _sin365i = 0;
    c_sin(_sub364r, _sub364i, &_sin365r, &_sin365i);
    double _add366r = 0, _add366i = 0;
    _add366r = _c359r + _sin365r; _add366i = _c359i + _sin365i;
    { int _idx = 57; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add366r; cIm[_idx] = _add366i; } }
    double _c367r = 0, _c367i = 0;
    _c367r = 281.0; _c367i = 0;
    double _c368r = 0, _c368i = 0;
    _c368r = 2.0; _c368i = 0;
    double _pow369r = 0, _pow369i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow369r, &_pow369i);
    double _mul370r = 0, _mul370i = 0;
    c_mul(_pow369r, _pow369i, x2r, x2i, &_mul370r, &_mul370i);
    double _conj371r = 0, _conj371i = 0;
    _conj371r = _mul370r; _conj371i = -(_mul370i);
    double _sub372r = 0, _sub372i = 0;
    _sub372r = _c367r - _conj371r; _sub372i = _c367i - _conj371i;
    { int _idx = 58; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub372r; cIm[_idx] = _sub372i; } }
    double _c373r = 0, _c373i = 0;
    _c373r = 5.0; _c373i = 0;
    double _pow374r = 0, _pow374i = 0;
    c_powr(x1r, x1i, 5.0, &_pow374r, &_pow374i);
    double _c375r = 0, _c375i = 0;
    _c375r = 5.0; _c375i = 0;
    double _pow376r = 0, _pow376i = 0;
    c_powr(x2r, x2i, 5.0, &_pow376r, &_pow376i);
    double _add377r = 0, _add377i = 0;
    _add377r = _pow374r + _pow376r; _add377i = _pow374i + _pow376i;
    double _conj378r = 0, _conj378i = 0;
    _conj378r = _add377r; _conj378i = -(_add377i);
    double _c379r = 0, _c379i = 0;
    _c379r = 283.0; _c379i = 0;
    double _add380r = 0, _add380i = 0;
    _add380r = _conj378r + _c379r; _add380i = _conj378i + _c379i;
    { int _idx = 59; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add380r; cIm[_idx] = _add380i; } }
    double _c381r = 0, _c381i = 0;
    _c381r = 3.0; _c381i = 0;
    double _pow382r = 0, _pow382i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow382r, &_pow382i);
    c_mul(_pow382r, _pow382i, x1r, x1i, &_pow382r, &_pow382i);
    double _c383r = 0, _c383i = 0;
    _c383r = 3.0; _c383i = 0;
    double _pow384r = 0, _pow384i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow384r, &_pow384i);
    c_mul(_pow384r, _pow384i, x2r, x2i, &_pow384r, &_pow384i);
    double _mul385r = 0, _mul385i = 0;
    c_mul(_pow382r, _pow382i, _pow384r, _pow384i, &_mul385r, &_mul385i);
    double _ang386r = 0, _ang386i = 0;
    _ang386r = c_arg(_mul385r, _mul385i); _ang386i = 0;
    double _c387r = 0, _c387i = 0;
    _c387r = 293.0; _c387i = 0;
    double _add388r = 0, _add388i = 0;
    _add388r = _ang386r + _c387r; _add388i = _ang386i + _c387i;
    { int _idx = 60; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add388r; cIm[_idx] = _add388i; } }
    double _c389r = 0, _c389i = 0;
    _c389r = 307.0; _c389i = 0;
    double _mul390r = 0, _mul390i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul390r, &_mul390i);
    double _c391r = 0, _c391i = 0;
    _c391r = 0.0; _c391i = 1.0;
    double _add392r = 0, _add392i = 0;
    _add392r = _mul390r + _c391r; _add392i = _mul390i + _c391i;
    double _sin393r = 0, _sin393i = 0;
    c_sin(_add392r, _add392i, &_sin393r, &_sin393i);
    double _sub394r = 0, _sub394i = 0;
    _sub394r = _c389r - _sin393r; _sub394i = _c389i - _sin393i;
    { int _idx = 61; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub394r; cIm[_idx] = _sub394i; } }
    double _c395r = 0, _c395i = 0;
    _c395r = 6.0; _c395i = 0;
    double _pow396r = 0, _pow396i = 0;
    c_powr(x1r, x1i, 6.0, &_pow396r, &_pow396i);
    double _c397r = 0, _c397i = 0;
    _c397r = 6.0; _c397i = 0;
    double _pow398r = 0, _pow398i = 0;
    c_powr(x2r, x2i, 6.0, &_pow398r, &_pow398i);
    double _add399r = 0, _add399i = 0;
    _add399r = _pow396r + _pow398r; _add399i = _pow396i + _pow398i;
    double _abs400r = 0, _abs400i = 0;
    _abs400r = c_abs(_add399r, _add399i); _abs400i = 0;
    double _c401r = 0, _c401i = 0;
    _c401r = 311.0; _c401i = 0;
    double _add402r = 0, _add402i = 0;
    _add402r = _abs400r + _c401r; _add402i = _abs400i + _c401i;
    { int _idx = 62; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add402r; cIm[_idx] = _add402i; } }
    double _c403r = 0, _c403i = 0;
    _c403r = 313.0; _c403i = 0;
    double _c404r = 0, _c404i = 0;
    _c404r = 3.0; _c404i = 0;
    double _pow405r = 0, _pow405i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow405r, &_pow405i);
    c_mul(_pow405r, _pow405i, x1r, x1i, &_pow405r, &_pow405i);
    double _c406r = 0, _c406i = 0;
    _c406r = 3.0; _c406i = 0;
    double _pow407r = 0, _pow407i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow407r, &_pow407i);
    c_mul(_pow407r, _pow407i, x2r, x2i, &_pow407r, &_pow407i);
    double _sub408r = 0, _sub408i = 0;
    _sub408r = _pow405r - _pow407r; _sub408i = _pow405i - _pow407i;
    double _cos409r = 0, _cos409i = 0;
    c_cos(_sub408r, _sub408i, &_cos409r, &_cos409i);
    double _sub410r = 0, _sub410i = 0;
    _sub410r = _c403r - _cos409r; _sub410i = _c403i - _cos409i;
    { int _idx = 63; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub410r; cIm[_idx] = _sub410i; } }
    double _mul411r = 0, _mul411i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul411r, &_mul411i);
    double _ang412r = 0, _ang412i = 0;
    _ang412r = c_arg(_mul411r, _mul411i); _ang412i = 0;
    double _c413r = 0, _c413i = 0;
    _c413r = 317.0; _c413i = 0;
    double _add414r = 0, _add414i = 0;
    _add414r = _ang412r + _c413r; _add414i = _ang412i + _c413i;
    { int _idx = 64; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add414r; cIm[_idx] = _add414i; } }
    double _c415r = 0, _c415i = 0;
    _c415r = 2.0; _c415i = 0;
    double _pow416r = 0, _pow416i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow416r, &_pow416i);
    double _c417r = 0, _c417i = 0;
    _c417r = 2.0; _c417i = 0;
    double _pow418r = 0, _pow418i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow418r, &_pow418i);
    double _sub419r = 0, _sub419i = 0;
    _sub419r = _pow416r - _pow418r; _sub419i = _pow416i - _pow418i;
    double _re420r = 0, _re420i = 0;
    _re420r = _sub419r; _re420i = 0;
    double _c421r = 0, _c421i = 0;
    _c421r = 331.0; _c421i = 0;
    double _sub422r = 0, _sub422i = 0;
    _sub422r = _re420r - _c421r; _sub422i = _re420i - _c421i;
    { int _idx = 65; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub422r; cIm[_idx] = _sub422i; } }
    double _c423r = 0, _c423i = 0;
    _c423r = 337.0; _c423i = 0;
    double _c424r = 0, _c424i = 0;
    _c424r = 6.0; _c424i = 0;
    double _pow425r = 0, _pow425i = 0;
    c_powr(x1r, x1i, 6.0, &_pow425r, &_pow425i);
    double _c426r = 0, _c426i = 0;
    _c426r = 6.0; _c426i = 0;
    double _pow427r = 0, _pow427i = 0;
    c_powr(x2r, x2i, 6.0, &_pow427r, &_pow427i);
    double _mul428r = 0, _mul428i = 0;
    c_mul(_pow425r, _pow425i, _pow427r, _pow427i, &_mul428r, &_mul428i);
    double _abs429r = 0, _abs429i = 0;
    _abs429r = c_abs(_mul428r, _mul428i); _abs429i = 0;
    double _add430r = 0, _add430i = 0;
    _add430r = _c423r + _abs429r; _add430i = _c423i + _abs429i;
    { int _idx = 66; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add430r; cIm[_idx] = _add430i; } }
    double _c431r = 0, _c431i = 0;
    _c431r = 347.0; _c431i = 0;
    double _c432r = 0, _c432i = 0;
    _c432r = 4.0; _c432i = 0;
    double _pow433r = 0, _pow433i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow433r, &_pow433i);
    c_mul(_pow433r, _pow433i, _pow433r, _pow433i, &_pow433r, &_pow433i);
    double _c434r = 0, _c434i = 0;
    _c434r = 4.0; _c434i = 0;
    double _pow435r = 0, _pow435i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow435r, &_pow435i);
    c_mul(_pow435r, _pow435i, _pow435r, _pow435i, &_pow435r, &_pow435i);
    double _sub436r = 0, _sub436i = 0;
    _sub436r = _pow433r - _pow435r; _sub436i = _pow433i - _pow435i;
    double _abs437r = 0, _abs437i = 0;
    _abs437r = c_abs(_sub436r, _sub436i); _abs437i = 0;
    double _sub438r = 0, _sub438i = 0;
    _sub438r = _c431r - _abs437r; _sub438i = _c431i - _abs437i;
    { int _idx = 67; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub438r; cIm[_idx] = _sub438i; } }
    double _c439r = 0, _c439i = 0;
    _c439r = 349.0; _c439i = 0;
    double _sub440r = 0, _sub440i = 0;
    _sub440r = x1r - x2r; _sub440i = x1i - x2i;
    double _conj441r = 0, _conj441i = 0;
    _conj441r = _sub440r; _conj441i = -(_sub440i);
    double _sin442r = 0, _sin442i = 0;
    c_sin(_conj441r, _conj441i, &_sin442r, &_sin442i);
    double _add443r = 0, _add443i = 0;
    _add443r = _c439r + _sin442r; _add443i = _c439i + _sin442i;
    { int _idx = 68; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add443r; cIm[_idx] = _add443i; } }
    double _c444r = 0, _c444i = 0;
    _c444r = 353.0; _c444i = 0;
    double _c445r = 0, _c445i = 0;
    _c445r = 2.0; _c445i = 0;
    double _pow446r = 0, _pow446i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow446r, &_pow446i);
    double _add447r = 0, _add447i = 0;
    _add447r = x1r + _pow446r; _add447i = x1i + _pow446i;
    double _cos448r = 0, _cos448i = 0;
    c_cos(_add447r, _add447i, &_cos448r, &_cos448i);
    double _sub449r = 0, _sub449i = 0;
    _sub449r = _c444r - _cos448r; _sub449i = _c444i - _cos448i;
    { int _idx = 69; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub449r; cIm[_idx] = _sub449i; } }
    double _add450r = 0, _add450i = 0;
    _add450r = x1r + x2r; _add450i = x1i + x2i;
    double _c451r = 0, _c451i = 0;
    _c451r = 3.0; _c451i = 0;
    double _pow452r = 0, _pow452i = 0;
    c_mul(_add450r, _add450i, _add450r, _add450i, &_pow452r, &_pow452i);
    c_mul(_pow452r, _pow452i, _add450r, _add450i, &_pow452r, &_pow452i);
    double _c453r = 0, _c453i = 0;
    _c453r = 359.0; _c453i = 0;
    double _sub454r = 0, _sub454i = 0;
    _sub454r = _pow452r - _c453r; _sub454i = _pow452i - _c453i;
    double _abs455r = 0, _abs455i = 0;
    _abs455r = c_abs(_sub454r, _sub454i); _abs455i = 0;
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _abs455r; cIm[_idx] = _abs455i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_137: too complex for auto-transpile, stubbed */
static void poly_giga_137_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_138: auto-stubbed (unhandled constructs in source) */
static void poly_giga_138_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_giga_139_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double t_r = _add1r, t_i = _add1i;
    double _add2r = 0, _add2i = 0;
    _add2r = x1r + x2r; _add2i = x1i + x2i;
    double _abs3r = 0, _abs3i = 0;
    _abs3r = c_abs(_add2r, _add2i); _abs3i = 0;
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _div5r = 0, _div5i = 0;
    c_div(_abs3r, _abs3i, _c4r, _c4i, &_div5r, &_div5i);
    double a_r = _div5r, a_i = _div5i;
    double _c6r = 0, _c6i = 0;
    _c6r = 251.0; _c6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(a_r, a_i, _c6r, _c6i, &_mul7r, &_mul7i);
    double _int8r = 0, _int8i = 0;
    _int8r = (int)(_mul7r); _int8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 37.0; _c9i = 0;
    double _mod10r = 0, _mod10i = 0;
    _mod10r = fmod(_int8r, _c9r); _mod10i = 0;
    double m_r = _mod10r, m_i = _mod10i;
    double _c11r = 0, _c11i = 0;
    _c11r = 1.0; _c11i = 0;
    double _add12r = 0, _add12i = 0;
    _add12r = 0 + _c11r; _add12i = 0 + _c11i;
    double _c13r = 0, _c13i = 0;
    _c13r = 4.0; _c13i = 0;
    double _add14r = 0, _add14i = 0;
    _add14r = t_r + _c13r; _add14i = t_i + _c13i;
    double _div15r = 0, _div15i = 0;
    c_div(_add12r, _add12i, _add14r, _add14i, &_div15r, &_div15i);
    double v_r = _div15r, v_i = _div15i;
    double _c16r = 0, _c16i = 0;
    _c16r = 7.0; _c16i = 0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_c16r, _c16i, a_r, a_i, &_mul17r, &_mul17i);
    double _len18r = 0, _len18i = 0;
    /* WARNING: len(v) unknown */
    double _mul19r = 0, _mul19i = 0;
    c_mul(_mul17r, _mul17i, _len18r, _len18i, &_mul19r, &_mul19i);
    double _int20r = 0, _int20i = 0;
    _int20r = (int)(_mul19r); _int20i = 0;
    double _len21r = 0, _len21i = 0;
    /* WARNING: len(v) unknown */
    double _mod22r = 0, _mod22i = 0;
    _mod22r = fmod(_int20r, _len21r); _mod22i = 0;
    double p1_r = _mod22r, p1_i = _mod22i;
    double _c23r = 0, _c23i = 0;
    _c23r = 1.0; _c23i = 0;
    double _add24r = 0, _add24i = 0;
    _add24r = p1_r + _c23r; _add24i = p1_i + _c23i;
    double _abs25r = 0, _abs25i = 0;
    _abs25r = c_abs(t_r, t_i); _abs25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_abs25r, _abs25i, _c26r, _c26i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = t_r + _mul27r; _add28i = t_i + _mul27i;
    double _c29r = 0, _c29i = 0;
    _c29r = 1.0; _c29i = 0;
    double _add30r = 0, _add30i = 0;
    _add30r = _add28r + _c29r; _add30i = _add28i + _c29i;
    double _add31r = 0, _add31i = 0;
    _add31r = _add30r + m_r; _add31i = _add30i + m_i;
    double _div32r = 0, _div32i = 0;
    c_div(_add24r, _add24i, _add31r, _add31i, &_div32r, &_div32i);
    /* v[(int)(p1_r)] assignment — skipped (local array) */
    double _c33r = 0, _c33i = 0;
    _c33r = 619.0; _c33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c33r, _c33i, a_r, a_i, &_mul34r, &_mul34i);
    double _len35r = 0, _len35i = 0;
    /* WARNING: len(v) unknown */
    double _mul36r = 0, _mul36i = 0;
    c_mul(_mul34r, _mul34i, _len35r, _len35i, &_mul36r, &_mul36i);
    double _int37r = 0, _int37i = 0;
    _int37r = (int)(_mul36r); _int37i = 0;
    double _len38r = 0, _len38i = 0;
    /* WARNING: len(v) unknown */
    double _mod39r = 0, _mod39i = 0;
    _mod39r = fmod(_int37r, _len38r); _mod39i = 0;
    double p2_r = _mod39r, p2_i = _mod39i;
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = p2_r + _c40r; _add41i = p2_i + _c40i;
    double _abs42r = 0, _abs42i = 0;
    _abs42r = c_abs(t_r, t_i); _abs42i = 0;
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_abs42r, _abs42i, _c43r, _c43i, &_mul44r, &_mul44i);
    double _add45r = 0, _add45i = 0;
    _add45r = t_r + _mul44r; _add45i = t_i + _mul44i;
    double _c46r = 0, _c46i = 0;
    _c46r = 1.0; _c46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = _add45r + _c46r; _add47i = _add45i + _c46i;
    double _c48r = 0, _c48i = 0;
    _c48r = 2.0; _c48i = 0;
    double _fdiv49r = 0, _fdiv49i = 0;
    c_div(m_r, m_i, _c48r, _c48i, &_fdiv49r, &_fdiv49i);
    _fdiv49r = floor(_fdiv49r); _fdiv49i = 0;
    double _add50r = 0, _add50i = 0;
    _add50r = _add47r + _fdiv49r; _add50i = _add47i + _fdiv49i;
    double _div51r = 0, _div51i = 0;
    c_div(_add41r, _add41i, _add50r, _add50i, &_div51r, &_div51i);
    /* v[(int)(p2_r)] assignment — skipped (local array) */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_giga_140: auto-stubbed (unhandled constructs in source) */
static void poly_giga_140_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_142: auto-stubbed (unhandled constructs in source) */
static void poly_giga_142_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_143: auto-stubbed (unhandled constructs in source) */
static void poly_giga_143_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_144: auto-stubbed (unhandled constructs in source) */
static void poly_giga_144_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_giga_145: auto-stubbed (unhandled constructs in source) */
static void poly_giga_145_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

