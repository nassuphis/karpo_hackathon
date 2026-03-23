/* AUTO-GENERATED from ops_poly.py — do not edit manually */
/* 109 coefficient functions */

static void g1_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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

static void g2_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 200.0; _c1i = 0;
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
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c1r, _c1i, _add6r, _add6i, &_mul7r, &_mul7i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 150.0; _c8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 3.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
    c_mul(_pow10r, _pow10i, x1r, x1i, &_pow10r, &_pow10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 5.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_powr(x2r, x2i, 5.0, &_pow12r, &_pow12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_pow10r, _pow10i, _pow12r, _pow12i, &_mul13r, &_mul13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c8r, _c8i, _mul13r, _mul13i, &_mul14r, &_mul14i);
    { int _idx = 15; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
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
    _c23r = 50.0; _c23i = 0;
    double _c24r = 0, _c24i = 0;
    _c24r = 3.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow25r, &_pow25i);
    c_mul(_pow25r, _pow25i, x1r, x1i, &_pow25r, &_pow25i);
    double _add26r = 0, _add26i = 0;
    _add26r = _pow25r + x2r; _add26i = _pow25i + x2i;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c23r, _c23i, _add26r, _add26i, &_mul27r, &_mul27i);
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 20.0; _c28i = 0;
    double _sub29r = 0, _sub29i = 0;
    _sub29r = x1r - x2r; _sub29i = x1i - x2i;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c28r, _c28i, _sub29r, _sub29i, &_mul30r, &_mul30i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 10.0; _c31i = 0;
    double _neg32r = 0, _neg32i = 0;
    _neg32r = -(_c31r); _neg32i = -(_c31i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul33r, &_mul33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_neg32r, _neg32i, _mul33r, _mul33i, &_mul34r, &_mul34i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 5.0; _c35i = 0;
    double _neg36r = 0, _neg36i = 0;
    _neg36r = -(_c35r); _neg36i = -(_c35i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _neg36r; cIm[_idx] = _neg36i; } }
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

static void g3_c(double x1r, double x1i, double x2r, double x2i,
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
    { int _idx = ((int)(n) - 11); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _exp3r; cIm[_idx] = _exp3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 0.0; _c4i = 0;
    double _add5r = 0, _add5i = 0;
    _add5r = x1r + x2r; _add5i = x1i + x2i;
    double _exp6r = 0, _exp6i = 0;
    c_exp2(_add5r, _add5i, &_exp6r, &_exp6i);
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c4r, _c4i, _exp6r, _exp6i, &_mul7r, &_mul7i);
    { int _idx = ((int)(n) - 10); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
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
    { int _idx = ((int)(n) - 9); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul12r; cIm[_idx] = _mul12i; } }
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _exp14r = 0, _exp14i = 0;
    c_exp2(x1r, x1i, &_exp14r, &_exp14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c13r, _c13i, _exp14r, _exp14i, &_mul15r, &_mul15i);
    { int _idx = ((int)(n) - 8); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul15r; cIm[_idx] = _mul15i; } }
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _neg17r = 0, _neg17i = 0;
    _neg17r = -(x1r); _neg17i = -(x1i);
    double _exp18r = 0, _exp18i = 0;
    c_exp2(_neg17r, _neg17i, &_exp18r, &_exp18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _exp18r, _exp18i, &_mul19r, &_mul19i);
    { int _idx = ((int)(n) - 7); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
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
    { int _idx = ((int)(n) - 6); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
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
    { int _idx = ((int)(n) - 2); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 1.0; _c31i = 0;
    double _c32r = 0, _c32i = 0;
    _c32r = 0.0; _c32i = 1.0;
    double _add33r = 0, _add33i = 0;
    _add33r = _c31r + _c32r; _add33i = _c31i + _c32i;
    { int _idx = ((int)(n) - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g4_c(double x1r, double x1i, double x2r, double x2i,
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

static void g5_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 0.0;
    double _add3r = 0, _add3i = 0;
    _add3r = _c1r + _c2r; _add3i = _c1i + _c2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add3r; cIm[_idx] = _add3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 4.0; _c4i = 0;
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 0.0;
    double _add6r = 0, _add6i = 0;
    _add6r = _c4r + _c5r; _add6i = _c4i + _c5i;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add6r; cIm[_idx] = _add6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 4.0; _c7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 0.0;
    double _add9r = 0, _add9i = 0;
    _add9r = _c7r + _c8r; _add9i = _c7i + _c8i;
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 9.0; _c10i = 0;
    double _neg11r = 0, _neg11i = 0;
    _neg11r = -(_c10r); _neg11i = -(_c10i);
    double _c12r = 0, _c12i = 0;
    _c12r = 0.0; _c12i = 0.0;
    double _add13r = 0, _add13i = 0;
    _add13r = _neg11r + _c12r; _add13i = _neg11i + _c12i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add13r; cIm[_idx] = _add13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 1.9; _c14i = 0;
    double _neg15r = 0, _neg15i = 0;
    _neg15r = -(_c14r); _neg15i = -(_c14i);
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 0.0;
    double _add17r = 0, _add17i = 0;
    _add17r = _neg15r + _c16r; _add17i = _neg15i + _c16i;
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
    double _c18r = 0, _c18i = 0;
    _c18r = 0.2; _c18i = 0;
    double _c19r = 0, _c19i = 0;
    _c19r = 0.0; _c19i = 0.0;
    double _add20r = 0, _add20i = 0;
    _add20r = _c18r + _c19r; _add20i = _c18i + _c19i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    double _c21r = 0, _c21i = 0;
    _c21r = 0.0; _c21i = 100.0;
    double _c22r = 0, _c22i = 0;
    _c22r = 3.0; _c22i = 0;
    double _pow23r = 0, _pow23i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow23r, &_pow23i);
    c_mul(_pow23r, _pow23i, x2r, x2i, &_pow23r, &_pow23i);
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c21r, _c21i, _pow23r, _pow23i, &_mul24r, &_mul24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 0.0; _c25i = 100.0;
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow27r, &_pow27i);
    double _mul28r = 0, _mul28i = 0;
    c_mul(_c25r, _c25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
    double _add29r = 0, _add29i = 0;
    _add29r = _mul24r + _mul28r; _add29i = _mul24i + _mul28i;
    double _c30r = 0, _c30i = 0;
    _c30r = 0.0; _c30i = 100.0;
    double _mul31r = 0, _mul31i = 0;
    c_mul(_c30r, _c30i, x2r, x2i, &_mul31r, &_mul31i);
    double _sub32r = 0, _sub32i = 0;
    _sub32r = _add29r - _mul31r; _sub32i = _add29i - _mul31i;
    double _c33r = 0, _c33i = 0;
    _c33r = 0.0; _c33i = 100.0;
    double _sub34r = 0, _sub34i = 0;
    _sub34r = _sub32r - _c33r; _sub34i = _sub32i - _c33i;
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub34r; cIm[_idx] = _sub34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 0.0; _c35i = 100.0;
    double _c36r = 0, _c36i = 0;
    _c36r = 3.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow37r, &_pow37i);
    c_mul(_pow37r, _pow37i, x1r, x1i, &_pow37r, &_pow37i);
    double _mul38r = 0, _mul38i = 0;
    c_mul(_c35r, _c35i, _pow37r, _pow37i, &_mul38r, &_mul38i);
    double _c39r = 0, _c39i = 0;
    _c39r = 0.0; _c39i = 100.0;
    double _c40r = 0, _c40i = 0;
    _c40r = 2.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow41r, &_pow41i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c39r, _c39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _mul38r + _mul42r; _add43i = _mul38i + _mul42i;
    double _c44r = 0, _c44i = 0;
    _c44r = 0.0; _c44i = 100.0;
    double _mul45r = 0, _mul45i = 0;
    c_mul(_c44r, _c44i, x2r, x2i, &_mul45r, &_mul45i);
    double _add46r = 0, _add46i = 0;
    _add46r = _add43r + _mul45r; _add46i = _add43i + _mul45i;
    double _c47r = 0, _c47i = 0;
    _c47r = 0.0; _c47i = 100.0;
    double _sub48r = 0, _sub48i = 0;
    _sub48r = _add46r - _c47r; _sub48i = _add46i - _c47i;
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub48r; cIm[_idx] = _sub48i; } }
    double _c49r = 0, _c49i = 0;
    _c49r = 0.0; _c49i = 100.0;
    double _c50r = 0, _c50i = 0;
    _c50r = 3.0; _c50i = 0;
    double _pow51r = 0, _pow51i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow51r, &_pow51i);
    c_mul(_pow51r, _pow51i, x2r, x2i, &_pow51r, &_pow51i);
    double _mul52r = 0, _mul52i = 0;
    c_mul(_c49r, _c49i, _pow51r, _pow51i, &_mul52r, &_mul52i);
    double _c53r = 0, _c53i = 0;
    _c53r = 0.0; _c53i = 100.0;
    double _c54r = 0, _c54i = 0;
    _c54r = 2.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow55r, &_pow55i);
    double _mul56r = 0, _mul56i = 0;
    c_mul(_c53r, _c53i, _pow55r, _pow55i, &_mul56r, &_mul56i);
    double _sub57r = 0, _sub57i = 0;
    _sub57r = _mul52r - _mul56r; _sub57i = _mul52i - _mul56i;
    double _c58r = 0, _c58i = 0;
    _c58r = 0.0; _c58i = 100.0;
    double _mul59r = 0, _mul59i = 0;
    c_mul(_c58r, _c58i, x2r, x2i, &_mul59r, &_mul59i);
    double _add60r = 0, _add60i = 0;
    _add60r = _sub57r + _mul59r; _add60i = _sub57i + _mul59i;
    double _c61r = 0, _c61i = 0;
    _c61r = 0.0; _c61i = 100.0;
    double _sub62r = 0, _sub62i = 0;
    _sub62r = _add60r - _c61r; _sub62i = _add60i - _c61i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub62r; cIm[_idx] = _sub62i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g6_c(double x1r, double x1i, double x2r, double x2i,
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

/* g7: too complex for auto-transpile, stubbed */
static void g7_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g8: too complex for auto-transpile, stubbed */
static void g8_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g9_c(double x1r, double x1i, double x2r, double x2i,
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

static void g10_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 120;
    for (int _i = 0; _i < 120; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 120.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double re1 = _attr1r;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double im1 = _attr2r;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double re2 = _attr3r;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double im2 = _attr4r;
    for (int k = 0; k < (int)(n); k++) {
        double _c5r = 0, _c5i = 0;
        _c5r = 100.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = re1 + im2; _add6i = 0 + 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c5r, _c5i, _add6r, _add6i, &_mul7r, &_mul7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = k + _c8r; _add9i = 0 + _c8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 10.0; _c10i = 0;
        double _div11r = 0, _div11i = 0;
        c_div(_add9r, _add9i, _c10r, _c10i, &_div11r, &_div11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_div11r, _div11i, _div11r, _div11i, &_pow13r, &_pow13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_mul7r, _mul7i, _pow13r, _pow13i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 1.0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = k + _c16r; _add17i = 0 + _c16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(re2, 0, _add17r, _add17i, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 20.0; _c19i = 0;
        double _div20r = 0, _div20i = 0;
        c_div(_mul18r, _mul18i, _c19r, _c19i, &_div20r, &_div20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c15r, _c15i, _div20r, _div20i, &_mul21r, &_mul21i);
        double _exp22r = 0, _exp22i = 0;
        c_exp2(_mul21r, _mul21i, &_exp22r, &_exp22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul14r, _mul14i, _exp22r, _exp22i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 50.0; _c24i = 0;
        double _sub25r = 0, _sub25i = 0;
        _sub25r = im1 - re2; _sub25i = 0 - 0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c24r, _c24i, _sub25r, _sub25i, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = k + _c27r; _add28i = 0 + _c27i;
        double _c29r = 0, _c29i = 0;
        _c29r = 0.1; _c29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_add28r, _add28i, _c29r, _c29i, &_mul30r, &_mul30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_mul30r, _mul30i, im2, 0, &_mul31r, &_mul31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(_mul31r, _mul31i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_mul26r, _mul26i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _neg35r = 0, _neg35i = 0;
        _neg35r = -(_c34r); _neg35i = -(_c34i);
        double _c36r = 0, _c36i = 0;
        _c36r = 1.0; _c36i = 0;
        double _add37r = 0, _add37i = 0;
        _add37r = k + _c36r; _add37i = 0 + _c36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_neg35r, _neg35i, _add37r, _add37i, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 0.05; _c39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(_mul38r, _mul38i, _c39r, _c39i, &_mul40r, &_mul40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_mul40r, _mul40i, re1, 0, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_mul33r, _mul33i, _exp42r, _exp42i, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul23r + _mul43r; _add44i = _mul23i + _mul43i;
        { int _idx = k; if (_idx >= 0 && _idx < 120) { cRe[_idx] = _add44r; cIm[_idx] = _add44i; } }
    }
    double _cf45r = 0, _cf45i = 0;
    { int _idx = 29; if (_idx >= 0 && _idx < 120) { _cf45r = cRe[_idx]; _cf45i = cIm[_idx]; } }
    double _c46r = 0, _c46i = 0;
    _c46r = 0.0; _c46i = 1000.0;
    double _add47r = 0, _add47i = 0;
    _add47r = _cf45r + _c46r; _add47i = _cf45i + _c46i;
    { int _idx = 29; if (_idx >= 0 && _idx < 120) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    double _cf48r = 0, _cf48i = 0;
    { int _idx = 59; if (_idx >= 0 && _idx < 120) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
    double _c49r = 0, _c49i = 0;
    _c49r = 500.0; _c49i = 0;
    double _sub50r = 0, _sub50i = 0;
    _sub50r = _cf48r - _c49r; _sub50i = _cf48i - _c49i;
    { int _idx = 59; if (_idx >= 0 && _idx < 120) { cRe[_idx] = _sub50r; cIm[_idx] = _sub50i; } }
    double _cf51r = 0, _cf51i = 0;
    { int _idx = 89; if (_idx >= 0 && _idx < 120) { _cf51r = cRe[_idx]; _cf51i = cIm[_idx]; } }
    double _c52r = 0, _c52i = 0;
    _c52r = 250.0; _c52i = 0;
    double _c53r = 0, _c53i = 0;
    _c53r = 0.0; _c53i = 1.0;
    double _mul54r = 0, _mul54i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul54r, &_mul54i);
    double _mul55r = 0, _mul55i = 0;
    c_mul(_c53r, _c53i, _mul54r, _mul54i, &_mul55r, &_mul55i);
    double _exp56r = 0, _exp56i = 0;
    c_exp2(_mul55r, _mul55i, &_exp56r, &_exp56i);
    double _mul57r = 0, _mul57i = 0;
    c_mul(_c52r, _c52i, _exp56r, _exp56i, &_mul57r, &_mul57i);
    double _add58r = 0, _add58i = 0;
    _add58r = _cf51r + _mul57r; _add58i = _cf51i + _mul57i;
    { int _idx = 89; if (_idx >= 0 && _idx < 120) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    for (int _i = 0; _i < 120; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* g11: too complex for auto-transpile, stubbed */
static void g11_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g12_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 50.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 3.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow4r, &_pow4i);
    c_mul(_pow4r, _pow4i, x1r, x1i, &_pow4r, &_pow4i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _pow4r, _pow4i, &_mul5r, &_mul5i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 50.0; _c6i = 0;
    double _c7r = 0, _c7i = 0;
    _c7r = 3.0; _c7i = 0;
    double _pow8r = 0, _pow8i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow8r, &_pow8i);
    c_mul(_pow8r, _pow8i, x2r, x2i, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_c6r, _c6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 30.0; _c10i = 0;
    double _neg11r = 0, _neg11i = 0;
    _neg11r = -(_c10r); _neg11i = -(_c10i);
    double _c12r = 0, _c12i = 0;
    _c12r = 2.0; _c12i = 0;
    double _pow13r = 0, _pow13i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow13r, &_pow13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_neg11r, _neg11i, _pow13r, _pow13i, &_mul14r, &_mul14i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 30.0; _c15i = 0;
    double _neg16r = 0, _neg16i = 0;
    _neg16r = -(_c15r); _neg16i = -(_c15i);
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _pow18r = 0, _pow18i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow18r, &_pow18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_neg16r, _neg16i, _pow18r, _pow18i, &_mul19r, &_mul19i);
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 100.0; _c20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul21r, &_mul21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c20r, _c20i, _mul21r, _mul21i, &_mul22r, &_mul22i);
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 50.0; _c23i = 0;
    double _c24r = 0, _c24i = 0;
    _c24r = 2.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow25r, &_pow25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_pow25r, _pow25i, x2r, x2i, &_mul26r, &_mul26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c23r, _c23i, _mul26r, _mul26i, &_mul27r, &_mul27i);
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 50.0; _c28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow30r, &_pow30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(x1r, x1i, _pow30r, _pow30i, &_mul31r, &_mul31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c28r, _c28i, _mul31r, _mul31i, &_mul32r, &_mul32i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 75.0; _c33i = 0;
    double _neg34r = 0, _neg34i = 0;
    _neg34r = -(_c33r); _neg34i = -(_c33i);
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
    double _mul39r = 0, _mul39i = 0;
    c_mul(_pow36r, _pow36i, _pow38r, _pow38i, &_mul39r, &_mul39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_neg34r, _neg34i, _mul39r, _mul39i, &_mul40r, &_mul40i);
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    double _c41r = 0, _c41i = 0;
    _c41r = 3.5; _c41i = 0;
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c41r, _c41i, x2r, x2i, &_mul42r, &_mul42i);
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _neg44r = 0, _neg44i = 0;
    _neg44r = -(_c43r); _neg44i = -(_c43i);
    double _mul45r = 0, _mul45i = 0;
    c_mul(_neg44r, _neg44i, x1r, x1i, &_mul45r, &_mul45i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g13_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 100.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 4.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, _pow3r, _pow3i, &_pow3r, &_pow3i);
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c1r, _c1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 100.0; _c5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 4.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow7r, &_pow7i);
    c_mul(_pow7r, _pow7i, _pow7r, _pow7i, &_pow7r, &_pow7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c5r, _c5i, _pow7r, _pow7i, &_mul8r, &_mul8i);
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul8r; cIm[_idx] = _mul8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 80.0; _c9i = 0;
    double _c10r = 0, _c10i = 0;
    _c10r = 3.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow11r, &_pow11i);
    c_mul(_pow11r, _pow11i, x1r, x1i, &_pow11r, &_pow11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_pow11r, _pow11i, x2r, x2i, &_mul12r, &_mul12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c9r, _c9i, _mul12r, _mul12i, &_mul13r, &_mul13i);
    { int _idx = 5; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 80.0; _c14i = 0;
    double _c15r = 0, _c15i = 0;
    _c15r = 3.0; _c15i = 0;
    double _pow16r = 0, _pow16i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow16r, &_pow16i);
    c_mul(_pow16r, _pow16i, x2r, x2i, &_pow16r, &_pow16i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(x1r, x1i, _pow16r, _pow16i, &_mul17r, &_mul17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c14r, _c14i, _mul17r, _mul17i, &_mul18r, &_mul18i);
    { int _idx = 7; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul18r; cIm[_idx] = _mul18i; } }
    double _c19r = 0, _c19i = 0;
    _c19r = 1.0; _c19i = 0;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c19r, _c19i, x1r, x1i, &_mul20r, &_mul20i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul20r; cIm[_idx] = _mul20i; } }
    double _c21r = 0, _c21i = 0;
    _c21r = 1.0; _c21i = 0;
    double _neg22r = 0, _neg22i = 0;
    _neg22r = -(_c21r); _neg22i = -(_c21i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_neg22r, _neg22i, x2r, x2i, &_mul23r, &_mul23i);
    { int _idx = 11; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    double _c24r = 0, _c24i = 0;
    _c24r = 5.0; _c24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow26r, &_pow26i);
    double _c27r = 0, _c27i = 0;
    _c27r = 2.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow28r, &_pow28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_pow26r, _pow26i, _pow28r, _pow28i, &_mul29r, &_mul29i);
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c24r, _c24i, _mul29r, _mul29i, &_mul30r, &_mul30i);
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 0.5; _c31i = 0;
    double _neg32r = 0, _neg32i = 0;
    _neg32r = -(_c31r); _neg32i = -(_c31i);
    double _c33r = 0, _c33i = 0;
    _c33r = 5.0; _c33i = 0;
    double _pow34r = 0, _pow34i = 0;
    c_powr(x1r, x1i, 5.0, &_pow34r, &_pow34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_neg32r, _neg32i, _pow34r, _pow34i, &_mul35r, &_mul35i);
    { int _idx = 17; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    double _c36r = 0, _c36i = 0;
    _c36r = 0.5; _c36i = 0;
    double _neg37r = 0, _neg37i = 0;
    _neg37r = -(_c36r); _neg37i = -(_c36i);
    double _c38r = 0, _c38i = 0;
    _c38r = 5.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_powr(x2r, x2i, 5.0, &_pow39r, &_pow39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_neg37r, _neg37i, _pow39r, _pow39i, &_mul40r, &_mul40i);
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    double _c41r = 0, _c41i = 0;
    _c41r = 2.3; _c41i = 0;
    double _c42r = 0, _c42i = 0;
    _c42r = 2.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow43r, &_pow43i);
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow45r, &_pow45i);
    double _sub46r = 0, _sub46i = 0;
    _sub46r = _pow43r - _pow45r; _sub46i = _pow43i - _pow45i;
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c41r, _c41i, _sub46r, _sub46i, &_mul47r, &_mul47i);
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    double _c48r = 0, _c48i = 0;
    _c48r = 10.0; _c48i = 0;
    double _c49r = 0, _c49i = 0;
    _c49r = 3.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow50r, &_pow50i);
    c_mul(_pow50r, _pow50i, x1r, x1i, &_pow50r, &_pow50i);
    double _c51r = 0, _c51i = 0;
    _c51r = 3.0; _c51i = 0;
    double _pow52r = 0, _pow52i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow52r, &_pow52i);
    c_mul(_pow52r, _pow52i, x2r, x2i, &_pow52r, &_pow52i);
    double _sub53r = 0, _sub53i = 0;
    _sub53r = _pow50r - _pow52r; _sub53i = _pow50i - _pow52i;
    double _mul54r = 0, _mul54i = 0;
    c_mul(_c48r, _c48i, _sub53r, _sub53i, &_mul54r, &_mul54i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul54r; cIm[_idx] = _mul54i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g14_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 200.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    c_mul(_pow3r, _pow3i, x1r, x1i, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_pow3r, _pow3i, _pow5r, _pow5i, &_mul6r, &_mul6i);
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c1r, _c1i, _mul6r, _mul6i, &_mul7r, &_mul7i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 200.0; _c8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 2.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 3.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, x2r, x2i, &_pow12r, &_pow12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_pow10r, _pow10i, _pow12r, _pow12i, &_mul13r, &_mul13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c8r, _c8i, _mul13r, _mul13i, &_mul14r, &_mul14i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 50.0; _c15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 4.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow17r, &_pow17i);
    c_mul(_pow17r, _pow17i, _pow17r, _pow17i, &_pow17r, &_pow17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c15r, _c15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul18r; cIm[_idx] = _mul18i; } }
    double _c19r = 0, _c19i = 0;
    _c19r = 50.0; _c19i = 0;
    double _c20r = 0, _c20i = 0;
    _c20r = 4.0; _c20i = 0;
    double _pow21r = 0, _pow21i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow21r, &_pow21i);
    c_mul(_pow21r, _pow21i, _pow21r, _pow21i, &_pow21r, &_pow21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c19r, _c19i, _pow21r, _pow21i, &_mul22r, &_mul22i);
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 100.0; _c23i = 0;
    double _neg24r = 0, _neg24i = 0;
    _neg24r = -(_c23r); _neg24i = -(_c23i);
    double _c25r = 0, _c25i = 0;
    _c25r = 3.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow26r, &_pow26i);
    c_mul(_pow26r, _pow26i, x1r, x1i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_neg24r, _neg24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 100.0; _c28i = 0;
    double _neg29r = 0, _neg29i = 0;
    _neg29r = -(_c28r); _neg29i = -(_c28i);
    double _c30r = 0, _c30i = 0;
    _c30r = 3.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow31r, &_pow31i);
    c_mul(_pow31r, _pow31i, x2r, x2i, &_pow31r, &_pow31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_neg29r, _neg29i, _pow31r, _pow31i, &_mul32r, &_mul32i);
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 10.0; _c33i = 0;
    double _c34r = 0, _c34i = 0;
    _c34r = 2.0; _c34i = 0;
    double _pow35r = 0, _pow35i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow35r, &_pow35i);
    double _c36r = 0, _c36i = 0;
    _c36r = 2.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow37r, &_pow37i);
    double _sub38r = 0, _sub38i = 0;
    _sub38r = _pow35r - _pow37r; _sub38i = _pow35i - _pow37i;
    double _mul39r = 0, _mul39i = 0;
    c_mul(_c33r, _c33i, _sub38r, _sub38i, &_mul39r, &_mul39i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    double _c40r = 0, _c40i = 0;
    _c40r = 20.0; _c40i = 0;
    double _sub41r = 0, _sub41i = 0;
    _sub41r = x1r - x2r; _sub41i = x1i - x2i;
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c40r, _c40i, _sub41r, _sub41i, &_mul42r, &_mul42i);
    { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    double _c43r = 0, _c43i = 0;
    _c43r = 0.1; _c43i = 0;
    double _c44r = 0, _c44i = 0;
    _c44r = 5.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_powr(x1r, x1i, 5.0, &_pow45r, &_pow45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(_c43r, _c43i, _pow45r, _pow45i, &_mul46r, &_mul46i);
    { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    double _c47r = 0, _c47i = 0;
    _c47r = 0.1; _c47i = 0;
    double _c48r = 0, _c48i = 0;
    _c48r = 5.0; _c48i = 0;
    double _pow49r = 0, _pow49i = 0;
    c_powr(x2r, x2i, 5.0, &_pow49r, &_pow49i);
    double _mul50r = 0, _mul50i = 0;
    c_mul(_c47r, _c47i, _pow49r, _pow49i, &_mul50r, &_mul50i);
    { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    double _c51r = 0, _c51i = 0;
    _c51r = 0.05; _c51i = 0;
    double _mul52r = 0, _mul52i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul52r, &_mul52i);
    double _mul53r = 0, _mul53i = 0;
    c_mul(_c51r, _c51i, _mul52r, _mul52i, &_mul53r, &_mul53i);
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    double _c54r = 0, _c54i = 0;
    _c54r = 10.0; _c54i = 0;
    double _neg55r = 0, _neg55i = 0;
    _neg55r = -(_c54r); _neg55i = -(_c54i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _neg55r; cIm[_idx] = _neg55i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g15_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 18;
    for (int _i = 0; _i < 18; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 200.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 5.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_powr(x1r, x1i, 5.0, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 5.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_powr(x2r, x2i, 5.0, &_pow5r, &_pow5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _pow3r + _pow5r; _add6i = _pow3i + _pow5i;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c1r, _c1i, _add6r, _add6i, &_mul7r, &_mul7i);
    { int _idx = 1; if (_idx >= 0 && _idx < 18) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 100.0; _c8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 4.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
    c_mul(_pow10r, _pow10i, _pow10r, _pow10i, &_pow10r, &_pow10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 4.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, _pow12r, _pow12i, &_pow12r, &_pow12i);
    double _sub13r = 0, _sub13i = 0;
    _sub13r = _pow10r - _pow12r; _sub13i = _pow10i - _pow12i;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c8r, _c8i, _sub13r, _sub13i, &_mul14r, &_mul14i);
    { int _idx = 3; if (_idx >= 0 && _idx < 18) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 80.0; _c15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 6.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_powr(x1r, x1i, 6.0, &_pow17r, &_pow17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c15r, _c15i, _pow17r, _pow17i, &_mul18r, &_mul18i);
    { int _idx = 5; if (_idx >= 0 && _idx < 18) { cRe[_idx] = _mul18r; cIm[_idx] = _mul18i; } }
    double _c19r = 0, _c19i = 0;
    _c19r = 80.0; _c19i = 0;
    double _c20r = 0, _c20i = 0;
    _c20r = 6.0; _c20i = 0;
    double _pow21r = 0, _pow21i = 0;
    c_powr(x2r, x2i, 6.0, &_pow21r, &_pow21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c19r, _c19i, _pow21r, _pow21i, &_mul22r, &_mul22i);
    { int _idx = 7; if (_idx >= 0 && _idx < 18) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c23r, _c23i, x1r, x1i, &_mul24r, &_mul24i);
    { int _idx = 9; if (_idx >= 0 && _idx < 18) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    double _c25r = 0, _c25i = 0;
    _c25r = 2.0; _c25i = 0;
    double _neg26r = 0, _neg26i = 0;
    _neg26r = -(_c25r); _neg26i = -(_c25i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_neg26r, _neg26i, x2r, x2i, &_mul27r, &_mul27i);
    { int _idx = 11; if (_idx >= 0 && _idx < 18) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 5.0; _c28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 3.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, x1r, x1i, &_pow30r, &_pow30i);
    double _c31r = 0, _c31i = 0;
    _c31r = 3.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow32r, &_pow32i);
    c_mul(_pow32r, _pow32i, x2r, x2i, &_pow32r, &_pow32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_pow30r, _pow30i, _pow32r, _pow32i, &_mul33r, &_mul33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c28r, _c28i, _mul33r, _mul33i, &_mul34r, &_mul34i);
    { int _idx = 13; if (_idx >= 0 && _idx < 18) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    double _c35r = 0, _c35i = 0;
    _c35r = 5.0; _c35i = 0;
    { int _idx = 17; if (_idx >= 0 && _idx < 18) { cRe[_idx] = _c35r; cIm[_idx] = _c35i; } }
    for (int _i = 0; _i < 18; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g16_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 22;
    for (int _i = 0; _i < 22; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 250.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 5.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_powr(x1r, x1i, 5.0, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 3.0; _c4i = 0;
    double _pow5r = 0, _pow5i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow5r, &_pow5i);
    c_mul(_pow5r, _pow5i, x2r, x2i, &_pow5r, &_pow5i);
    double _sub6r = 0, _sub6i = 0;
    _sub6r = _pow3r - _pow5r; _sub6i = _pow3i - _pow5i;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c1r, _c1i, _sub6r, _sub6i, &_mul7r, &_mul7i);
    { int _idx = 12; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul7r; cIm[_idx] = _mul7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 200.0; _c8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 4.0; _c9i = 0;
    double _pow10r = 0, _pow10i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow10r, &_pow10i);
    c_mul(_pow10r, _pow10i, _pow10r, _pow10i, &_pow10r, &_pow10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 4.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow12r, &_pow12i);
    c_mul(_pow12r, _pow12i, _pow12r, _pow12i, &_pow12r, &_pow12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_pow10r, _pow10i, _pow12r, _pow12i, &_mul13r, &_mul13i);
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c8r, _c8i, _mul13r, _mul13i, &_mul14r, &_mul14i);
    { int _idx = 17; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 80.0; _c15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 2.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow17r, &_pow17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_pow17r, _pow17i, x2r, x2i, &_mul18r, &_mul18i);
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow20r, &_pow20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = _mul18r - _pow20r; _sub21i = _mul18i - _pow20i;
    double _mul22r = 0, _mul22i = 0;
    c_mul(_c15r, _c15i, _sub21r, _sub21i, &_mul22r, &_mul22i);
    { int _idx = 8; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 60.0; _c23i = 0;
    double _neg24r = 0, _neg24i = 0;
    _neg24r = -(_c23r); _neg24i = -(_c23i);
    double _c25r = 0, _c25i = 0;
    _c25r = 3.0; _c25i = 0;
    double _pow26r = 0, _pow26i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow26r, &_pow26i);
    c_mul(_pow26r, _pow26i, x1r, x1i, &_pow26r, &_pow26i);
    double _mul27r = 0, _mul27i = 0;
    c_mul(_neg24r, _neg24i, _pow26r, _pow26i, &_mul27r, &_mul27i);
    { int _idx = 6; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _c28r = 0, _c28i = 0;
    _c28r = 40.0; _c28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 3.0; _c29i = 0;
    double _pow30r = 0, _pow30i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow30r, &_pow30i);
    c_mul(_pow30r, _pow30i, x2r, x2i, &_pow30r, &_pow30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(_c28r, _c28i, _pow30r, _pow30i, &_mul31r, &_mul31i);
    { int _idx = 4; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    double _c32r = 0, _c32i = 0;
    _c32r = 15.0; _c32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 0.5; _c33i = 0;
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c33r, _c33i, x2r, x2i, &_mul34r, &_mul34i);
    double _sub35r = 0, _sub35i = 0;
    _sub35r = x1r - _mul34r; _sub35i = x1i - _mul34i;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c32r, _c32i, _sub35r, _sub35i, &_mul36r, &_mul36i);
    { int _idx = 2; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    double _c37r = 0, _c37i = 0;
    _c37r = 20.0; _c37i = 0;
    double _neg38r = 0, _neg38i = 0;
    _neg38r = -(_c37r); _neg38i = -(_c37i);
    double _mul39r = 0, _mul39i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul39r, &_mul39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_neg38r, _neg38i, _mul39r, _mul39i, &_mul40r, &_mul40i);
    { int _idx = 3; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul40r; cIm[_idx] = _mul40i; } }
    double _c41r = 0, _c41i = 0;
    _c41r = 5.0; _c41i = 0;
    double _mul42r = 0, _mul42i = 0;
    c_mul(_c41r, _c41i, x2r, x2i, &_mul42r, &_mul42i);
    { int _idx = 1; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    double _c43r = 0, _c43i = 0;
    _c43r = 10.0; _c43i = 0;
    double _neg44r = 0, _neg44i = 0;
    _neg44r = -(_c43r); _neg44i = -(_c43i);
    { int _idx = 0; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _neg44r; cIm[_idx] = _neg44i; } }
    double _c45r = 0, _c45i = 0;
    _c45r = 30.0; _c45i = 0;
    double _neg46r = 0, _neg46i = 0;
    _neg46r = -(_c45r); _neg46i = -(_c45i);
    double _c47r = 0, _c47i = 0;
    _c47r = 6.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_powr(x1r, x1i, 6.0, &_pow48r, &_pow48i);
    double _add49r = 0, _add49i = 0;
    _add49r = _pow48r + x2r; _add49i = _pow48i + x2i;
    double _mul50r = 0, _mul50i = 0;
    c_mul(_neg46r, _neg46i, _add49r, _add49i, &_mul50r, &_mul50i);
    { int _idx = 21; if (_idx >= 0 && _idx < 22) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
    for (int _i = 0; _i < 22; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g17_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _ang1r = 0, _ang1i = 0;
    _ang1r = c_arg(x1r, x1i); _ang1i = 0;
    double theta1_r = _ang1r, theta1_i = _ang1i;
    double _ang2r = 0, _ang2i = 0;
    _ang2r = c_arg(x2r, x2i); _ang2i = 0;
    double theta2_r = _ang2r, theta2_i = _ang2i;
    double _c3r = 0, _c3i = 0;
    _c3r = 1.0; _c3i = 0;
    double _c4r = 0, _c4i = 0;
    _c4r = 0.5; _c4i = 0;
    double _c5r = 0, _c5i = 0;
    _c5r = 5.0; _c5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c5r, _c5i, theta1_r, theta1_i, &_mul6r, &_mul6i);
    double _c7r = 0, _c7i = 0;
    _c7r = 3.0; _c7i = 0;
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c7r, _c7i, theta2_r, theta2_i, &_mul8r, &_mul8i);
    double _sub9r = 0, _sub9i = 0;
    _sub9r = _mul6r - _mul8r; _sub9i = _mul6i - _mul8i;
    double _sin10r = 0, _sin10i = 0;
    c_sin(_sub9r, _sub9i, &_sin10r, &_sin10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c4r, _c4i, _sin10r, _sin10i, &_mul11r, &_mul11i);
    double _add12r = 0, _add12i = 0;
    _add12r = _c3r + _mul11r; _add12i = _c3i + _mul11i;
    double scale1_r = _add12r, scale1_i = _add12i;
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 0.3; _c14i = 0;
    double _c15r = 0, _c15i = 0;
    _c15r = 7.0; _c15i = 0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c15r, _c15i, theta1_r, theta1_i, &_mul16r, &_mul16i);
    double _c17r = 0, _c17i = 0;
    _c17r = 2.0; _c17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c17r, _c17i, theta2_r, theta2_i, &_mul18r, &_mul18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _mul16r + _mul18r; _add19i = _mul16i + _mul18i;
    double _cos20r = 0, _cos20i = 0;
    c_cos(_add19r, _add19i, &_cos20r, &_cos20i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c14r, _c14i, _cos20r, _cos20i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _c13r + _mul21r; _add22i = _c13i + _mul21i;
    double scale2_r = _add22r, scale2_i = _add22i;
    double _sub23r = 0, _sub23i = 0;
    _sub23r = theta1_r - theta2_r; _sub23i = theta1_i - theta2_i;
    double _sin24r = 0, _sin24i = 0;
    c_sin(_sub23r, _sub23i, &_sin24r, &_sin24i);
    double _c25r = 0, _c25i = 0;
    _c25r = 0.0; _c25i = 0;
    double _c26r = 0, _c26i = 0;
    _c26r = 20.0; _c26i = 0;
    double _c27r = 0, _c27i = 0;
    _c27r = 15.0; _c27i = 0;
    double _tern28r = 0, _tern28i = 0;
    if (_sin24r > _c25r) { _tern28r = _c26r; _tern28i = _c26i; }
    else { _tern28r = _c27r; _tern28i = _c27i; }
    double high_deg_index_r = _tern28r, high_deg_index_i = _tern28i;
    double _c29r = 0, _c29i = 0;
    _c29r = 300.0; _c29i = 0;
    double _c30r = 0, _c30i = 0;
    _c30r = 7.0; _c30i = 0;
    double _pow31r = 0, _pow31i = 0;
    c_powr(x1r, x1i, 7.0, &_pow31r, &_pow31i);
    double _c32r = 0, _c32i = 0;
    _c32r = 9.0; _c32i = 0;
    double _pow33r = 0, _pow33i = 0;
    c_powr(x2r, x2i, 9.0, &_pow33r, &_pow33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_pow31r, _pow31i, _pow33r, _pow33i, &_mul34r, &_mul34i);
    double _mul35r = 0, _mul35i = 0;
    c_mul(_c29r, _c29i, _mul34r, _mul34i, &_mul35r, &_mul35i);
    double _mul36r = 0, _mul36i = 0;
    c_mul(_mul35r, _mul35i, scale1_r, scale1_i, &_mul36r, &_mul36i);
    { int _idx = (int)(high_deg_index_r); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    double _add37r = 0, _add37i = 0;
    _add37r = theta1_r + theta2_r; _add37i = theta1_i + theta2_i;
    double _cos38r = 0, _cos38i = 0;
    c_cos(_add37r, _add37i, &_cos38r, &_cos38i);
    double _c39r = 0, _c39i = 0;
    _c39r = 0.0; _c39i = 0;
    if (_cos38r < _c39r) {
        double _c40r = 0, _c40i = 0;
        _c40r = 250.0; _c40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 4.0; _c41i = 0;
        double _pow42r = 0, _pow42i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow42r, &_pow42i);
        c_mul(_pow42r, _pow42i, _pow42r, _pow42i, &_pow42r, &_pow42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 7.0; _c43i = 0;
        double _pow44r = 0, _pow44i = 0;
        c_powr(x2r, x2i, 7.0, &_pow44r, &_pow44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_pow42r, _pow42i, _pow44r, _pow44i, &_mul45r, &_mul45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c40r, _c40i, _mul45r, _mul45i, &_mul46r, &_mul46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_mul46r, _mul46i, scale2_r, scale2_i, &_mul47r, &_mul47i);
        { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    } else {
        double _c48r = 0, _c48i = 0;
        _c48r = 250.0; _c48i = 0;
        double _c49r = 0, _c49i = 0;
        _c49r = 4.0; _c49i = 0;
        double _pow50r = 0, _pow50i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow50r, &_pow50i);
        c_mul(_pow50r, _pow50i, _pow50r, _pow50i, &_pow50r, &_pow50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 7.0; _c51i = 0;
        double _pow52r = 0, _pow52i = 0;
        c_powr(x2r, x2i, 7.0, &_pow52r, &_pow52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_pow50r, _pow50i, _pow52r, _pow52i, &_mul53r, &_mul53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c48r, _c48i, _mul53r, _mul53i, &_mul54r, &_mul54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_mul54r, _mul54i, scale2_r, scale2_i, &_mul55r, &_mul55i);
        { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    }
    double _c56r = 0, _c56i = 0;
    _c56r = 1.0; _c56i = 0;
    double _c57r = 0, _c57i = 0;
    _c57r = 0.4; _c57i = 0;
    double _c58r = 0, _c58i = 0;
    _c58r = 2.0; _c58i = 0;
    double _mul59r = 0, _mul59i = 0;
    c_mul(_c58r, _c58i, theta2_r, theta2_i, &_mul59r, &_mul59i);
    double _add60r = 0, _add60i = 0;
    _add60r = theta1_r + _mul59r; _add60i = theta1_i + _mul59i;
    double _sin61r = 0, _sin61i = 0;
    c_sin(_add60r, _add60i, &_sin61r, &_sin61i);
    double _mul62r = 0, _mul62i = 0;
    c_mul(_c57r, _c57i, _sin61r, _sin61i, &_mul62r, &_mul62i);
    double _add63r = 0, _add63i = 0;
    _add63r = _c56r + _mul62r; _add63i = _c56i + _mul62i;
    double scale3_r = _add63r, scale3_i = _add63i;
    double _c64r = 0, _c64i = 0;
    _c64r = 80.0; _c64i = 0;
    double _c65r = 0, _c65i = 0;
    _c65r = 5.0; _c65i = 0;
    double _pow66r = 0, _pow66i = 0;
    c_powr(x1r, x1i, 5.0, &_pow66r, &_pow66i);
    double _sub67r = 0, _sub67i = 0;
    _sub67r = _pow66r - x2r; _sub67i = _pow66i - x2i;
    double _mul68r = 0, _mul68i = 0;
    c_mul(_c64r, _c64i, _sub67r, _sub67i, &_mul68r, &_mul68i);
    double _mul69r = 0, _mul69i = 0;
    c_mul(_mul68r, _mul68i, scale3_r, scale3_i, &_mul69r, &_mul69i);
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul69r; cIm[_idx] = _mul69i; } }
    double _c70r = 0, _c70i = 0;
    _c70r = 100.0; _c70i = 0;
    double _neg71r = 0, _neg71i = 0;
    _neg71r = -(_c70r); _neg71i = -(_c70i);
    double _c72r = 0, _c72i = 0;
    _c72r = 2.0; _c72i = 0;
    double _pow73r = 0, _pow73i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow73r, &_pow73i);
    double _c74r = 0, _c74i = 0;
    _c74r = 2.0; _c74i = 0;
    double _pow75r = 0, _pow75i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow75r, &_pow75i);
    double _mul76r = 0, _mul76i = 0;
    c_mul(_pow73r, _pow73i, _pow75r, _pow75i, &_mul76r, &_mul76i);
    double _mul77r = 0, _mul77i = 0;
    c_mul(_neg71r, _neg71i, _mul76r, _mul76i, &_mul77r, &_mul77i);
    double _mul78r = 0, _mul78i = 0;
    c_mul(_mul77r, _mul77i, scale3_r, scale3_i, &_mul78r, &_mul78i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul78r; cIm[_idx] = _mul78i; } }
    double _c79r = 0, _c79i = 0;
    _c79r = 20.0; _c79i = 0;
    double _neg80r = 0, _neg80i = 0;
    _neg80r = -(_c79r); _neg80i = -(_c79i);
    double _c81r = 0, _c81i = 0;
    _c81r = 3.0; _c81i = 0;
    double _pow82r = 0, _pow82i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow82r, &_pow82i);
    c_mul(_pow82r, _pow82i, x1r, x1i, &_pow82r, &_pow82i);
    double _add83r = 0, _add83i = 0;
    _add83r = _pow82r + x2r; _add83i = _pow82i + x2i;
    double _mul84r = 0, _mul84i = 0;
    c_mul(_neg80r, _neg80i, _add83r, _add83i, &_mul84r, &_mul84i);
    double _mul85r = 0, _mul85i = 0;
    c_mul(_mul84r, _mul84i, scale3_r, scale3_i, &_mul85r, &_mul85i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul85r; cIm[_idx] = _mul85i; } }
    double _c86r = 0, _c86i = 0;
    _c86r = 1.0; _c86i = 0;
    double _c87r = 0, _c87i = 0;
    _c87r = 0.2; _c87i = 0;
    double _c88r = 0, _c88i = 0;
    _c88r = 2.0; _c88i = 0;
    double _mul89r = 0, _mul89i = 0;
    c_mul(_c88r, _c88i, theta1_r, theta1_i, &_mul89r, &_mul89i);
    double _sub90r = 0, _sub90i = 0;
    _sub90r = _mul89r - theta2_r; _sub90i = _mul89i - theta2_i;
    double _cos91r = 0, _cos91i = 0;
    c_cos(_sub90r, _sub90i, &_cos91r, &_cos91i);
    double _mul92r = 0, _mul92i = 0;
    c_mul(_c87r, _c87i, _cos91r, _cos91i, &_mul92r, &_mul92i);
    double _add93r = 0, _add93i = 0;
    _add93r = _c86r + _mul92r; _add93i = _c86i + _mul92i;
    double scale4_r = _add93r, scale4_i = _add93i;
    double _c94r = 0, _c94i = 0;
    _c94r = 5.0; _c94i = 0;
    double _neg95r = 0, _neg95i = 0;
    _neg95r = -(_c94r); _neg95i = -(_c94i);
    double _c96r = 0, _c96i = 0;
    _c96r = 2.0; _c96i = 0;
    double _pow97r = 0, _pow97i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow97r, &_pow97i);
    double _c98r = 0, _c98i = 0;
    _c98r = 2.0; _c98i = 0;
    double _pow99r = 0, _pow99i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow99r, &_pow99i);
    double _sub100r = 0, _sub100i = 0;
    _sub100r = _pow97r - _pow99r; _sub100i = _pow97i - _pow99i;
    double _mul101r = 0, _mul101i = 0;
    c_mul(_neg95r, _neg95i, _sub100r, _sub100i, &_mul101r, &_mul101i);
    double _mul102r = 0, _mul102i = 0;
    c_mul(_mul101r, _mul101i, scale4_r, scale4_i, &_mul102r, &_mul102i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul102r; cIm[_idx] = _mul102i; } }
    double _c103r = 0, _c103i = 0;
    _c103r = 3.0; _c103i = 0;
    double _pow104r = 0, _pow104i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow104r, &_pow104i);
    c_mul(_pow104r, _pow104i, x2r, x2i, &_pow104r, &_pow104i);
    double _sub105r = 0, _sub105i = 0;
    _sub105r = _pow104r - x1r; _sub105i = _pow104i - x1i;
    double _mul106r = 0, _mul106i = 0;
    c_mul(_sub105r, _sub105i, scale4_r, scale4_i, &_mul106r, &_mul106i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul106r; cIm[_idx] = _mul106i; } }
    double _c107r = 0, _c107i = 0;
    _c107r = 5.0; _c107i = 0;
    double _neg108r = 0, _neg108i = 0;
    _neg108r = -(_c107r); _neg108i = -(_c107i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _neg108r; cIm[_idx] = _neg108i; } }
    double _c109r = 0, _c109i = 0;
    _c109r = 0.5; _c109i = 0;
    double _c110r = 0, _c110i = 0;
    _c110r = 0.5; _c110i = 0;
    double _c111r = 0, _c111i = 0;
    _c111r = 3.0; _c111i = 0;
    double _mul112r = 0, _mul112i = 0;
    c_mul(_c111r, _c111i, theta1_r, theta1_i, &_mul112r, &_mul112i);
    double _sin113r = 0, _sin113i = 0;
    c_sin(_mul112r, _mul112i, &_sin113r, &_sin113i);
    double _mul114r = 0, _mul114i = 0;
    c_mul(_c110r, _c110i, _sin113r, _sin113i, &_mul114r, &_mul114i);
    double _c115r = 0, _c115i = 0;
    _c115r = 4.0; _c115i = 0;
    double _mul116r = 0, _mul116i = 0;
    c_mul(_c115r, _c115i, theta2_r, theta2_i, &_mul116r, &_mul116i);
    double _cos117r = 0, _cos117i = 0;
    c_cos(_mul116r, _mul116i, &_cos117r, &_cos117i);
    double _mul118r = 0, _mul118i = 0;
    c_mul(_mul114r, _mul114i, _cos117r, _cos117i, &_mul118r, &_mul118i);
    double _add119r = 0, _add119i = 0;
    _add119r = _c109r + _mul118r; _add119i = _c109i + _mul118i;
    double perturb_scale_r = _add119r, perturb_scale_i = _add119i;
    double _c120r = 0, _c120i = 0;
    _c120r = 4.0; _c120i = 0;
    double _pow121r = 0, _pow121i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow121r, &_pow121i);
    c_mul(_pow121r, _pow121i, _pow121r, _pow121i, &_pow121r, &_pow121i);
    double _c122r = 0, _c122i = 0;
    _c122r = 4.0; _c122i = 0;
    double _pow123r = 0, _pow123i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow123r, &_pow123i);
    c_mul(_pow123r, _pow123i, _pow123r, _pow123i, &_pow123r, &_pow123i);
    double _sub124r = 0, _sub124i = 0;
    _sub124r = _pow121r - _pow123r; _sub124i = _pow121i - _pow123i;
    double _mul125r = 0, _mul125i = 0;
    c_mul(_sub124r, _sub124i, perturb_scale_r, perturb_scale_i, &_mul125r, &_mul125i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul125r; cIm[_idx] = _mul125i; } }
    double _c126r = 0, _c126i = 0;
    _c126r = 0.5; _c126i = 0;
    double _neg127r = 0, _neg127i = 0;
    _neg127r = -(_c126r); _neg127i = -(_c126i);
    double _c128r = 0, _c128i = 0;
    _c128r = 9.0; _c128i = 0;
    double _pow129r = 0, _pow129i = 0;
    c_powr(x1r, x1i, 9.0, &_pow129r, &_pow129i);
    double _c130r = 0, _c130i = 0;
    _c130r = 9.0; _c130i = 0;
    double _pow131r = 0, _pow131i = 0;
    c_powr(x2r, x2i, 9.0, &_pow131r, &_pow131i);
    double _add132r = 0, _add132i = 0;
    _add132r = _pow129r + _pow131r; _add132i = _pow129i + _pow131i;
    double _mul133r = 0, _mul133i = 0;
    c_mul(_neg127r, _neg127i, _add132r, _add132i, &_mul133r, &_mul133i);
    double _mul134r = 0, _mul134i = 0;
    c_mul(_mul133r, _mul133i, perturb_scale_r, perturb_scale_i, &_mul134r, &_mul134i);
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul134r; cIm[_idx] = _mul134i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g18_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _ang1r = 0, _ang1i = 0;
    _ang1r = c_arg(x1r, x1i); _ang1i = 0;
    double theta1_r = _ang1r, theta1_i = _ang1i;
    double _ang2r = 0, _ang2i = 0;
    _ang2r = c_arg(x2r, x2i); _ang2i = 0;
    double theta2_r = _ang2r, theta2_i = _ang2i;
    double _c3r = 0, _c3i = 0;
    _c3r = 1000.0; _c3i = 0;
    double _c4r = 0, _c4i = 0;
    _c4r = 0.5; _c4i = 0;
    double _c5r = 0, _c5i = 0;
    _c5r = 10.0; _c5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c5r, _c5i, theta1_r, theta1_i, &_mul6r, &_mul6i);
    double _c7r = 0, _c7i = 0;
    _c7r = 7.0; _c7i = 0;
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c7r, _c7i, theta2_r, theta2_i, &_mul8r, &_mul8i);
    double _sub9r = 0, _sub9i = 0;
    _sub9r = _mul6r - _mul8r; _sub9i = _mul6i - _mul8i;
    double _sin10r = 0, _sin10i = 0;
    c_sin(_sub9r, _sub9i, &_sin10r, &_sin10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c4r, _c4i, _sin10r, _sin10i, &_mul11r, &_mul11i);
    double _exp12r = 0, _exp12i = 0;
    c_exp2(_mul11r, _mul11i, &_exp12r, &_exp12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c3r, _c3i, _exp12r, _exp12i, &_mul13r, &_mul13i);
    double base_scale_r = _mul13r, base_scale_i = _mul13i;
    double _c14r = 0, _c14i = 0;
    _c14r = 500.0; _c14i = 0;
    double _c15r = 0, _c15i = 0;
    _c15r = 12.0; _c15i = 0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c15r, _c15i, theta1_r, theta1_i, &_mul16r, &_mul16i);
    double _c17r = 0, _c17i = 0;
    _c17r = 15.0; _c17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c17r, _c17i, theta2_r, theta2_i, &_mul18r, &_mul18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _mul16r + _mul18r; _add19i = _mul16i + _mul18i;
    double _cos20r = 0, _cos20i = 0;
    c_cos(_add19r, _add19i, &_cos20r, &_cos20i);
    double _c21r = 0, _c21i = 0;
    _c21r = 3.0; _c21i = 0;
    double _pow22r = 0, _pow22i = 0;
    c_mul(_cos20r, _cos20i, _cos20r, _cos20i, &_pow22r, &_pow22i);
    c_mul(_pow22r, _pow22i, _cos20r, _cos20i, &_pow22r, &_pow22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_c14r, _c14i, _pow22r, _pow22i, &_mul23r, &_mul23i);
    double secondary_scale_r = _mul23r, secondary_scale_i = _mul23i;
    double toggle_scale = 0;
    double _sub24r = 0, _sub24i = 0;
    _sub24r = theta1_r - theta2_r; _sub24i = theta1_i - theta2_i;
    double _sin25r = 0, _sin25i = 0;
    c_sin(_sub24r, _sub24i, &_sin25r, &_sin25i);
    double _c26r = 0, _c26i = 0;
    _c26r = 0.0; _c26i = 0;
    if (_sin25r > _c26r) {
        double _c27r = 0, _c27i = 0;
        _c27r = 2000.0; _c27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 5.0; _c28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c28r, _c28i, theta1_r, theta1_i, &_mul29r, &_mul29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(_mul29r, _mul29i, &_sin30r, &_sin30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c27r, _c27i, _sin30r, _sin30i, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 3.0; _c32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c32r, _c32i, theta2_r, theta2_i, &_mul33r, &_mul33i);
        double _cos34r = 0, _cos34i = 0;
        c_cos(_mul33r, _mul33i, &_cos34r, &_cos34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_mul31r, _mul31i, _cos34r, _cos34i, &_mul35r, &_mul35i);
        toggle_scale = _mul35r;
    } else {
        double _c36r = 0, _c36i = 0;
        _c36r = 2000.0; _c36i = 0;
        double _neg37r = 0, _neg37i = 0;
        _neg37r = -(_c36r); _neg37i = -(_c36i);
        double _c38r = 0, _c38i = 0;
        _c38r = 4.0; _c38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, theta1_r, theta1_i, &_mul39r, &_mul39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_mul39r, _mul39i, &_cos40r, &_cos40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_neg37r, _neg37i, _cos40r, _cos40i, &_mul41r, &_mul41i);
        double _c42r = 0, _c42i = 0;
        _c42r = 2.0; _c42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, theta2_r, theta2_i, &_mul43r, &_mul43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_mul43r, _mul43i, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_mul41r, _mul41i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        toggle_scale = _mul45r;
    }
    double _add46r = 0, _add46i = 0;
    _add46r = theta1_r + theta2_r; _add46i = theta1_i + theta2_i;
    double _cos47r = 0, _cos47i = 0;
    c_cos(_add46r, _add46i, &_cos47r, &_cos47i);
    double _c48r = 0, _c48i = 0;
    _c48r = 0.5; _c48i = 0;
    if (_cos47r > _c48r) {
        double _c49r = 0, _c49i = 0;
        _c49r = 7.0; _c49i = 0;
        double _pow50r = 0, _pow50i = 0;
        c_powr(x1r, x1i, 7.0, &_pow50r, &_pow50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 9.0; _c51i = 0;
        double _pow52r = 0, _pow52i = 0;
        c_powr(x2r, x2i, 9.0, &_pow52r, &_pow52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(_pow50r, _pow50i, _pow52r, _pow52i, &_mul53r, &_mul53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_mul53r, _mul53i, base_scale_r, base_scale_i, &_mul54r, &_mul54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_mul54r, _mul54i, toggle_scale, 0, &_mul55r, &_mul55i);
        { int _idx = 20; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    } else {
        double _add56r = 0, _add56i = 0;
        _add56r = theta1_r + theta2_r; _add56i = theta1_i + theta2_i;
        double _cos57r = 0, _cos57i = 0;
        c_cos(_add56r, _add56i, &_cos57r, &_cos57i);
        double _c58r = 0, _c58i = 0;
        _c58r = 0.5; _c58i = 0;
        double _neg59r = 0, _neg59i = 0;
        _neg59r = -(_c58r); _neg59i = -(_c58i);
        if (_cos57r < _neg59r) {
            double _c60r = 0, _c60i = 0;
            _c60r = 10.0; _c60i = 0;
            double _pow61r = 0, _pow61i = 0;
            c_powr(x1r, x1i, 10.0, &_pow61r, &_pow61i);
            double _c62r = 0, _c62i = 0;
            _c62r = 10.0; _c62i = 0;
            double _pow63r = 0, _pow63i = 0;
            c_powr(x2r, x2i, 10.0, &_pow63r, &_pow63i);
            double _sub64r = 0, _sub64i = 0;
            _sub64r = _pow61r - _pow63r; _sub64i = _pow61i - _pow63i;
            double _mul65r = 0, _mul65i = 0;
            c_mul(_sub64r, _sub64i, secondary_scale_r, secondary_scale_i, &_mul65r, &_mul65i);
            double _mul66r = 0, _mul66i = 0;
            c_mul(_mul65r, _mul65i, toggle_scale, 0, &_mul66r, &_mul66i);
            { int _idx = 18; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul66r; cIm[_idx] = _mul66i; } }
        } else {
            double _c67r = 0, _c67i = 0;
            _c67r = 4.0; _c67i = 0;
            double _pow68r = 0, _pow68i = 0;
            c_mul(x1r, x1i, x1r, x1i, &_pow68r, &_pow68i);
            c_mul(_pow68r, _pow68i, _pow68r, _pow68i, &_pow68r, &_pow68i);
            double _c69r = 0, _c69i = 0;
            _c69r = 7.0; _c69i = 0;
            double _pow70r = 0, _pow70i = 0;
            c_powr(x2r, x2i, 7.0, &_pow70r, &_pow70i);
            double _mul71r = 0, _mul71i = 0;
            c_mul(_pow68r, _pow68i, _pow70r, _pow70i, &_mul71r, &_mul71i);
            double _c72r = 0, _c72i = 0;
            _c72r = 5.0; _c72i = 0;
            double _pow73r = 0, _pow73i = 0;
            c_powr(x1r, x1i, 5.0, &_pow73r, &_pow73i);
            double _sub74r = 0, _sub74i = 0;
            _sub74r = _mul71r - _pow73r; _sub74i = _mul71i - _pow73i;
            double _mul75r = 0, _mul75i = 0;
            c_mul(_sub74r, _sub74i, base_scale_r, base_scale_i, &_mul75r, &_mul75i);
            double _mul76r = 0, _mul76i = 0;
            c_mul(_mul75r, _mul75i, secondary_scale_r, secondary_scale_i, &_mul76r, &_mul76i);
            { int _idx = 12; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul76r; cIm[_idx] = _mul76i; } }
        }
    }
    double _c77r = 0, _c77i = 0;
    _c77r = 300.0; _c77i = 0;
    double _c78r = 0, _c78i = 0;
    _c78r = 3.0; _c78i = 0;
    double _mul79r = 0, _mul79i = 0;
    c_mul(_c78r, _c78i, theta1_r, theta1_i, &_mul79r, &_mul79i);
    double _c80r = 0, _c80i = 0;
    _c80r = 4.0; _c80i = 0;
    double _mul81r = 0, _mul81i = 0;
    c_mul(_c80r, _c80i, theta2_r, theta2_i, &_mul81r, &_mul81i);
    double _add82r = 0, _add82i = 0;
    _add82r = _mul79r + _mul81r; _add82i = _mul79i + _mul81i;
    double _sin83r = 0, _sin83i = 0;
    c_sin(_add82r, _add82i, &_sin83r, &_sin83i);
    double _sin84r = 0, _sin84i = 0;
    c_sin(_sin83r, _sin83i, &_sin84r, &_sin84i);
    double _c85r = 0, _c85i = 0;
    _c85r = 2.0; _c85i = 0;
    double _pow86r = 0, _pow86i = 0;
    c_mul(_sin84r, _sin84i, _sin84r, _sin84i, &_pow86r, &_pow86i);
    double _mul87r = 0, _mul87i = 0;
    c_mul(_c77r, _c77i, _pow86r, _pow86i, &_mul87r, &_mul87i);
    double complex_scale_r = _mul87r, complex_scale_i = _mul87i;
    double _c88r = 0, _c88i = 0;
    _c88r = 5.0; _c88i = 0;
    double _pow89r = 0, _pow89i = 0;
    c_powr(x1r, x1i, 5.0, &_pow89r, &_pow89i);
    double _sub90r = 0, _sub90i = 0;
    _sub90r = _pow89r - x2r; _sub90i = _pow89i - x2i;
    double _mul91r = 0, _mul91i = 0;
    c_mul(_sub90r, _sub90i, complex_scale_r, complex_scale_i, &_mul91r, &_mul91i);
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul91r; cIm[_idx] = _mul91i; } }
    double _c92r = 0, _c92i = 0;
    _c92r = 100.0; _c92i = 0;
    double _sin93r = 0, _sin93i = 0;
    c_sin(theta1_r, theta1_i, &_sin93r, &_sin93i);
    double _cos94r = 0, _cos94i = 0;
    c_cos(theta2_r, theta2_i, &_cos94r, &_cos94i);
    double _mul95r = 0, _mul95i = 0;
    c_mul(_sin93r, _sin93i, _cos94r, _cos94i, &_mul95r, &_mul95i);
    double _exp96r = 0, _exp96i = 0;
    c_exp2(_mul95r, _mul95i, &_exp96r, &_exp96i);
    double _mul97r = 0, _mul97i = 0;
    c_mul(_c92r, _c92i, _exp96r, _exp96i, &_mul97r, &_mul97i);
    double another_scale_r = _mul97r, another_scale_i = _mul97i;
    double _neg98r = 0, _neg98i = 0;
    _neg98r = -(another_scale_r); _neg98i = -(another_scale_i);
    double _c99r = 0, _c99i = 0;
    _c99r = 2.0; _c99i = 0;
    double _pow100r = 0, _pow100i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow100r, &_pow100i);
    double _c101r = 0, _c101i = 0;
    _c101r = 2.0; _c101i = 0;
    double _pow102r = 0, _pow102i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow102r, &_pow102i);
    double _mul103r = 0, _mul103i = 0;
    c_mul(_pow100r, _pow100i, _pow102r, _pow102i, &_mul103r, &_mul103i);
    double _mul104r = 0, _mul104i = 0;
    c_mul(_neg98r, _neg98i, _mul103r, _mul103i, &_mul104r, &_mul104i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul104r; cIm[_idx] = _mul104i; } }
    double _add105r = 0, _add105i = 0;
    _add105r = theta1_r + theta2_r; _add105i = theta1_i + theta2_i;
    double _c106r = 0, _c106i = 0;
    _c106r = 3.0; _c106i = 0;
    double _mul107r = 0, _mul107i = 0;
    c_mul(_add105r, _add105i, _c106r, _c106i, &_mul107r, &_mul107i);
    double _flr108r = 0, _flr108i = 0;
    _flr108r = floor(_mul107r); _flr108i = 0;
    double _c109r = 0, _c109i = 0;
    _c109r = 2.0; _c109i = 0;
    double _mod110r = 0, _mod110i = 0;
    _mod110r = fmod(_flr108r, _c109r); _mod110i = 0;
    double _c111r = 0, _c111i = 0;
    _c111r = 0.0; _c111i = 0;
    double _c112r = 0, _c112i = 0;
    _c112r = 1.0; _c112i = 0;
    double _c113r = 0, _c113i = 0;
    _c113r = 1.0; _c113i = 0;
    double _neg114r = 0, _neg114i = 0;
    _neg114r = -(_c113r); _neg114i = -(_c113i);
    double _tern115r = 0, _tern115i = 0;
    if (_mod110r == _c111r) { _tern115r = _c112r; _tern115i = _c112i; }
    else { _tern115r = _neg114r; _tern115i = _neg114i; }
    double sign_flip_r = _tern115r, sign_flip_i = _tern115i;
    double _c116r = 0, _c116i = 0;
    _c116r = 50.0; _c116i = 0;
    double _mul117r = 0, _mul117i = 0;
    c_mul(sign_flip_r, sign_flip_i, _c116r, _c116i, &_mul117r, &_mul117i);
    double _c118r = 0, _c118i = 0;
    _c118r = 3.0; _c118i = 0;
    double _pow119r = 0, _pow119i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow119r, &_pow119i);
    c_mul(_pow119r, _pow119i, x1r, x1i, &_pow119r, &_pow119i);
    double _add120r = 0, _add120i = 0;
    _add120r = _pow119r + x2r; _add120i = _pow119i + x2i;
    double _mul121r = 0, _mul121i = 0;
    c_mul(_mul117r, _mul117i, _add120r, _add120i, &_mul121r, &_mul121i);
    double _c122r = 0, _c122i = 0;
    _c122r = 2.0; _c122i = 0;
    double _mul123r = 0, _mul123i = 0;
    c_mul(_c122r, _c122i, theta1_r, theta1_i, &_mul123r, &_mul123i);
    double _sub124r = 0, _sub124i = 0;
    _sub124r = _mul123r - theta2_r; _sub124i = _mul123i - theta2_i;
    double _sin125r = 0, _sin125i = 0;
    c_sin(_sub124r, _sub124i, &_sin125r, &_sin125i);
    double _mul126r = 0, _mul126i = 0;
    c_mul(_mul121r, _mul121i, _sin125r, _sin125i, &_mul126r, &_mul126i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul126r; cIm[_idx] = _mul126i; } }
    double _c127r = 0, _c127i = 0;
    _c127r = 5.0; _c127i = 0;
    double _neg128r = 0, _neg128i = 0;
    _neg128r = -(_c127r); _neg128i = -(_c127i);
    double _c129r = 0, _c129i = 0;
    _c129r = 2.0; _c129i = 0;
    double _pow130r = 0, _pow130i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow130r, &_pow130i);
    double _c131r = 0, _c131i = 0;
    _c131r = 2.0; _c131i = 0;
    double _pow132r = 0, _pow132i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow132r, &_pow132i);
    double _sub133r = 0, _sub133i = 0;
    _sub133r = _pow130r - _pow132r; _sub133i = _pow130i - _pow132i;
    double _mul134r = 0, _mul134i = 0;
    c_mul(_neg128r, _neg128i, _sub133r, _sub133i, &_mul134r, &_mul134i);
    double _c135r = 0, _c135i = 0;
    _c135r = 10.0; _c135i = 0;
    double _c136r = 0, _c136i = 0;
    _c136r = 5.0; _c136i = 0;
    double _mul137r = 0, _mul137i = 0;
    c_mul(_c136r, _c136i, theta2_r, theta2_i, &_mul137r, &_mul137i);
    double _cos138r = 0, _cos138i = 0;
    c_cos(_mul137r, _mul137i, &_cos138r, &_cos138i);
    double _mul139r = 0, _mul139i = 0;
    c_mul(_c135r, _c135i, _cos138r, _cos138i, &_mul139r, &_mul139i);
    double _mul140r = 0, _mul140i = 0;
    c_mul(_mul134r, _mul134i, _mul139r, _mul139i, &_mul140r, &_mul140i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul140r; cIm[_idx] = _mul140i; } }
    double _c141r = 0, _c141i = 0;
    _c141r = 3.0; _c141i = 0;
    double _pow142r = 0, _pow142i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow142r, &_pow142i);
    c_mul(_pow142r, _pow142i, x2r, x2i, &_pow142r, &_pow142i);
    double _sub143r = 0, _sub143i = 0;
    _sub143r = _pow142r - x1r; _sub143i = _pow142i - x1i;
    double _c144r = 0, _c144i = 0;
    _c144r = 200.0; _c144i = 0;
    double _c145r = 0, _c145i = 0;
    _c145r = 3.0; _c145i = 0;
    double _mul146r = 0, _mul146i = 0;
    c_mul(_c145r, _c145i, theta1_r, theta1_i, &_mul146r, &_mul146i);
    double _sin147r = 0, _sin147i = 0;
    c_sin(_mul146r, _mul146i, &_sin147r, &_sin147i);
    double _mul148r = 0, _mul148i = 0;
    c_mul(_c144r, _c144i, _sin147r, _sin147i, &_mul148r, &_mul148i);
    double _sin149r = 0, _sin149i = 0;
    c_sin(theta2_r, theta2_i, &_sin149r, &_sin149i);
    double _mul150r = 0, _mul150i = 0;
    c_mul(_mul148r, _mul148i, _sin149r, _sin149i, &_mul150r, &_mul150i);
    double _mul151r = 0, _mul151i = 0;
    c_mul(_sub143r, _sub143i, _mul150r, _mul150i, &_mul151r, &_mul151i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul151r; cIm[_idx] = _mul151i; } }
    double _c152r = 0, _c152i = 0;
    _c152r = 5.0; _c152i = 0;
    double _neg153r = 0, _neg153i = 0;
    _neg153r = -(_c152r); _neg153i = -(_c152i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _neg153r; cIm[_idx] = _neg153i; } }
    double _c154r = 0, _c154i = 0;
    _c154r = 4.0; _c154i = 0;
    double _pow155r = 0, _pow155i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow155r, &_pow155i);
    c_mul(_pow155r, _pow155i, _pow155r, _pow155i, &_pow155r, &_pow155i);
    double _c156r = 0, _c156i = 0;
    _c156r = 4.0; _c156i = 0;
    double _pow157r = 0, _pow157i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow157r, &_pow157i);
    c_mul(_pow157r, _pow157i, _pow157r, _pow157i, &_pow157r, &_pow157i);
    double _sub158r = 0, _sub158i = 0;
    _sub158r = _pow155r - _pow157r; _sub158i = _pow155i - _pow157i;
    double _c159r = 0, _c159i = 0;
    _c159r = 100.0; _c159i = 0;
    double _mul160r = 0, _mul160i = 0;
    c_mul(_sub158r, _sub158i, _c159r, _c159i, &_mul160r, &_mul160i);
    double _sin161r = 0, _sin161i = 0;
    c_sin(theta1_r, theta1_i, &_sin161r, &_sin161i);
    double _mul162r = 0, _mul162i = 0;
    c_mul(_sin161r, _sin161i, theta2_r, theta2_i, &_mul162r, &_mul162i);
    double _cos163r = 0, _cos163i = 0;
    c_cos(_mul162r, _mul162i, &_cos163r, &_cos163i);
    double _mul164r = 0, _mul164i = 0;
    c_mul(_mul160r, _mul160i, _cos163r, _cos163i, &_mul164r, &_mul164i);
    double _c165r = 0, _c165i = 0;
    _c165r = 2.0; _c165i = 0;
    double _mul166r = 0, _mul166i = 0;
    c_mul(_c165r, _c165i, theta1_r, theta1_i, &_mul166r, &_mul166i);
    double _c167r = 0, _c167i = 0;
    _c167r = 3.0; _c167i = 0;
    double _mul168r = 0, _mul168i = 0;
    c_mul(_c167r, _c167i, theta2_r, theta2_i, &_mul168r, &_mul168i);
    double _sub169r = 0, _sub169i = 0;
    _sub169r = _mul166r - _mul168r; _sub169i = _mul166i - _mul168i;
    double _cos170r = 0, _cos170i = 0;
    c_cos(_sub169r, _sub169i, &_cos170r, &_cos170i);
    double _exp171r = 0, _exp171i = 0;
    c_exp2(_cos170r, _cos170i, &_exp171r, &_exp171i);
    double _mul172r = 0, _mul172i = 0;
    c_mul(_mul164r, _mul164i, _exp171r, _exp171i, &_mul172r, &_mul172i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul172r; cIm[_idx] = _mul172i; } }
    double _c173r = 0, _c173i = 0;
    _c173r = 10.0; _c173i = 0;
    double _neg174r = 0, _neg174i = 0;
    _neg174r = -(_c173r); _neg174i = -(_c173i);
    double _c175r = 0, _c175i = 0;
    _c175r = 9.0; _c175i = 0;
    double _pow176r = 0, _pow176i = 0;
    c_powr(x1r, x1i, 9.0, &_pow176r, &_pow176i);
    double _c177r = 0, _c177i = 0;
    _c177r = 9.0; _c177i = 0;
    double _pow178r = 0, _pow178i = 0;
    c_powr(x2r, x2i, 9.0, &_pow178r, &_pow178i);
    double _add179r = 0, _add179i = 0;
    _add179r = _pow176r + _pow178r; _add179i = _pow176i + _pow178i;
    double _mul180r = 0, _mul180i = 0;
    c_mul(_neg174r, _neg174i, _add179r, _add179i, &_mul180r, &_mul180i);
    double _c181r = 0, _c181i = 0;
    _c181r = 7.0; _c181i = 0;
    double _mul182r = 0, _mul182i = 0;
    c_mul(_c181r, _c181i, theta1_r, theta1_i, &_mul182r, &_mul182i);
    double _c183r = 0, _c183i = 0;
    _c183r = 8.0; _c183i = 0;
    double _mul184r = 0, _mul184i = 0;
    c_mul(_c183r, _c183i, theta2_r, theta2_i, &_mul184r, &_mul184i);
    double _sub185r = 0, _sub185i = 0;
    _sub185r = _mul182r - _mul184r; _sub185i = _mul182i - _mul184i;
    double _sin186r = 0, _sin186i = 0;
    c_sin(_sub185r, _sub185i, &_sin186r, &_sin186i);
    double _c187r = 0, _c187i = 0;
    _c187r = 3.0; _c187i = 0;
    double _pow188r = 0, _pow188i = 0;
    c_mul(_sin186r, _sin186i, _sin186r, _sin186i, &_pow188r, &_pow188i);
    c_mul(_pow188r, _pow188i, _sin186r, _sin186i, &_pow188r, &_pow188i);
    double _mul189r = 0, _mul189i = 0;
    c_mul(_mul180r, _mul180i, _pow188r, _pow188i, &_mul189r, &_mul189i);
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul189r; cIm[_idx] = _mul189i; } }
    double _c190r = 0, _c190i = 0;
    _c190r = 500.0; _c190i = 0;
    double _c191r = 0, _c191i = 0;
    _c191r = 6.0; _c191i = 0;
    double _pow192r = 0, _pow192i = 0;
    c_powr(x1r, x1i, 6.0, &_pow192r, &_pow192i);
    double _c193r = 0, _c193i = 0;
    _c193r = 3.0; _c193i = 0;
    double _pow194r = 0, _pow194i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow194r, &_pow194i);
    c_mul(_pow194r, _pow194i, x2r, x2i, &_pow194r, &_pow194i);
    double _sub195r = 0, _sub195i = 0;
    _sub195r = _pow192r - _pow194r; _sub195i = _pow192i - _pow194i;
    double _mul196r = 0, _mul196i = 0;
    c_mul(_c190r, _c190i, _sub195r, _sub195i, &_mul196r, &_mul196i);
    double _add197r = 0, _add197i = 0;
    _add197r = theta1_r + theta2_r; _add197i = theta1_i + theta2_i;
    double _c198r = 0, _c198i = 0;
    _c198r = 2.0; _c198i = 0;
    double _pow199r = 0, _pow199i = 0;
    c_mul(_add197r, _add197i, _add197r, _add197i, &_pow199r, &_pow199i);
    double _sin200r = 0, _sin200i = 0;
    c_sin(_pow199r, _pow199i, &_sin200r, &_sin200i);
    double _mul201r = 0, _mul201i = 0;
    c_mul(_mul196r, _mul196i, _sin200r, _sin200i, &_mul201r, &_mul201i);
    double _sub202r = 0, _sub202i = 0;
    _sub202r = theta1_r - theta2_r; _sub202i = theta1_i - theta2_i;
    double _c203r = 0, _c203i = 0;
    _c203r = 2.0; _c203i = 0;
    double _pow204r = 0, _pow204i = 0;
    c_mul(_sub202r, _sub202i, _sub202r, _sub202i, &_pow204r, &_pow204i);
    double _cos205r = 0, _cos205i = 0;
    c_cos(_pow204r, _pow204i, &_cos205r, &_cos205i);
    double _mul206r = 0, _mul206i = 0;
    c_mul(_mul201r, _mul201i, _cos205r, _cos205i, &_mul206r, &_mul206i);
    { int _idx = 15; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul206r; cIm[_idx] = _mul206i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g19_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 90;
    for (int _i = 0; _i < 90; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _sub1r = 0, _sub1i = 0;
    _sub1r = x1r - x2r; _sub1i = x1i - x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _sub1r; cIm[_idx] = _sub1i; } }
    for (int k = 1; k < 90; k++) {
        double _cf2r = 0, _cf2i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 90) { _cf2r = cRe[_idx]; _cf2i = cIm[_idx]; } }
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
        double v_r = _add7r, v_i = _add7i;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(v_r, v_i); _abs8i = 0;
        double av_r = _abs8r, av_i = _abs8i;
        double _fin9r = 0, _fin9i = 0;
        _fin9r = (isfinite(av_r) && isfinite(av_i)) ? 1.0 : 0.0; _fin9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1e-10; _c10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 0.0; _c11i = 1.0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_c11r, _c11i, v_r, v_i, &_mul12r, &_mul12i);
        double _div13r = 0, _div13i = 0;
        c_div(_mul12r, _mul12i, av_r, av_i, &_div13r, &_div13i);
        double _add14r = 0, _add14i = 0;
        _add14r = x1r + x2r; _add14i = x1i + x2i;
        double _tern15r = 0, _tern15i = 0;
        if (((_fin9r != 0 || _fin9i != 0)) && (av_r > _c10r)) { _tern15r = _div13r; _tern15i = _div13i; }
        else { _tern15r = _add14r; _tern15i = _add14i; }
        { int _idx = k; if (_idx >= 0 && _idx < 90) { cRe[_idx] = _tern15r; cIm[_idx] = _tern15i; } }
    }
    for (int _i = 0; _i < 90; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g20_c(double x1r, double x1i, double x2r, double x2i,
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

static void g21_c(double x1r, double x1i, double x2r, double x2i,
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

static void g22_c(double x1r, double x1i, double x2r, double x2i,
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

static void g23_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    _c9r = 100.0; _c9i = 0;
    double _c10r = 0, _c10i = 0;
    _c10r = 3.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow11r, &_pow11i);
    c_mul(_pow11r, _pow11i, x2r, x2i, &_pow11r, &_pow11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_c9r, _c9i, _pow11r, _pow11i, &_mul12r, &_mul12i);
    double _c13r = 0, _c13i = 0;
    _c13r = 100.0; _c13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 2.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow15r, &_pow15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c13r, _c13i, _pow15r, _pow15i, &_mul16r, &_mul16i);
    double _add17r = 0, _add17i = 0;
    _add17r = _mul12r + _mul16r; _add17i = _mul12i + _mul16i;
    double _c18r = 0, _c18i = 0;
    _c18r = 100.0; _c18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c18r, _c18i, x2r, x2i, &_mul19r, &_mul19i);
    double _sub20r = 0, _sub20i = 0;
    _sub20r = _add17r - _mul19r; _sub20i = _add17i - _mul19i;
    double _c21r = 0, _c21i = 0;
    _c21r = 100.0; _c21i = 0;
    double _sub22r = 0, _sub22i = 0;
    _sub22r = _sub20r - _c21r; _sub22i = _sub20i - _c21i;
    { int _idx = 6; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub22r; cIm[_idx] = _sub22i; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 100.0; _c23i = 0;
    double _c24r = 0, _c24i = 0;
    _c24r = 3.0; _c24i = 0;
    double _pow25r = 0, _pow25i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow25r, &_pow25i);
    c_mul(_pow25r, _pow25i, x1r, x1i, &_pow25r, &_pow25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c23r, _c23i, _pow25r, _pow25i, &_mul26r, &_mul26i);
    double _c27r = 0, _c27i = 0;
    _c27r = 100.0; _c27i = 0;
    double _c28r = 0, _c28i = 0;
    _c28r = 2.0; _c28i = 0;
    double _pow29r = 0, _pow29i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow29r, &_pow29i);
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c27r, _c27i, _pow29r, _pow29i, &_mul30r, &_mul30i);
    double _add31r = 0, _add31i = 0;
    _add31r = _mul26r + _mul30r; _add31i = _mul26i + _mul30i;
    double _c32r = 0, _c32i = 0;
    _c32r = 100.0; _c32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c32r, _c32i, x2r, x2i, &_mul33r, &_mul33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _add31r + _mul33r; _add34i = _add31i + _mul33i;
    double _c35r = 0, _c35i = 0;
    _c35r = 100.0; _c35i = 0;
    double _sub36r = 0, _sub36i = 0;
    _sub36r = _add34r - _c35r; _sub36i = _add34i - _c35i;
    { int _idx = 8; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub36r; cIm[_idx] = _sub36i; } }
    double _c37r = 0, _c37i = 0;
    _c37r = 100.0; _c37i = 0;
    double _c38r = 0, _c38i = 0;
    _c38r = 3.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow39r, &_pow39i);
    c_mul(_pow39r, _pow39i, x2r, x2i, &_pow39r, &_pow39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c37r, _c37i, _pow39r, _pow39i, &_mul40r, &_mul40i);
    double _c41r = 0, _c41i = 0;
    _c41r = 100.0; _c41i = 0;
    double _c42r = 0, _c42i = 0;
    _c42r = 2.0; _c42i = 0;
    double _pow43r = 0, _pow43i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow43r, &_pow43i);
    double _mul44r = 0, _mul44i = 0;
    c_mul(_c41r, _c41i, _pow43r, _pow43i, &_mul44r, &_mul44i);
    double _sub45r = 0, _sub45i = 0;
    _sub45r = _mul40r - _mul44r; _sub45i = _mul40i - _mul44i;
    double _c46r = 0, _c46i = 0;
    _c46r = 100.0; _c46i = 0;
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c46r, _c46i, x2r, x2i, &_mul47r, &_mul47i);
    double _add48r = 0, _add48i = 0;
    _add48r = _sub45r + _mul47r; _add48i = _sub45i + _mul47i;
    double _c49r = 0, _c49i = 0;
    _c49r = 100.0; _c49i = 0;
    double _sub50r = 0, _sub50i = 0;
    _sub50r = _add48r - _c49r; _sub50i = _add48i - _c49i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub50r; cIm[_idx] = _sub50i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g24_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 26;
    for (int _i = 0; _i < 26; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
    _c18r = 100.0; _c18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c18r, _c18i, x2r, x2i, &_mul19r, &_mul19i);
    double _sub20r = 0, _sub20i = 0;
    _sub20r = _add17r - _mul19r; _sub20i = _add17i - _mul19i;
    double _c21r = 0, _c21i = 0;
    _c21r = 100.0; _c21i = 0;
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
    _c32r = 100.0; _c32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c32r, _c32i, x2r, x2i, &_mul33r, &_mul33i);
    double _add34r = 0, _add34i = 0;
    _add34r = _add31r + _mul33r; _add34i = _add31i + _mul33i;
    double _c35r = 0, _c35i = 0;
    _c35r = 100.0; _c35i = 0;
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
    _c46r = 100.0; _c46i = 0;
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c46r, _c46i, x2r, x2i, &_mul47r, &_mul47i);
    double _add48r = 0, _add48i = 0;
    _add48r = _sub45r + _mul47r; _add48i = _sub45i + _mul47i;
    double _c49r = 0, _c49i = 0;
    _c49r = 100.0; _c49i = 0;
    double _sub50r = 0, _sub50i = 0;
    _sub50r = _add48r - _c49r; _sub50i = _add48i - _c49i;
    { int _idx = 14; if (_idx >= 0 && _idx < 26) { cRe[_idx] = _sub50r; cIm[_idx] = _sub50i; } }
    for (int _i = 0; _i < 26; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g25_c(double x1r, double x1i, double x2r, double x2i,
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

static void g26_c(double x1r, double x1i, double x2r, double x2i,
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

/* g27: auto-stubbed (unhandled constructs in source) */
static void g27_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 12;
    for (int _i = 0; _i < 12; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g28_c(double x1r, double x1i, double x2r, double x2i,
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

static void g29_c(double x1r, double x1i, double x2r, double x2i,
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

static void g30_c(double x1r, double x1i, double x2r, double x2i,
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

/* g31: too complex for auto-transpile, stubbed */
static void g31_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 100;
    for (int _i = 0; _i < 100; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g32: auto-stubbed (unhandled constructs in source) */
static void g32_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 12;
    for (int _i = 0; _i < 12; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g33_c(double x1r, double x1i, double x2r, double x2i,
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

static void g34_c(double x1r, double x1i, double x2r, double x2i,
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

static void g35_c(double x1r, double x1i, double x2r, double x2i,
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

static void g36_c(double x1r, double x1i, double x2r, double x2i,
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

static void g37_c(double x1r, double x1i, double x2r, double x2i,
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

/* g38: too complex for auto-transpile, stubbed */
static void g38_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g39_c(double x1r, double x1i, double x2r, double x2i,
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

static void g40_c(double x1r, double x1i, double x2r, double x2i,
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

static void g41_c(double x1r, double x1i, double x2r, double x2i,
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

static void g42_c(double x1r, double x1i, double x2r, double x2i,
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

static void g43_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double i[] = {0.0, 4.0, 14.0, 29.0};
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

static void g44_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 30;
    for (int _i = 0; _i < 30; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double i[] = {0.0, 5.0, 11.0, 19.0};
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

static void g45_c(double x1r, double x1i, double x2r, double x2i,
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

static void g46_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    static const double i[] = {0.0, 7.0, 15.0, 23.0, 31.0};
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

/* g47: auto-stubbed (unhandled constructs in source) */
static void g47_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 30;
    for (int _i = 0; _i < 30; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g48: too complex for auto-transpile, stubbed */
static void g48_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    for (int _i = 0; _i < 40; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g49: too complex for auto-transpile, stubbed */
static void g49_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 30;
    for (int _i = 0; _i < 30; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g50: too complex for auto-transpile, stubbed */
static void g50_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 1000;
    for (int _i = 0; _i < 1000; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g51: too complex for auto-transpile, stubbed */
static void g51_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 1000;
    for (int _i = 0; _i < 1000; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g52: auto-stubbed (unhandled constructs in source) */
static void g52_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 100;
    for (int _i = 0; _i < 100; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g53_c(double x1r, double x1i, double x2r, double x2i,
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

static void g54_c(double x1r, double x1i, double x2r, double x2i,
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

static void g55_c(double x1r, double x1i, double x2r, double x2i,
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

static void g56_c(double x1r, double x1i, double x2r, double x2i,
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

/* g57: auto-stubbed (unhandled constructs in source) */
static void g57_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g58: auto-stubbed (unhandled constructs in source) */
static void g58_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    for (int _i = 0; _i < 10; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g59_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 11;
    for (int _i = 0; _i < 11; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double EPS = 0.0;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = x1r; _attr1i = 0;
    double t1r = _attr1r;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1i; _attr2i = 0;
    double t1i = _attr2r;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2r; _attr3i = 0;
    double t2r = _attr3r;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x2i; _attr4i = 0;
    double t2i = _attr4r;
    double _mul5r = 0, _mul5i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul5r, &_mul5i);
    double prod_r = _mul5r, prod_i = _mul5i;
    double _abs6r = 0, _abs6i = 0;
    _abs6r = c_abs(prod_r, prod_i); _abs6i = 0;
    double prod_abs_r = _abs6r, prod_abs_i = _abs6i;
    double _c7r = 0, _c7i = 0;
    _c7r = 100.0; _c7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 4.0; _c8i = 0;
    double _pow9r = 0, _pow9i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow9r, &_pow9i);
    c_mul(_pow9r, _pow9i, _pow9r, _pow9i, &_pow9r, &_pow9i);
    double _c10r = 0, _c10i = 0;
    _c10r = 4.0; _c10i = 0;
    double _pow11r = 0, _pow11i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow11r, &_pow11i);
    c_mul(_pow11r, _pow11i, _pow11r, _pow11i, &_pow11r, &_pow11i);
    double _sub12r = 0, _sub12i = 0;
    _sub12r = _pow9r - _pow11r; _sub12i = _pow9i - _pow11i;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c7r, _c7i, _sub12r, _sub12i, &_mul13r, &_mul13i);
    { int _idx = 0; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul13r; cIm[_idx] = _mul13i; } }
    double _c14r = 0, _c14i = 0;
    _c14r = 0.0; _c14i = 100.0;
    double _neg15r = 0, _neg15i = 0;
    _neg15r = -(_c14r); _neg15i = -(_c14i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul16r, &_mul16i);
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
    double _mul22r = 0, _mul22i = 0;
    c_mul(_mul16r, _mul16i, _add21r, _add21i, &_mul22r, &_mul22i);
    double _mul23r = 0, _mul23i = 0;
    c_mul(_neg15r, _neg15i, _mul22r, _mul22i, &_mul23r, &_mul23i);
    { int _idx = 1; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    double _mul24r = 0, _mul24i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_mul24r, &_mul24i);
    double _mul25r = 0, _mul25i = 0;
    c_mul(_mul24r, _mul24i, x2r, x2i, &_mul25r, &_mul25i);
    double _mul26r = 0, _mul26i = 0;
    c_mul(_mul25r, _mul25i, x2r, x2i, &_mul26r, &_mul26i);
    double _attr27r = 0, _attr27i = 0;
    _attr27r = _mul26r; _attr27i = 0;
    double v2_r = _attr27r, v2_i = _attr27i;
    double _fin28r = 0, _fin28i = 0;
    _fin28r = (isfinite(v2_r) && isfinite(v2_i)) ? 1.0 : 0.0; _fin28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 0.0; _c29i = 0;
    if ((!((_fin28r != 0 || _fin28i != 0))) || (v2_r < _c29r)) {
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 0;
        v2_r = _c30r; v2_i = _c30i;
    }
    double _c31r = 0, _c31i = 0;
    _c31r = 100.0; _c31i = 0;
    double _sqrt32r = 0, _sqrt32i = 0;
    c_powr(v2_r, v2_i, 0.5, &_sqrt32r, &_sqrt32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_c31r, _c31i, _sqrt32r, _sqrt32i, &_mul33r, &_mul33i);
    { int _idx = 2; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    double _c34r = 0, _c34i = 0;
    _c34r = 100.0; _c34i = 0;
    double _sub35r = 0, _sub35i = 0;
    _sub35r = t1i - t2i; _sub35i = 0 - 0;
    double _mul36r = 0, _mul36i = 0;
    c_mul(_c34r, _c34i, _sub35r, _sub35i, &_mul36r, &_mul36i);
    { int _idx = 3; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    double _c37r = 0, _c37i = 0;
    _c37r = 0.0; _c37i = 100.0;
    double _c38r = 0, _c38i = 0;
    _c38r = 2.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow39r, &_pow39i);
    double _c40r = 0, _c40i = 0;
    _c40r = 2.0; _c40i = 0;
    double _pow41r = 0, _pow41i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow41r, &_pow41i);
    double _add42r = 0, _add42i = 0;
    _add42r = _pow39r + _pow41r; _add42i = _pow39i + _pow41i;
    double _mul43r = 0, _mul43i = 0;
    c_mul(_c37r, _c37i, _add42r, _add42i, &_mul43r, &_mul43i);
    { int _idx = 4; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    double _sub44r = 0, _sub44i = 0;
    _sub44r = x1r - x2r; _sub44i = x1i - x2i;
    double _abs45r = 0, _abs45i = 0;
    _abs45r = c_abs(_sub44r, _sub44i); _abs45i = 0;
    double a5_r = _abs45r, a5_i = _abs45i;
    double _c46r = 0, _c46i = 0;
    _c46r = 100.0; _c46i = 0;
    double _cos47r = 0, _cos47i = 0;
    c_cos(a5_r, a5_i, &_cos47r, &_cos47i);
    double _c48r = 0, _c48i = 0;
    _c48r = 0.0; _c48i = 1.0;
    double _sin49r = 0, _sin49i = 0;
    c_sin(a5_r, a5_i, &_sin49r, &_sin49i);
    double _mul50r = 0, _mul50i = 0;
    c_mul(_c48r, _c48i, _sin49r, _sin49i, &_mul50r, &_mul50i);
    double _add51r = 0, _add51i = 0;
    _add51r = _cos47r + _mul50r; _add51i = _cos47i + _mul50i;
    double _mul52r = 0, _mul52i = 0;
    c_mul(_c46r, _c46i, _add51r, _add51i, &_mul52r, &_mul52i);
    { int _idx = 5; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul52r; cIm[_idx] = _mul52i; } }
    double _c53r = 0, _c53i = 0;
    _c53r = 100.0; _c53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = t1r + t2r; _add54i = 0 + 0;
    double _sin55r = 0, _sin55i = 0;
    c_sin(_add54r, _add54i, &_sin55r, &_sin55i);
    double _mul56r = 0, _mul56i = 0;
    c_mul(_c53r, _c53i, _sin55r, _sin55i, &_mul56r, &_mul56i);
    { int _idx = 6; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    double _c57r = 0, _c57i = 0;
    _c57r = 0.0; _c57i = 100.0;
    double _neg58r = 0, _neg58i = 0;
    _neg58r = -(_c57r); _neg58i = -(_c57i);
    double _sub59r = 0, _sub59i = 0;
    _sub59r = t1i - t2i; _sub59i = 0 - 0;
    double _cos60r = 0, _cos60i = 0;
    c_cos(_sub59r, _sub59i, &_cos60r, &_cos60i);
    double _mul61r = 0, _mul61i = 0;
    c_mul(_neg58r, _neg58i, _cos60r, _cos60i, &_mul61r, &_mul61i);
    { int _idx = 7; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul61r; cIm[_idx] = _mul61i; } }
    double _fin62r = 0, _fin62i = 0;
    _fin62r = (isfinite(prod_abs_r) && isfinite(prod_abs_i)) ? 1.0 : 0.0; _fin62i = 0;
    if (((_fin62r != 0 || _fin62i != 0)) && (prod_abs_r > EPS)) {
        double _c63r = 0, _c63i = 0;
        _c63r = 100.0; _c63i = 0;
        double _div64r = 0, _div64i = 0;
        c_div(prod_r, prod_i, prod_abs_r, prod_abs_i, &_div64r, &_div64i);
        double _mul65r = 0, _mul65i = 0;
        c_mul(_c63r, _c63i, _div64r, _div64i, &_mul65r, &_mul65i);
        { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul65r; cIm[_idx] = _mul65i; } }
    } else {
        double _c66r = 0, _c66i = 0;
        _c66r = 0.0; _c66i = 0;
        double _c67r = 0, _c67i = 0;
        _c67r = 0.0; _c67i = 0.0;
        double _add68r = 0, _add68i = 0;
        _add68r = _c66r + _c67r; _add68i = _c66i + _c67i;
        { int _idx = 8; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _add68r; cIm[_idx] = _add68i; } }
    }
    double _mul69r = 0, _mul69i = 0;
    c_mul(t1r, 0, t2r, 0, &_mul69r, &_mul69i);
    double rpart_r = _mul69r, rpart_i = _mul69i;
    double _mul70r = 0, _mul70i = 0;
    c_mul(t1i, 0, t2i, 0, &_mul70r, &_mul70i);
    double ipart_r = _mul70r, ipart_i = _mul70i;
    double _fin71r = 0, _fin71i = 0;
    _fin71r = (isfinite(rpart_r) && isfinite(rpart_i)) ? 1.0 : 0.0; _fin71i = 0;
    double _c72r = 0, _c72i = 0;
    _c72r = 0.0; _c72i = 0;
    if ((!((_fin71r != 0 || _fin71i != 0))) || (rpart_r < _c72r)) {
        double _c73r = 0, _c73i = 0;
        _c73r = 0.0; _c73i = 0;
        rpart_r = _c73r; rpart_i = _c73i;
    }
    double _fin74r = 0, _fin74i = 0;
    _fin74r = (isfinite(ipart_r) && isfinite(ipart_i)) ? 1.0 : 0.0; _fin74i = 0;
    double _c75r = 0, _c75i = 0;
    _c75r = 0.0; _c75i = 0;
    if ((!((_fin74r != 0 || _fin74i != 0))) || (ipart_r < _c75r)) {
        double _c76r = 0, _c76i = 0;
        _c76r = 0.0; _c76i = 0;
        ipart_r = _c76r; ipart_i = _c76i;
    }
    double _c77r = 0, _c77i = 0;
    _c77r = 100.0; _c77i = 0;
    double _sqrt78r = 0, _sqrt78i = 0;
    c_powr(rpart_r, rpart_i, 0.5, &_sqrt78r, &_sqrt78i);
    double _mul79r = 0, _mul79i = 0;
    c_mul(_c77r, _c77i, _sqrt78r, _sqrt78i, &_mul79r, &_mul79i);
    double _c80r = 0, _c80i = 0;
    _c80r = 0.0; _c80i = 100.0;
    double _sqrt81r = 0, _sqrt81i = 0;
    c_powr(ipart_r, ipart_i, 0.5, &_sqrt81r, &_sqrt81i);
    double _mul82r = 0, _mul82i = 0;
    c_mul(_c80r, _c80i, _sqrt81r, _sqrt81i, &_mul82r, &_mul82i);
    double _sub83r = 0, _sub83i = 0;
    _sub83r = _mul79r - _mul82r; _sub83i = _mul79i - _mul82i;
    { int _idx = 9; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _sub83r; cIm[_idx] = _sub83i; } }
    double _at284r = 0, _at284i = 0;
    _at284r = atan2(t1i, t1r); _at284i = 0;
    double ang1_r = _at284r, ang1_i = _at284i;
    double _at285r = 0, _at285i = 0;
    _at285r = atan2(t2i, t2r); _at285i = 0;
    double ang2_r = _at285r, ang2_i = _at285i;
    double _sub86r = 0, _sub86i = 0;
    _sub86r = ang1_r - ang2_r; _sub86i = ang1_i - ang2_i;
    double dphi_r = _sub86r, dphi_i = _sub86i;
    double _c87r = 0, _c87i = 0;
    _c87r = 100.0; _c87i = 0;
    double _cos88r = 0, _cos88i = 0;
    c_cos(dphi_r, dphi_i, &_cos88r, &_cos88i);
    double _c89r = 0, _c89i = 0;
    _c89r = 0.0; _c89i = 1.0;
    double _sin90r = 0, _sin90i = 0;
    c_sin(dphi_r, dphi_i, &_sin90r, &_sin90i);
    double _mul91r = 0, _mul91i = 0;
    c_mul(_c89r, _c89i, _sin90r, _sin90i, &_mul91r, &_mul91i);
    double _add92r = 0, _add92i = 0;
    _add92r = _cos88r + _mul91r; _add92i = _cos88i + _mul91i;
    double _mul93r = 0, _mul93i = 0;
    c_mul(_c87r, _c87i, _add92r, _add92i, &_mul93r, &_mul93i);
    { int _idx = 10; if (_idx >= 0 && _idx < 11) { cRe[_idx] = _mul93r; cIm[_idx] = _mul93i; } }
    for (int _i = 0; _i < 11; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g60_c(double x1r, double x1i, double x2r, double x2i,
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

static void g61_c(double x1r, double x1i, double x2r, double x2i,
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

/* g62: auto-stubbed (unhandled constructs in source) */
static void g62_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g63_c(double x1r, double x1i, double x2r, double x2i,
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

static void g64_c(double x1r, double x1i, double x2r, double x2i,
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

static void g65_c(double x1r, double x1i, double x2r, double x2i,
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

static void g66_c(double x1r, double x1i, double x2r, double x2i,
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

/* g67: auto-stubbed (unhandled constructs in source) */
static void g67_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g68_c(double x1r, double x1i, double x2r, double x2i,
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

static void g69_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double EPS = 0.0;
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double s_r = _add1r, s_i = _add1i;
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(s_r, s_i); _abs2i = 0;
    double s_abs_r = _abs2r, s_abs_i = _abs2i;
    double _c3r = 0, _c3i = 0;
    _c3r = 1.0; _c3i = 0;
    double _add4r = 0, _add4i = 0;
    _add4r = s_abs_r + _c3r; _add4i = s_abs_i + _c3i;
    double s_den_r = _add4r, s_den_i = _add4i;
    double _fin5r = 0, _fin5i = 0;
    _fin5r = (isfinite(s_den_r) && isfinite(s_den_i)) ? 1.0 : 0.0; _fin5i = 0;
    if (!((_fin5r != 0 || _fin5i != 0))) {
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        s_den_r = _c6r; s_den_i = _c6i;
    }
    double _attr7r = 0, _attr7i = 0;
    _attr7r = s_i; _attr7i = 0;
    double _attr8r = 0, _attr8i = 0;
    _attr8r = s_r; _attr8i = 0;
    double _at29r = 0, _at29i = 0;
    _at29r = atan2(_attr7r, _attr8r); _at29i = 0;
    double theta_r = _at29r, theta_i = _at29i;
    double _cos10r = 0, _cos10i = 0;
    c_cos(theta_r, theta_i, &_cos10r, &_cos10i);
    double _c11r = 0, _c11i = 0;
    _c11r = 0.0; _c11i = 1.0;
    double _sin12r = 0, _sin12i = 0;
    c_sin(theta_r, theta_i, &_sin12r, &_sin12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c11r, _c11i, _sin12r, _sin12i, &_mul13r, &_mul13i);
    double _add14r = 0, _add14i = 0;
    _add14r = _cos10r + _mul13r; _add14i = _cos10i + _mul13i;
    double phasor_r = _add14r, phasor_i = _add14i;
    for (int i = 0; i < 25; i++) {
        double _attr15r = 0, _attr15i = 0;
        _attr15r = x1r; _attr15i = 0;
        double rpart_r = _attr15r, rpart_i = _attr15i;
        double _attr16r = 0, _attr16i = 0;
        _attr16r = x2i; _attr16i = 0;
        double ipart_r = _attr16r, ipart_i = _attr16i;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = i + _c17r; _add18i = 0 + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powc(rpart_r, rpart_i, _add18r, _add18i, &_pow19r, &_pow19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 25.0; _c20i = 0;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _c20r - i; _sub21i = _c20i - 0;
        double _pow22r = 0, _pow22i = 0;
        c_powc(ipart_r, ipart_i, _sub21r, _sub21i, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _pow19r + _pow22r; _add23i = _pow19i + _pow22i;
        double num_r = _add23r, num_i = _add23i;
        double _div24r = 0, _div24i = 0;
        c_div(num_r, num_i, s_den_r, s_den_i, &_div24r, &_div24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_div24r, _div24i, phasor_r, phasor_i, &_mul25r, &_mul25i);
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    double _c26r = 0, _c26i = 0;
    _c26r = 3.0; _c26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _mul27r + x2r; _add28i = _mul27i + x2i;
    double _conj29r = 0, _conj29i = 0;
    _conj29r = _add28r; _conj29i = -(_add28i);
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c26r, _c26i, _conj29r, _conj29i, &_mul30r, &_mul30i);
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _c31r = 0, _c31i = 0;
    _c31r = 7.0; _c31i = 0;
    double _mul32r = 0, _mul32i = 0;
    c_mul(_c31r, _c31i, s_abs_r, s_abs_i, &_mul32r, &_mul32i);
    { int _idx = 6; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul32r; cIm[_idx] = _mul32i; } }
    double _c33r = 0, _c33i = 0;
    _c33r = 0.0; _c33i = 0;
    double _c34r = 0, _c34i = 0;
    _c34r = 0.0; _c34i = 0.0;
    double _add35r = 0, _add35i = 0;
    _add35r = _c33r + _c34r; _add35i = _c33i + _c34i;
    double term1_r = _add35r, term1_i = _add35i;
    double _c36r = 0, _c36i = 0;
    _c36r = 0.0; _c36i = 0;
    double _c37r = 0, _c37i = 0;
    _c37r = 0.0; _c37i = 0.0;
    double _add38r = 0, _add38i = 0;
    _add38r = _c36r + _c37r; _add38i = _c36i + _c37i;
    double term2_r = _add38r, term2_i = _add38i;
    double _abs39r = 0, _abs39i = 0;
    _abs39r = c_abs(x2r, x2i); _abs39i = 0;
    double a2_r = _abs39r, a2_i = _abs39i;
    double _abs40r = 0, _abs40i = 0;
    _abs40r = c_abs(x1r, x1i); _abs40i = 0;
    double a1_r = _abs40r, a1_i = _abs40i;
    double _fin41r = 0, _fin41i = 0;
    _fin41r = (isfinite(a2_r) && isfinite(a2_i)) ? 1.0 : 0.0; _fin41i = 0;
    if (((_fin41r != 0 || _fin41i != 0)) && (a2_r > EPS)) {
        double _div42r = 0, _div42i = 0;
        c_div(x1r, x1i, x2r, x2i, &_div42r, &_div42i);
        term1_r = _div42r; term1_i = _div42i;
    }
    double _fin43r = 0, _fin43i = 0;
    _fin43r = (isfinite(a1_r) && isfinite(a1_i)) ? 1.0 : 0.0; _fin43i = 0;
    if (((_fin43r != 0 || _fin43i != 0)) && (a1_r > EPS)) {
        double _div44r = 0, _div44i = 0;
        c_div(x2r, x2i, x1r, x1i, &_div44r, &_div44i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = _div44r; _conj45i = -(_div44i);
        term2_r = _conj45r; term2_i = _conj45i;
    }
    double _c46r = 0, _c46i = 0;
    _c46r = 11.0; _c46i = 0;
    double _add47r = 0, _add47i = 0;
    _add47r = term1_r + term2_r; _add47i = term1_i + term2_i;
    double _mul48r = 0, _mul48i = 0;
    c_mul(_c46r, _c46i, _add47r, _add47i, &_mul48r, &_mul48i);
    { int _idx = 10; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul48r; cIm[_idx] = _mul48i; } }
    double _mul49r = 0, _mul49i = 0;
    c_mul(s_abs_r, s_abs_i, s_abs_r, s_abs_i, &_mul49r, &_mul49i);
    double denom16_r = _mul49r, denom16_i = _mul49i;
    double _fin50r = 0, _fin50i = 0;
    _fin50r = (isfinite(denom16_r) && isfinite(denom16_i)) ? 1.0 : 0.0; _fin50i = 0;
    if (((_fin50r != 0 || _fin50i != 0)) && (denom16_r > EPS)) {
        double _c51r = 0, _c51i = 0;
        _c51r = 17.0; _c51i = 0;
        double _abs52r = 0, _abs52i = 0;
        _abs52r = c_abs(x1r, x1i); _abs52i = 0;
        double _abs53r = 0, _abs53i = 0;
        _abs53r = c_abs(x2r, x2i); _abs53i = 0;
        double _mul54r = 0, _mul54i = 0;
        c_mul(_abs52r, _abs52i, _abs53r, _abs53i, &_mul54r, &_mul54i);
        double _div55r = 0, _div55i = 0;
        c_div(_mul54r, _mul54i, denom16_r, denom16_i, &_div55r, &_div55i);
        double _mul56r = 0, _mul56i = 0;
        c_mul(_c51r, _c51i, _div55r, _div55i, &_mul56r, &_mul56i);
        { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    } else {
        double _c57r = 0, _c57i = 0;
        _c57r = 0.0; _c57i = 0;
        double _c58r = 0, _c58i = 0;
        _c58r = 0.0; _c58i = 0.0;
        double _add59r = 0, _add59i = 0;
        _add59r = _c57r + _c58r; _add59i = _c57i + _c58i;
        { int _idx = 16; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    }
    double _conj60r = 0, _conj60i = 0;
    _conj60r = x2r; _conj60i = -(x2i);
    double _mul61r = 0, _mul61i = 0;
    c_mul(x1r, x1i, _conj60r, _conj60i, &_mul61r, &_mul61i);
    double prod12_r = _mul61r, prod12_i = _mul61i;
    double _c62r = 0, _c62i = 0;
    _c62r = 1.0; _c62i = 0;
    double _abs63r = 0, _abs63i = 0;
    _abs63r = c_abs(prod12_r, prod12_i); _abs63i = 0;
    double _add64r = 0, _add64i = 0;
    _add64r = _c62r + _abs63r; _add64i = _c62i + _abs63i;
    double den22_r = _add64r, den22_i = _add64i;
    double _fin65r = 0, _fin65i = 0;
    _fin65r = (isfinite(den22_r) && isfinite(den22_i)) ? 1.0 : 0.0; _fin65i = 0;
    double _c66r = 0, _c66i = 0;
    _c66r = 0.0; _c66i = 0;
    if ((!((_fin65r != 0 || _fin65i != 0))) || (den22_r <= _c66r)) {
        double _c67r = 0, _c67i = 0;
        _c67r = 1.0; _c67i = 0;
        den22_r = _c67r; den22_i = _c67i;
    }
    double _c68r = 0, _c68i = 0;
    _c68r = 23.0; _c68i = 0;
    double _conj69r = 0, _conj69i = 0;
    _conj69r = x1r; _conj69i = -(x1i);
    double _add70r = 0, _add70i = 0;
    _add70r = _conj69r + x2r; _add70i = _conj69i + x2i;
    double _div71r = 0, _div71i = 0;
    c_div(_add70r, _add70i, den22_r, den22_i, &_div71r, &_div71i);
    double _mul72r = 0, _mul72i = 0;
    c_mul(_c68r, _c68i, _div71r, _div71i, &_mul72r, &_mul72i);
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul72r; cIm[_idx] = _mul72i; } }
    double _mul73r = 0, _mul73i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul73r, &_mul73i);
    double prod_r = _mul73r, prod_i = _mul73i;
    double _abs74r = 0, _abs74i = 0;
    _abs74r = c_abs(prod_r, prod_i); _abs74i = 0;
    double prod_abs_r = _abs74r, prod_abs_i = _abs74i;
    double _fin75r = 0, _fin75i = 0;
    _fin75r = (isfinite(prod_abs_r) && isfinite(prod_abs_i)) ? 1.0 : 0.0; _fin75i = 0;
    if (((_fin75r != 0 || _fin75i != 0)) && (prod_abs_r > EPS)) {
        double _c76r = 0, _c76i = 0;
        _c76r = 25.0; _c76i = 0;
        double _conj77r = 0, _conj77i = 0;
        _conj77r = x1r; _conj77i = -(x1i);
        double _conj78r = 0, _conj78i = 0;
        _conj78r = x2r; _conj78i = -(x2i);
        double _add79r = 0, _add79i = 0;
        _add79r = _conj77r + _conj78r; _add79i = _conj77i + _conj78i;
        double _div80r = 0, _div80i = 0;
        c_div(_add79r, _add79i, prod_abs_r, prod_abs_i, &_div80r, &_div80i);
        double _mul81r = 0, _mul81i = 0;
        c_mul(_c76r, _c76i, _div80r, _div80i, &_mul81r, &_mul81i);
        { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul81r; cIm[_idx] = _mul81i; } }
    } else {
        double _c82r = 0, _c82i = 0;
        _c82r = 0.0; _c82i = 0;
        double _c83r = 0, _c83i = 0;
        _c83r = 0.0; _c83i = 0.0;
        double _add84r = 0, _add84i = 0;
        _add84r = _c82r + _c83r; _add84i = _c82i + _c83i;
        { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add84r; cIm[_idx] = _add84i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void g70_c(double x1r, double x1i, double x2r, double x2i,
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

/* g71: too complex for auto-transpile, stubbed */
static void g71_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g72_c(double x1r, double x1i, double x2r, double x2i,
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

static void g73_c(double x1r, double x1i, double x2r, double x2i,
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

static void g74_c(double x1r, double x1i, double x2r, double x2i,
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

static void g75_c(double x1r, double x1i, double x2r, double x2i,
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

static void g76_c(double x1r, double x1i, double x2r, double x2i,
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

static void g77_c(double x1r, double x1i, double x2r, double x2i,
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

static void g78_c(double x1r, double x1i, double x2r, double x2i,
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

static void g79_c(double x1r, double x1i, double x2r, double x2i,
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

static void g80_c(double x1r, double x1i, double x2r, double x2i,
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

/* g81: auto-stubbed (unhandled constructs in source) */
static void g81_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g82: too complex for auto-transpile, stubbed */
static void g82_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g83: auto-stubbed (unhandled constructs in source) */
static void g83_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g84: too complex for auto-transpile, stubbed */
static void g84_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g85_c(double x1r, double x1i, double x2r, double x2i,
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

static void g86_c(double x1r, double x1i, double x2r, double x2i,
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

static void g87_c(double x1r, double x1i, double x2r, double x2i,
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

/* g88: auto-stubbed (unhandled constructs in source) */
static void g88_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g89_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* t1, t2 = z[0].real, z[0].imag — scalar-from-first-input */
    double t1 = x1r;
    double t2 = x1i;
    for (int i = 0; i < 51; i++) {
        double _mul1r = 0, _mul1i = 0;
        c_mul(i, 0, t1, 0, &_mul1r, &_mul1i);
        double _cos2r = 0, _cos2i = 0;
        c_cos(_mul1r, _mul1i, &_cos2r, &_cos2i);
        double _mul3r = 0, _mul3i = 0;
        c_mul(i, 0, t2, 0, &_mul3r, &_mul3i);
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
    c_powr(t1, 0, 50.0, &_pow8r, &_pow8i);
    double _mul9r = 0, _mul9i = 0;
    c_mul(_cf6r, _cf6i, _pow8r, _pow8i, &_mul9r, &_mul9i);
    { int _idx = 0; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul9r; cIm[_idx] = _mul9i; } }
    double _cf10r = 0, _cf10i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 51) { _cf10r = cRe[_idx]; _cf10i = cIm[_idx]; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 49.0; _c11i = 0;
    double _pow12r = 0, _pow12i = 0;
    c_powr(t2, 0, 49.0, &_pow12r, &_pow12i);
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
        c_powc(t1, 0, _sub17r, _sub17i, &_pow18r, &_pow18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_cf14r, _cf14i, _pow18r, _pow18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 2.0; _c20i = 0;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = i - _c20r; _sub21i = 0 - _c20i;
        double _pow22r = 0, _pow22i = 0;
        c_powc(t2, 0, _sub21r, _sub21i, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mul19r, _mul19i, _pow22r, _pow22i, &_mul23r, &_mul23i);
        { int _idx = i; if (_idx >= 0 && _idx < 51) { cRe[_idx] = _mul23r; cIm[_idx] = _mul23i; } }
    }
    for (int _i = 0; _i < 51; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* g90: too complex for auto-transpile, stubbed */
static void g90_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g91: auto-stubbed (unhandled constructs in source) */
static void g91_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g92: too complex for auto-transpile, stubbed */
static void g92_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g93_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* t1, t2 = z[0].real, z[0].imag — scalar-from-first-input */
    double t1 = x1r;
    double t2 = x1i;
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
        c_mul(_arr4r, _arr4i, t1, 0, &_mul5r, &_mul5i);
        double _cos6r = 0, _cos6i = 0;
        c_cos(_mul5r, _mul5i, &_cos6r, &_cos6i);
        double _arr7r = 0, _arr7i = 0;
        { int _idx = i; _arr7r = (_idx >= 0 && _idx < 35) ? angles[_idx] : 0.0; _arr7i = 0; }
        double _mul8r = 0, _mul8i = 0;
        c_mul(_arr7r, _arr7i, t2, 0, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(t1, 0); _abs10i = 0;
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
        c_mul(t1, 0, _c13r, _c13i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 71.0; _c15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _c15r - _c16r; _sub17i = _c15i - _c16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(t2, 0, _sub17r, _sub17i, &_mul18r, &_mul18i);
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
        c_mul(t1, 0, _c22r, _c22i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_c24r, _c24i, t2, 0, &_mul25r, &_mul25i);
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

static void g94_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* t1, t2 = z[0].real, z[0].imag — scalar-from-first-input */
    double t1 = x1r;
    double t2 = x1i;
    double _attr1r = 0, _attr1i = 0;
    _attr1r = t1; _attr1i = 0;
    double _attr2r = 0, _attr2i = 0;
    _attr2r = 0; _attr2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_attr1r, _attr1i, _attr2r, _attr2i, &_mul3r, &_mul3i);
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    for (int k = 1; k < 71; k++) {
        double _cf4r = 0, _cf4i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 71) { _cf4r = cRe[_idx]; _cf4i = cIm[_idx]; } }
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, _cf4r, _cf4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, t1, 0, &_mul7r, &_mul7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_mul7r, _mul7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _sin6r + _cos8r; _add9i = _sin6i + _cos8i;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
        double _cf10r = 0, _cf10i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { _cf10r = cRe[_idx]; _cf10i = cIm[_idx]; } }
        double _cf11r = 0, _cf11i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 71) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_cf11r, _cf11i); _abs12i = 0;
        double _div13r = 0, _div13i = 0;
        c_div(_cf10r, _cf10i, _abs12r, _abs12i, &_div13r, &_div13i);
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div13r; cIm[_idx] = _div13i; } }
    }
    double _cf14r = 0, _cf14i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 71) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
    double _abs15r = 0, _abs15i = 0;
    _abs15r = c_abs(_cf14r, _cf14i); _abs15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 2.0; _c16i = 0;
    double _pow17r = 0, _pow17i = 0;
    c_mul(_abs15r, _abs15i, _abs15r, _abs15i, &_pow17r, &_pow17i);
    double _ang18r = 0, _ang18i = 0;
    _ang18r = c_arg(t2, 0); _ang18i = 0;
    double _c19r = 0, _c19i = 0;
    _c19r = 2.0; _c19i = 0;
    double _pow20r = 0, _pow20i = 0;
    c_mul(_ang18r, _ang18i, _ang18r, _ang18i, &_pow20r, &_pow20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _pow17r + _pow20r; _add21i = _pow17i + _pow20i;
    { int _idx = 30; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    double _cf22r = 0, _cf22i = 0;
    { int _idx = 20; if (_idx >= 0 && _idx < 71) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
    double _abs23r = 0, _abs23i = 0;
    _abs23r = c_abs(t1, 0); _abs23i = 0;
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(t2, 0); _abs24i = 0;
    double _c25r = 0, _c25i = 0;
    _c25r = 1.0; _c25i = 0;
    double _add26r = 0, _add26i = 0;
    _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
    double _log27r = 0, _log27i = 0;
    c_log(_add26r, _add26i, &_log27r, &_log27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _abs23r + _log27r; _add28i = _abs23i + _log27i;
    double _mul29r = 0, _mul29i = 0;
    c_mul(_cf22r, _cf22i, _add28r, _add28i, &_mul29r, &_mul29i);
    { int _idx = 40; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    double _cf30r = 0, _cf30i = 0;
    { int _idx = 30; if (_idx >= 0 && _idx < 71) { _cf30r = cRe[_idx]; _cf30i = cIm[_idx]; } }
    double _mul31r = 0, _mul31i = 0;
    c_mul(t1, 0, t2, 0, &_mul31r, &_mul31i);
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(_mul31r, _mul31i); _abs32i = 0;
    double _c33r = 0, _c33i = 0;
    _c33r = 1.0; _c33i = 0;
    double _add34r = 0, _add34i = 0;
    _add34r = _abs32r + _c33r; _add34i = _abs32i + _c33i;
    double _log35r = 0, _log35i = 0;
    c_log(_add34r, _add34i, &_log35r, &_log35i);
    double _add36r = 0, _add36i = 0;
    _add36r = _cf30r + _log35r; _add36i = _cf30i + _log35i;
    { int _idx = 50; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    double _add37r = 0, _add37i = 0;
    _add37r = t1 + t2; _add37i = 0 + 0;
    double _abs38r = 0, _abs38i = 0;
    _abs38r = c_abs(_add37r, _add37i); _abs38i = 0;
    double _cf39r = 0, _cf39i = 0;
    { int _idx = 30; if (_idx >= 0 && _idx < 71) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
    double _ang40r = 0, _ang40i = 0;
    _ang40r = c_arg(_cf39r, _cf39i); _ang40i = 0;
    double _mul41r = 0, _mul41i = 0;
    c_mul(_abs38r, _abs38i, _ang40r, _ang40i, &_mul41r, &_mul41i);
    { int _idx = 60; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    double _cf42r = 0, _cf42i = 0;
    { int _idx = 34; if (_idx >= 0 && _idx < 71) { _cf42r = cRe[_idx]; _cf42i = cIm[_idx]; } }
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(_cf42r, _cf42i); _abs43i = 0;
    double _c44r = 0, _c44i = 0;
    _c44r = 0.0; _c44i = 1.0;
    double _mul45r = 0, _mul45i = 0;
    c_mul(_c44r, _c44i, t2, 0, &_mul45r, &_mul45i);
    double _add46r = 0, _add46i = 0;
    _add46r = t1 + _mul45r; _add46i = 0 + _mul45i;
    double _div47r = 0, _div47i = 0;
    c_div(_abs43r, _abs43i, _add46r, _add46i, &_div47r, &_div47i);
    { int _idx = 70; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _div47r; cIm[_idx] = _div47i; } }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* g95: too complex for auto-transpile, stubbed */
static void g95_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g96_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* t1, t2 = z[0].real, z[0].imag — scalar-from-first-input */
    double t1 = x1r;
    double t2 = x1i;
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
        c_mul(t1, 0, t1, 0, &_pow10r, &_pow10i);
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
        c_mul(t2, 0, t2, 0, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_add22r, _add22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul11r + _mul25r; _add26i = _mul11i + _mul25i;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(t1, 0, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul28r + t2; _add29i = _mul28i + 0;
        double _sin30r = 0, _sin30i = 0;
        c_sin(_add29r, _add29i, &_sin30r, &_sin30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add26r + _sin30r; _add31i = _add26i + _sin30i;
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(t1, 0, _c32r, _c32i, &_mul33r, &_mul33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul33r - t2; _sub34i = _mul33i - 0;
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
            c_mul(_c46r, _c46i, t2, 0, &_mul47r, &_mul47i);
            double _add48r = 0, _add48i = 0;
            _add48r = t1 + _mul47r; _add48i = 0 + _mul47i;
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
                c_mul(_c58r, _c58i, t2, 0, &_mul59r, &_mul59i);
                double _add60r = 0, _add60i = 0;
                _add60r = t1 + _mul59r; _add60i = 0 + _mul59i;
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
                c_mul(_c66r, _c66i, t2, 0, &_mul67r, &_mul67i);
                double _add68r = 0, _add68i = 0;
                _add68r = t1 + _mul67r; _add68i = 0 + _mul67i;
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

/* g97: too complex for auto-transpile, stubbed */
static void g97_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g98: too complex for auto-transpile, stubbed */
static void g98_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g99_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* t1, t2 = z[0].real, z[0].imag — scalar-from-first-input */
    double t1 = x1r;
    double t2 = x1i;
    double _add1r = 0, _add1i = 0;
    _add1r = t1 + t2; _add1i = 0 + 0;
    double _sin2r = 0, _sin2i = 0;
    c_sin(_add1r, _add1i, &_sin2r, &_sin2i);
    double _sub3r = 0, _sub3i = 0;
    _sub3r = t1 - t2; _sub3i = 0 - 0;
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
            _abs15r = c_abs(t1, 0); _abs15i = 0;
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
            _attr28r = 0; _attr28i = 0;
            double _add29r = 0, _add29i = 0;
            _add29r = _add27r + _attr28r; _add29i = _add27i + _attr28i;
            double _attr30r = 0, _attr30i = 0;
            _attr30r = t1; _attr30i = 0;
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

/* gp7f: too complex for auto-transpile, stubbed */
static void gp7f_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g221_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
        double mag_r = _add20r, mag_i = _add20i;
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
        double angle_r = _add36r, angle_i = _add36i;
        double _cos37r = 0, _cos37i = 0;
        c_cos(angle_r, angle_i, &_cos37r, &_cos37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(angle_r, angle_i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c38r, _c38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _cos37r + _mul40r; _add41i = _cos37i + _mul40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(mag_r, mag_i, _add41r, _add41i, &_mul42r, &_mul42i);
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

/* g224: too complex for auto-transpile, stubbed */
static void g224_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    for (int _i = 0; _i < 51; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g227: too complex for auto-transpile, stubbed */
static void g227_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void g230_c(double x1r, double x1i, double x2r, double x2i,
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

/* g232: too complex for auto-transpile, stubbed */
static void g232_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g2863: too complex for auto-transpile, stubbed */
static void g2863_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g2864: too complex for auto-transpile, stubbed */
static void g2864_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* g2864a: too complex for auto-transpile, stubbed */
static void g2864a_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* gpt1: too complex for auto-transpile, stubbed */
static void gpt1_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 121;
    for (int _i = 0; _i < 121; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

